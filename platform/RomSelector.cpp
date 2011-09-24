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
#include "RomSelectorUtil.h"
#include "GLUtil.h"
#include "OptionMenu.h"
#include "ApplySurface.h"
#include "Scroller.h"
#include "pdl.h"
#include <assert.h>

int rom_selector_event_handler( const SDL_Event * event );

typedef struct
{
  const char * msg;
  SDL_Color color;
} line;
static line no_roms[] = {
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

static int romSelected;
static int selector_w;
static SDL_Rect drawRect;
static int filecount;
static int top, bottom;
char ** filenames;


#define min(a,b) (((a) < (b)) ? (a) : (b))

TTF_Font * font_small = NULL;
TTF_Font * font_normal = NULL;
TTF_Font * font_large = NULL;

// TODO: These aren't tied to anything anymore!!
static int scroll_offset = 0;
static float scroll_offset_actual = 0.0f;

// Display rom selector GUI, and return a string indicating the selection.
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

  font_small = TTF_OpenFont( FONT, 18 );
  font_normal = TTF_OpenFont( FONT, 24 );
  font_large = TTF_OpenFont( FONT, 30 ); // FOR TESTING
  if ( !font_small || !font_normal || !font_large )
  {
    fprintf( stderr, "Failed to open font: %s\n", FONT );
    exit( 1 );
  }

  ensureRomPathExists();

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

  romSelected = -1;
  selector_w = selector->w;

  Scroller::RenderInfo RI = {
    selector->w - 20,
    bottom - top,
    textColor,
    font_large
  };
  Scroller * scroll = new Scroller(filenames, filecount, RI);

  while( romSelected == -1 )
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (!rom_selector_event_handler(&e))
      {
        romSelected = scroll->event(&e, 20, top);
      }
    }

    //Draw border/text
    SDL_FillRect( selector, NULL, borderColor );
    apply_surface( selector->w - author->w - 10, selector->h - author->h - 10, author, selector );
    apply_surface( 20, selector->h - options->h - 10, options, selector );
    apply_surface( 10, 10, title, selector );

    scroll->update();
    scroll->drawToSurface(selector, 20, top);

    //Update screen.
    SDL_DrawSurfaceAsGLTexture( selector, portrait_vertexCoords );
  }
  SDL_Delay(100);
  SDL_FreeSurface( title );
  SDL_FreeSurface( author );
  SDL_FreeSurface( options );
  SDL_FreeSurface( selector );

  char * rom_base = roms[romSelected]->d_name;
  char * rom_full_path = (char *)malloc( strlen( ROM_PATH ) + strlen( rom_base ) + 2 );
  strcpy( rom_full_path, ROM_PATH );
  rom_full_path[strlen(ROM_PATH)] = '/';
  strcpy( rom_full_path + strlen( ROM_PATH ) + 1, rom_base );


  // CLEANUP :)

  delete scroll;

  //Free all the titles of the ROMs!
  for (int i = 0; i < filecount; ++i)
  {
    free(filenames[i]);
  }
  filenames = NULL;

  // Done with the fonts...
  TTF_CloseFont( font_small );
  TTF_CloseFont( font_normal );
  TTF_CloseFont( font_large );
  font_small = font_normal = font_large = NULL;

  // Now free the scandir entries..
  while(filecount--)
    free(roms[filecount]);
  free(roms);

  return rom_full_path;
}

// Returns iff event was handled
int rom_selector_event_handler( const SDL_Event * event )
{
  switch( event->type )
  {
    case SDL_MOUSEBUTTONDOWN:
      if ( event->button.y > bottom && event->button.x < selector_w / 2 )
      {
        optionsMenu();
      }
    default:
      return false;
  }
  return true;
}
