#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "snes9x.h"
#include "platform.h"
#include "display.h"
#include "gfx.h"
#include "ppu.h"
#include "sdlv.h"
#include "scaler.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

struct gui GUI;

SDL_Surface* screen;

static SDL_Rect windowSize, screenSize;
static bool gotWindowSize, gotScreenSize;

/** The current scaler object */
static Scaler* scaler;

static void calculateScreenSize()
{
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);

	if ( SDL_GetWMInfo(&wminfo) ) {
		Display *dpy = wminfo.info.x11.display;
		Window w;
		SDL_Rect* size;
		XWindowAttributes xwa;

		if (Config.fullscreen) {
			w =  wminfo.info.x11.fswindow;
			size = &screenSize;
			gotScreenSize = true;
		} else {
			w =  wminfo.info.x11.wmwindow;
			size = &windowSize;
			gotWindowSize = true;
		}

		XGetWindowAttributes(dpy, w, &xwa);
		size->x = xwa.x;
		size->y = xwa.y;
		size->w = xwa.width;
		size->h = xwa.height;
	}
}

void S9xSetTitle(const char *title)
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if ( SDL_GetWMInfo(&info) ) {
		Display *dpy = info.info.x11.display;
		Window win;
		if (dpy) {
			win = info.info.x11.fswindow;
			if (win) XStoreName(dpy, win, title);
			win = info.info.x11.wmwindow;
			if (win) XStoreName(dpy, win, title);
		}
	}
}

static void freeVideoSurface()
{
	screen = 0; // There's no need to free the screen surface.
	GFX.Screen = 0;

	free(GFX.SubScreen); GFX.SubScreen = 0;
	free(GFX.ZBuffer); GFX.ZBuffer = 0;
	free(GFX.SubZBuffer); GFX.SubZBuffer = 0;

	delete scaler; scaler = 0;
}

static void setupVideoSurface()
{
	// Real surface area.
	const unsigned gameWidth = IMAGE_WIDTH;
	const unsigned gameHeight = IMAGE_HEIGHT;

#ifdef MAEMO
	if ((Config.fullscreen && !gotScreenSize) ||
		(!Config.fullscreen && !gotWindowSize)) {
		// Do a first try, in order to get window/screen size
		screen = SDL_SetVideoMode(gameWidth, gameHeight, 16,
			SDL_SWSURFACE | SDL_RESIZABLE |
			(Config.fullscreen ? SDL_FULLSCREEN : 0));
		if (!screen) DIE("SDL_SetVideoMode: %s", SDL_GetError());
		calculateScreenSize();
	}
	if (Config.fullscreen) {
		GUI.Width = screenSize.w;
		GUI.Height = screenSize.h;
	} else {
		GUI.Width = windowSize.w;
		GUI.Height = windowSize.h;
	}
#else
	GUI.Width = gameWidth;
	GUI.Height = gameHeight;
#endif
#if CONF_EXIT_BUTTON
	exitReset();
#endif

	// Safeguard
	if (gameHeight > GUI.Height || gameWidth > GUI.Width)
		DIE("Video is larger than window size!");

	const ScalerFactory* sFactory =
		searchForScaler(Settings.SixteenBit ? 16 : 8, gameWidth, gameHeight);

	screen = SDL_SetVideoMode(GUI.Width, GUI.Height,
								Settings.SixteenBit ? 16 : 8,
								SDL_SWSURFACE |
								(Config.fullscreen ? SDL_FULLSCREEN : 0));
	if (!screen)
		DIE("SDL_SetVideoMode: %s", SDL_GetError());
	
	SDL_ShowCursor(SDL_DISABLE);

	scaler = sFactory->instantiate(screen, gameWidth, gameHeight);

	// We get pitch surface values from SDL
	GFX.RealPitch = GFX.Pitch = scaler->getDrawBufferPitch();
	GFX.ZPitch = GFX.Pitch / 2; // gfx & tile.cpp depend on this, unfortunately.
	GFX.PixSize = screen->format->BitsPerPixel / 8;
	
	GFX.Screen = scaler->getDrawBuffer();
	GFX.SubScreen = (uint8 *) malloc(GFX.Pitch * IMAGE_HEIGHT);
	GFX.ZBuffer =  (uint8 *) malloc(GFX.ZPitch * IMAGE_HEIGHT);
	GFX.SubZBuffer = (uint8 *) malloc(GFX.ZPitch * IMAGE_HEIGHT);

	GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
	GFX.PPL = GFX.Pitch >> 1;
	GFX.PPLx2 = GFX.Pitch;

	scaler->getRenderedGUIArea(GUI.RenderX, GUI.RenderY, GUI.RenderW, GUI.RenderH);
	scaler->getRatio(GUI.ScaleX, GUI.ScaleY);

	printf("Video: %dx%d (%dx%d output), %hu bits per pixel, %s, %s\n",
		gameWidth, gameHeight,
		screen->w, screen->h, screen->format->BitsPerPixel,
		Config.fullscreen ? "fullscreen" : "windowed",
		scaler->getName());
}

static void drawOnscreenControls()
{
	if (Config.touchscreenInput) {
		S9xInputScreenChanged();
	}

	if (Config.touchscreenShow) {
		scaler->pause();
		SDL_FillRect(screen, NULL, 0);
		S9xInputScreenDraw(Settings.SixteenBit ? 2 : 1,
							screen->pixels, screen->pitch);
		SDL_Flip(screen);
		scaler->resume();
	}
}

void S9xInitDisplay(int argc, const char ** argv)
{	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
		DIE("SDL_InitSubSystem(VIDEO): %s", SDL_GetError());

#if CONF_HD
	hd_setup();
#endif

	setupVideoSurface();
	drawOnscreenControls();
}

void S9xDeinitDisplay()
{
	freeVideoSurface();	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void S9xVideoToggleFullscreen()
{
	freeVideoSurface();
	Config.fullscreen = !Config.fullscreen;
	setupVideoSurface();
	drawOnscreenControls();
}

void S9xVideoOutputFocus(bool hasFocus)
{
#if MAEMO
	if (scaler) {
		if (hasFocus) {
			scaler->resume();
		} else {
			scaler->pause();
		}
	}
#endif
}

// This is here for completeness, but palette mode is useless on N8x0
void S9xSetPalette ()
{
	if (Settings.SixteenBit) return;
	
	SDL_Color colors[256];
	int brightness = IPPU.MaxBrightness *138;
	for (int i = 0; i < 256; i++)
	{
		colors[i].r = ((PPU.CGDATA[i] >> 0) & 0x1F) * brightness;
		colors[i].g = ((PPU.CGDATA[i] >> 5) & 0x1F) * brightness;
		colors[i].b = ((PPU.CGDATA[i] >> 10) & 0x1F) * brightness;
	}
	
	SDL_SetColors(screen, colors, 0, 256);
}

bool8_32 S9xInitUpdate ()
{
	scaler->prepare();

	return TRUE;
}

bool8_32 S9xDeinitUpdate (int width, int height, bool8_32 sixteenBit)
{
	scaler->finish();

#if CONF_EXIT_BUTTON
	if (exitRequiresDraw()) {
		//scaler->pause();
		exitDraw(screen);
		SDL_Flip(screen);
		//scaler->resume();
	}
#endif

	return TRUE;
}

