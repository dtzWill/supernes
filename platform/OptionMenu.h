/*
 * ===========================================================================
 *
 *       Filename:  OptionMenu.h
 *
 *    Description:  Options menu
 *
 *        Version:  1.0
 *        Created:  08/12/2010 08:53:10 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _OPTIONS_MENU_H_
#define _OPTIONS_MENU_H_

#include <SDL.h>

typedef enum menuResponse
{
  MENU_RESPONSE_RESUME,
  MENU_RESPONSE_ROMSELECTOR
} eMenuResponse;

eMenuResponse optionsMenu();

//Show help screen!
void doHelpExternal( SDL_Surface * s );

#endif //_OPTIONS_MENU_H_
