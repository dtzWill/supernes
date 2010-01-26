#include <stdio.h>
#include <libgen.h>
#include <hgw/hgw.h>

#include "snes9x.h"

#include <glib.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "platform.h"
#include "hgw.h"
#include "../gui/gconf.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

bool hgwLaunched;
static HgwContext *hgw;

static void createActionMappingsOnly();
static void parseGConfKeyMappings(GConfClient* gcc);

void HgwInit()
{
	// hildon-games-wrapper sets this env variable for itself.
	char* service = getenv("HGW_EXEC_SERVICE");

	if (!service) {
		// Not launched from hildon-games-wrapper
		hgwLaunched = false;
		return;
	}

	g_type_init();
	hgw = hgw_context_init();

	if (!hgw) {
		fprintf(stderr, "Error opening hgw context\n");
		hgwLaunched = false;
	}

	hgwLaunched = true;
	printf("Loading in HGW mode\n");
}

void HgwDeinit()
{
	if (!hgwLaunched) return;

	hgw_context_destroy(hgw,
		(Config.snapshotSave ? HGW_BYE_PAUSED : HGW_BYE_INACTIVE));

	hgw = 0;
}

void HgwConfig()
{
	if (!hgwLaunched) return;

	GConfClient *gcc = gconf_client_get_default();

	Config.fullscreen = true;

	char romFile[PATH_MAX + 1];
	if (hgw_conf_request_string(hgw, kGConfRomFile, romFile) == HGW_ERR_NONE
		&& strlen(romFile) > 0) {
		S9xSetRomFile(romFile);
	} else {
		printf("Exiting gracefully because there's no ROM in Gconf\n");
		HgwDeinit();
		exit(0);
	}

	char sound = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfSound, &sound) == HGW_ERR_NONE) {
		Config.enableAudio = sound ? true : false;
	}

	char turbo = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfTurboMode, &turbo) == HGW_ERR_NONE) {
		Settings.TurboMode = turbo ? TRUE : FALSE;
	}

	int frameskip = 0;
	if (hgw_conf_request_int(hgw, kGConfFrameskip, &frameskip) == HGW_ERR_NONE) {
		Settings.SkipFrames = (frameskip > 0 ? frameskip : AUTO_FRAMERATE);
	}

	char transparency = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfTransparency, &transparency) == HGW_ERR_NONE) {
		Settings.Transparency = transparency ? TRUE : FALSE;
	}

	char scaler[NAME_MAX];
	if (hgw_conf_request_string(hgw, kGConfScaler, scaler) == HGW_ERR_NONE
		&& strlen(scaler) > 0) {
		free(Config.scaler);
		Config.scaler = strdup(scaler);
	}

	char displayFramerate = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfDisplayFramerate, &displayFramerate) == HGW_ERR_NONE) {
		Settings.DisplayFrameRate = displayFramerate ? TRUE : FALSE;
	}

#if TODO
	char displayControls = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfDisplayControls, &displayControls) == HGW_ERR_NONE) {
		Config.touchscreenShow = displayControls ? true : false;
	}
#endif

	int speedhacks = 0;
	if (hgw_conf_request_int(hgw, kGConfSpeedhacks, &speedhacks) == HGW_ERR_NONE) {
		if (speedhacks <= 0) {
			Settings.HacksEnabled = FALSE;
			Settings.HacksFilter = FALSE;
		} else if (speedhacks == 1) {
			Settings.HacksEnabled = TRUE;
			Settings.HacksFilter = TRUE;
		} else {
			Settings.HacksEnabled = TRUE;
			Settings.HacksFilter = FALSE;
		}
	}
	if (Settings.HacksEnabled && !Config.hacksFile) {
		// Provide a default speedhacks file
		if (asprintf(&Config.hacksFile, "%s/snesadvance.dat", dirname(romFile))
				< 0) {
			Config.hacksFile = 0; // malloc error.
		}
		// remember that dirname garbles romFile.
	}

	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key, kGConfPlayerPath, 1);

	strcpy(relKey, kGConfPlayerKeyboardEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		parseGConfKeyMappings(gcc);
	} else {
		createActionMappingsOnly();
	}

	HgwStartCommand cmd = hgw_context_get_start_command(hgw);
	switch (cmd) {
		default:
		case HGW_COMM_NONE:	// called from libosso
		case HGW_COMM_CONT:
			Config.snapshotLoad = true;
			Config.snapshotSave = true;
			break;
		case HGW_COMM_RESTART:
			Config.snapshotLoad = false;
			Config.snapshotSave = true;
			break;
		case HGW_COMM_QUIT:
			// hum, what?
			Config.snapshotLoad = false;
			Config.snapshotSave = false;
			Config.quitting = true;
			break;
	}

	g_object_unref(G_OBJECT(gcc));
}

void HgwPollEvents()
{
	if (!hgwLaunched) return;
	
	HgwMessage msg;
	HgwMessageFlags flags = HGW_MSG_FLAG_NONE;
	
	if ( hgw_msg_check_incoming(hgw, &msg, flags) == HGW_ERR_COMMUNICATION ) {
		// Message Incoming, process msg
		
		switch (msg.type) {
			case HGW_MSG_TYPE_CBREQ:
				switch (msg.e_val) {
					case HGW_CB_QUIT:
					case HGW_CB_EXIT:
						Config.quitting = true;
						break;
				}
				break;
			case HGW_MSG_TYPE_DEVSTATE:
				switch (msg.e_val) {
					case HGW_DEVICE_STATE_SHUTDOWN:
						Config.quitting = true;	// try to quit gracefully
						break;
				}
				break;
			default:
				// do nothing
				break;
		}
		
		hgw_msg_free_data(&msg);
	}
}

// For now, please keep this in sync with ../gui/controls.c
typedef struct ButtonEntry {
	const char * gconf_key;
	unsigned long mask;
	bool is_action;
} ButtonEntry;
#define BUTTON_INITIALIZER(button, name) \
	{ kGConfKeysPath "/" name, SNES_##button##_MASK, false }
#define ACTION_INITIALIZER(action, name) \
	{ kGConfKeysPath "/" name, kAction##action, true }
#define BUTTON_LAST	\
	{ 0 }

static const ButtonEntry buttons[] = {
#define HELP(...)
#define P(x) SNES_##x##_MASK
#define A(x) kAction##x
#define BUTTON(description, slug, actions, d, f) \
	{ G_STRINGIFY(slug), actions, false },
#define ACTION(description, slug, actions, d, f) \
	{ G_STRINGIFY(slug), actions, true },
#define LAST \
	{ 0 }
#include "../gui/buttons.inc"
#undef HELP
#undef P
#undef A
#undef BUTTON
#undef ACTION
#undef LAST
};

static void createActionMappingsOnly()
{
	// Map quit to fullscreen, escape and task switch.
	Config.action[72] = kActionQuit;
	Config.action[9] = kActionQuit;
	Config.action[71] = kActionQuit;
}

static void parseGConfKeyMappings(GConfClient* gcc)
{
	// Build player 1 keyboard gconf key relative path
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", 1);

	// If the user does not map fullscreen or quit
	bool quit_mapped = false;

	printf("Hgw: Using gconf key mappings\n");
	// Thus ignoring config file key mappings
	ZeroMemory(Config.joypad1Mapping, sizeof(Config.joypad1Mapping));
	ZeroMemory(Config.action, sizeof(Config.action));

	int i, scancode;
	for (i = 0; buttons[i].gconf_key; i++) {
		strcpy(relKey, buttons[i].gconf_key);
		scancode = gconf_client_get_int(gcc, key, NULL);

		if (scancode <= 0 || scancode > 255) continue;

		if (buttons[i].is_action) {
			Config.action[scancode] |= buttons[i].mask;
			if (buttons[i].mask & (kActionQuit | kActionToggleFullscreen)) {
				quit_mapped = true;
			}
		} else {
			Config.joypad1Mapping[scancode] |= buttons[i].mask;
		}
	}

#if MAEMO && !CONF_EXIT_BUTTON
	// Safeguards
	if (!quit_mapped) {
		// Newbie user won't know how to quit game.
		if (!Config.joypad1Mapping[72] && !Config.action[72]) {
			// Fullscreen key is not mapped, map
			Config.action[72] = kActionQuit;
			quit_mapped = true;
		}
		if (!quit_mapped && !Config.joypad1Mapping[9] && !Config.action[9]) {
			// Escape key is not mapped, map
			// But only if we couldn't map quit to fullscreen. Some people
			// actually want Quit not to be mapped.
			Config.action[9] = kActionQuit;
			quit_mapped = true;
		}
		if (!quit_mapped) {
			// Force mapping of fullscreen to Quit if can't map anywhere else.
			Config.joypad1Mapping[72] = 0;
			Config.action[72] = kActionQuit;
		}
	}

	// If task switch key is not mapped, map it to Quit by default.
	if (!Config.action[71] && !Config.joypad1Mapping[71]) {
		Config.action[71] = kActionQuit;
	}
#endif
}

