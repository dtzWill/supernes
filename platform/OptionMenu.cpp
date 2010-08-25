/*
 * ===========================================================================
 *
 *       Filename:  OptionMenu.cpp
 *
 *    Description:  Options menu
 *       Subtitle:  Because using an OO language would be too easy.
 *
 *        Version:  1.0
 *        Created:  08/12/2010 08:53:33 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "OptionMenu.h"
#include "GLUtil.h"
#include "VBA.h"
#include "RomSelector.h"
#include "Options.h"
#include "Controller.h"
#include "pdl.h"
#include "resize++.h"

#include <SDL.h>
#include <SDL_ttf.h>

#define OPTION_SIZE 40
#define OPTION_SPACING 50
#define OPTION_WIDTH 300

//Toggle stuff
#define TOGGLE_TXT_X 10
#define TOGGLE_ON_X 160
#define TOGGLE_OFF_X 240
#define TOGGLE_Y 5

//Skin stuff
#define SKIN_TXT_X TOGGLE_TXT_X
#define SKIN_NUM_X TOGGLE_ON_X
#define SKIN_Y TOGGLE_Y
#define SKIN_PREVIEW_HEIGHT (NATIVE_RES_WIDTH/2 + 20)
#define SKIN_PREVIEW_WIDTH (NATIVE_RES_HEIGHT/2 + 20)
#define SKIN_SPACING (SKIN_PREVIEW_HEIGHT + 10)

//Colors (BGR format)
static SDL_Color textColor = { 255, 255, 255 };
static SDL_Color onColor   = { 255, 200, 200 };
static SDL_Color offColor  = {  50,  50,  50 };
static SDL_Color itemColor = {   0,   0,   0 };
static SDL_Color linkColor = { 255, 200, 200 };
static SDL_Color hiColor   = { 200, 200, 255 };

#include "HelpText.h"

enum menuState
{
  MENU_MAIN,
  MENU_SAVES,
  MENU_OPTIONS,
  MENU_SKINS,
  MENU_HELP
};

enum optionType {
  WIDGET_TOGGLE,
  WIDGET_SAVE,
  WIDGET_BUTTON,
  WIDGET_SKIN
};

enum helpState
{
  HELP_ROMS,
  HELP_CONTROLS,
  HELP_SETTINGS,
  HELP_WIKI
};

typedef struct
{
  char * on_text;
  char * off_text;
  void (*set)(bool);
  bool (*get)(void);
} toggle_data;

typedef struct
{
  int save_num;
} save_data;

typedef struct
{
  void (*action)(void);
} button_data;

typedef struct menuOption
{
  char * text;
  int y;
  enum optionType type;
  SDL_Surface * surface;
  void (*updateSurface)(struct menuOption*);
  union
  {
    toggle_data toggle;
    save_data save;
    button_data button;
  };
} menuOption;

static menuOption * topMenu = NULL;
static menuOption * saveMenu = NULL;
static menuOption * optionMenu = NULL;
static menuOption * helpMenu = NULL;
static menuOption * skinMenu = NULL;
static TTF_Font * menu_font = NULL;

static enum menuState menuState;
static enum helpState helpState;
static bool menuDone;
static eMenuResponse menuResponse;

void initializeMenu();
void doMenu( SDL_Surface * s, menuOption * options, int numOptions );
void doHelp( SDL_Surface * s );
bool optionHitCheck( menuOption * opt, int x, int y );
void freeMenu();
bool showLines( SDL_Surface * s, line * lines, int numlines, bool center );

void updateToggleSurface( menuOption * opt );
void updateSkinSurface( menuOption * opt );

/*-----------------------------------------------------------------------------
 *  Constructors for menu items
 *-----------------------------------------------------------------------------*/
menuOption createButton( char * text, void (*action)(void), int y )
{
  menuOption opt;
  opt.text = strdup( text );
  opt.type = WIDGET_BUTTON;
  opt.button.action = action;
  opt.y = y;
  opt.surface = NULL;
  opt.updateSurface = NULL;

  //Black rectangle
  opt.surface = SDL_CreateRGBSurface( SDL_SWSURFACE, OPTION_WIDTH, OPTION_SIZE, 24, 
      0x0000ff, 0x00ff00, 0xff0000, 0);
  int black = SDL_MapRGB( opt.surface->format, 0, 0, 0);
  SDL_FillRect( opt.surface, NULL, black );

  //With text on it
  SDL_Surface * s_txt = TTF_RenderText_Blended( menu_font, text, textColor );
  int xx = opt.surface->w/2 - s_txt->w/2;
  int yy = opt.surface->h/2 - s_txt->h/2;
  apply_surface( xx, yy, s_txt, opt.surface );

  SDL_FreeSurface( s_txt );


  return opt;
}

menuOption createToggle( char * text, char * on, char * off, int y, void (*set)(bool), bool (*get)(void) )
{
  menuOption opt;
  opt.text = strdup( text );
  opt.type = WIDGET_TOGGLE;
  opt.toggle.on_text = strdup( on );
  opt.toggle.off_text = strdup( off );
  opt.toggle.set = set;
  opt.toggle.get = get;
  opt.y = y;
  opt.surface = NULL;
  opt.updateSurface = updateToggleSurface;

  return opt;
}

menuOption createSkinWidget( int y )
{
  menuOption opt;
  opt.text = strdup( "Selected Skin" );
  opt.type = WIDGET_SKIN;
  opt.y = y;
  opt.surface = NULL;
  opt.updateSurface = updateSkinSurface;

  return opt;
}

menuOption createSave( int num, int y )
{
  char buf[1024];
  sprintf( buf, "Save %d", num + 1 );
  menuOption opt;
  opt.text = strdup( buf );
  opt.type = WIDGET_SAVE;
  opt.save.save_num = num;
  opt.y = y;
  opt.updateSurface = NULL;

  //Black rectangle
  opt.surface = SDL_CreateRGBSurface( SDL_SWSURFACE, OPTION_WIDTH, OPTION_SIZE, 24, 
      0xff0000, 0x00ff00, 0x0000ff, 0);
  int black = SDL_MapRGB( opt.surface->format, 0, 0, 0);
  SDL_FillRect( opt.surface, NULL, black );

  //Create "Save 1:  load  save"-style display
  SDL_Surface * s_txt  = TTF_RenderText_Blended( menu_font, opt.text, textColor );
  SDL_Surface * s_load = TTF_RenderText_Blended( menu_font, "Load",   textColor );
  SDL_Surface * s_save = TTF_RenderText_Blended( menu_font, "Save",   textColor );

  apply_surface( TOGGLE_TXT_X, TOGGLE_Y, s_txt,  opt.surface );
  apply_surface( TOGGLE_ON_X,  TOGGLE_Y, s_load, opt.surface );
  apply_surface( TOGGLE_OFF_X, TOGGLE_Y, s_save, opt.surface );

  SDL_FreeSurface( s_txt  );
  SDL_FreeSurface( s_load );
  SDL_FreeSurface( s_save );

  return opt;
}


/*-----------------------------------------------------------------------------
 *  Menu option rendering routines
 *-----------------------------------------------------------------------------*/
void updateToggleSurface( menuOption * opt )
{
  if ( opt->surface )
    SDL_FreeSurface( opt->surface );

  //Black rectangle
  opt->surface = SDL_CreateRGBSurface( SDL_SWSURFACE, OPTION_WIDTH, OPTION_SIZE, 24, 
      0xff0000, 0x00ff00, 0x0000ff, 0);
  int black = SDL_MapRGB( opt->surface->format, 0, 0, 0);
  SDL_FillRect( opt->surface, NULL, black );

  //Render each piece...
  SDL_Color on_color = opt->toggle.get() ? onColor : offColor;
  SDL_Color off_color = opt->toggle.get() ? offColor : onColor;
  SDL_Surface * s_txt = TTF_RenderText_Blended( menu_font, opt->text,            textColor );
  SDL_Surface * s_on  = TTF_RenderText_Blended( menu_font, opt->toggle.on_text,  on_color );
  SDL_Surface * s_off = TTF_RenderText_Blended( menu_font, opt->toggle.off_text, off_color );

  apply_surface( TOGGLE_TXT_X, TOGGLE_Y, s_txt, opt->surface );
  apply_surface( TOGGLE_ON_X,  TOGGLE_Y, s_on,  opt->surface );
  apply_surface( TOGGLE_OFF_X, TOGGLE_Y, s_off, opt->surface );

  SDL_FreeSurface( s_txt );
  SDL_FreeSurface( s_on  );
  SDL_FreeSurface( s_off );
}

void updateSkinSurface( menuOption * opt )
{
  if ( opt->surface )
    SDL_FreeSurface( opt->surface );

  char skin_num[20];
  snprintf( skin_num, sizeof(skin_num), "%d : %s", skin_index+1, getSkinName( skin ) );
  skin_num[sizeof(skin_num)-1] = '\0';

  //Black rectangle
  SDL_Surface * rect_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, OPTION_WIDTH, OPTION_SIZE, 24, 
      0xff0000, 0x00ff00, 0x0000ff, 0);
  int black = SDL_MapRGB(rect_surface->format, 0, 0, 0);
  SDL_FillRect( rect_surface, NULL, black );

  //Render each piece...
  SDL_Surface * s_txt = TTF_RenderText_Blended( menu_font, opt->text, textColor );
  SDL_Surface * s_num = TTF_RenderText_Blended( menu_font, skin_num,  onColor );

  apply_surface( SKIN_TXT_X, TOGGLE_Y, s_txt,  rect_surface );
  apply_surface( SKIN_NUM_X,  TOGGLE_Y, s_num, rect_surface );

  opt->surface = SDL_CreateRGBSurface( SDL_SWSURFACE, OPTION_WIDTH, 
      OPTION_SIZE + 10 + SKIN_PREVIEW_HEIGHT, 24, 
      0xff0000, 0x00ff00, 0x0000ff, 0);

  SDL_Surface * skin_preview = IMG_Load( skin->image_path );
  if ( skin_preview )
  {
    //Scale the image by half:
    skin_preview = SDL_Resize( skin_preview, 0.5f, true, 1 );

    //Convert to BGR (d'oh)
    SDL_Surface * rgb_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, skin_preview->w, skin_preview->h, 24,
            0xff0000, 0x00ff00, 0x0000ff, 0);
    SDL_Surface * bgr_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, skin_preview->w, skin_preview->h, 24,
            0x0000ff, 0x00ff00, 0xff0000, 0);

    SDL_BlitSurface( skin_preview, NULL, rgb_surface, NULL );
    int pixels_size = 320*480 /* resolution */
                     / 4      /* scaling */
                     * 3;     /* bpp */
    memcpy( bgr_surface->pixels, rgb_surface->pixels, pixels_size );
    SDL_FreeSurface( rgb_surface );
    SDL_FreeSurface( skin_preview );
    skin_preview = bgr_surface;

    //Draw it
    int w = OPTION_WIDTH/2 - skin_preview->w/2;
    int h = OPTION_SIZE + 10 + SKIN_PREVIEW_HEIGHT/2 - skin_preview->h/2;
    apply_surface( w, h, skin_preview, opt->surface );
    SDL_FreeSurface( skin_preview);
  }
  apply_surface( 0, 0, rect_surface, opt->surface );

  SDL_FreeSurface( s_txt );
  SDL_FreeSurface( s_num  );
  SDL_FreeSurface( rect_surface );
}

/*-----------------------------------------------------------------------------
 *  Functors for menu options...
 *-----------------------------------------------------------------------------*/
void changeToMainState(void)     { menuState = MENU_MAIN;    }
void changeToSaveState(void)     { menuState = MENU_SAVES;   }
void changeToSkinState(void)     { menuState = MENU_SKINS;   }
void changeToOptionsState(void)  { menuState = MENU_OPTIONS; }
void changeToHelpState(void)     { menuState = MENU_HELP;    }

void exitMenu(void)              { menuDone = true;          }
void moveToRomSelector(void);
void handleMenuSaveState(int,bool);

void changeToHelpROMsState(void)     { helpState = HELP_ROMS;     }
void changeToHelpControlsState(void) { helpState = HELP_CONTROLS; }
void changeToHelpSettingsState(void) { helpState = HELP_SETTINGS; }
void changeToHelpWikiState(void)     { helpState = HELP_WIKI;     }

void menuSetOrientation( bool portrait )
{
  orientation = portrait ? ORIENTATION_PORTRAIT : ORIENTATION_LANDSCAPE_R;
  updateOrientation();
}

void menuSetAutoSkip( bool on )
{
  //Update autoframeskip
  autoFrameSkip = on;
  //Reset frameskip-related variables
  systemFrameSkip = 0;
  frameskipadjust = 0;
}

void menuSetSound( bool sound )   { soundMute = !sound;                          }
void menuSetFilter( bool smooth ) { gl_filter = smooth ? GL_LINEAR : GL_NEAREST;
                                    GL_InitTexture();                            }
void menuSetSpeed( bool show )    { showSpeed = show ? 1 : 0;                    }
void menuSetAutoSave( bool on )   { autosave = on;                               }
void menuSetOnscreen( bool on )   { use_on_screen = on; updateOrientation();     }
void menuSetTurboToggle( bool on ){ turbo_toggle = on;                           }

bool menuGetOrientation() { return orientation == ORIENTATION_PORTRAIT; }
bool menuGetSound()       { return !soundMute;                          }
bool menuGetFilter()      { return gl_filter == GL_LINEAR;              }
bool menuGetSpeed()       { return showSpeed != 0;                      }
bool menuGetAutoSave()    { return autosave;                            }
bool menuGetAutoSkip()    { return autoFrameSkip;                       }
bool menuGetOnscreen()    { return use_on_screen;                       }
bool menuGetTurboToggle() { return turbo_toggle;                        }

//Call this to display the options menu...
eMenuResponse optionsMenu()
{
  SDL_Surface * surface = SDL_GetVideoSurface();
  SDL_Surface * options_screen = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, 24, 
      0x0000ff, 0x00ff00, 0xff0000, 0);

  if (!options_screen)
  {
    fprintf( stderr, "Error creating options menu!\n" );
    exit( 1 );
  }

  //Make sure we have the proper values...
  readOptions();

  initializeMenu();

  //Start out at top-level menu
  menuState = MENU_MAIN;
  menuDone = false;
  menuResponse = MENU_RESPONSE_RESUME;
  while (!menuDone)
  {
    switch( menuState )
    {
      case MENU_MAIN:
        doMenu( options_screen, topMenu, emulating? 6 : 4 );
        break;
      case MENU_OPTIONS:
        doMenu( options_screen, optionMenu, 8 );
        break;
      case MENU_SKINS:
        doMenu( options_screen, skinMenu, 3 );
        break;
      case MENU_SAVES:
        doMenu( options_screen, saveMenu, 4 );
        break;
      case MENU_HELP:
        doHelp( options_screen );
        break;
      default:
        break;
    }
  }

  SDL_FreeSurface( options_screen );

  freeMenu();

  return menuResponse;
}

void initializeMenu()
{
  menu_font = TTF_OpenFont( FONT, 18 );

  //Static initializers for all this is a PITA, so do it dynamically.
  
  //Tell the user they have skins, and how many.
  char skins_label[1024];
  snprintf( skins_label, 1024, "Skins (%d)", skin_count );
  skins_label[1024]='\0';

  //Top-level menu
  int x = 0;
  topMenu = (menuOption*)malloc( ( emulating ? 6 : 4 )*sizeof(menuOption));
  if (emulating)
    topMenu[x++] = createButton( "Save states",           changeToSaveState,   100+x*OPTION_SPACING);
  topMenu[x++] =   createButton( "Options",               changeToOptionsState,100+x*OPTION_SPACING);
  topMenu[x++] =   createButton( skins_label,             changeToSkinState,   100+x*OPTION_SPACING);
  topMenu[x++] =   createButton( "Help",                  changeToHelpState,   100+x*OPTION_SPACING);
  if (emulating)
    topMenu[x++] = createButton( "Choose different game", moveToRomSelector,   100+x*OPTION_SPACING);
  topMenu[x++] =   createButton( "Return",                exitMenu,            100+x*OPTION_SPACING);

  //Save menu
  x = 0;
  saveMenu = (menuOption*)malloc(4*sizeof(menuOption));
  saveMenu[x++] = createSave( x, 100+x*OPTION_SPACING );
  saveMenu[x++] = createSave( x, 100+x*OPTION_SPACING );
  saveMenu[x++] = createSave( x, 100+x*OPTION_SPACING );
  saveMenu[x++] = createButton( "Return", changeToMainState, 100+x*OPTION_SPACING );
  
  //Options menu
  x = 0;
  optionMenu = (menuOption*)malloc(8*sizeof(menuOption));
  optionMenu[x++] = createToggle( "Orientation",   "Port",   "Land",  50+x*OPTION_SPACING,
      menuSetOrientation, menuGetOrientation );
  optionMenu[x++] = createToggle( "Sound",         "On",     "Off",   50+x*OPTION_SPACING,
      menuSetSound, menuGetSound );
  optionMenu[x++] = createToggle( "Filter",        "Smooth", "Sharp", 50+x*OPTION_SPACING,
      menuSetFilter, menuGetFilter );
  optionMenu[x++] = createToggle( "Show Speed",    "On",     "Off",   50+x*OPTION_SPACING,
      menuSetSpeed, menuGetSpeed );
  optionMenu[x++] = createToggle( "Autosave",      "On",     "Off",   50+x*OPTION_SPACING,
      menuSetAutoSave, menuGetAutoSave );
  optionMenu[x++] = createToggle( "Autoframeskip", "On",     "Off",   50+x*OPTION_SPACING,
      menuSetAutoSkip, menuGetAutoSkip );
  optionMenu[x++] = createToggle( "Turbo toggles", "On",     "Off",   50+x*OPTION_SPACING,
      menuSetTurboToggle, menuGetTurboToggle );
  optionMenu[x++] = createButton( "Return", changeToMainState, 50+x*OPTION_SPACING );
  
  //Skin menu
  x = 0;
  skinMenu = (menuOption*)malloc(3*sizeof(menuOption));
  skinMenu[x++] = createToggle( "Display skin",   "On",     "Off",   100+x*OPTION_SPACING,
      menuSetOnscreen, menuGetOnscreen );
  skinMenu[x++] = createSkinWidget( 100+x*OPTION_SPACING );
  skinMenu[x++] = createButton( "Return", changeToMainState, 100+x*OPTION_SPACING+SKIN_SPACING );

  //Help menu
  x = 0;
  helpMenu = (menuOption*)malloc(5*sizeof(menuOption));
  helpMenu[x++] = createButton( "Getting Started", changeToHelpROMsState,     100+x*OPTION_SPACING);
  helpMenu[x++] = createButton( "Controls",        changeToHelpControlsState, 100+x*OPTION_SPACING);
  helpMenu[x++] = createButton( "Settings",        changeToHelpSettingsState, 100+x*OPTION_SPACING);
  helpMenu[x++] = createButton( "Wiki",            changeToHelpWikiState,     100+x*OPTION_SPACING);
  helpMenu[x++] = createButton( "Return",          changeToMainState,         100+x*OPTION_SPACING );
}

void freeMenu( menuOption ** opt, int numOptions )
{
  for ( int i = 0; i < numOptions; ++i )
  {
    menuOption o = (*opt)[i];
    free( o.text );
    SDL_FreeSurface( o.surface );
    switch( o.type )
    {
      case WIDGET_TOGGLE:
        free( o.toggle.on_text );
        free( o.toggle.off_text );
        break;
      case WIDGET_SAVE:
        break;
      case WIDGET_BUTTON:
        break;
      default:
        break;
    }
  }

  //Free the array...
  free( *opt );
  *opt = NULL;
}

void freeMenu()
{
  freeMenu( &topMenu, emulating ? 6 : 4 );
  freeMenu( &saveMenu,   4 );
  freeMenu( &skinMenu,   3 );
  freeMenu( &helpMenu,   5 );
  freeMenu( &optionMenu, 8 );
}

void doMenu( SDL_Surface * s, menuOption * options, int numOptions )
{
  // Menu background, same as rom selector
  int menuBGColor = SDL_MapRGB( s->format, 85, 0, 0 );//BGR
  SDL_FillRect( s, NULL, menuBGColor );

  //Draw the menu we were given...
  for ( int i = 0; i < numOptions; ++i )
  {
    //If the option has an update functor, use it.
    if ( options[i].updateSurface )
      options[i].updateSurface( &options[i] );

    //Draw option on top of the box we drew
    int x = s->w / 2 - options[i].surface->w/2;
    int y = options[i].y;
    apply_surface( x, y, options[i].surface, s );
  }

  // Loop, redrawing menu (in case card gets invalidated, etc)
  // until something interesting happens...
  bool done = false;
  SDL_Event event;
  while( !done )
  {
    SDL_DrawSurfaceAsGLTexture( s, portrait_vertexCoords );
    while( SDL_PollEvent( &event ) )
    {
      switch ( event.type )
      {
        case SDL_MOUSEBUTTONUP:
          //Find which option this was, if it's somehow multiple we just take the first such option
          for ( int i = 0; i < numOptions; ++i )
          {
            if ( optionHitCheck( &options[i], event.button.x, event.button.y ) )
            {
              printf( "Chose: %s\n", options[i].text );
              done = true;
              //Make sure any changes to options are saved.
              writeOptions();
              break;
            }
          }
          break;
        case SDL_KEYUP:
          //Back-gesture /swipe back
          if ( event.key.keysym.sym == SDLK_ESCAPE )
          {
            //If top-level, exit menu, else go to main menu
            if ( menuState == MENU_MAIN )
              menuDone = true;
            else
              menuState = MENU_MAIN;
            done = true;
          }
          break;
        default:
          break;
      }
    }
  }
}

#define DO_HELP( HELP_SCREEN, s ) \
  do {                                                              \
    int count = sizeof(HELP_SCREEN)/sizeof(HELP_SCREEN[0]);         \
    for (int i = 0; i < count; ++i )                                \
    {                                                               \
      int lines = sizeof(HELP_SCREEN[i])/sizeof(HELP_SCREEN[i][0]); \
      if (showLines( s, HELP_SCREEN[i],lines, false )) break;       \
    }                                                               \
  } while(0)

void doHelp( SDL_Surface * s )
{
  while( menuState == MENU_HELP )
  {
    //Show menu asking user which help they want...
    doMenu( s, helpMenu, 5 );
    
    //This is weak, but will do:
    //If we exit the above menu, two things are true:
    //--either user wanted to exit the help menu altogether
    //--or the user wants to see a particular help topic

    //Cover the former here
    if ( menuState != MENU_HELP ) break;

    //And the latter here
    switch( helpState )
    {
      case HELP_ROMS:
        DO_HELP( helpROMs, s );
        break;
      case HELP_CONTROLS:
        DO_HELP( helpControls, s );
        break;
      case HELP_SETTINGS:
        DO_HELP( helpSettings, s );
        break;
      case HELP_WIKI:
      {
        bool launchWiki = !showLines( s, helpWiki, 14, false );
        if (launchWiki)
          PDL_LaunchBrowser( VBA_WIKI );
        break;
      }
      default:
        printf( "Unexpected help state?!\n" );
        break;
    }
    
    //Okay, done showing the help topic the user selected,
    //loop back and let them pick again.
  }
}

void doHelpExternal( SDL_Surface * s )
{
  initializeMenu();
  menuState = MENU_HELP;
  doHelp( s );
  freeMenu();
}

//Determine if this click hits this option... if so, take the right action!
bool optionHitCheck( menuOption * opt, int x, int y )
{
  bool hit = false;

  if ( y >= opt->y && y <= opt->y + OPTION_SIZE )
  {
    switch( opt->type )
    {
      case WIDGET_BUTTON:
        //Buttons are easy :)
        hit = true;
        opt->button.action();
        break;
      case WIDGET_SAVE:
        if ( x >= TOGGLE_ON_X && x < TOGGLE_OFF_X )
        {
          hit = true;
          sdlReadState(opt->save.save_num);
          menuDone = true;
        } else if ( x >= TOGGLE_OFF_X )
        {
          hit = true;
          sdlWriteState(opt->save.save_num);
          menuDone = true;
        }
        break;
      case WIDGET_TOGGLE:
#if 0
        if ( x >= TOGGLE_ON_X && x < TOGGLE_OFF_X )
        {
          hit = true;
          opt->toggle.set(true);
        } else if ( x >= TOGGLE_OFF_X )
        {
          hit = true;
          opt->toggle.set(false);
        }
#else
        //Instead of having the user CHOOSE which, tapping this option
        //toggles between the two available options.
        hit = true;
        opt->toggle.set(!opt->toggle.get());
#endif
        break;
      case WIDGET_SKIN:
        hit = true;
        nextSkin();
        writeOptions();

        break;
    }
  }

  return hit;

}

void moveToRomSelector()
{
  menuDone = true;
  menuResponse = MENU_RESPONSE_ROMSELECTOR;
}

bool showLines( SDL_Surface * s, line * lines, int numlines, bool center )
{
    // Menu background, same as rom selector
    int menuBGColor = SDL_MapRGB( s->format, 85, 0, 0 );//BGR
    SDL_FillRect( s, NULL, menuBGColor );
    bool exit = false;

    //Black rectangle behind text...
    SDL_Rect drawRect;
    drawRect.x = 10;
    drawRect.y = 20;
    drawRect.h = s->h - 30;
    drawRect.w = s->w-20;
    int black = SDL_MapRGB(s->format, 0, 0, 0);
    SDL_FillRect(s, &drawRect, black);
    
    //Draw the lines specified...
    SDL_Surface * nr[numlines];
    int offset = 30;//arbitrary offset, centering all this isn't worth it.
    TTF_Font * font_normal = TTF_OpenFont( FONT, 16 );
    for ( int i = 0; i < numlines; ++i )
    {
        nr[i] = TTF_RenderText_Blended( font_normal, lines[i].msg, lines[i].color );
        int x;
        if (center)
            x = s->w/2-nr[i]->w/2;
        else
            x = 20;
        apply_surface( x, offset, nr[i], s );
        offset += nr[i]->h + 10;
    }
    TTF_CloseFont( font_normal );

    SDL_Event event;
    bool done = false;
    while (!done)
    {
        SDL_DrawSurfaceAsGLTexture( s, portrait_vertexCoords );
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_MOUSEBUTTONUP )
            {
                //User clicked, we're done.
                done = true;
                break;
            }
            if ( event.type == SDL_KEYUP &&
                 event.key.keysym.sym == SDLK_ESCAPE )
            {
              done = true;
              exit = true;
              break;
            }
            
        }
    }

    //Free the surfaces we created...
    for ( int i = 0; i < numlines; ++i )
    {
        SDL_FreeSurface( nr[i] );
    }

    return exit;
}
