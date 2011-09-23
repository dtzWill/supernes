/*
 * ===========================================================================
 *
 *       Filename:  Keyboard.cpp
 *
 *    Description:  Keybindings, etc
 *
 *        Version:  1.0
 *        Created:  08/24/2010 09:01:13 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "Keyboard.h"

#include "snes9x.h"
#include <SDL.h>
#include "display.h"
#include "Options.h"
#include "Types.h"

#define BINDING_CFG "controller.cfg"

//Note these next arrays have to match the SNES_KEY enum

//User-friendly names while walking them through the config process.
const char * bindingNames[]= 
{
    "Press key for Up",
    "Press key for Down",
    "Press key for Left",
    "Press key for Right",
    "Press key for Start",
    "Press key for Select",
    "Press key for L",
    "Press key for R",
    "Press key for Y",
    "Press key for X",
    "Press key for B",
    "Press key for A",
    "Press key for Turbo",
    "Done binding keys"
};

//Define how/what gets written to file...
game_option controllerOptions[] =
{
  { "Joy0_Up",     &Config.joypad1Mapping[0]  },
  { "Joy0_Down",   &Config.joypad1Mapping[1]  },
  { "Joy0_Left",   &Config.joypad1Mapping[2]  },
  { "Joy0_Right",  &Config.joypad1Mapping[3]  },
  { "Joy0_Start",  &Config.joypad1Mapping[4]  },
  { "Joy0_Select", &Config.joypad1Mapping[5]  },
  { "Joy0_L",      &Config.joypad1Mapping[6]  },
  { "Joy0_R",      &Config.joypad1Mapping[7]  },
  { "Joy0_Y",      &Config.joypad1Mapping[8]  },
  { "Joy0_X",      &Config.joypad1Mapping[9]  },
  { "Joy0_B",      &Config.joypad1Mapping[10] },
  { "Joy0_A",      &Config.joypad1Mapping[11] },
  { "Joy0_Turbo",  &Config.joypad1Mapping[12] }
};
#define NOT_BINDING -1
//One more than the max SNES_KEY enum val
#define BINDING_DONE ( SNES_KEY_TURBO + 1 )
static int keyBindingMode = NOT_BINDING;

int bindingJoypad[13];


//Default keybindings...
void default_keymappings(struct config * C)
{
  //Action keys
  Config.joypad1Mapping[SNES_KEY_Y] = SDLK_k;
  Config.joypad1Mapping[SNES_KEY_X] = SDLK_l;
  Config.joypad1Mapping[SNES_KEY_B] = SDLK_m;
  Config.joypad1Mapping[SNES_KEY_A] = SDLK_COMMA;

  //Triggers
  Config.joypad1Mapping[SNES_KEY_L] = SDLK_i;
  Config.joypad1Mapping[SNES_KEY_R] = SDLK_o;

  //Start/select
  Config.joypad1Mapping[SNES_KEY_START]  = SDLK_BACKSPACE;
  Config.joypad1Mapping[SNES_KEY_SELECT] = SDLK_RETURN;

  //D-pad
  Config.joypad1Mapping[SNES_KEY_UP]    = SDLK_e;
  Config.joypad1Mapping[SNES_KEY_DOWN]  = SDLK_s;
  Config.joypad1Mapping[SNES_KEY_LEFT]  = SDLK_w;
  Config.joypad1Mapping[SNES_KEY_RIGHT] = SDLK_d;

  //Turbo
  Config.joypad1Mapping[SNES_KEY_TURBO] = SDLK_SPACE;

  //Back gesture--'quit'
  Config.action[SDLK_ESCAPE] = kActionMenu;

  //Save states
  Config.action[SDLK_1] = kActionQuickSave1;
  Config.action[SDLK_2] = kActionQuickSave2;
  Config.action[SDLK_3] = kActionQuickSave3;

  //Load states
  Config.action[SDLK_4] = kActionQuickLoad1;
  Config.action[SDLK_5] = kActionQuickLoad2;
  Config.action[SDLK_6] = kActionQuickLoad3;

}

void initialize_keymappings(struct config *C)
{
  //Load default keymappings first.
  default_keymappings( C );
  //Override anything the user has set.
  readOptions( BINDING_CFG, controllerOptions,
      sizeof(controllerOptions)/sizeof(controllerOptions[0]), true );
}

void updateBindingMessage()
{
  if ( keyBindingMode != NOT_BINDING )
  {
    //Hack. Should just have to reset the timer...
    S9xSetInfoString( bindingNames[keyBindingMode] );
  }
}

bool keyboardBindingFilter( const SDL_Event& event )
{
  if (event.type != SDL_KEYDOWN ) return false;

  int key = event.key.keysym.sym;
  bool bindingKeyPressed = ( key == SDLK_EQUALS || key == SDLK_QUESTION );

  if ( keyBindingMode == NOT_BINDING )
  {
    if ( bindingKeyPressed )
    {
      //Enter key-binding mode.
      keyBindingMode = NOT_BINDING;
      keyBindingMode++;
      return true;
    } else
    {
      //We're not presently binding, and they're not trying to bind.
      //Carry on!
      return false;
    }
  }

  //We're in key-binding mode...
  if ( bindingKeyPressed )
  {
    //cancel;
    keyBindingMode = NOT_BINDING;
    S9xSetInfoString( "Cancelled binding!" );
  }

  //Check that this is a valid key.
  //XXX: right now we don't support
  //orange, shift, or sym as keys b/c they are meta keys.
  int valid = 0
    || ( key >= SDLK_a && key <= SDLK_z ) //Alpha
    || key == SDLK_BACKSPACE
    || key == SDLK_RETURN
    || key == SDLK_COMMA
    || key == SDLK_PERIOD
    || key == SDLK_SPACE
    || key == SDLK_AT;

  if ( valid )
  {
    //We're in binding mode, and they pressed a bindable key!
    bindingJoypad[keyBindingMode] = key;
    keyBindingMode++;
  }

  if ( keyBindingMode == BINDING_DONE )
  {
    //make this the current joy
    memcpy( Config.joypad1Mapping, bindingJoypad, sizeof(bindingJoypad) );
    
    //Save their settings...
    writeOptions( BINDING_CFG, controllerOptions,
        sizeof(controllerOptions)/sizeof(controllerOptions[0]), true );

    //Tell user we're done...
    updateBindingMessage();

    //we're done here!
    keyBindingMode = NOT_BINDING;
  }

  return true;
}

