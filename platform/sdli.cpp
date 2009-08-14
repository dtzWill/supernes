#include <SDL.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"

#define kPollEveryNFrames	3

static uint32 joypads[2];

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

bool8 S9xReadMousePosition (int /* which1 */, int &/* x */, int & /* y */,
		    uint32 & /* buttons */)
{
	return FALSE;
}

bool8 S9xReadSuperScopePosition (int & /* x */, int & /* y */,
			 uint32 & /* buttons */)
{
	return FALSE;
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
	joypads[0] = 0x80000000UL;
	joypads[1] = 0;
	
	printf("Input: 1 joypad, hw keyboard input only\n");
}

