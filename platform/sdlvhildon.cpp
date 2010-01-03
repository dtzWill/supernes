#if CONF_HD

#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "sdlv.h"
#include "hgw.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);

static const char * hdAtomNames[] = {
	"_HILDON_NON_COMPOSITED_WINDOW",
	"_HILDON_STACKABLE_WINDOW",
	"_NET_WM_STATE",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_HILDON_WM_WINDOW_TYPE_ANIMATION_ACTOR",
	"_HILDON_ANIMATION_CLIENT_READY",
	"_HILDON_ANIMATION_CLIENT_MESSAGE_SHOW",	
	"_HILDON_ANIMATION_CLIENT_MESSAGE_POSITION",
	"_HILDON_ANIMATION_CLIENT_MESSAGE_ROTATION",
	"_HILDON_ANIMATION_CLIENT_MESSAGE_SCALE",
	"_HILDON_ANIMATION_CLIENT_MESSAGE_ANCHOR",
	"_HILDON_ANIMATION_CLIENT_MESSAGE_PARENT",
	"_HILDON_WM_WINDOW_TYPE_REMOTE_TEXTURE",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_SHM",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_DAMAGE",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_SHOW",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_POSITION",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_OFFSET",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_SCALE",
	"_HILDON_TEXTURE_CLIENT_MESSAGE_PARENT",
	"_HILDON_TEXTURE_CLIENT_READY",
	""
};

static bool hd_ok = false;

Atom hdAtomsValues[ATOM_COUNT];

SDL_SysWMinfo WMinfo;

static void hd_setup_stackable_window();

void hd_setup()
{
	SDL_VERSION(&WMinfo.version);
	if (!SDL_GetWMInfo(&WMinfo)) {
		DIE("Bad SDL version");
	}

	Display * display = WMinfo.info.x11.display;

	if (!hd_ok) {
		XInternAtoms(display, (char**)hdAtomNames, ATOM_COUNT, True, hdAtomsValues);

		if (HDATOM(_HILDON_NON_COMPOSITED_WINDOW) == None) {
			DIE("Hildon Desktop seems not be loaded, since %s is not defined",
				"_HILDON_NON_COMPOSITED_WINDOW");
			return;
		}

		if (hgwLaunched) {
			hd_setup_stackable_window();
		}

		hd_ok = true;
	}
};

/** Enables or disables the Hildon NonCompositedWindow property */
void hd_set_non_compositing(bool enable)
{
	Display *display;
	Window xwindow;
	XSetWindowAttributes xattr;
	Atom atom;
	int one = 1;

	WMinfo.info.x11.lock_func();
	display = WMinfo.info.x11.display;
	xwindow = WMinfo.info.x11.fswindow;

	if (enable) {
		/* 
		 * The basic idea behind this is to disable the override_redirect
		 * window attribute, which SDL sets, and instead use _NET_WM_STATE
		 * to tell hildon-desktop to fullscreen the app.
		 * I am not really happy with this, which should ideally be fixed
		 * at the libsdl level, but seems to work.
		 * As soon as the window is managed by Hildon-Desktop again, set for it
		 * not to be composited.
		 */
		XUnmapWindow(display, xwindow);
		xattr.override_redirect = False;
		XChangeWindowAttributes(display, xwindow, CWOverrideRedirect, &xattr);

		atom = HDATOM(_NET_WM_STATE_FULLSCREEN);
		XChangeProperty(display, xwindow, HDATOM(_NET_WM_STATE),
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *) &atom, 1);

		XChangeProperty(display, xwindow, HDATOM(_HILDON_NON_COMPOSITED_WINDOW),
			XA_INTEGER, 32, PropModeReplace,
			(unsigned char *) &one, 1);
		XMapWindow(display, xwindow);
	} else {
		xattr.override_redirect = True;
		XDeleteProperty(display, xwindow,
			HDATOM(_HILDON_NON_COMPOSITED_WINDOW));
		XChangeWindowAttributes(display, xwindow, CWOverrideRedirect, &xattr);
	}

	WMinfo.info.x11.unlock_func();
}

static Atom get_window_type(Display *dpy, Window win)
{
	Atom window_type = None;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *prop_return = NULL;

    if(Success == XGetWindowProperty(dpy, win, HDATOM(_NET_WM_WINDOW_TYPE),
    								0L, sizeof(Atom),
                                     False, XA_ATOM, &actual_type,
                                     &actual_format, &nitems, &bytes_after,
                                     &prop_return) && prop_return) {
		window_type = *(Atom *)prop_return;
		XFree(prop_return);
	}

	return window_type;
}

static Window find_launcher_window(Display *display, Window start)
{
	Window root, parent, *children;
	unsigned int count, i;

	if (XQueryTree(display, start, &root, &parent, &children, &count)) {
		XClassHint class_hints;
		for (i = 0; i < count; i++) {
			Window cur = children[i];
			Window sub = find_launcher_window(display, cur);
			if (sub) return sub;

			if (XGetClassHint(display, cur, &class_hints)) {
				if (strcasecmp(class_hints.res_class, "drnoksnes_startup") == 0) {
					XFree(class_hints.res_name);
					XFree(class_hints.res_class);

					if (get_window_type(display, cur) != 
						HDATOM(_NET_WM_WINDOW_TYPE_NORMAL)) {
						continue;
					}

					XFree(children);

					return cur;
				}
				XFree(class_hints.res_name);
				XFree(class_hints.res_class);
			}
		}
		XFree(children);
	}
	
	return 0;
}

/** Finds the drnoksnes_startup window. */
static Window find_launcher_window(Display *display)
{
	return find_launcher_window(display,
		RootWindow(display, DefaultScreen(display)));
}

/** Converts the aux SDL windows into tops of the HildonWindowStack */
static void hd_setup_stackable_window()
{
	Display *display;
	Window window;
	Atom atom = HDATOM(_NET_WM_WINDOW_TYPE_NORMAL);
	XWMHints *hints;
	int one = 1;

	WMinfo.info.x11.lock_func();
	display = WMinfo.info.x11.display;

	Window launcher = find_launcher_window(display);

	if (!launcher) {
		printf("HD: Games startup window was not found\n");
		return;
	}

	hints = XGetWMHints(display, launcher);
	hints->flags &= WindowGroupHint;

	window = WMinfo.info.x11.wmwindow;
	XSetTransientForHint(display, window, launcher);
	XSetWMHints(display, window, hints);
	XChangeProperty(display, window, HDATOM(_NET_WM_WINDOW_TYPE),
		XA_ATOM, 32, PropModeReplace,
		(unsigned char *) &atom, 1);
	XChangeProperty(display, window, HDATOM(_HILDON_STACKABLE_WINDOW),
		XA_INTEGER, 32, PropModeReplace,
		(unsigned char *) &one, 1);

	window = WMinfo.info.x11.fswindow;
	XSetTransientForHint(display, window, launcher);
	XSetWMHints(display, window, hints);
	XChangeProperty(display, window, HDATOM(_NET_WM_WINDOW_TYPE),
		XA_ATOM, 32, PropModeReplace,
		(unsigned char *) &atom, 1);
	XChangeProperty(display, window, HDATOM(_HILDON_STACKABLE_WINDOW),
		XA_INTEGER, 32, PropModeReplace,
		(unsigned char *) &one, 1);

	XFree(hints);

	WMinfo.info.x11.unlock_func();
}

#endif
