#include <stdio.h>

#include "snes9x.h"

#include <glib.h>
#include <libosso.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

/* Zeemote driver contributed by Till Harbaum */

extern "C" {
#include <zeemote.h>
#include <zeemote-conf.h>
}

#include "platform.h"
#include "osso.h"
#include "../gui/gconf.h"
#include "zeemote.h"

/* Zeemote analog stick sensivity limit */
#define ZEEMOTE_LIMIT  8000

static const unsigned short zeemote_mask[] = 
{
	SNES_A_MASK, SNES_B_MASK, SNES_X_MASK, SNES_Y_MASK 
};

typedef struct zeemote_player {
	/** Pointer to zeemote_t struct. If NULL, zeemote was not enabled for this player. */
	zeemote_t *zeemote;
	uint32 prev_buttons;
} zeemote_player_t;

static zeemote_player_t players[2] = { {0, 0}, {0, 0} };

void ZeeInit()
{
	/* Check if Osso initialization was succesful. */
	if (!OssoOk()) return;

	/* Since Zeemote means GConf, we can read our own configuration from it */
	GConfClient *gcc = gconf_client_get_default();

	/* Check which players (if any) enabled Zeemote */
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey;
	bool enabled[2];

	/* Player 1? */
	relKey = key + sprintf(key, kGConfPlayerPath, 1);
	strcpy(relKey, kGConfPlayerZeemoteEnable);
	enabled[0] = gconf_client_get_bool(gcc, key, NULL);

	/* Player 2? */
	relKey = key + sprintf(key, kGConfPlayerPath, 2);
	strcpy(relKey, kGConfPlayerZeemoteEnable);
	enabled[1] = gconf_client_get_bool(gcc, key, NULL);

	if (!enabled[0] && !enabled[1]) {
		/* No player wanted a zeemote! */
		return;
	}

	/* Get list of configured zeemotes from the conf tool. One could */
	/* alternally call zeemote_scan() which would return the same type */
	/* of list, but instead from the devices currently visible. Also */
	/* zeemote_scan() blocks for about 10 seconds */
	zeemote_scan_result_t *scan_result = 
		zeemote_get_scan_results_from_gconf();

	if (!scan_result) {
		fprintf(stderr, "Zeemote: No scan results\n");
	}

	/* If we found a zeemote, assign it to the first player. If we found
	 * two zeemotes, assign the first found one to the first player, etc. */
	/* Since the order comes from gconf, the user could potentially change
	   it */
	if (enabled[0] && scan_result->number_of_devices > 0) {
		players[0].zeemote =
			zeemote_connect(&scan_result->device[0].bdaddr);
		if (players[0].zeemote) {
			Config.joypad1Enabled = true;
		} else {
			fprintf(stderr, "Zeemote: Failed to connect for player %d", 1);
		}
	}
	if (enabled[1] && scan_result->number_of_devices > 1) {
		players[1].zeemote =
			zeemote_connect(&scan_result->device[1].bdaddr);
		if (players[1].zeemote) {
			Config.joypad2Enabled = true;
		} else {
			fprintf(stderr, "Zeemote: Failed to connect for player %d", 2);
		}
	}

	if (players[0].zeemote && players[1].zeemote) {
		printf("Zeemote: enabled for two players\n");
	} else if (players[0].zeemote) {
		printf("Zeemote: enabled for player %d\n", 1);
	} else if (players[1].zeemote) {
		printf("Zeemote: enabled for player %d\n", 2);
	} else {
		printf("Zeemote: disabled because of error\n");
	}
}

static void read_for_player(zeemote_player_t *player, uint32 *joypad)
{
	if (!player->zeemote) return;

	zeemote_state_t *state = zeemote_get_state(player->zeemote);
	if (!state) return; // Some error
	 // Zeemote was disconnected
	if (state->state != ZEEMOTE_STATE_CONNECTED) return;

	uint32 buttons = 0;
	int i;

	/* check zeemote buttons A-D */
	for (i = 0; i < 4; i++) {
		if (state->buttons & (1<<i))
			buttons |= zeemote_mask[i];
	}

	/* handle direction */
	if (state->axis[0] < -ZEEMOTE_LIMIT) buttons |= SNES_LEFT_MASK;
	if (state->axis[0] >  ZEEMOTE_LIMIT) buttons |= SNES_RIGHT_MASK;
	if (state->axis[1] < -ZEEMOTE_LIMIT) buttons |= SNES_UP_MASK;
	if (state->axis[1] >  ZEEMOTE_LIMIT) buttons |= SNES_DOWN_MASK;

	/* check which actual buttons were pressed or released */
	uint32 buttons_changed = buttons ^ player->prev_buttons;
	uint32 buttons_pressed = buttons_changed & buttons;
	uint32 buttons_released = buttons_changed & player->prev_buttons;

	/* prevent device screensaver when zeemote state changes */
	if (buttons_changed)
	{
		osso_display_blanking_pause(ossoContext);
		player->prev_buttons = buttons;
	}

	*joypad = (*joypad & ~buttons_released) | buttons_pressed;
}

void ZeeRead(uint32* joypads)
{
	read_for_player(&players[0], &joypads[0]);
	read_for_player(&players[1], &joypads[1]);
}

void ZeeQuit()
{
	if (players[0].zeemote) {
		zeemote_disconnect(players[0].zeemote);
		players[0].zeemote = 0;
	}
	if (players[1].zeemote) {
		zeemote_disconnect(players[1].zeemote);
		players[1].zeemote = 0;
	}
}

