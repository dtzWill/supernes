#include <SDL.h>
#include <math.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"
#include "sdlv.h" // Dispatching video-related events

#if CONF_ZEEMOTE
#include "zeemote.h"
#endif

struct TouchButton {
	unsigned short mask;
	unsigned short x, y;
	unsigned short x2, y2;
	double fx, fy;
	double fw, fh;
};

#define kCornerButtonWidth 	(0.375)
#define kCornerButtonHeight	(0.0833333333334)
#define kBigButtonWidth		(0.125)
#define kBigButtonHeight	(0.2777777777778)

static TouchButton touchbuttons[] = {
#define TB(actions, x, y, w, h) \
	{actions, 0, 0, 0, 0, x, y, w, h}
#define P(x) SNES_##x##_MASK
	TB(P(TL), 0.0, 0.0, kCornerButtonWidth, kCornerButtonHeight),
	TB(P(TR), 0.625, 0.0, kCornerButtonWidth, kCornerButtonHeight),
	TB(P(LEFT) | P(UP), 0.0, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(UP), kBigButtonWidth, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(RIGHT) | P(UP), 2.0 * kBigButtonWidth, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(LEFT), 0.0, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(RIGHT), 2.0 * kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(LEFT) | P(DOWN), 0, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TB(P(DOWN), kBigButtonWidth, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TB(P(RIGHT) | P(DOWN), 2.0 * kBigButtonWidth, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TB(P(SELECT), 0.0, 1.0 - kCornerButtonHeight, kCornerButtonWidth, kCornerButtonHeight),
	TB(P(X), 1.0 - 2.0 * kBigButtonWidth, kCornerButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(Y), 1.0 - 3.0 * kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(A), 1.0 - kBigButtonWidth, kCornerButtonHeight + kBigButtonHeight, kBigButtonWidth, kBigButtonHeight),
	TB(P(B), 1.0 - 2.0 * kBigButtonWidth, 1.0 - (kCornerButtonHeight + kBigButtonHeight), kBigButtonWidth, kBigButtonHeight),
	TB(P(START), 1.0 - kCornerButtonWidth, 1.0 - kCornerButtonHeight, kCornerButtonWidth, kCornerButtonHeight),
#undef P
#undef TB
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
	joypads[Config.touchscreenInput - 1] &= ~b->mask;
}
static inline void press(TouchButton* b) {
	joypads[Config.touchscreenInput - 1] |= b->mask;
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

		// mouse.{x,y} are system coordinates.
		// Scale them to emulated screen coordinates.
		mouse.x = static_cast<unsigned int>(mouse.x / GUI.ScaleX);
		mouse.y = static_cast<unsigned int>(mouse.y / GUI.ScaleY);

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
			joypads[1] |= Config.joypad2Mapping[event.key.keysym.scancode];
			break;
		case SDL_KEYUP:
			joypads[0] &= ~Config.joypad1Mapping[event.key.keysym.scancode];
			joypads[1] &= ~Config.joypad2Mapping[event.key.keysym.scancode];
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

/** This function is called to return a bit-wise mask of the state of one of the
	five emulated SNES controllers.

	@return 0 if you're not supporting controllers past a certain number or
		return the mask representing the current state of the controller number
		passed as a parameter or'ed with 0x80000000.
*/

uint32 S9xReadJoypad (int which)
{
	if (which < 0 || which >= 2) {
		// More joypads that we currently handle (could happen if bad conf)
		return 0;
	}

	return joypads[which];
}

/** Get the current position of the host pointing device, usually a mouse,
	used to emulated the SNES mouse.

	@param buttons The buttons return value is a bit-wise mask of the two SNES
		mouse buttons, bit 0 for button 1 (left) and bit 1 for button 2 (right).
*/
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

/** Get and process system/input events.
	@param block true to block, false to poll until the queue is empty.
*/
void S9xProcessEvents(bool block)
{
	SDL_Event event;

#if CONF_ZEEMOTE
	// Wheter blocking or non blocking, poll zeemotes now.
	ZeeRead(joypads);
#endif

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

#if CONF_ZEEMOTE
	ZeeInit();
#endif

	if (Config.joypad1Enabled) {
		joypads[0] = 0x80000000UL;
	}
	if (Config.joypad2Enabled) {
		joypads[1] = 0x80000000UL;
	}

	// Pretty print some information
	printf("Input: ");
	if (Config.joypad1Enabled) {
		printf("Player 1 (joypad)");
		if (Config.joypad2Enabled) {
			printf("+ player 2 (joypad)");
		}
	} else if (Config.joypad2Enabled) {
		printf("Player 2 (joypad)");
	} else {
		printf("Nothing");
	}
	printf("\n");

	// TODO Non-awful mouse & superscope support

	S9xInputScreenChanged();
}

void S9xDeinitInputDevices()
{
#if CONF_ZEEMOTE
	ZeeQuit();
#endif
	joypads[0] = 0;
	joypads[1] = 0;
	mouse.enabled = false;
	mouse.pressed = false;
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

