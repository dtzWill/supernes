#include <stdio.h>
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
	if (hgw_conf_request_string(hgw, kGConfRomFile, romFile) == HGW_ERR_NONE) {
		S9xSetRomFile(romFile);
	} else {
		hgw_context_destroy(hgw, HGW_BYE_INACTIVE);
		DIE("No Rom in Gconf!");
	}

	char no_audio = FALSE;
	if (hgw_conf_request_bool(hgw, kGConfDisableAudio, &no_audio) == HGW_ERR_NONE) {
		Config.enableAudio = no_audio ? true : false;
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

	int speedhacks = 0;
	if (hgw_conf_request_int(hgw, kGConfFrameskip, &speedhacks) == HGW_ERR_NONE) {
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


