#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "port.h"

// Configuration and command line parsing
void S9xLoadConfig(int argc, const char ** argv);
void S9xUnloadConfig();
void S9xSetRomFile(const char * file);
extern struct config {
	/** Unfreeze from .frz.gz snapshot on start */
	bool snapshotLoad;
	/** Freeze to .frz.gz on exit */
	bool snapshotSave;
	/** Create fullscreen surface */
	bool fullscreen;
	/** Using xsp (thus take care of doubling coordinates where appropiate) */
	bool xsp;
	/** Audio output enabled */
	bool enableAudio;
	/** Speedhacks file to use */
	char * hacksFile;
	/** Enable touchscreen controls */
	bool touchscreenInput;
	/** Current scancode->joypad mapping */
	unsigned short joypad1Mapping[256];
	unsigned char action[256];
	/** If true, next time the main loop is entered application will close */
	bool quitting;
} Config;

// Video
void S9xVideoToggleFullscreen();
void S9xVideoOutputFocus(bool hasFocus);

// Audio output
void S9xInitAudioOutput();
void S9xDeinitAudioOutput();
void S9xAudioOutputEnable(bool enable);

// Input devices
EXTERN_C void S9xInitInputDevices();
void S9xDeinitInputDevices();
void S9xInputFullscreenChanged();

// Input actions
#define kActionNone						0
#define kActionQuit					(1U << 0)
#define	kActionToggleFullscreen		(1U << 1)

void S9xDoAction(unsigned char action);

#endif
