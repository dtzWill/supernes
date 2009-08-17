#include <SDL.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"

#define kPollEveryNFrames	3

static uint32 joypads[2];
static struct {
	unsigned x;
	unsigned y;
	bool pressed;
} mouse;

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
		mouse.x = event.button.x;
		mouse.y = event.button.y;
		if (Config.xsp) {
			mouse.x /= 2;
			mouse.y /= 2;
		}
		mouse.pressed = event.button.state == SDL_PRESSED;
		break;
	case SDL_MOUSEMOTION:
		mouse.x = event.motion.x;
		mouse.y = event.motion.y;
		if (Config.xsp) {
			mouse.x /= 2;
			mouse.y /= 2;
		}
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
			printf("Input: 1 joypad + mouse\n");
			break;
		case SNES_MOUSE_SWAPPED:
			printf("Input: mouse\n");
			break;
		case SNES_SUPERSCOPE:
			joypads[0] = 0x80000000UL;
			printf("Input: 1 joypad + superscope\n");
			break;
		default:
			printf("Input: unknown\n");
			break;
	}
}

