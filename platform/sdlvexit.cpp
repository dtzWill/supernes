#include <SDL.h>
#include <SDL_image.h>

#include "platform.h"
#include "sdlv.h"

#define BACK_BUTTON_IMG_PATH \
	"/usr/share/icons/hicolor/scalable/hildon/general_overlay_back.png"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

static SDL_Surface* buttonSrf = 0;
static SDL_Rect buttonRect;

static const unsigned long totalAnimLen = 1;

static unsigned long frameCounter = 0;

void ExitBtnReset()
{
	frameCounter = 0;
	if (!buttonSrf) {
		buttonSrf = IMG_Load(BACK_BUTTON_IMG_PATH);
	}

	buttonRect.x = GUI.Width - buttonSrf->w;
	buttonRect.y = 0;
	buttonRect.w = buttonSrf->w;
	buttonRect.h = buttonSrf->h;
}

bool ExitBtnRequiresDraw()
{
	if (!Config.fullscreen) return false;
	if (frameCounter > totalAnimLen) {
		return false;
	} else {
		frameCounter++;
		return true;
	}
	
};

void ExitBtnDraw(SDL_Surface* where)
{
	SDL_BlitSurface(buttonSrf, 0, where, &buttonRect);
	SDL_UpdateRects(where, 1, &buttonRect);
};

