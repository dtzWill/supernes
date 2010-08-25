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

void initialize_keymappings(struct config * C)
{
  //Action keys
  Config.joypad1Mapping[SDLK_k] =     SNES_X_MASK;
  Config.joypad1Mapping[SDLK_l] =     SNES_Y_MASK;
  Config.joypad1Mapping[SDLK_m] =     SNES_B_MASK;
  Config.joypad1Mapping[SDLK_COMMA] = SNES_A_MASK;

  //Triggers
  Config.joypad1Mapping[SDLK_m] = SNES_B_MASK;
  Config.joypad1Mapping[SDLK_m] = SNES_B_MASK;

  //Start/select
  Config.joypad1Mapping[SDLK_BACKSPACE] = SNES_START_MASK;
  Config.joypad1Mapping[SDLK_RETURN] =    SNES_SELECT_MASK;

  //D-pad
  Config.joypad1Mapping[SDLK_e] = SNES_UP_MASK;
  Config.joypad1Mapping[SDLK_s] = SNES_DOWN_MASK;
  Config.joypad1Mapping[SDLK_w] = SNES_LEFT_MASK;
  Config.joypad1Mapping[SDLK_d] = SNES_RIGHT_MASK;

  //Back gesture--'quit'
  Config.action[SDLK_ESCAPE] = kActionQuit;

}

