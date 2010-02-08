#include <stdio.h> 
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL.h>

#if CONF_XSP
#	include <SDL_syswm.h>
#	include <X11/extensions/Xsp.h>
#endif
#if CONF_HD
#	include <SDL_haa.h>
#endif

#include "snes9x.h"
#include "display.h"
#include "platform.h"
#include "sdlv.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

/* Helper functions */

static void centerRectangle(SDL_Rect& result, int areaW, int areaH, int w, int h)
{
	result.x = areaW / 2 - w / 2;
	result.w = w;
	result.y = areaH / 2 - h / 2;
	result.h = h;
}

/* Base scaler for stupid scalers */
/** Does nothing but center the image */
class DummyScaler : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;

protected:
	DummyScaler(SDL_Surface* screen, int w, int h)
	: m_screen(screen)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height, w, h);
	}

public:

	~DummyScaler()
	{
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "none";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new DummyScaler(screen, w, h);
		}
	};

	static const Factory factory;

	virtual const char * getName() const
	{
		return "no scaling";
	}

	virtual uint8* getDrawBuffer() const
	{
		const int Bpp = screen->format->BitsPerPixel / 8;
		const int pitch = screen->pitch;
		return ((uint8*) screen->pixels)
			+ (m_area.x * Bpp)
			+ (m_area.y * pitch);
	};

	virtual unsigned int getDrawBufferPitch() const
	{
		return screen->pitch;
	};

	virtual void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	virtual void getRatio(float & x, float & y) const
	{
		x = 1.0f; y = 1.0f;
	};

	virtual void prepare() { };

	virtual void finish()
	{
		SDL_UpdateRects(m_screen, 1, &m_area);
	};

	virtual void pause() { };
	virtual void resume() { };

	virtual bool filter(const SDL_Event& event) { return false; };
};
const DummyScaler::Factory DummyScaler::factory;

/* Basic and slow software scaler */

class SWScaler : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;
	uint8 * m_surface;
	const int m_w, m_h, m_Bpp;

protected:
	SWScaler(SDL_Surface* screen, int w, int h)
	: m_screen(screen), m_w(w), m_h(h),
	 m_Bpp(m_screen->format->BitsPerPixel / 8)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height, w * 2, h * 2);
		m_surface = reinterpret_cast<uint8*>(malloc(w * h * m_Bpp));
	}
public:
	virtual ~SWScaler()
	{
		free(m_surface);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "soft2x";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return w * 2 < GUI.Width && h * 2 < GUI.Height;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new SWScaler(screen, w, h);
		}
	};

	static const Factory factory;

	virtual const char * getName() const
	{
		return "software 2x scaling";
	}

	uint8* getDrawBuffer() const
	{
		return m_surface;
	};

	unsigned int getDrawBufferPitch() const
	{
		return m_w * m_Bpp;
	};

	void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	void getRatio(float & x, float & y) const
	{
		x = 2.0f; y = 2.0f;
	};

	void prepare() { };

	void finish()
	{
		uint16 * src = reinterpret_cast<uint16*>(m_surface);
		uint16 * dst = reinterpret_cast<uint16*>(
			((uint8*) m_screen->pixels)
			+ (m_area.x * m_Bpp)
			+ (m_area.y * m_screen->pitch));
		const int src_pitch = m_w;
		const int dst_pitch = m_screen->pitch / m_Bpp;
		int x, y;

		for (y = 0; y < m_h*2; y++) {
			for (x = 0; x < m_w*2; x+=2) {
				dst[x] = src[x/2];
				dst[x + 1] = src[x/2];
			}
			dst += dst_pitch;
			if (y&1) src += src_pitch;
		}

		SDL_UpdateRects(m_screen, 1, &m_area);
	};

	void pause() { };
	void resume() { };

	bool filter(const SDL_Event& event) { return false; };
};
const SWScaler::Factory SWScaler::factory;

/* Platform specific scalers */

#ifdef __arm__
class ARMScaler : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;
	uint8 * m_surface;
	const int m_w, m_h, m_Bpp;

protected:
	ARMScaler(SDL_Surface* screen, int w, int h)
	: m_screen(screen), m_w(w), m_h(h),
	 m_Bpp(m_screen->format->BitsPerPixel / 8)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height, w * 2, h * 2);
		m_surface = reinterpret_cast<uint8*>(malloc(w * h * m_Bpp));
	}
public:
	virtual ~ARMScaler()
	{
		free(m_surface);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "arm2x";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return bpp == 16 && w * 2 < GUI.Width && h * 2 < GUI.Height &&
				w % 16 == 0 /* asm assumes w div by 16 */;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new ARMScaler(screen, w, h);
		}
	};

	static const Factory factory;

	virtual const char * getName() const
	{
		return "software ARM 2x scaling";
	}

	uint8* getDrawBuffer() const
	{
		return m_surface;
	};

	unsigned int getDrawBufferPitch() const
	{
		return m_w * m_Bpp;
	};

	void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	void getRatio(float & x, float & y) const
	{
		x = 2.0f; y = 2.0f;
	};

	void prepare() { };

	void finish()
	{
		uint16 * src = reinterpret_cast<uint16*>(m_surface);
		uint16 * dst = reinterpret_cast<uint16*>(
			((uint8*) m_screen->pixels)
			+ (m_area.x * m_Bpp)
			+ (m_area.y * m_screen->pitch));
		const int src_pitch = m_w;
		const int dst_pitch = m_screen->pitch / m_Bpp;
		int y;

		for (y = 0; y < m_h*2; y++) {
			asm volatile
			(
				"mov r0, %0; mov r1, %1; mov r2, %2;"
				"stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14};"
				"1:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10};"
				"mov r14,r5,lsr #16;"
				"mov r12,r5,lsl #16;"
				"orr r14,r14,r14,lsl #16;"
				"orr r12,r12,r12,lsr #16;"
				"mov r11,r4,lsr #16;"
				"mov r5,r4,lsl #16;"
				"orr r11,r11,r11,lsl #16;"
				"orr r5,r5,r5,lsr #16;"
				"mov r4,r3,lsr #16;"
				"mov r3,r3,lsl #16;"
				"orr r4,r4,r4,lsl #16;"
				"orr r3,r3,r3,lsr #16;"
				"stmia r0!,{r3,r4,r5,r11,r12,r14};"
				"mov r3,r6,lsl #16;"
				"mov r4,r6,lsr #16;"
				"orr r3,r3,r3,lsr #16;"
				"orr r4,r4,r4,lsl #16;"
				"mov r5,r7,lsl #16;"
				"mov r6,r7,lsr #16;"
				"orr r5,r5,r5,lsr #16;"
				"orr r6,r6,r6,lsl #16;"
				"mov r7,r8,lsl #16;"
				"mov r8,r8,lsr #16;"
				"orr r7,r7,r7,lsr #16;"
				"orr r8,r8,r8,lsl #16;"
				"mov r12,r10,lsr #16;"
				"mov r11,r10,lsl #16;"
				"orr r12,r12,r12,lsl #16;"
				"orr r11,r11,r11,lsr #16;"
				"mov r10,r9,lsr #16;"
				"mov r9,r9,lsl #16;"
				"orr r10,r10,r10,lsl #16;"
				"orr r9,r9,r9,lsr #16;"
				"stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10,r11,r12};"
				"subs r2,r2,#16;"
				"bhi 1b;"
				"ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14};"
			:
			: "r" (dst), "r" (src), "r" (m_w)
			: "r0", "r1", "r2", "r3"
			);
			dst += dst_pitch;
			if (y&1) src += src_pitch;
		}

		SDL_UpdateRects(m_screen, 1, &m_area);
	};

	void pause() { };
	void resume() { };
	bool filter(const SDL_Event& event) { return false; };
};
const ARMScaler::Factory ARMScaler::factory;
#endif

#if CONF_HD
class HAAScalerBase : public Scaler
{
	SDL_Surface *m_screen;
	SDL_Rect m_area;
	HAA_Actor *actor;
	const int m_w, m_h, m_Bpp;
	const float ratio_x, ratio_y;

protected:
	HAAScalerBase(SDL_Surface* screen, int w, int h, float r_x, float r_y)
	: m_screen(screen), m_w(w), m_h(h),
	 m_Bpp(m_screen->format->BitsPerPixel / 8),
	 ratio_x(r_x), ratio_y(r_y)
	{
		const bool fullscreen = m_screen->flags & SDL_FULLSCREEN;
		centerRectangle(m_area, GUI.Width, GUI.Height, w * r_x, h * r_y);

		// Clear the SDL screen with black, just in case it gets drawn.
		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

		HAA_Init(fullscreen ? SDL_FULLSCREEN : 0);
		actor = HAA_CreateActor(0, m_w, m_h, m_screen->format->BitsPerPixel);
		HAA_SetPosition(actor, m_area.x, m_area.y + (fullscreen ? 0 : 60));
		HAA_SetScale(actor, r_x, r_y);
		HAA_Show(actor);
	}

public:
	virtual ~HAAScalerBase()
	{
		HAA_FreeActor(actor);
		HAA_Quit();
	};

	uint8* getDrawBuffer() const
	{
		return reinterpret_cast<uint8*>(actor->surface->pixels);
	};

	unsigned int getDrawBufferPitch() const
	{
		return actor->surface->pitch;
	};

	void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	void getRatio(float & x, float & y) const
	{
		x = ratio_x; y = ratio_y;
	};

	void prepare()
	{

	};

	void finish()
	{
		HAA_Flip(actor);
	};

	void pause() { };
	void resume() { };

	bool filter(const SDL_Event& event)
	{
		return HAA_FilterEvent(&event) == 0;
	};
};

class HAAFillScaler : public HAAScalerBase
{
	HAAFillScaler(SDL_Surface* screen, int w, int h)
	: HAAScalerBase(screen, w, h,
		GUI.Width / (float)w, GUI.Height / (float)h)
	{
	}

public:
	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "haafill";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HAAFillScaler(screen, w, h-20);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "HAA fill screen scaling";
	}
};
const HAAFillScaler::Factory HAAFillScaler::factory;

class HAASquareScaler : public HAAScalerBase
{
	HAASquareScaler(SDL_Surface* screen, int w, int h, float ratio)
	: HAAScalerBase(screen, w, h, ratio, ratio)
	{
	}

public:
	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "haasq";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HAASquareScaler(screen, w, h,
				fminf(GUI.Width / (float)w, GUI.Height / (float)h));
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "HAA square screen scaling";
	}
};
const HAASquareScaler::Factory HAASquareScaler::factory;

#endif /* CONF_HD */

#if CONF_XSP
class XSPScaler : public Scaler
{
	SDL_Surface* m_screen;
	SDL_Rect m_area;
	SDL_Rect m_real_area;
	bool m_should_enable, m_enabled; // Try to avoid flicker.

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

	XSPScaler(SDL_Surface* screen, int w, int h)
	: m_screen(screen), m_should_enable(true), m_enabled(false)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height,
			w * 2, h * 2);

		m_real_area.x = m_area.x;
		m_real_area.y = m_area.y;
		m_real_area.w = m_area.w / 2;
		m_real_area.h = m_area.h / 2;
	};
public:
	~XSPScaler()
	{
		if (m_enabled) setDoubling(false);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "xsp";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return w * 2 < GUI.Width && h * 2 < GUI.Height;
		};

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new XSPScaler(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "XSP pixel doubling";
	}

	uint8* getDrawBuffer() const
	{
		const int Bpp = screen->format->BitsPerPixel / 8;
		const int pitch = screen->pitch;
		return ((uint8*) screen->pixels)
			+ (m_area.x * Bpp)
			+ (m_area.y * pitch);
	};

	unsigned int getDrawBufferPitch() const
	{
		return screen->pitch;
	};

	void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	void getRatio(float & x, float & y) const
	{
		x = 2.0f; y = 2.0f;
	};

	void prepare() 
	{
		if (m_should_enable && !m_enabled) {
			setDoubling(true);
			m_enabled = true;
		}
	};

	void finish()
	{
		SDL_UpdateRects(m_screen, 1, &m_real_area);
	};

	void pause()
	{
		m_should_enable = false;
		if (m_enabled) {
			setDoubling(false);
			m_enabled = false;
		}
	};

	void resume()
	{
		m_should_enable = true; // Will enable later
	};

	bool filter(const SDL_Event& event)
	{
		if (event.type == SDL_ACTIVEEVENT &&
		  (event.active.state & SDL_APPINPUTFOCUS)) {
			if (event.active.gain) {
				resume();
			} else {
				pause();
			}

			return true;
		}

		return false;
	};
};
const XSPScaler::Factory XSPScaler::factory;
#endif

static const ScalerFactory* scalers[] = {
/* More useful scalers come first */
#if CONF_XSP
	&XSPScaler::factory,			/* n8x0 pixel doubling */
#endif
#ifdef __arm__
	&ARMScaler::factory,			/* arm 2x scaling */
#endif
#if CONF_HD
	&HAASquareScaler::factory,		/* n900 animation actor scaling */
#endif
	&SWScaler::factory,				/* soft 2x scaling */
	&DummyScaler::factory,			/* failsafe */
/* The following scalers will not be automatically enabled, no matter what */
#if CONF_HD
	&HAAFillScaler::factory,
#endif
};

/* Entry point functions */

const ScalerFactory* searchForScaler(int bpp, int w, int h)
{
	const int n = sizeof(scalers) / sizeof(ScalerFactory*);
	int i;

	if (Config.scaler && strcasecmp(Config.scaler, "help") == 0 ) {
		// List scalers
		printf("Scalers list:\n");
		for (i = 0; i < n; i++) {
			printf(" %s\n", scalers[i]->getName());
		}
		DIE("End of scalers list");
	} else if (Config.scaler && strcasecmp(Config.scaler, "auto") != 0 ) {
		// We prefer a specific scaler
		for (i = 0; i < n; i++) {
			if (strcasecmp(scalers[i]->getName(), Config.scaler) == 0) {
				if (scalers[i]->canEnable(bpp, w, h)) {
					// Found the scaler selected by the user, and we can use it.
					return scalers[i];
				} else {
					fprintf(stderr,
						"Selected scaler '%s' cannot be enabled in this mode\n",
						Config.scaler);
					break; // Fallback to another scaler.
				}
			}
		}
		if (i == n) {
			fprintf(stderr, "Selected scaler '%s' does not exist\n",
				Config.scaler);
		}
	}

	// Just try them all now, in a buildtime set priority.
	for (i = 0; i < n; i++) {
		if (scalers[i]->canEnable(bpp, w, h)) {
			return scalers[i];
		}
	}

	DIE("Can't use any scaler; this shouldn't happen.");
}

