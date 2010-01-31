#include <SDL.h>
#include <math.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"
#include "sdlv.h" // Dispatching video-related events

struct TouchButton {
	unsigned short mask;
	unsigned short x, y;
	unsigned short x2, y2;
	double fx, fy;
	double fw, fh;
};

#define TOUCH_BUTTON_INITIALIZER(name, x, y, w, h) \
	{SNES_##name##_MASK, 0, 0, 0, 0, x, y, w, h}

#define kCornerButtonWidth 	(0.375)
#define kCornerButtonHeight	(0.0833333333334)
#define kBigButtonWidth		(0.125)
#define kBigButtonHeight	(0.2777777777778)

static TouchButton touchbuttons[] = {
	TOUCH_BUTTON_INITIALIZER(TL, 0.0, 0.0, kCornerButtonWidth, kCornerButtonHeight),
	TOUCH_BUTTON_INITIALIZER(TR, 0.625, 0.0, kCornerButtonWidth, kCornerButtonHeight),
	TOUCH_BUTTON_INITIALIZER(UP, kBigButtonWidth, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(LEFT, 0.0, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(RIGHT, 2.0 * kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(DOWN, kBigButtonWidth, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(SELECT, 0.0, 1.0 - kCornerButtonHeight, kCornerButtonWidth, kCornerButtonHeight),
	TOUCH_BUTTON_INITIALIZER(X, 1.0 - 2.0 * kBigButtonWidth, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(Y, 1.0 - 3.0 * kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(A, 1.0 - kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(B, 1.0 - 2.0 * kBigButtonWidth, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TOUCH_BUTTON_INITIALIZER(START, 1.0 - kCornerButtonWidth, 1.0 - kCornerButtonHeight, kCornerButtonWidth, kCornerButtonHeight),
};

static TouchButton* current = 0;

static uint32 joypads[2];
static struct {
	unsigned x;
	unsigned y;
	bool enabled, pressed;
} mouse;

static TouchButton* getButtonFor(unsigned int x, unsigned int y) {
	unsigned int i;

	for (i = 0; i < sizeof(touchbuttons)/sizeof(TouchButton); i++) {
		if (x >= touchbuttons[i].x && x < touchbuttons[i].x2 &&
			y >= touchbuttons[i].y && y < touchbuttons[i].y2) {

			return &touchbuttons[i];
		}
	}

	return 0;
}

static inline void unpress(TouchButton* b) {
	joypads[0] &= ~b->mask;
}
static inline void press(TouchButton* b) {
	joypads[0] |= b->mask;
}

static void processMouse(unsigned int x, unsigned int y, int pressed = 0)
{
#if CONF_EXIT_BUTTON
	/* no fullscreen escape button, we have to simulate one! */
	/* TODO: don't hardcode sizes */
	if (Config.fullscreen && x > (800 - 100) && y < 50 && pressed > 0) {
		S9xDoAction(kActionQuit);
	}
#endif
	if (Config.touchscreenInput) {
		if (pressed < 0) {
			// Button up.
			if (current) {
				// Leaving button
				unpress(current);
				current = 0;
			}
		} else {
			// Button down, or mouse motion.
			TouchButton* b = getButtonFor(x, y);
			if (current && b && current != b) {
				// Moving from button to button
				unpress(current);
				current = b;
				press(current);
			} else if (current && !b) {
				// Leaving button
				unpress(current);
				current = 0;
			} else if (!current && b) {
				// Entering button
				current = b;
				press(current);
			}
		}
	} else if (mouse.enabled) {
		mouse.x = x;
		mouse.y = y;

		if (mouse.x < GUI.RenderX) mouse.x = 0;
		else {
			mouse.x -= GUI.RenderX;
			if (mouse.x > GUI.RenderW) mouse.x = GUI.RenderW;
		}

		if (mouse.y < GUI.RenderY) mouse.y = 0;
		else {
			mouse.y -= GUI.RenderY;
			if (mouse.y > GUI.RenderH) mouse.y = GUI.RenderH;
		}

		// Take care of scaling
		mouse.x /= GUI.ScaleX;
		mouse.y /= GUI.ScaleY;

		if (pressed > 0)
			mouse.pressed = true;
		else if (pressed < 0)
			mouse.pressed = false;
	}
}

static void processEvent(const SDL_Event& event)
{
	switch (event.type) 
	{
		case SDL_KEYDOWN:
			if (Config.action[event.key.keysym.scancode]) 
				S9xDoAction(Config.action[event.key.keysym.scancode]);
			joypads[0] |= Config.joypad1Mapping[event.key.keysym.scancode];
			break;
		case SDL_KEYUP:
			joypads[0] &= ~Config.joypad1Mapping[event.key.keysym.scancode];
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			processMouse(event.button.x, event.button.y,
					(event.button.state == SDL_PRESSED) ? 1 : - 1);
			break;
		case SDL_MOUSEMOTION:
			processMouse(event.motion.x, event.motion.y);
			break;
		case SDL_QUIT:
			Config.quitting = true;
			break;
		case SDL_ACTIVEEVENT:
		case SDL_SYSWMEVENT:
			processVideoEvent(event);
			break;
	}
}

uint32 S9xReadJoypad (int which)
{
	if (which < 0 || which > 2) {
		return 0;
	}

	return joypads[which];
}

bool8 S9xReadMousePosition(int which1, int& x, int& y, uint32& buttons)
{
	if (which1 != 0) return FALSE;

	x = mouse.x;
	y = mouse.y;
	buttons = mouse.pressed ? 1 : 0;

	return TRUE;
}

bool8 S9xReadSuperScopePosition(int& x, int& y, uint32& buttons)
{
	x = mouse.x;
	y = mouse.y;
	buttons = mouse.pressed ? 8 : 0;

	return TRUE;
}

void S9xProcessEvents(bool block)
{
	SDL_Event event;

	if (block) {
		SDL_WaitEvent(&event);
		processEvent(event);
	} else {
		while(SDL_PollEvent(&event)) {
			processEvent(event);
		}
	}
}

void S9xInitInputDevices()
{
	joypads[0] = 0;
	joypads[1] = 0;
	mouse.enabled = false;
	mouse.pressed = false;

	switch (Settings.ControllerOption) {
		case SNES_JOYPAD:
			joypads[0] = 0x80000000UL;
			printf("Input: 1 joypad, keyboard only\n");
			break;
		case SNES_MOUSE:
			joypads[0] = 0x80000000UL;
			mouse.enabled = true;
			printf("Input: 1 joypad + mouse\n");
			break;
		case SNES_MOUSE_SWAPPED:
			printf("Input: mouse\n");
			mouse.enabled = true;
			break;
		case SNES_SUPERSCOPE:
			joypads[0] = 0x80000000UL;
			mouse.enabled = true;
			printf("Input: 1 joypad + superscope\n");
			break;
		default:
			printf("Input: unknown\n");
			break;
	}

	S9xInputScreenChanged();
}

void S9xDeinitInputDevices()
{

}

void S9xInputScreenChanged()
{
	unsigned int i = 0;
	const unsigned int w = GUI.Width, h = GUI.Height;
	for (i = 0; i < sizeof(touchbuttons)/sizeof(TouchButton); i++) {
		touchbuttons[i].x = (unsigned int)(touchbuttons[i].fx * w);
		touchbuttons[i].y = (unsigned int)(touchbuttons[i].fy * h);
		touchbuttons[i].x2 = (unsigned int)(touchbuttons[i].x + touchbuttons[i].fw * w);
		touchbuttons[i].y2 = (unsigned int)(touchbuttons[i].y + touchbuttons[i].fh * h);
	}
}

template <typename T>
static void drawControls(T * buffer, const int pitch)
{
	unsigned int i = 0;
	int x, y;
	const T black = static_cast<T>(0xFFFFFFFFU);
	T* temp;

	for (i = 0; i < sizeof(touchbuttons)/sizeof(TouchButton); i++) {
		temp = buffer + touchbuttons[i].y * pitch + touchbuttons[i].x;
		for (x = touchbuttons[i].x; x < touchbuttons[i].x2; x++) {
			*temp = black;
			temp++;
		}
		temp = buffer + touchbuttons[i].y2 * pitch + touchbuttons[i].x;
		for (x = touchbuttons[i].x; x < touchbuttons[i].x2; x++) {
			*temp = black;
			temp++;
		}
		temp = buffer + touchbuttons[i].y * pitch + touchbuttons[i].x;
		for (y = touchbuttons[i].y; y < touchbuttons[i].y2; y++) {
			*temp = black;
			temp+=pitch;
		}
		temp = buffer + touchbuttons[i].y * pitch + touchbuttons[i].x2;
		for (y = touchbuttons[i].y; y < touchbuttons[i].y2; y++) {
			*temp = black;
			temp+=pitch;
		}
	}
}

void S9xInputScreenDraw(int pixelSize, void * buffer, int pitch)
{
	switch (pixelSize)
	{
		case 1:
			drawControls(reinterpret_cast<uint8*>(buffer), pitch);
			break;
		case 2:
			drawControls(reinterpret_cast<uint16*>(buffer), pitch / 2);
			break;
	}
}

