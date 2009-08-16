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
	HgwStartCommand cmd = hgw_context_get_start_command(hgw);
	// TODO Handle cmd in some way other than assuming HGW_COMM_RESTART
	cmd;
	
	
	
}

void HgwDeinit()
{
	if (!hgwLaunched) return;
	
	hgw_context_destroy(hgw, HGW_BYE_PAUSED);  // TODO
	hgw = 0;
}

void HgwConfig()
{
	if (!hgwLaunched) return;
	
	Config.fullscreen = true;
	
	char romFile[PATH_MAX];
	if (hgw_conf_request_string(hgw, kGConfRomFile, romFile) == HGW_ERR_NONE) {
		S9xSetRomFile(romFile);
	} else {
		DIE("No Rom in Gconf!");
	}
}

void HgwPollEvents()
{
	if (!hgwLaunched) return;
	
	HgwMessage *msg = 0;
	HgwMessageFlags flags = HGW_MSG_FLAG_NONE;
	
	if ( hgw_msg_check_incoming(hgw, msg, flags) == HGW_ERR_COMMUNICATION ) {
		// Message Incoming, process msg
		
		switch (msg->type) {
			case HGW_MSG_TYPE_CBREQ:
				switch (msg->e_val) {
					case HGW_CB_QUIT:
					case HGW_CB_EXIT:
						Config.quitting = true;
						break;
				}
				break;
		}
		
		hgw_msg_free_data(msg);
	}
}


