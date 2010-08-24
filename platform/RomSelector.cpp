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

#include "VBA.h"
#include "RomSelector.h"
#include "GLUtil.h"
#include "OptionMenu.h"
#include "pdl.h"

#include <dirent.h>

//In 'BGR' format...
static SDL_Color textColor = { 255, 255, 255 };
static SDL_Color hiColor = { 200, 200, 255 };
static SDL_Color linkColor = { 255, 200, 200 };

typedef struct
{
  char * msg;
  SDL_Color color;
} line;
static line no_roms[] {
{ "Welcome to VBA!",                     textColor},
{ "Looks like you don't have any ROMs.", textColor},
{ "To play games, put the roms in ",     textColor},
{ "/vba/roms",                           hiColor},
{ "using USB mode",                      textColor},
{ "(make the directory if needed)",      textColor},
{ "and then launch VBA again",           textColor},
{ "For more information, see the help",  textColor},
{ "(click here to launch help)",         linkColor}
};

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
            strcasecmp( extPtr, "gb" ) &&
            strcasecmp( extPtr, "gbc" ) &&
            strcasecmp( extPtr, "gba" ) &&
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

char * romSelector()
{
    // Create buffer we render selector into
    SDL_Surface * surface = SDL_GetVideoSurface();
    SDL_Surface * selector = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, 24, 
      0x0000ff, 0x00ff00, 0xff0000, 0);

    if (!selector )
    {
        fprintf( stderr, "Error creating rom selector buffer!\n" );
        exit( 1 );
    }

    TTF_Font * font_small = TTF_OpenFont( FONT, 14 );
    TTF_Font * font_normal = TTF_OpenFont( FONT, 18 );
    TTF_Font * font_large = TTF_OpenFont( FONT, 22 );
    if ( !font_small || !font_normal || !font_large )
    {
        fprintf( stderr, "Failed to open font: %s\n", FONT );
        exit( 1 );
    }

    //Don't bail here, we can't write to /media/internal on 1.4.5
#if 0
    //Make sure rom dir exists
    //XXX: This assumes /media/internal (parent directory) already exists
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;
    int result = mkdir( VBA_HOME, mode );
    if ( result && ( errno != EEXIST ) )
    {
        fprintf( stderr, "Error creating directory %s!\n", VBA_HOME );
        exit( 1 );
    }
    result = mkdir( ROM_PATH, mode );
    if ( result && ( errno != EEXIST ) )
    {
        fprintf( stderr, "Error creating directory %s for roms!\n", ROM_PATH );
        exit( 1 );
    }
#endif


    struct dirent ** roms;
    int filecount = scandir( ROM_PATH, &roms, romFilter, sortComparD );
    printf( "Rom count: %d\n", filecount );

    //Display general information
    int top, bottom;
    int borderColor = SDL_MapRGB( selector->format, 85, 0, 0 );//BGR
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
    SDL_Rect drawRect;
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

    //Generate text for each rom...
    SDL_Surface * roms_surface[filecount];
    for ( int i = 0; i < filecount; i++ )
    {
        //Here we remove everything in '()'s or '[]'s
        //which is usually annoying release information, etc
        char buffer[100];
        char * src = roms[i]->d_name;
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

        roms_surface[i] = TTF_RenderText_Blended( font_large, buffer, textColor );
    }

    int scroll_offset = 0;
    SDL_Event event;
    bool tap = false;
    bool down = false;
    int romSelected = -1;
    SDL_EnableUNICODE( 1 );
    while( romSelected == -1 )
    {
        //Calculate scroll, etc
        int num_roms_display = ( bottom - top + 10 ) / ( roms_surface[0]->h + 10 );
        //Get key input, process.
        while ( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
                case SDL_MOUSEBUTTONDOWN:
                    down = tap = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    down = false;
                    if ( tap )
                    {
                        // If tap was in the rom selection scrollbox...
                        if ( event.button.y >= top && event.button.y <= bottom )
                        {
                          //Calculate which rom this would be, and verify that makes sense
                          int rom_index = ( event.button.y - top ) / ( roms_surface[0]->h + 10 );
                          if ( rom_index >= 0 && rom_index < num_roms_display &&
                              rom_index + scroll_offset < filecount )
                          {
                            romSelected = rom_index+scroll_offset;
                          }
                        }
                        if ( event.button.y > bottom && event.button.x < selector->w / 2 )
                        {
                          optionsMenu();
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    //If the mouse moves before going up, it's not a tap
                    tap = false;

                    //scroll accordingly..
                    if ( down )
                    {
                        scroll_offset -= event.motion.yrel / SCROLL_FACTOR;
                        if ( scroll_offset > filecount - num_roms_display ) scroll_offset = filecount - num_roms_display;
                        if ( scroll_offset < 0 ) scroll_offset = 0;
                    }

                    break;
                case SDL_KEYDOWN:
                {
                    //Filter based on letter presses.
                    //For now, just jump to the first thing that starts at or after that letter.
                    char c = (char)event.key.keysym.unicode;
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
                            char c_file = *roms[offset]->d_name;
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
                    }
                }
                default:
                    break;
            }
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

        //Draw roms...

        for ( int i = 0; i < num_roms_display; i++ )
        {
           int index = scroll_offset + i;
           if ( index == romSelected )
           {
               int hiColor = SDL_MapRGB( selector->format, 0, 128, 128 );//BGR
               SDL_Rect hiRect;
               hiRect.x = 10;
               hiRect.y = top+(10+roms_surface[0]->h)*i - 5;
               hiRect.h = roms_surface[index]->h+5;
               hiRect.w = selector->w - 20;
               SDL_FillRect( selector, &hiRect, hiColor );
           }
           apply_surface( 20, top + (10+roms_surface[0]->h)*i, selector->w - 40, roms_surface[index], selector );
        }

        //Update screen.
        SDL_DrawSurfaceAsGLTexture( selector, portrait_vertexCoords );
        if ( romSelected != -1 )
        {
            SDL_Delay( 20 );
        }
    }
    SDL_FreeSurface( title );
    SDL_FreeSurface( author );
    SDL_FreeSurface( options );
    SDL_FreeSurface( selector );

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

