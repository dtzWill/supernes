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
#include "RegionTimer.h"

#include <snes9x.h>

// If finger moves less than this, it's still considered a tap
static const int TAP_TOLERANCE = 15;

static const int POINT_HISTORY_SIZE = 10;

static const float MIN_VEL = 0.5f;
static const float MAX_VEL = 3.0f;

static const float SCROLL_ACCEL = 1.0f;

#define min(a,b) (((a) < (b)) ? (a) : (b))

void Scroller::init()
{
  // Light sanity checking
  assert(names);
  assert(count > 0);

  SDL_Surface * current =
    TTF_RenderText_Blended(RI.textFont, names[0], RI.textColor);
  text_height = current->h + 10;
  SDL_FreeSurface(current);

  // Render all the items into a giant texture. weeeeeeeee
  int total_height = count * text_height;

  SDL_Surface * scroll =
    SDL_CreateRGBSurface(SDL_SWSURFACE, RI.width, total_height,
        24, 0x0000ff, 0x00ff00, 0xff0000, 0);
  assert(scroll);

  unsigned i = 0;

  // Make it all black.
  int black = SDL_MapRGB(scroll->format, 0, 0, 0);
  SDL_FillRect(scroll, NULL, black);

  // Render each internal item
  for(unsigned i = 0; i < count; ++i)
  {
    SDL_Surface * current =
      TTF_RenderText_Blended(RI.textFont, names[i], RI.textColor);
    apply_surface(5, i*text_height, RI.width - 10,
        current, scroll);
    SDL_FreeSurface(current);
  }

  // For now, pretend these coords are right.
  // They are not, as they're fullscreen.
  full_scroll = GL_SurfaceToTexture(scroll, vertexCoords);

  full_scroll.textureCoords = texCoords;

  update();
}

GLLayer Scroller::getGLLayer(int x, int y)
{
  // Update texture coordinates to reflect
  // current scroll status:
  // texCoords:
  // (UL), (UR), (LL), (LR)
  {
    float total_height = count * text_height;

    float width = 1.0f; // full texture

    float height = ((float)RI.height) / total_height;

    float y_offset = offset * (1.0f - height);

    // UL
    texCoords[0] = 0.0f;
    texCoords[1] = y_offset;
    // LL
    texCoords[2] = 0.0f;
    texCoords[3] = y_offset + height;
    // UR
    texCoords[4] = width;
    texCoords[5] = y_offset;
    // LR
    texCoords[6] = width;
    texCoords[7] = y_offset + height;
  }

  // Update our vertex coordinates.  This should really be done by GLUtil...
  // And done only once...
  {
    x = NATIVE_RES_WIDTH - (RI.width + x); // measured from wrong side
    float scaled_x = ((float)x)/NATIVE_RES_WIDTH;
    float scaled_y = ((float)y)/NATIVE_RES_HEIGHT;

    float scaled_width = ((float)RI.width)/NATIVE_RES_WIDTH;
    float scaled_height = ((float)RI.height)/NATIVE_RES_HEIGHT;

    memcpy(vertexCoords, portrait_vertexCoords, sizeof(vertexCoords));

    for (unsigned i = 0; i < 4; ++i)
    {
      vertexCoords[2*i] *= scaled_width;
      vertexCoords[2*i+1] *= scaled_height;
    }

    float x_offset = (1.0 - scaled_width) - scaled_x * 2;
    float y_offset = (1.0 - scaled_height) - scaled_y * 2;

    for (unsigned i = 0; i < 4; ++i)
    {
      vertexCoords[2*i] += x_offset;
      vertexCoords[2*i+1] += y_offset;
    }

  }

  return full_scroll;
}

// TODO: Keeping code around for now, REMOVE
#if 0
void Scroller::drawToSurface(SDL_Surface *s, int x, int y)
{
  RegionTimer T("drawToSurface");

  // Fill background
  // TODO: make the background set externally?
  {
    RegionTimer T("fillrect");
    int black = SDL_MapRGB(s->format, 0, 0, 0);
    SDL_Rect drawRect = {x, y, RI.width, RI.height};
    SDL_FillRect(s, &drawRect, black);
  }

  float total_height = count * text_height;
  float y_offset = 0.0f;
  if (total_height > RI.height)
    y_offset = offset * (total_height - (float)RI.height);

  // Now render the items
  {
    RegionTimer T("loop");
    for(int cur = 0; cur < RI.height; )
    {
      int index = (cur + y_offset) / total_height * count;
      float actual_text_y = ((float)index / (float)count) * total_height;
      int text_y = actual_text_y - y_offset;
      cur = text_y + text_height + 1;

      if (index >= 0 && index < count)
      {
        // Render it
        SDL_Surface * txt = cacheLookup(index);
        SDL_Rect srcRect = {0, 0, txt->w, txt->h };
        SDL_Rect destRect = { x + 5, y + text_y, RI.width - 5, text_height };
        if (text_y < 0)
        {
          destRect.y = y;
          srcRect.y = -text_y;
          srcRect.h += text_y;
        } else if (text_y + txt->h > RI.height)
        {
          destRect.h -= text_y + txt->h - RI.height;
          srcRect.h -= text_y + txt->h - RI.height;
        }
        SDL_BlitSurface( txt, &srcRect, s, &destRect);
      }
    }
  }

  // Scrollbar!
  int barColor = SDL_MapRGB(s->format, 200, 200, 255);
  int tabColor = SDL_MapRGB(s->format, 255, 255, 255);
  SDL_Rect scrollRect = { x + RI.width - 10, y, 10, RI.height };
  SDL_FillRect(s, &scrollRect, barColor);
  SDL_Rect scrollTab;
  scrollTab.w = scrollTab.h = 20;
  scrollTab.x = scrollRect.x + scrollRect.w/2 - 10;
  scrollTab.y = scrollRect.y;
  scrollTab.y += ((float)(scrollRect.h - scrollTab.h))*offset;
  SDL_FillRect(s, &scrollTab, tabColor);
}
#endif
void Scroller::update()
{
  if (vel < MIN_VEL && vel > -MIN_VEL) {
    vel = 0.0f;
    return;
  }

  int time = SDL_GetTicks();
  float dt = (time - last_update);
  last_update = time;

  float adjust = ((float)1) / (float)count;

  // Clamp velocty to some multiple of screen's worth of listing.
  float dx = vel * dt;
  float max_speed = (float)RI.height / text_height * 1.5f;
  if (dx > max_speed) dx = max_speed;
  if (dx < -max_speed) dx = -max_speed;

  // Update offset, and bound check
  offset += dx * adjust;
  if (offset <= 0.0f ) {
    offset = 0.0f;
    vel = 0.0f;
  } else if (offset >= 1.0f ) {
    offset = 1.0f;
    vel = 0.0f;
  } else {
    // Update velocity
    if (vel > 0.0f) {
      vel -= SCROLL_ACCEL * dt * adjust;
      if (vel < 0.0f) vel = 0.0f;
    } else {
      vel += SCROLL_ACCEL * dt * adjust;
      if (vel > 0.0f) vel = 0.0f;
    }
  }

}

int Scroller::event(SDL_Event *e, int x_offset, int y_offset)
{
  // TODO: Refactor out the event conversion state machine?
  switch(e->type)
  {
    case SDL_MOUSEBUTTONDOWN:
      // Update event state
      e_down = e_tap = true;
      e_mouse_x = e->button.x;
      e_mouse_y = e->button.y;

      // When there's a tap, set scroll speed immediately to zero.
      vel = 0;
      pt_history.clear();
      recordPtEvent(e_mouse_x, e_mouse_y);
      break;
    case SDL_MOUSEMOTION:
    {
      // Update event state
      // This is intentionally the total distance traveled from original finger down
      // (Not from previous event)
      int delta_x = (e->motion.x - e_mouse_x);
      int delta_y = (e->motion.y - e_mouse_y);
      bool withinTapTolerance =
        delta_x*delta_x + delta_y*delta_y <= TAP_TOLERANCE * TAP_TOLERANCE;
      if ( e_down && !withinTapTolerance) e_tap = false;

      if (e_down && !e_tap)
      {
        // drag event

        // Move the text accordingly
        float dy = pt_history.back().y - e->motion.y;
        float effective_count = (float)count - (float)RI.height / text_height;
        float delta_items = dy / text_height;
        offset += delta_items / effective_count;
        if (offset > 1.0f ) offset = 1.0f;
        if (offset < 0.0f ) offset = 0.0f;
      }

      recordPtEvent(e->motion.x, e->motion.y);
      break;
    }
    case SDL_MOUSEBUTTONUP:
    {
      // Update event state
      bool was_tap = e_tap;
      e_down = e_tap = false;

      if (was_tap)
      {
        // Convert tap to index, validate it, return that value!
        float total_height = count * text_height;
        float scroll_offset = 0.0f;
        if (total_height > RI.height)
          scroll_offset = offset * (total_height - (float)RI.height);
        int index = ( (e->button.y - y_offset) + scroll_offset ) / total_height * count;

        if (index >= 0 && index < count) {
          // TODO: Highlight the item like we did before?
          return index;
        }
      } else {
        // Scroll event!
        if (pt_history.empty()) break; // really shouldn't happen

        pt_event_t p = pt_history.front();
        int time = SDL_GetTicks();
        if (time <= p.time + 1) break; // too fast

        float dy = e->button.y - p.y;
        float dt = time - p.time;

        vel = -dy/dt;

        if (vel > MAX_VEL) vel = MAX_VEL;
        if (vel < -MAX_VEL) vel = -MAX_VEL;

        last_update = time;
      }
      break;
    }
    case SDL_KEYDOWN:
    {
      //Filter based on letter presses.
      //For now, just jump to the first thing that starts at or after that letter.
      char c = (char)e->key.keysym.unicode;
      if ( 'A' <= c && c <= 'Z' )
        c -= 'A' - 'a';

      if ( 'a' <= c && c <= 'z' )
      {
        //find this letter in the roms...
        int index = 0;
        while( index < count )
        {
          char c_file = *names[index];
          if ( 'A' <= c_file && c_file <= 'Z' )
            c_file -= 'A' - 'a';
          if ( c_file >= c )
            break;
          index++;
        }
        // Didn't find it, scroll to bottom
        if (index > count)
        {
          offset = 1.0f;
          break;
        }

        // If we have less roms than fit on the screen,
        // offset is always zero.
        float total_height = count * text_height;
        if (total_height > RI.height)
        {
          offset = 0.0f;
          break;
        }

        // given index, get offset:
        offset = (index * total_height / (float)count) / (total_height - (float)RI.height);

        vel = 0; // stop scrolling
      }
      break;
    }
    default:
      break;
  }

  // TODO: Implement me!
  return -1;
}

void Scroller::recordPtEvent(int x, int y)
{
  pt_event_t p = { x, y, SDL_GetTicks() };
  pt_history.push_back(p);
  if (pt_history.size() > POINT_HISTORY_SIZE)
    pt_history.pop_front();
}

Scroller::~Scroller()
{
  GL_FreeLayer(full_scroll);
}

#if 0
    case SDL_KEYDOWN:
      {
        //Filter based on letter presses.
        //For now, just jump to the first thing that starts at or after that letter.
        char c = (char)event->key.keysym.unicode;
        if ( 'A' <= c && c <= 'Z' )
        {
          //lowercase...
          c -= ( 'A' - 'a' );
        }
        if ( 'a' <= c && c <= 'z' )
        {
          //find this letter in the roms...
          int offset = 0;
          while( offset < filecount )
          {
            char c_file = *filenames[offset];
            if ( 'A' <= c_file && c_file <= 'Z' )
            {
              //lowercase..
              c_file -= ( 'A' - 'a' );
            }
            if ( c_file >= c )
            {
              break;
            }
            offset++;
          }
          scroll_offset = offset;
          if ( scroll_offset > filecount - num_roms_display ) scroll_offset = filecount - num_roms_display;
          if ( scroll_offset < 0 ) scroll_offset = 0;
          scroll_offset_actual = scroll_offset;
          autoscrolling = false;
        }
      }
#endif

