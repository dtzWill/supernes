#ifndef _PLATFORM_SDL_H_
#define _PLATFORM_SDL_H_

#include "port.h"

// Configuration and command line parsing
void S9xLoadConfig(int argc, const char ** argv);
void S9xSetRomFile(const char * file);
extern struct config {
	char romFile[PATH_MAX];
	char hacksFile[PATH_MAX];
	bool fullscreen;
	bool xsp;
	bool enableAudio;
	bool enableMouse;
	bool enableSuperScope;
	unsigned short joypad1Mapping[256];
	unsigned char action[256];
	bool quitting;
} Config;

// Video
void S9xVideoToggleFullscreen();
void S9xVideoOutputFocus(bool hasFocus);

// Audio output
void S9xInitAudioOutput();
void S9xDeinitAudioOutput();
void S9xAudioOutputEnable(bool enable);

// Input actions
#define kActionNone						0
#define kActionQuit					(1U << 0)
#define	kActionToggleFullscreen		(1U << 1)

void S9xDoAction(unsigned char action);

// Path things
const char * S9xFiletitle (const char * f);

#endif
