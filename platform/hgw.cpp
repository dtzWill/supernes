#include <stdio.h>
#include <libgen.h>
#include <hgw/hgw.h>

#include "platform.h"
#include "hgw.h"
#include "snes9x.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);


bool hgwLaunched;
static HgwContext *hgw;

static void createActionMappingsOnly();
static void parseGConfKeyMappings();

void HgwInit()
{
	// hildon-games-wrapper sets this env variable for itself.
	char* service = getenv("HGW_EXEC_SERVICE");

	if (!service) {
		// Not launched from hildon-games-wrapper
		hgwLaunched = false;
		return;
	}

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
	if (hgw_conf_request_bool(hgw, kGConfScaler, scaler) == HGW_ERR_NONE
		&& strlen(scaler) > 0) {
		free(Config.scaler);
		Config.scaler = strdup(scaler);
	}

	char displayFramerate = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfDisplayFramerate, &displayFramerate) == HGW_ERR_NONE) {
		Settings.DisplayFrameRate = displayFramerate ? TRUE : FALSE;
	}

	char displayControls = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfDisplayControls, &displayControls) == HGW_ERR_NONE) {
		Config.touchscreenShow = displayControls ? true : false;
	}

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
		// romFile[] is garbled from now on.
	}

	int mappings = 0;
	if (hgw_conf_request_int(hgw, kGConfMapping, &mappings) == HGW_ERR_NONE) {
		switch (mappings) {
			case 0:
				// Do nothing, leave mappings as is.
				break;
			case 1: // Keys
				parseGConfKeyMappings();
				break;
			case 2: // Touchscreen
				Config.touchscreenInput = true;
				createActionMappingsOnly();
				break;
			case 3: // Touchscreen + keys
				Config.touchscreenInput = true;
				parseGConfKeyMappings();
				break;
			case 4: // Mouse
				Settings.Mouse = TRUE;
				Settings.ControllerOption = SNES_MOUSE_SWAPPED;
				createActionMappingsOnly();
				break;
			case 5: // Mouse + keys
				Settings.Mouse = TRUE;
				Settings.ControllerOption = SNES_MOUSE;
				parseGConfKeyMappings();
				break;
		}
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
	char * gconf_key;
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
	BUTTON_INITIALIZER(A, "a"),
	BUTTON_INITIALIZER(B, "b"),
	BUTTON_INITIALIZER(X, "x"),
	BUTTON_INITIALIZER(Y, "y"),
	BUTTON_INITIALIZER(TL, "l"),
	BUTTON_INITIALIZER(TR, "r"),
	BUTTON_INITIALIZER(START, "start"),
	BUTTON_INITIALIZER(SELECT, "select"),
	BUTTON_INITIALIZER(UP, "up"),
	BUTTON_INITIALIZER(DOWN, "down"),
	BUTTON_INITIALIZER(LEFT, "left"),
	BUTTON_INITIALIZER(RIGHT, "right"),
	ACTION_INITIALIZER(Quit, "quit"),
	ACTION_INITIALIZER(ToggleFullscreen, "fullscreen"),
	ACTION_INITIALIZER(QuickLoad1, "quickload1"),
	ACTION_INITIALIZER(QuickSave1, "quicksave1"),
	ACTION_INITIALIZER(QuickLoad2, "quickload2"),
	ACTION_INITIALIZER(QuickSave2, "quicksave2"),
	BUTTON_LAST
};

static void createActionMappingsOnly()
{
	// Discard any other mapping
	ZeroMemory(Config.joypad1Mapping, sizeof(Config.joypad1Mapping));
	ZeroMemory(Config.action, sizeof(Config.action));
	
	// Map quit to fullscreen, escape and task switch.
	Config.action[72] = kActionQuit;
	Config.action[9] = kActionQuit;
	Config.action[71] = kActionQuit;
}

static void parseGConfKeyMappings()
{
	// Discard any other mapping
	ZeroMemory(Config.joypad1Mapping, sizeof(Config.joypad1Mapping));
	ZeroMemory(Config.action, sizeof(Config.action));

	// If the user does not map fullscreen or quit
	bool quit_mapped = false;

	printf("Hgw: Using gconf key mappings\n");

	int i, scancode;
	for (i = 0; buttons[i].gconf_key; i++) {
		if (hgw_conf_request_int(hgw, buttons[i].gconf_key, &scancode) == HGW_ERR_NONE) {
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
	}

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
}

