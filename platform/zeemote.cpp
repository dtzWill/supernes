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

static zeemote_t *zeemote;
static uint32 prev_buttons;
static int player;

void ZeeInit()
{
	/* Check if Osso initialization was succesful. */
	if (!OssoOk()) return;

	/* Since Zeemote means GConf, we can read our own configuration from it */
	GConfClient *gcc = gconf_client_get_default();

	/* Check which player (if any) enabled Zeemote */
	player = 0;
	/* Player 1? */
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key, kGConfPlayerPath, 1);
	strcpy(relKey, kGConfPlayerZeemoteEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		player = 1;
	} else {
		/* Player 2? */
		relKey = key + sprintf(key, kGConfPlayerPath, 2);
		strcpy(relKey, kGConfPlayerZeemoteEnable);
		if (gconf_client_get_bool(gcc, key, NULL)) {
			player = 2;
		}
	}

	if (player) {
		printf("Zeemote: Enabled for player %d\n", player);
	} else {
		/* No player wanted a zeemote! */
		return;
	}

	/* Get list of configured zeemotes from the conf tool. One could */
	/* alternally call zeemote_scan() which would return the same type */
	/* of list, but instead from the devices currently visible. Also */
	/* zeemote_scan() blocks for about 10 seconds */
	zeemote_scan_result_t *scan_result = 
		zeemote_get_scan_results_from_gconf();

	/* if devices are configured use the first one in the list as this */
	/* is supposed to be a single player game */
	if(scan_result && scan_result->number_of_devices > 0) {
		zeemote = zeemote_connect(&scan_result->device[0].bdaddr);
	}
}

void ZeeRead(uint32* joypads)
{
	if (!zeemote) return;

	zeemote_state_t *state = zeemote_get_state(zeemote);
	if (!state) return; // Some error
	if (state->state != ZEEMOTE_STATE_CONNECTED) return; // Not connected

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
 
	/* prevent device screensaver when zeemote state changes */
	if (buttons != prev_buttons) 
	{
		osso_display_blanking_pause(ossoContext);
		prev_buttons = buttons;
	}

	joypads[player-1] |= buttons;
}

void ZeeQuit()
{
	if (zeemote) {
		zeemote_disconnect(zeemote);
		zeemote = 0;
	}
}

