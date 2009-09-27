#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL.h>
#include <SDL_syswm.h>

#if CONF_XSP
#	include <X11/extensions/Xsp.h>
#endif

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

struct gui GUI;

static SDL_Surface* screen;

static SDL_Rect windowSize, screenSize;
static bool gotWindowSize, gotScreenSize;

class Scaler;
/** The current scaler object */
static Scaler* scaler;

static void centerRectangle(SDL_Rect& result, int areaW, int areaH, int w, int h)
{
	result.x = areaW / 2 - w / 2;
	result.w = w;
	result.y = areaH / 2 - h / 2;
	result.h = h;
}

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
};

class ScalerFactory
{
public:
	ScalerFactory() { };
	virtual ~ScalerFactory() { };
	virtual const char * getName() const = 0;
	virtual bool canEnable(int w, int h) const = 0;
	virtual Scaler* instantiate(SDL_Surface* screen, int w, int h) const = 0;
};

class DummyScaler : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;

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

		bool canEnable(int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new DummyScaler(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "no scaling";
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

	int getRatio() const
	{
		return 1;
	};

	void prepare() { };

	void finish()
	{
		SDL_UpdateRects(m_screen, 1, &m_area);
	};
};
const DummyScaler::Factory DummyScaler::factory;

class SWScaler : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;
	uint8 * m_surface;
	const int m_w, m_h, m_Bpp;

	SWScaler(SDL_Surface* screen, int w, int h)
	: m_screen(screen), m_w(w), m_h(h),
	 m_Bpp(m_screen->format->BitsPerPixel / 8)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height, w * 2, h * 2);
		m_surface = reinterpret_cast<uint8*>(malloc(w * h * m_Bpp));
	}
public:
	~SWScaler()
	{
		free(m_surface);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "2x";
		}

		bool canEnable(int w, int h) const
		{
			return w * 2 < GUI.Width && h * 2 < GUI.Height;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new SWScaler(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
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

	int getRatio() const
	{
		return 2;
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
};
const SWScaler::Factory SWScaler::factory;

#if CONF_XSP
class XSPScaler : public Scaler
{
	SDL_Surface* m_screen;
	SDL_Rect m_area;
	SDL_Rect m_real_area;

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
	: m_screen(screen)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height,
			w * 2, h * 2);
		setDoubling(true);

		m_real_area.x = m_area.x;
		m_real_area.y = m_area.y;
		m_real_area.w = m_area.w / 2;
		m_real_area.h = m_area.h / 2;
	};
public:
	~XSPScaler()
	{
		setDoubling(false);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "xsp";
		}

		bool canEnable(int w, int h) const
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

	int getDrawBufferPitch() const
	{
		return screen->pitch;
	};

	void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	int getRatio() const
	{
		return 2;
	};

	void prepare() { };

	void finish()
	{
		SDL_UpdateRects(m_screen, 1, &m_real_area);
	};
};
const XSPScaler::Factory XSPScaler::factory;
#endif

static const ScalerFactory* scalers[] = {
#if CONF_XSP
	&XSPScaler::factory,
#endif
	&SWScaler::factory,
	&DummyScaler::factory
};

static const ScalerFactory* searchForScaler(int w, int h)
{
	const int n = sizeof(scalers) / sizeof(ScalerFactory*);
	int i;

	if (Config.scaler) {
		// We prefer a specific scaler
		for (i = 0; i < n; i++) {
			if (strcasecmp(scalers[i]->getName(), Config.scaler) == 0) {
				if (!scalers[i]->canEnable(w, h)) {
					DIE("Cannot use selected scaler");
				}
				return scalers[i];
			}
		}
		DIE("Select scaler does not exist");
	} else {
		// Just try them all
		for (i = 0; i < n; i++) {
			if (scalers[i]->canEnable(w, h)) {
				return scalers[i];
			}
		}
		DIE("Can't use any scaler");
	}
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

	free(scaler); scaler = 0;
}

static void setupVideoSurface()
{
	// Real surface area.
	const unsigned gameWidth = IMAGE_WIDTH;
	const unsigned gameHeight = IMAGE_HEIGHT;

	if (scaler) {
		delete scaler;
		scaler = 0;
	}

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

	// Safeguard
	if (gameHeight > GUI.Height || gameWidth > GUI.Width)
		DIE("Video is larger than window size!");

	const ScalerFactory* sFactory = searchForScaler(gameWidth, gameHeight);

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
	GUI.Scale = scaler->getRatio();

	printf("Video: %dx%d (%dx%d output), %hu bits per pixel, %s, %s\n",
		gameWidth, gameHeight,
		screen->w, screen->h, screen->format->BitsPerPixel,
		Config.fullscreen ? "fullscreen" : "windowed",
		scaler->getName());
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
#if 0 // TODO
	if (Config.xsp) {
		setDoubling(hasFocus);
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

	return TRUE;
}

