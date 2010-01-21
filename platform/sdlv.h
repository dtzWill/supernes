#ifndef _PLATFORM_SDLV_H_
#define _PLATFORM_SDLV_H_

#include <SDL.h>

class Scaler
{
public:
	Scaler() { };
	virtual ~Scaler() { };

	virtual const char * getName() const = 0;

	virtual uint8* getDrawBuffer() const = 0;
	virtual unsigned int getDrawBufferPitch() const = 0;
	virtual void getRenderedGUIArea(unsigned short& x, unsigned short& y,
									unsigned short& w, unsigned short& h)
									const = 0;
	virtual void getRatio(float& x, float& y) const = 0;

	virtual void prepare() = 0;
	virtual void finish() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual bool filter(const SDL_Event& event) = 0;
};

class ScalerFactory
{
public:
	ScalerFactory() { };
	virtual ~ScalerFactory() { };
	virtual const char * getName() const = 0;
	virtual bool canEnable(int bpp, int w, int h) const = 0;
	virtual Scaler* instantiate(SDL_Surface* screen, int w, int h) const = 0;
};

const ScalerFactory* searchForScaler(int bpp, int w, int h);

/** SDL screen */
extern SDL_Surface* screen;
/** The current scaler object */
extern Scaler* scaler;

void processVideoEvent(const SDL_Event& event);

#if CONF_EXIT_BUTTON
void ExitBtnReset();
bool ExitBtnRequiresDraw();
void ExitBtnDraw(SDL_Surface* where);
#endif

#endif
