/*
 * ===========================================================================
 *
 *       Filename:  RomSelector.h
 *
 *    Description:  Rom selection code
 *
 *        Version:  1.0
 *        Created:  08/12/2010 05:17:04 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _ROM_SELECTOR_H_
#define _ROM_SELECTOR_H_

#include <SDL.h>

char * romSelector();
//FIXME: This doesn't really belong public from 'RomSelector'
void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination );

#endif //_ROM_SELECTOR_H_
