#include <SDL.h>
#include <math.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"

struct TouchButton {
	unsigned short mask;
	unsigned short x, y;
	unsigned short x2, y2;
	float fx, fy;
	float fw, fh;
};

#define TOUCH_BUTTON_INITIALIZER(name, x, y, w, h) \
	{SNES_##name##_MASK, 0, 0, 0, 0, x, y, w, h}

static TouchButton touchbuttons[] = {
	TOUCH_BUTTON_INITIALIZER(TL, 0, 0, 0.375, 0.0833),
	TOUCH_BUTTON_INITIALIZER(TR, 0.625, 0, 0.375, 0.0833),
	TOUCH_BUTTON_INITIALIZER(UP, 0.125, 0.0833, 0.125, 0.2777), //2
	TOUCH_BUTTON_INITIALIZER(LEFT, 0.0, 0.3611, 0.125, 0.2777), //3
	TOUCH_BUTTON_INITIALIZER(RIGHT, 0.25, 0.3611, 0.125, 0.2777), //4
	TOUCH_BUTTON_INITIALIZER(DOWN, 0.125, 0.6388, 0.125, 0.2777), //5
	TOUCH_BUTTON_INITIALIZER(START, 0, 0.9166, 0.375, 0.0833),
	TOUCH_BUTTON_INITIALIZER(Y, 0.75, 0.0833, 0.125, 0.2777),
	TOUCH_BUTTON_INITIALIZER(X, 0.625, 0.3611, 0.125, 0.2777),
	TOUCH_BUTTON_INITIALIZER(A, 0.875, 0.3611, 0.125, 0.2777),
	TOUCH_BUTTON_INITIALIZER(B, 0.75, 0.6388, 0.125, 0.2777),
	TOUCH_BUTTON_INITIALIZER(SELECT, 0.625, 0.9166, 0.375, 0.0833),
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
		if (x > touchbuttons[i].x && x < touchbuttons[i].x2 &&
			y > touchbuttons[i].y && y < touchbuttons[i].y2) {

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
		// TODO Review this
		mouse.x = x;
		mouse.y = y;
		if (Config.xsp) {
			mouse.x /= 2;
			mouse.y /= 2;
		}
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

void S9xProcessEvents(bool8_32 block)
{
	SDL_Event event;

	if (block) {
		SDL_WaitEvent(&event);
		processEvent(event);
	} else {
		while(SDL_PollEvent(&event)) 
		{      
			processEvent(event);
		}
	}
}

void S9xInitInputDevices()
{
	joypads[0] = 0;
	joypads[1] = 0;

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
	unsigned int i = 0, w = 0, h = 0;
	S9xVideoGetWindowSize(&w, &h);
	for (i = 0; i < sizeof(touchbuttons)/sizeof(TouchButton); i++) {
		touchbuttons[i].x = (unsigned)round(touchbuttons[i].fx * w);
		touchbuttons[i].y = (unsigned)round(touchbuttons[i].fy * h);
		touchbuttons[i].x2 = (unsigned)round(touchbuttons[i].x + touchbuttons[i].fw * w);
		touchbuttons[i].y2 = (unsigned)round(touchbuttons[i].y + touchbuttons[i].fh * h);
	}
}

