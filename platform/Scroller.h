/*
 * ===========================================================================
 *
 *       Filename:  Scroller.h
 *
 *    Description:  Text scroller
 *
 *        Version:  1.0
 *        Created:  09/23/2011 12:40:41 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

#include <SDL.h>
#include <SDL_ttf.h>
#include <list>

class Scroller
{
public:
  static const int CACHE_SIZE;

  typedef struct {
    int width;
    int height;
    SDL_Color textColor;
    TTF_Font * textFont;
  } RenderInfo;
private:

  // Text to scroll
  char ** names;
  int count;

  // Rendering information
  RenderInfo renderInfo;

  // Internal state
  typedef struct {
    char * name;
    SDL_Surface * surface;
  } rom_cache_element;
  typedef std::list<rom_cache_element> rom_cache_t;
  rom_cache_t rom_cache;

  public:
    Scroller(char ** n, int c, RenderInfo RI) :
      names(n), count(c), renderInfo(RI) {};

    // Draw ourselves to the specified surface at the specified offsets.
    void drawToSurface(SDL_Surface *s, int x, int y);

    // Update ourselves based on elapsed time
    void update();

    // Handle the given event, taking the given mouse offsets into account
    // Returns the index of the item selected, if any (-1 if not).
    int event(SDL_Event *e, int x, int y);
  private:
    SDL_Surface * cacheLookup(const char * text);
};

#endif // _SCROLLER_H_
