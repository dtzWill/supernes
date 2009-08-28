#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xsp.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "snes9x.h"
#include "platform.h"
#include "display.h"
#include "gfx.h"
#include "ppu.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

static SDL_Surface *screen;

static SDL_Rect windowSize, screenSize;
static bool gotWindowSize, gotScreenSize;

/** Inside the surface, where are we drawing */
static SDL_Rect renderArea;

#ifdef MAEMO
static void setDoubling(bool enable)
{
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	if ( SDL_GetWMInfo(&wminfo) ) {
		Display *dpy = wminfo.info.x11.display;
		XSPSetPixelDoubling(dpy, 0, enable ? 1 : 0);
		XFlush(dpy);
	}
}
#endif

static void centerRectangle(SDL_Rect& result, int areaW, int areaH, int w, int h)
{
	result.x = areaW / 2 - w / 2;
	result.w = w;
	result.y = areaH / 2 - h / 2;
	result.h = h;
}

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
}

static void setupVideoSurface()
{
	// Real surface area.
	unsigned realWidth = IMAGE_WIDTH;
	unsigned realHeight = IMAGE_HEIGHT;
	// SDL Window/Surface size (bigger, so we can get mouse events there).
	unsigned srfWidth, srfHeight;

#ifdef MAEMO
	if ((Config.fullscreen && !gotScreenSize) ||
		(!Config.fullscreen && !gotWindowSize)) {
		// Do a first try, in order to get window/screen size
		screen = SDL_SetVideoMode(realWidth, realHeight, 16,
			SDL_SWSURFACE | SDL_RESIZABLE |
			(Config.fullscreen ? SDL_FULLSCREEN : 0));
		if (!screen) DIE("SDL_SetVideoMode: %s", SDL_GetError());
		calculateScreenSize();
	}
	if (Config.fullscreen) {
		srfWidth = screenSize.w;
		srfHeight = screenSize.h;
	} else {
		srfWidth = windowSize.w;
		srfHeight = windowSize.h;
	}
	
	// By now, just assume xsp == fullscreen. This has to change.
	Config.xsp = Config.fullscreen;
	if (!Config.xsp) {
		setDoubling(false); // Before switching video modes; avoids flicker.
	}
#else
	srfWidth = realWidth;
	srfHeight = realHeight;
#endif

	screen = SDL_SetVideoMode(srfWidth, srfHeight,
								Settings.SixteenBit ? 16 : 8,
								SDL_SWSURFACE |
								(Config.fullscreen ? SDL_FULLSCREEN : 0));
	if (!screen)
		DIE("SDL_SetVideoMode: %s", SDL_GetError());
	
	SDL_ShowCursor(SDL_DISABLE);

	// We get pitch surface values from SDL
	GFX.RealPitch = GFX.Pitch = screen->pitch;
	GFX.ZPitch = realWidth; // The ZBuffer is independent of SDL surface size.
	GFX.PixSize = screen->format->BitsPerPixel / 8;

	// Ok, calculate renderArea
#ifdef MAEMO
	if (Config.xsp) {
		setDoubling(true);
		centerRectangle(renderArea, srfWidth, srfHeight,
			realWidth * 2, realHeight * 2);
		renderArea.w /= 2;
		renderArea.h /= 2;
	} else {
		centerRectangle(renderArea, srfWidth, srfHeight, realWidth, realHeight);
	}
#else
	centerRectangle(renderArea, srfWidth, srfHeight, realWidth, realHeight);
#endif
	
	GFX.Screen = ((uint8*) screen->pixels)
		+ (renderArea.x * GFX.PixSize)
		+ (renderArea.y * GFX.Pitch);
	GFX.SubScreen = (uint8 *) malloc(GFX.Pitch * IMAGE_HEIGHT);
	GFX.ZBuffer =  (uint8 *) malloc(GFX.Pitch * IMAGE_HEIGHT);
	GFX.SubZBuffer = (uint8 *) malloc(GFX.Pitch * IMAGE_HEIGHT);
	
	GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
	GFX.PPL = GFX.Pitch >> 1;
	GFX.PPLx2 = GFX.Pitch;
	GFX.ZPitch = GFX.Pitch >> 1; // TODO

	printf("Video: %dx%d (%dx%d output), %hu bits per pixel, %s %s\n",
		realWidth, realHeight,
		screen->w, screen->h, screen->format->BitsPerPixel,
		Config.fullscreen ? "fullscreen" : "windowed",
		Config.xsp ? "with pixel doubling" : "");
}

void S9xInitDisplay(int argc, const char ** argv)
{	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
		DIE("SDL_InitSubSystem(VIDEO): %s", SDL_GetError());

	setupVideoSurface();
}

void S9xDeinitDisplay()
{
	freeVideoSurface();	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void S9xVideoToggleFullscreen()
{
	Config.fullscreen = !Config.fullscreen;
	freeVideoSurface();
	setupVideoSurface();
}

void S9xVideoOutputFocus(bool hasFocus)
{
	if (Config.xsp) {
		setDoubling(hasFocus);
	} 
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
	if(SDL_MUSTLOCK(screen)) 
	{
		if(SDL_LockSurface(screen) < 0) {
			DIE("Failed to lock SDL surface: %s", SDL_GetError());
		}
	}

	return TRUE;
}

bool8_32 S9xDeinitUpdate (int width, int height, bool8_32 sixteenBit)
{
	if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

	if (Config.xsp) {	
		width *= 2;
		height *= 2;
	}

	SDL_UpdateRects(screen, 1, &renderArea);
	
	return TRUE;
}

