#include <stdio.h> 
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL.h>

#if CONF_XSP
#	include <X11/extensions/Xsp.h>
#endif
#if CONF_HD
#	include <X11/Xatom.h>
#	include <sys/ipc.h>
#	include <sys/shm.h>
#endif

#include "snes9x.h"
#include "display.h"
#include "platform.h"
#include "scaler.h"
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
};
const ARMScaler::Factory ARMScaler::factory;
#endif

#if CONF_HD

class HDScalerBase : public Scaler
{
	SDL_Surface * m_screen;
	SDL_Rect m_area;
	const int m_w, m_h, m_Bpp;
	const float ratio_x, ratio_y;

	// SDL/X11 stuff we save for faster access.
	Display* display;
	Window window;

	// Shared memory segment info.
	key_t shmkey;
	int shmid;
	void *shmaddr;

private:
	/** Sends a message to hildon-desktop.
	  * This function comes mostly straight from libhildon.
	  */
	void sendMessage(Atom message_type,
		uint32 l0, uint32 l1, uint32 l2, uint32 l3, uint32 l4)
	{
		XEvent event = { 0 };

		event.xclient.type = ClientMessage;
		event.xclient.window = window;
		event.xclient.message_type = message_type;
		event.xclient.format = 32;
		event.xclient.data.l[0] = l0;
		event.xclient.data.l[1] = l1;
		event.xclient.data.l[2] = l2;
		event.xclient.data.l[3] = l3;
		event.xclient.data.l[4] = l4;

		XSendEvent (display, window, True,
		            StructureNotifyMask,
		            (XEvent *)&event);
	}

	/** Sends all configuration parameters for the remote texture. */
	void reconfigure()
	{
		Window parent;
		int yoffset = 0;
		if (Config.fullscreen) {
			parent = WMinfo.info.x11.fswindow;
		} else {
			parent = WMinfo.info.x11.wmwindow;
			yoffset = 60; // Hardcode the title bar size for now.
		}

		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_SHM),
			(uint32) shmkey, m_w, m_h, m_Bpp, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_PARENT),
			(uint32) parent, 0, 0, 0, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_POSITION),
			m_area.x, yoffset + m_area.y, m_area.w, m_area.h, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_SCALE),
			ratio_x * (1 << 16), ratio_y * (1 << 16), 0, 0, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_SHOW),
			1, 255, 0, 0, 0);
	}

protected:
	HDScalerBase(SDL_Surface* screen, int w, int h, float r_x, float r_y)
	: m_screen(screen), m_w(w), m_h(h),
	 m_Bpp(m_screen->format->BitsPerPixel / 8),
	 ratio_x(r_x), ratio_y(r_y)
	{
		centerRectangle(m_area, GUI.Width, GUI.Height, w * r_x, h * r_y);

		// What we're going to do:
		//  - Create a new window that we're going to manage
		//  - Set up that window as a Hildon Remote Texture
		//  - Render to that new window, instead of the SDL window ("screen").
		// Yet another load of uglyness, but hey.

		// Clear the SDL screen with black, just in case it gets drawn.
		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

		display = WMinfo.info.x11.display;

		// The parent window needs to be mapped, so we sync it.
		XSync(display, True);

		// Create our alternative window.
		const int blackColor = BlackPixel(display, DefaultScreen(display));
		window = XCreateSimpleWindow(display, DefaultRootWindow(display),
			0, 0, m_w, m_h, 0, blackColor, blackColor);
		XStoreName(display, window, "DrNokSnes Video output window");
		Atom atom = HDATOM(_HILDON_WM_WINDOW_TYPE_REMOTE_TEXTURE);
		XChangeProperty(display, window, HDATOM(_NET_WM_WINDOW_TYPE),
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *) &atom, 1);
		XSelectInput(display, window, PropertyChangeMask | StructureNotifyMask);
		XMapWindow(display, window);

		// Wait for "ready" property, set up by hildon-desktop after a while
		// For now, loop here. In the future, merge with main event loop.
		bool ready = false;
		while (!ready) {
			XEvent e;
			XNextEvent(display, &e);
			switch(e.type) {
				case PropertyNotify:
					if (e.xproperty.atom ==
					  HDATOM(_HILDON_TEXTURE_CLIENT_READY)) {
						ready = true;
					}
					break;
				default:
					break;
			}
		}

		// Create a shared memory segment with hildon-desktop
		shmkey = ftok(S9xGetFilename(FILE_ROM), 'v');
		shmid = shmget(shmkey, m_w * m_h * m_Bpp, IPC_CREAT | 0777);
		if (shmid < 0) {
			DIE("Failed to create shared memory");
		}
		shmaddr = shmat(shmid, 0, 0);
		if (shmaddr == (void*)-1) {
			DIE("Failed to attach shared memory");
		}

		// Send all configuration events to hildon-desktop
		reconfigure();
	}

public:
	virtual ~HDScalerBase()
	{
		// Hide, unparent and deattach the remote texture
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_SHOW),
			0, 255, 0, 0, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_PARENT),
			0, 0, 0, 0, 0);
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_SHM),
			0, 0, 0, 0, 0);
		XFlush(display);
		// Destroy our managed window and shared memory segment
		XDestroyWindow(display, window);
		XSync(display, True);
		shmdt(shmaddr);
		shmctl(shmid, IPC_RMID, 0);
	};

	virtual uint8* getDrawBuffer() const
	{
		return reinterpret_cast<uint8*>(shmaddr);
	};

	virtual unsigned int getDrawBufferPitch() const
	{
		return m_w * m_Bpp;
	};

	virtual void getRenderedGUIArea(unsigned short & x, unsigned short & y,
							unsigned short & w, unsigned short & h) const
	{
		x = m_area.x; y = m_area.y; w = m_area.w; h = m_area.h;
	};

	virtual void getRatio(float & x, float & y) const
	{
		x = ratio_x; y = ratio_y;
	};

	virtual void prepare()
	{

	};

	virtual void finish()
	{
		// Send a damage event to hildon-desktop.
		sendMessage(HDATOM(_HILDON_TEXTURE_CLIENT_MESSAGE_DAMAGE),
			0, 0, m_w, m_h, 0);
		XSync(display, False);
	};

	virtual void pause() { };
	virtual void resume() { };
};

class HDFillScaler : public HDScalerBase
{
	HDFillScaler(SDL_Surface* screen, int w, int h)
	: HDScalerBase(screen, w, h,
		GUI.Width / (float)w, GUI.Height / (float)h)
	{
	}

public:
	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "hdfill";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HDFillScaler(screen, w, h-20);
		}
	};

	static const Factory factory;

	virtual const char * getName() const
	{
		return "hildon-desktop fill screen scaling";
	}
};
const HDFillScaler::Factory HDFillScaler::factory;

class HDSquareScaler : public HDScalerBase
{
	HDSquareScaler(SDL_Surface* screen, int w, int h, float ratio)
	: HDScalerBase(screen, w, h, ratio, ratio)
	{
	}

public:
	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "hdsq";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return true;
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HDSquareScaler(screen, w, h,
				fminf(GUI.Width / (float)w, GUI.Height / (float)h));
		}
	};

	static const Factory factory;

	virtual const char * getName() const
	{
		return "hildon-desktop square screen scaling";
	}
};
const HDSquareScaler::Factory HDSquareScaler::factory;

class HDDummy : public DummyScaler
{
	HDDummy(SDL_Surface* screen, int w, int h)
	: DummyScaler(screen, w, h)
	{
		hd_set_non_compositing(true);
	}
	
public:
	~HDDummy()
	{
		hd_set_non_compositing(false);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "hddummy";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return Config.fullscreen; // This makes sense only in fullscreen
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HDDummy(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "compositor disabled and no scaling";
	}
};
const HDDummy::Factory HDDummy::factory;

class HDSW : public SWScaler
{
	HDSW(SDL_Surface* screen, int w, int h)
	: SWScaler(screen, w, h)
	{
		hd_set_non_compositing(true);
	}
	
public:
	~HDSW()
	{
		hd_set_non_compositing(false);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "hdsoft2x";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return Config.fullscreen; // This makes sense only in fullscreen
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HDSW(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "compositor disabled and software 2x scaling";
	}
};
const HDSW::Factory HDSW::factory;

#ifdef __arm__
class HDARM : public ARMScaler
{
	HDARM(SDL_Surface* screen, int w, int h)
	: ARMScaler(screen, w, h)
	{
		hildon_set_non_compositing(true);
	}
	
public:
	~HDARM()
	{
		hildon_set_non_compositing(false);
	};

	class Factory : public ScalerFactory
	{
		const char * getName() const
		{
			return "hdarm2x";
		}

		bool canEnable(int bpp, int w, int h) const
		{
			return Config.fullscreen; // This makes sense only in fullscreen
		}

		Scaler* instantiate(SDL_Surface* screen, int w, int h) const
		{
			return new HDARM(screen, w, h);
		}
	};

	static const Factory factory;

	const char * getName() const
	{
		return "compositor disabled and software ARM 2x scaling";
	}
};
const HDARM::Factory HDARM::factory;
#endif /* __arm__ */
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
};
const XSPScaler::Factory XSPScaler::factory;
#endif

static const ScalerFactory* scalers[] = {
/* More useful scalers come first */
#if CONF_HD && defined(__arm__)
	&HDARM::factory,				/* non-composited arm 2x scaling */
#endif
#if CONF_HD
	&HDSquareScaler::factory,		/* h-d assisted square scaling */
	&HDSW::factory,					/* non-composited soft 2x scaling */
#endif
#if CONF_XSP
	&XSPScaler::factory,			/* n8x0 pixel doubling */
#endif
#ifdef __arm__
	&ARMScaler::factory,			/* arm 2x scaling */
#endif
	&SWScaler::factory,				/* soft 2x scaling */
#if CONF_HD
	&HDDummy::factory,				/* non composited */
#endif
	&DummyScaler::factory,			/* failsafe */
/* The following scalers will not be automatically enabled, no matter what */
#if CONF_HD
	&HDFillScaler::factory,
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

