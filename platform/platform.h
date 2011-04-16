#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "port.h"

#define MAX_KEYS 20

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
	/** If false, next time the main loop is entered application will close */
	bool running;
	/** Current joypad->scancode mapping */
	int joypad1Mapping[MAX_KEYS];
	int joypad2Mapping[MAX_KEYS];
  /** Joypad->scancode mapping */
	int action[1024];
} Config;

typedef enum {
  SNES_KEY_UP,
  SNES_KEY_DOWN,
  SNES_KEY_LEFT,
  SNES_KEY_RIGHT,
  SNES_KEY_START,
  SNES_KEY_SELECT,
  SNES_KEY_L,
  SNES_KEY_R,
  SNES_KEY_Y,
  SNES_KEY_X,
  SNES_KEY_B,
  SNES_KEY_A,
  SNES_KEY_TURBO
} SNES_KEY;

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
void S9xVideoReset();
void S9xSetTitle (const char *title);
void S9xVideoTakeScreenshot(void);

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
#define kActionMenu           (1U << 0)
#define kActionQuickLoad1			(1U << 1)
#define kActionQuickSave1			(1U << 2)
#define kActionQuickLoad2			(1U << 3)
#define kActionQuickSave2			(1U << 4)
#define kActionQuickLoad3			(1U << 5)
#define kActionQuickSave3			(1U << 6)

void S9xDoAction(unsigned char action);

void S9xSaveState(int state_num);
void S9xLoadState(int state_num);

#endif
