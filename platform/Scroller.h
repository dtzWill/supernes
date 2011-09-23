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
#include <cassert>

class Scroller
{
public:
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
  RenderInfo RI;
  int text_height;

  // Internal state
  float offset;       // [0,1], 'percent' scrolled
  Uint32 last_update; // Time of last update
  float accel;        // Scroll acceleration

  typedef struct {
    char * name;
    SDL_Surface * surface;
  } rom_cache_element;
  typedef std::list<rom_cache_element> rom_cache_t;
  static const int CACHE_SIZE;
  static rom_cache_t rom_cache;

  SDL_Surface * buffer;

  public:
    Scroller(char ** n, int c, RenderInfo R) :
      names(n), count(c), RI(R),
      offset(0.0f), last_update(0) { init(); }

    // Draw ourselves to the specified surface at the specified offsets.
    void drawToSurface(SDL_Surface *s, int x, int y);

    // Update ourselves based on elapsed time
    void update();

    // Handle the given event, taking the given mouse offsets into account
    // Returns the index of the item selected, if any (-1 if not).
    int event(SDL_Event *e, int x, int y);

    ~Scroller();
  private:
    SDL_Surface * cacheLookup(const char * text);
    void init();
};

#endif // _SCROLLER_H_
