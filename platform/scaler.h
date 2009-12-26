#ifndef _PLATFORM_SCALER_H_
#define _PLATFORM_SCALER_H_

class Scaler
{
public:
	Scaler() { };
	virtual ~Scaler() { };

	virtual const char * getName() const = 0;

	virtual uint8* getDrawBuffer() const = 0;
	virtual unsigned int getDrawBufferPitch() const = 0;
	virtual void getRenderedGUIArea(unsigned short & x, unsigned short & y,
									unsigned short & w, unsigned short & h)
									const = 0;
	virtual int getRatio() const = 0;
	virtual void prepare() = 0;
	virtual void finish() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
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

#endif
