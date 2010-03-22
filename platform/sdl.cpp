#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <SDL.h>

#include "platform.h"
#include "snes9x.h"
#include "cpuexec.h"
#include "gfx.h"
#include "ppu.h"
#include "display.h"
#include "memmap.h"
#include "soundux.h"
#include "hacks.h"
#include "snapshot.h"

#define kPollEveryNFrames		5		//Poll input only every this many frames

#if CONF_GUI
#include "osso.h"
#define kPollOssoEveryNFrames	10		//Poll dbus only every this many frames
#endif

#define TRACE printf("trace: %s:%s\n", __FILE__, __func__);
#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

void S9xMessage(int type, int number, const char * message)
{
	printf("%s\n", message);
}

void S9xAutoSaveSRAM()
{
	Memory.SaveSRAM(S9xGetFilename(FILE_SRAM));
}

static void S9xInit() 
{
	if (!Memory.Init () || !S9xInitAPU())
         DIE("Memory or APU failed");

	if (!S9xInitSound ())
		DIE("Sound failed");
	S9xSetSoundMute (TRUE);
	
	// TODO: PAL/NTSC something better than this
	Settings.PAL = Settings.ForcePAL;
	
	Settings.FrameTime = Settings.PAL?Settings.FrameTimePAL:Settings.FrameTimeNTSC;
	Memory.ROMFramesPerSecond = Settings.PAL?50:60;
	
	IPPU.RenderThisFrame = TRUE;
}

static void loadRom()
{
	const char * file = S9xGetFilename(FILE_ROM);

	printf("ROM: %s\n", file);

	if (!Memory.LoadROM(file))
		DIE("Loading ROM failed");
	
	file = S9xGetFilename(FILE_SRAM);
	printf("SRAM: %s\n", file);
	Memory.LoadSRAM(file); 
}

static void resumeGame()
{
	if (!Config.snapshotLoad) return;

	const char * file = S9xGetFilename(FILE_FREEZE);
	int result = S9xUnfreezeGame(file);

	printf("Unfreeze: %s", file);

	if (!result) {
		printf(" failed");
		FILE* fp = fopen(file, "rb");
		if (fp) {
			if (Config.snapshotSave) {
				puts(", but the file exists, so I'm not going to overwrite it");
				Config.snapshotSave = false;
			} else {
				puts(" (bad file?)");
			}
			fclose(fp);
		} else {
			puts(" (file does not exist)");
		}
	} else {
		puts(" ok");
	}
}

static void pauseGame()
{
	if (!Config.snapshotSave) return;

	const char * file = S9xGetFilename(FILE_FREEZE);
	int result = S9xFreezeGame(file);

	printf("Freeze: %s", file);

	if (!result) {
		Config.snapshotSave = false; // Serves as a flag to Hgw
		puts(" failed");
	} else {
		puts(" ok");
	}
}

/* This comes nearly straight from snes9x */
/** Calculates framerate, enables frame skip if to low, sleeps if too high, etc. */
static void frameSync() {
	Uint32 now = SDL_GetTicks();

	if (Settings.TurboMode)
	{
		// In Turbo mode, just skip as many frames as desired, but don't sleep.
		if(Settings.SkipFrames == AUTO_FRAMERATE || 
			++IPPU.FrameSkip >= Settings.SkipFrames)
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			++IPPU.SkippedFrames;
			IPPU.RenderThisFrame = FALSE;
		}

		// Take care of framerate display
		if (Settings.DisplayFrameRate) {
			static Uint32 last = 0;
			// Update framecounter every second
			if (now > last && (now - last > 1000)) {
				IPPU.DisplayedRenderedFrameCount =
					IPPU.RenderedFramesCount;
				IPPU.RenderedFramesCount = 0;
				last = now;
			}
		}
	} else {
		static Uint32 next1 = 0;

		// If there is no known "next" frame, initialize it now
		if (next1 == 0) {
			next1 = now + 1;
		}

		// If we're on AUTO_FRAMERATE, we'll display frames always
		// only if there's excess time.
		// Otherwise we'll display around 1 frame every 10.
		unsigned limit = Settings.SkipFrames == AUTO_FRAMERATE
					? (next1 < now ? 10 : 1)
					: Settings.SkipFrames;

		IPPU.RenderThisFrame = ++IPPU.SkippedFrames >= limit;
		if (IPPU.RenderThisFrame) {
			IPPU.SkippedFrames = 0;
		} else {
			// If we were behind the schedule, check how much it is
			if (next1 < now)
			{
		        unsigned long lag = now - next1;
		        if (lag >= 500)
				{
					// More than a half-second behind means probably
					// pause. The next line prevents the magic
					// fast-forward effect.
					next1 = now;
				}
			}
		}

		// If we're now ahead of time, sleep a while
		if (next1 > now)
		{
			SDL_Delay(next1 - now);
			// SDL will take care if a signal arrives, restarting sleep.
		}

		// Calculate the timestamp of the next frame.
		next1 += Settings.FrameTime;

		// Take care of framerate display
		if (Settings.DisplayFrameRate) {
			// Update every theoretical 60 frames
			if (IPPU.FrameCount % Memory.ROMFramesPerSecond == 0) {
				IPPU.DisplayedRenderedFrameCount =
					IPPU.RenderedFramesCount;
				IPPU.RenderedFramesCount = 0;
			}
		}
	}
}

/** Wraps s9xProcessEvents, taking care of kPollEveryNFrames */
static inline void pollEvents() {
	static int frames = 0;
	
	if (++frames > kPollEveryNFrames) {
		S9xProcessEvents(false);
		frames = 0;
	}
}

#if CONF_GUI
/** Wraps OssoPollEvents, taking care of kPollOssoEveryNFrames */
static inline void pollOssoEvents() {
	static int frames = 0;
	
	if (!OssoOk()) return;
	
	if (++frames > kPollOssoEveryNFrames) {
		OssoPollEvents();
		frames = 0;
	}
}
#endif

int main(int argc, char ** argv) {
	// Initialise SDL
	if (SDL_Init(0) < 0) 
		DIE("SDL_Init: %s", SDL_GetError());

	// Configure snes9x
#if CONF_GUI
	OssoInit();						// Hildon-games-wrapper initialization.
#endif
	S9xLoadConfig(argc, argv);		// Load config files and parse cmd line.
#if CONF_GUI
	OssoConfig();					// Apply specific hildon-games config.
#endif

	// S9x initialization
	S9xInitDisplay(argc, argv);
	S9xInitAudioOutput();
	S9xInitInputDevices();
	S9xInit();
	S9xReset();

	// Load rom and related files: state, unfreeze if needed
	loadRom();
	resumeGame();

	// Late initialization
	sprintf(String, "DrNokSnes - %s", Memory.ROMName);
	S9xSetTitle(String);
	S9xHacksLoadFile(Config.hacksFile);
	if (!S9xGraphicsInit())
         DIE("S9xGraphicsInit failed");
	S9xAudioOutputEnable(true);

	do {
		frameSync();			// May block, or set frameskip to true.
		S9xMainLoop();			// Does CPU things, renders if needed.
		pollEvents();
#if CONF_GUI
		pollOssoEvents();
#endif
	} while (!Config.quitting);
	
	// Deinitialization
	S9xAudioOutputEnable(false);
	S9xDeinitInputDevices();
	S9xDeinitAudioOutput();
	S9xDeinitDisplay();

	// Save state
	Memory.SaveSRAM(S9xGetFilename(FILE_SRAM));
	pauseGame();

	// Late deinitialization
	S9xGraphicsDeinit();
	Memory.Deinit();
	S9xUnloadConfig();
#if CONF_GUI
	OssoDeinit();
#endif

	SDL_Quit();

	return 0;
}

void S9xDoAction(unsigned char action)
{
	if (action & kActionQuit) 
		Config.quitting = true;

	if (action & kActionToggleFullscreen) {
		S9xVideoToggleFullscreen();
	}

	if (action & kActionQuickLoad1) {
		const char * file = S9xGetQuickSaveFilename(1);
		int result = S9xUnfreezeGame(file);
		S9xSetInfoString("Load slot %u: %s", 1,
			(result ? "done" : "failed"));
	}

	if (action & kActionQuickSave1) {
		const char * file = S9xGetQuickSaveFilename(1);
		int result = S9xFreezeGame(file);
		S9xSetInfoString("Save slot %u: %s", 1,
			(result ? "done" : "failed"));
	}

	if (action & kActionQuickLoad2) {
		const char * file = S9xGetQuickSaveFilename(2);
		int result = S9xUnfreezeGame(file);
		S9xSetInfoString("Load slot %u: %s", 2,
			(result ? "done" : "failed"));
	}

	if (action & kActionQuickSave2) {
		const char * file = S9xGetQuickSaveFilename(2);
		int result = S9xFreezeGame(file);
		S9xSetInfoString("Save slot %u: %s", 2,
			(result ? "done" : "failed"));
	}
}

