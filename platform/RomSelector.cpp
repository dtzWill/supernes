/*
 * ===========================================================================
 *
 *       Filename:  RomSelector.cpp
 *
 *    Description:  Rom selection code
 *
 *        Version:  1.0
 *        Created:  08/12/2010 05:02:53 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "snes9x.h"
#include "RomSelector.h"
#include "GLUtil.h"
#include "OptionMenu.h"
#include "pdl.h"
#include <SDL_ttf.h>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>

// We render the name of each rom, but instead of doing them all at once
// (initially slow, and memory-consuming), we render just what is needed.
// However that's lame, so we use a cache for a happy middle ground.
#define CACHE_SIZE 30

#define SLOW_FACTOR 0.8f
#define MIN_SCROLL_SPEED 5.0f
#define SCROLL_DELAY 100

#define FRAME_INTERVAL 75

char * strip_rom_name( char * rom_name );
SDL_Surface * getSurfaceFor( char * filename );
int rom_selector_event_handler( const SDL_Event * event );

static SDL_Color textColor = { 255, 255, 255 };
static SDL_Color hiColor = { 255, 200, 200 };
static SDL_Color linkColor = { 200, 200, 255 };

typedef struct
{
  char * msg;
  SDL_Color color;
} line;
static line no_roms[] {
{ "Welcome to SuperNES!",                textColor},
{ "Looks like you don't have any ROMs.", textColor},
{ "To play games, put the roms in ",     textColor},
{ "/snes/roms",                          hiColor},
{ "using USB mode",                      textColor},
{ "(make the directory if needed)",      textColor},
{ "and then launch SuperNES again",      textColor},
{ "For more information, see the help",  textColor},
{ "(click here to launch help)",         linkColor}
};

static bool tap;
static bool down;
static bool autoscrolling;
static bool on_scrollbar;
static int romSelected;
static int rom_height;
static int num_roms_display;
static float scroll_speed;
static u32 scroll_time;
static int scroll_mouse_y;
static int selector_w;
static SDL_Rect drawRect;
static int filecount;
static int top, bottom;
char ** filenames;

int romFilter( const struct dirent * file )
{
    const char * curPtr = file->d_name;
    const char * extPtr = NULL;
    //Don't show 'hidden' files (that start with a '.')
    if ( *curPtr == '.' )
    {
        return false;
    }

    //Find the last period
    while ( *curPtr )
    {
        if( *curPtr == '.' )
        {
            extPtr = curPtr;
        }
        curPtr++;
    }
    if ( !extPtr )
    {
        //No extension, not allowed.
        return 0;
    }
    //We don't want the period...
    extPtr++;

    return !(
            strcasecmp( extPtr, "smc" ) &&
            strcasecmp( extPtr, "zip" ) );
}

void apply_surface( int x, int y, int w, SDL_Surface* source, SDL_Surface* destination )
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

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
    apply_surface( x, y, source->w, source, destination );
}

//XXX: Figure out if there isn't something we can #ifdef for these
//autoconf maybe?
int sortComparD( const struct dirent ** a, const struct dirent ** b )
{
    return strcasecmp( (*a)->d_name, (*b)->d_name );
}

int sortCompar( const void * a, const void * b )
{
    return sortComparD( (const struct dirent **)a, (const struct dirent**)b );
}

#define min(a,b) (((a) < (b)) ? (a) : (b))

static TTF_Font * font_small = NULL;
static TTF_Font * font_normal = NULL;
static TTF_Font * font_large = NULL;

static int scroll_offset = 0;
static float scroll_offset_actual = 0.0f;

char * romSelector()
{
    //Portrait orientation
    PDL_SetOrientation( PDL_ORIENTATION_BOTTOM );

    // Create buffer we render selector into
    SDL_Surface * surface = SDL_GetVideoSurface();
    SDL_Surface * selector = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, 24, 
      0x0000ff, 0x00ff00, 0xff0000, 0);

    if (!selector )
    {
        fprintf( stderr, "Error creating rom selector buffer!\n" );
        exit( 1 );
    }

    font_small = TTF_OpenFont( FONT, 14 );
    font_normal = TTF_OpenFont( FONT, 18 );
    font_large = TTF_OpenFont( FONT, 22 );
    if ( !font_small || !font_normal || !font_large )
    {
        fprintf( stderr, "Failed to open font: %s\n", FONT );
        exit( 1 );
    }

    //Don't bail here, we can't write to /media/internal on 1.4.5
#define FSTAB_BUG 1
    
    //Make sure rom dir exists
    //XXX: This assumes /media/internal (parent directory) already exists
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;
    int result = mkdir( SNES_HOME, mode );
#ifndef FSTAB_BUG
    if ( result && ( errno != EEXIST ) )
    {
        fprintf( stderr, "Error creating directory %s!\n", SNES_HOME );
        exit( 1 );
    }
#endif
    result = mkdir( ROM_PATH, mode );
#ifndef FSTAB_BUG
    if ( result && ( errno != EEXIST ) )
    {
        fprintf( stderr, "Error creating directory %s for roms!\n", ROM_PATH );
        exit( 1 );
    }
#endif

    struct dirent ** roms;
    filecount = scandir( ROM_PATH, &roms, romFilter, sortComparD );
    printf( "Rom count: %d\n", filecount );

    //Display general information
    int borderColor = SDL_MapRGB( selector->format, 0, 0, 85 );
    SDL_Surface * title = TTF_RenderText_Blended( font_normal, TITLE, textColor );
    top = 10+title->h+10;

    SDL_Surface * author = TTF_RenderText_Blended( font_small, AUTHOR_TAG, textColor );
    SDL_Surface * options = TTF_RenderText_Blended( font_large, OPTIONS_TEXT, linkColor );
    bottom = min(selector->h - author->h - 10, selector->h - options->h - 20);

    //Draw border/text
    SDL_FillRect( selector, NULL, borderColor );
    apply_surface( selector->w - author->w - 10, selector->h - author->h - 10, author, selector );
    apply_surface( 10, 10, title, selector );

    SDL_DrawSurfaceAsGLTexture( selector, portrait_vertexCoords );
    drawRect.x = 10;
    drawRect.y = top;
    drawRect.h = bottom-top;
    drawRect.w = selector->w-20;
    int black = SDL_MapRGB(selector->format, 0, 0, 0);
    SDL_FillRect(selector, &drawRect, black);

    if ( filecount < 1 )
    {
        //No roms found! Tell the user with a nice screen.
        //(Note this is where first-time users most likely end up);
        int lines = sizeof(no_roms)/sizeof(no_roms[0]);
        SDL_Surface * nr[lines];
        for ( int i = 0; i < lines; ++i )
        {
          nr[i] = TTF_RenderText_Blended( font_normal, no_roms[i].msg, no_roms[i].color );
        }

        SDL_Event event;
        while (1)
        {
            SDL_FillRect( selector, NULL, borderColor );
            apply_surface( selector->w - author->w - 10, selector->h - author->h - 10, author, selector );
            apply_surface( 10, 10, title, selector );
            SDL_FillRect(selector, &drawRect, black);
            int offset = 100;//arbitrary offset, centering all this isn't worth it.
            for ( int i = 0; i < lines; ++i )
            {
                apply_surface( selector->w/2-nr[i]->w/2, offset, nr[i], selector );
                offset += nr[i]->h + 10;
            }

            SDL_DrawSurfaceAsGLTexture( selector, portrait_vertexCoords );
            while ( SDL_PollEvent( &event ) )
            {
                if ( event.type == SDL_MOUSEBUTTONUP )
                {
                  //Regardless of what the text says, if the user clicks, launch the help...
                  //PDL_LaunchBrowser( VBA_WIKI );
                  doHelpExternal( selector );
                }

            }
            SDL_Delay( 100 );
        }
    }

    // Convert the rom names to something we can display
    char * filenames_internal[filecount];
    filenames = filenames_internal;
    for ( int i = 0; i < filecount; ++i )
    {
      filenames[i] = strip_rom_name(roms[i]->d_name);
    }

    SDL_EnableUNICODE( 1 );


    // Initialize are unnecessarily long set of globals...
    tap = false;
    down = false;
    autoscrolling = false;
    on_scrollbar = false;
    romSelected = -1;
    rom_height = getSurfaceFor(filenames[0])->h;
    num_roms_display = ( bottom - top + 10 ) / ( rom_height + 10 );
    scroll_speed = 0.0f;
    scroll_mouse_y = 0;
    selector_w = selector->w;

    while( romSelected == -1 )
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
          rom_selector_event_handler(&e);
        }

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
              scroll_speed > MIN_SCROLL_SPEED )
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

        //Update screen.
        SDL_DrawSurfaceAsGLTexture( selector, portrait_vertexCoords );
    }
    SDL_Delay(100);
    SDL_FreeSurface( title );
    SDL_FreeSurface( author );
    SDL_FreeSurface( options );
    SDL_FreeSurface( selector );

    //Free all the titles of the ROMs!
    for (int i = 0; i < filecount; ++i)
    {
      free(filenames[i]);
    }
    filenames = NULL;

    TTF_CloseFont( font_small );
    TTF_CloseFont( font_normal );
    TTF_CloseFont( font_large );

    char * rom_base = roms[romSelected]->d_name;
    char * rom_full_path = (char *)malloc( strlen( ROM_PATH ) + strlen( rom_base ) + 2 );
    strcpy( rom_full_path, ROM_PATH );
    rom_full_path[strlen(ROM_PATH)] = '/';
    strcpy( rom_full_path + strlen( ROM_PATH ) + 1, rom_base );
    return rom_full_path;
}

typedef struct {
  char * name;
  SDL_Surface * surface;
} rom_cache_element;
typedef std::list<rom_cache_element> rom_cache_t;
static rom_cache_t rom_cache;

SDL_Surface * getSurfaceFor( char * filename )
{
    // First, check cache.
    // If we already have a surface, use that and update it in the 'LRU' policy.

    rom_cache_t::iterator I = rom_cache.begin(),
                          E = rom_cache.end();
    for ( ; I != E; ++I )
    {
      if ( !strcmp(I->name, filename ) )
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
    e.name = strdup(filename);
    e.surface = TTF_RenderText_Blended( font_large, filename, textColor );

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

char * strip_rom_name( char * rom_name )
{
  //Here we remove everything in '()'s or '[]'s
  //which is usually annoying release information, etc
  char buffer[100];
  char * src = rom_name;
  char * dst = buffer;
  int inParen = 0;
  while ( *src && dst < buffer+sizeof(buffer) - 1 )
  {
    char c = *src;
    if ( c == '(' || c == '[' )
    {
      inParen++;
    }
    if ( !inParen )
    {
      *dst++ = *src;
    }
    if ( c == ')' || c == ']' )
    {
      inParen--;
    }

    src++;
  }
  *dst = '\0';

  //now remove the extension..
  char * extPtr = NULL;
  dst = buffer;
  while ( *dst )
  {
    if( *dst == '.' )
    {
      extPtr = dst;
    }
    dst++;
  }
  //If we found an extension, end the string at that period
  if ( extPtr )
  {
    *extPtr = '\0';
  }

  return strdup(buffer);
}

int rom_selector_event_handler( const SDL_Event * event )
{
  switch( event->type )
  {
    case SDL_MOUSEBUTTONDOWN:
      down = tap = true;
      autoscrolling = false;
      on_scrollbar = ( event->button.x >= selector_w - 50 );
      break;
    case SDL_MOUSEBUTTONUP:
      down = false;
      if ( tap )
      {
        if ( on_scrollbar )
        {
          int diff = event->button.y - drawRect.y;
          float percent =  (float)diff/drawRect.h;
          scroll_offset = (filecount - num_roms_display)*percent;
          if ( scroll_offset > filecount - num_roms_display )
            scroll_offset = filecount - num_roms_display;
          if ( scroll_offset < 0 ) scroll_offset = 0;
          scroll_offset_actual = scroll_offset;
          autoscrolling = false;
        }
        else if ( event->button.y >= top && event->button.y <= bottom )
        {
          //Calculate which rom this would be, and verify that makes sense
          int rom_index = ( event->button.y - top ) / ( rom_height + 10 );
          if ( rom_index >= 0 && rom_index < num_roms_display &&
              rom_index + scroll_offset < filecount )
          {
            romSelected = rom_index+scroll_offset;
          }
        }
        else if ( event->button.y > bottom && event->button.x < selector_w / 2 )
        {
          optionsMenu();
        }
      }
      if ( SDL_GetTicks() - scroll_time <= SCROLL_DELAY )
      {
        autoscrolling = true;
      }

      break;
    case SDL_MOUSEMOTION:
      if ( down )
      {
        //If the mouse moves before going up, it's not a tap
        tap = false;

        // If ( user is scrolling on scrollbar...)
        if ( on_scrollbar )
        {
          int diff = event->motion.y - drawRect.y;
          float percent =  (float)diff/drawRect.h;
          scroll_offset = (filecount - num_roms_display)*percent;
          if ( scroll_offset > filecount - num_roms_display )
            scroll_offset = filecount - num_roms_display;
          if ( scroll_offset < 0 ) scroll_offset = 0;
          scroll_offset_actual = scroll_offset;
          autoscrolling = false;
        }
        else
        {
          scroll_speed = ((float)-event->motion.yrel)/(float)rom_height;
          scroll_time = SDL_GetTicks();

          // Do the scroll
          float diff = ((float)-event->motion.yrel)/(float)rom_height;
          scroll_offset_actual += diff;
          scroll_offset = (int)scroll_offset_actual;

          // Make sure still in-bounds
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
        }
        
      }

      break;
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
    default:
      return true;
  }

  return false;
}

