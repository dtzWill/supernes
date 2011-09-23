/*
 * ===========================================================================
 *
 *       Filename:  ApplySurface.h
 *
 *    Description:  Apply Surface utility function
 *
 *        Version:  1.0
 *        Created:  09/23/2011 12:34:39 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _APPLY_SURFACE_
#define _APPLY_SURFACE_

static void apply_surface( int x, int y, int w, SDL_Surface* source, SDL_Surface* destination )
{
  //Holds offsets
  SDL_Rect offset;

  //Source rect
  SDL_Rect src;

  //Get offsets
  offset.x = x;
  offset.y = y;

  src.x = 0;
  src.y = 0;
  src.w = w;
  src.h = source->h;

  //Blit
  SDL_BlitSurface( source, &src, destination, &offset );
}

static void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
  apply_surface( x, y, source->w, source, destination );
}

#endif // _APPLY_SURFACE_
