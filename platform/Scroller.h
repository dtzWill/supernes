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

#include "GLUtil.h"

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
  float text_height;

  // Scrolling state
  float offset;       // [0,1], 'percent' scrolled
  Uint32 last_update; // Time of last update
  float vel;          // Scroll velocity

  // Array of the underlying textures used in the scroller
  int tex_count;
  GLLayer *scroll_tex;
  float * vertexCoords;

  // Event state
  bool e_tap;
  bool e_down;
  int e_mouse_x, e_mouse_y;
  typedef struct {
    int x;
    int y;
    Uint32 time;
  } pt_event_t;
  std::list<pt_event_t> pt_history;

  public:
    Scroller(char ** n, int c, RenderInfo R) :
      names(n), count(c), RI(R),
      offset(0.0f), last_update(0),
      vel(0.0f)
    { init(); }

    // Render! Does not flush
    void draw(int x, int y);

    // Update ourselves based on elapsed time
    void update();

    // Handle the given event, taking the given mouse offsets into account
    // Returns the index of the item selected, if any (-1 if not).
    int event(SDL_Event *e, int x, int y);

    ~Scroller();
  private:
    void init();
    void recordPtEvent(int x, int y);

};

#endif // _SCROLLER_H_
