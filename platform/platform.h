#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "port.h"

// Configuration and command line parsing
void S9xLoadConfig(int argc, char ** argv);
void S9xUnloadConfig();
void S9xSetRomFile(const char * file);
extern struct config {
	/** Unfreeze from .frz.gz snapshot on start */
	bool snapshotLoad;
	/** Freeze to .frz.gz on exit */
	bool snapshotSave;
	/** Create fullscreen surface */
	bool fullscreen;
	/** Name of the scaler to use or NULL for default */
	char * scaler;
	/** Audio output enabled */
	bool enableAudio;
	/** Quit when the emulator window is deactivated */
	bool saver;
	/** Speedhacks file to use */
	char * hacksFile;
	/** Enable player 1 joypad */
	bool joypad1Enabled;
	/** Enable player 2 joypad */
	bool joypad2Enabled;
	/** Enable touchscreen controls (0 = no, 1 = for player 1, 2 = for player 2) */
	char touchscreenInput;
	/** Display touchscreen controls grid */
	bool touchscreenShow;
	/** If true, next time the main loop is entered application will close */
	bool quitting;
	/** Current scancode->joypad mapping */
	unsigned short joypad1Mapping[256];
	unsigned short joypad2Mapping[256];
	unsigned char action[256];
} Config;

// Video
extern struct gui {
	/** Size of the GUI Window */
	unsigned short Width, Height;
	/** Size of the (scaled) rendering area, relative to window. */
	unsigned short RenderX, RenderY, RenderW, RenderH;
	/** Scaling ratio */
	float ScaleX, ScaleY;
} GUI;

void S9xInitDisplay(int argc, char **argv);
void S9xDeinitDisplay();
void S9xVideoToggleFullscreen();
void S9xSetTitle (const char *title);

// Audio output
void S9xInitAudioOutput();
void S9xDeinitAudioOutput();
void S9xAudioOutputEnable(bool enable);

// Input devices
void S9xInitInputDevices();
void S9xDeinitInputDevices();
void S9xInputScreenChanged();
void S9xInputScreenDraw(int pixelSize, void * buffer, int pitch);
void S9xProcessEvents(bool block);

// Input actions
#define kActionNone						0
#define kActionQuit					(1U << 0)
#define	kActionToggleFullscreen		(1U << 1)
#define kActionQuickLoad1			(1U << 2)
#define kActionQuickSave1			(1U << 3)
#define kActionQuickLoad2			(1U << 4)
#define kActionQuickSave2			(1U << 5)

void S9xDoAction(unsigned char action);

#endif
