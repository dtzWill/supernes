/*
 * ===========================================================================
 *
 *       Filename:  Scroller.cpp
 *
 *    Description:  Text scroller
 *
 *        Version:  1.0
 *        Created:  09/23/2011 12:47:12 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "Scroller.h"
#include <SDL_ttf.h>
#include "ApplySurface.h"

const int Scroller::CACHE_SIZE = 50;
Scroller::rom_cache_t Scroller::rom_cache;

#define min(a,b) (((a) < (b)) ? (a) : (b))

void Scroller::init()
{
  // Light sanity checking
  assert(names);
  assert(count > 0);

  text_height = cacheLookup(names[0])->h + 10;
}

void Scroller::drawToSurface(SDL_Surface *s, int x, int y)
{
  if (!buffer) {
    buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, RI.width, RI.height,
        s->format->BitsPerPixel,
        s->format->Rmask,
        s->format->Gmask,
        s->format->Bmask,
        s->format->Amask);
  }

  // Fill background
  // TODO: make the background set externally?
  int black = SDL_MapRGB(buffer->format, 0, 0, 0);
  SDL_FillRect(buffer, 0, black);

  float total_height = count * text_height;
  float y_offset = 0.0f;
  if (total_height > RI.height)
    y_offset = offset * (total_height - (float)RI.height);

  // Now render the items
  for(int cur = 0; cur < RI.height; cur += text_height)
  {
    int index = (cur + y_offset) / total_height * count;
    if (index >= 0 && index < count)
    {
      float actual_text_y = ((float)index / (float)count) * total_height;
      int text_y = actual_text_y - y_offset;
      apply_surface(0, text_y, RI.width,
          cacheLookup(names[index]), buffer);
    }
  }

  // Blit to the requrested surface
  apply_surface(x, y, buffer, s);
}

void Scroller::update()
{
  // TODO: Implement me!
}

int Scroller::event(SDL_Event *e, int x_offset, int y_offset)
{
  // TODO: Implement me!
  return -1;
}

Scroller::~Scroller()
{
  SDL_FreeSurface(buffer);
}

SDL_Surface * Scroller::cacheLookup( const char * text )
{
  // First, check cache.
  // If we already have a surface, use that and update it in the 'LRU' policy.

  rom_cache_t::iterator I = rom_cache.begin(),
    E = rom_cache.end();
  for ( ; I != E; ++I )
  {
    if ( !strcmp(I->name, text ) )
    {
      // Found it!
      rom_cache_element result = *I;

      // Move it to the front...
      rom_cache.erase(I);
      rom_cache.push_front(result);

      return result.surface;
    }
  }

  // Okay, so it's not in the cache.
  // Create the surface requested:
  rom_cache_element e;
  e.name = strdup(text);
  e.surface = TTF_RenderText_Blended( RI.textFont, text, RI.textColor );

  // Add to front
  rom_cache.push_front(e);

  // Is the cache too large as a result of adding this element?
  if ( rom_cache.size() > CACHE_SIZE )
  {
    rom_cache_element remove = rom_cache.back();
    rom_cache.pop_back();

    // Free memory for this item
    free(remove.name);
    SDL_FreeSurface(remove.surface);
  }

  // Return the surface we created!
  return e.surface;
}

#if 0
    if (autoscrolling)
    {
      scroll_offset_actual += scroll_speed;
      scroll_offset = (int)scroll_offset_actual;
      if ( scroll_offset > filecount - num_roms_display )
      {
        scroll_offset = filecount - num_roms_display;
        scroll_offset_actual = scroll_offset;
        autoscrolling = false;
      }
      if ( scroll_offset < 0 )
      {
        scroll_offset = 0;
        scroll_offset_actual = scroll_offset;
        autoscrolling = false;
      }

      scroll_speed *= SLOW_FACTOR;

      if ( scroll_speed < MIN_SCROLL_SPEED &&
          scroll_speed > -MIN_SCROLL_SPEED )
        autoscrolling = false;
    }

    if ( scroll_offset + num_roms_display > filecount )
    {
      num_roms_display = filecount - scroll_offset;
    }

    //Draw border/text
    SDL_FillRect( selector, NULL, borderColor );
    apply_surface( selector->w - author->w - 10, selector->h - author->h - 10, author, selector );
    apply_surface( 20, selector->h - options->h - 10, options, selector );
    apply_surface( 10, 10, title, selector );

    //Clear middle
    SDL_FillRect(selector, &drawRect, black);

    //Draw roms list
    for ( int i = 0; i < num_roms_display; i++ )
    {
      int index = scroll_offset + i;
      if ( index == romSelected )
      {
        int hiColor = SDL_MapRGB( selector->format, 128, 128, 0 );
        SDL_Rect hiRect;
        hiRect.x = 10;
        hiRect.y = top+(10+rom_height)*i - 5;
        hiRect.h = rom_height+5;
        hiRect.w = selector->w - 20;
        SDL_FillRect( selector, &hiRect, hiColor );
      }
      apply_surface( 20, top + (10+rom_height)*i, selector->w - 40, getSurfaceFor(filenames[index]), selector );
    }

    //Draw scrollbar :)
    int barColor = SDL_MapRGB(selector->format, 200, 200, 255);
    int tabColor = SDL_MapRGB(selector->format, 255, 255, 255);
    SDL_Rect scrollRect;
    scrollRect.x = drawRect.x + drawRect.w - 5;
    scrollRect.y = drawRect.y;
    scrollRect.h = drawRect.h;
    scrollRect.w = 10;
    SDL_FillRect(selector, &scrollRect, barColor);
    SDL_Rect scrollTab;
    scrollTab.w = scrollTab.h = 20;
    scrollTab.x = scrollRect.x + scrollRect.w/2 - 10;
    scrollTab.y = scrollRect.y;
    float percent = 0.0f;
    if ( filecount > num_roms_display )
      percent = ((float)scroll_offset)/((float)(filecount - num_roms_display));
    scrollTab.y += ((float)(scrollRect.h - scrollTab.h))*percent;
    SDL_FillRect(selector, &scrollTab, tabColor);
#endif
