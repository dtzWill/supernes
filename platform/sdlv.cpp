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

static void setDoubling(bool enable)
{
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	if ( SDL_GetWMInfo(&wminfo) ) {
		XSPSetPixelDoubling(wminfo.info.x11.display, 0, enable ? 1 : 0);
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
	int w = IMAGE_WIDTH;
	int h = IMAGE_HEIGHT;
	
	// By now, just assume xsp == fullscreen. This has to change.
	Config.xsp = Config.fullscreen;
	if (Config.xsp) {
		w *= 2;
		h *= 2;
	} else {
		setDoubling(false); // Before switching video modes
	}

	screen = SDL_SetVideoMode(w, h,
								Settings.SixteenBit ? 16 : 8,
								SDL_SWSURFACE |
								(Config.fullscreen ? SDL_FULLSCREEN : 0));

	if (!screen)
		DIE("SDL_SetVideoMode: %s", SDL_GetError());
	
	SDL_ShowCursor(SDL_DISABLE);

	if (Config.xsp) setDoubling(true);
	
	GFX.RealPitch = GFX.Pitch = screen->pitch;
	
	GFX.Screen = (uint8*) screen->pixels;
	GFX.SubScreen = (uint8 *) malloc(GFX.RealPitch * IMAGE_HEIGHT);
	GFX.ZBuffer =  (uint8 *) malloc(GFX.RealPitch * IMAGE_HEIGHT);
	GFX.SubZBuffer = (uint8 *) malloc(GFX.RealPitch * IMAGE_HEIGHT);
	
	GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
	GFX.PPL = GFX.Pitch >> 1;
	GFX.PPLx2 = GFX.Pitch;
	GFX.ZPitch = GFX.Pitch >> 1;
	
	printf("Video: %dx%d, %hu bits per pixel, %s %s\n", screen->w, screen->h,
		screen->format->BitsPerPixel,
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

	SDL_UpdateRect(screen, 0, 0, width, height);
	
	return TRUE;
}

