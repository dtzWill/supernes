#include <SDL.h>
#include <math.h>

#include "platform.h"
#include "snes9x.h"
#include "display.h"
#include "sdlv.h" // Dispatching video-related events
#include "Keyboard.h"
#include "Controller.h"
#include "snes9x.h"
#include "GLUtil.h"
#include "screenshot.h"

#if CONF_ZEEMOTE
#include "zeemote.h"
#endif

//Touchscreen controls primary joystick
#define TOUCH_JOY 0

static uint32 joypads[2];
static struct {
	unsigned x;
	unsigned y;
	bool enabled, pressed;
} mouse;

static void processMouse(unsigned int x, unsigned int y, bool pressed)
{
  //printf( "Mouse pressed: %d, %d, pressed=%d\n", x, y, pressed );
  if ( pressed )
  {
    joypads[TOUCH_JOY] |= controllerHitCheck( x, y );
  } else
  {
    joypads[TOUCH_JOY] &= ~controllerHitCheck( x, y );
  }
}

static void processMouseMotion(unsigned int x, unsigned int y,
    unsigned int xnew, unsigned int ynew)
{
  //If we move from (x,y) to (xnew,ynew), then the effect on the controller is
  //to unpress the buttons for the old values, and press the buttons for the values
  //Note nothing reads this data while this is happening.
  //printf( "Mouse moved: (%d,%d) -> (%d,%d)\n", x, y, xnew, ynew );
  joypads[TOUCH_JOY] &= ~controllerHitCheck( x, y );
  joypads[TOUCH_JOY] |= controllerHitCheck( xnew, ynew );
}

static int getJoyMask( int * mapping, int key )
{
  int mask = 0;

  if ( mapping[SNES_KEY_UP] == key )     mask |= SNES_UP_MASK;
  if ( mapping[SNES_KEY_DOWN] == key )   mask |= SNES_DOWN_MASK;
  if ( mapping[SNES_KEY_LEFT] == key )   mask |= SNES_LEFT_MASK;
  if ( mapping[SNES_KEY_RIGHT] == key )  mask |= SNES_RIGHT_MASK;
  if ( mapping[SNES_KEY_START] == key )  mask |= SNES_START_MASK;
  if ( mapping[SNES_KEY_SELECT] == key ) mask |= SNES_SELECT_MASK;
  if ( mapping[SNES_KEY_L] == key )      mask |= SNES_TL_MASK;
  if ( mapping[SNES_KEY_R] == key )      mask |= SNES_TR_MASK;
  if ( mapping[SNES_KEY_Y] == key )      mask |= SNES_Y_MASK;
  if ( mapping[SNES_KEY_X] == key )      mask |= SNES_X_MASK;
  if ( mapping[SNES_KEY_B] == key )      mask |= SNES_B_MASK;
  if ( mapping[SNES_KEY_A] == key )      mask |= SNES_A_MASK;

  return mask;
}

static void checkOther( int * mapping, int key )
{
  if ( mapping[SNES_KEY_TURBO] == key )
  {
    //Turbo presed, toggle it!
    Settings.TurboMode = !Settings.TurboMode;
  }
  //Special key that saves the screen to disk
  if ( key == SDLK_HASH )
  {
    S9xVideoTakeScreenshot();
  }
}


static void processEvent(const SDL_Event& event)
{
	if (videoEventFilter(event)) return;
	if (keyboardBindingFilter(event)) return;

  int key = 0;
  int x, y, xnew, ynew;
	switch (event.type) 
	{
		case SDL_KEYDOWN:
      key = event.key.keysym.sym;

			if (Config.action[key]) 
				S9xDoAction(Config.action[key]);

      joypads[0] |= getJoyMask(Config.joypad1Mapping, key);
      joypads[1] |= getJoyMask(Config.joypad2Mapping, key);

      checkOther(Config.joypad1Mapping, key );
      checkOther(Config.joypad2Mapping, key );
			break;
		case SDL_KEYUP:
      key = event.key.keysym.sym;
      joypads[0] &= ~getJoyMask(Config.joypad1Mapping, key);
      joypads[1] &= ~getJoyMask(Config.joypad2Mapping, key);
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
      x = event.button.x; y = event.button.y;
      // We interpret mouse input differently depending on orientation
      if ( orientation != ORIENTATION_PORTRAIT )
      {
        // XXX: Make this not magical
        y = 320 - event.button.x;
        x = event.button.y;
      }
			processMouse(x, y,
					(event.button.state == SDL_PRESSED));
			break;
		case SDL_MOUSEMOTION:
      xnew = event.motion.x; ynew = event.motion.y;
      x = xnew - event.motion.xrel; y = ynew - event.motion.yrel;

      // We interpret mouse input differently depending on orientation
      if ( orientation != ORIENTATION_PORTRAIT )
      {
        // XXX: Make this not magical
        ynew = 320 - event.motion.x;
        xnew = event.motion.y;
        x = xnew - event.motion.yrel;
        y = ynew + event.motion.xrel;
      }
			processMouseMotion(x, y, xnew, ynew);
			break;
//		case SDL_QUIT:
//			Config.running = false;
//			break;
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
  initialize_keymappings(&Config);
  loadSkins();
  updateOrientation();
  GL_InitTexture(IMAGE_WIDTH,IMAGE_HEIGHT);
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
#if 0
	unsigned int i = 0;
	const unsigned int w = GUI.Width, h = GUI.Height;
	for (i = 0; i < sizeof(touchbuttons)/sizeof(TouchButton); i++) {
		touchbuttons[i].x = (unsigned int)(touchbuttons[i].fx * w);
		touchbuttons[i].y = (unsigned int)(touchbuttons[i].fy * h);
		touchbuttons[i].x2 = (unsigned int)(touchbuttons[i].x + touchbuttons[i].fw * w);
		touchbuttons[i].y2 = (unsigned int)(touchbuttons[i].y + touchbuttons[i].fh * h);
	}
#endif
}

#if 0
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
#endif
