/*
 * ===========================================================================
 *
 *       Filename:  RomSelectorUtil.h
 *
 *    Description:  Various Utility functions for the RomSelector
 *
 *        Version:  1.0
 *        Created:  09/22/2011 11:58:05 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <SDL_ttf.h>

#ifndef _ROM_SELECTOR_UTIL_H_
#define _ROM_SELECTOR_UTIL_H_

// We render the name of each rom, but instead of doing them all at once
// (initially slow, and memory-consuming), we render just what is needed.
// However that's lame, so we use a cache for a happy middle ground.
#define CACHE_SIZE 50

extern TTF_Font * font_small;
extern TTF_Font * font_normal;
extern TTF_Font * font_large;

static const SDL_Color textColor = { 255, 255, 255 };
static const SDL_Color hiColor = { 255, 200, 200 };
static const SDL_Color linkColor = { 200, 200, 255 };

// Here we remove everything in '()'s or '[]'s
// which is usually annoying release information, etc
char * strip_rom_name( char * rom_name )
{
  char buffer[100];
  char * src = rom_name;
  char * dst = buffer;
  int inParen = 0;

  // Search for open paren/brackets, and erase
  // all until find ending brace.
  // NOTE: Will convert 'Testing [fdas)fdsa]"
  // to "Testing asdf]", it doesn't care which is used where.
  // It *does* handle brace depth, however, so it will handle
  // "Testing[adf(fdsa)fdsas]" -> "Testing"
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

void ensureRomPathExists(void)
{
  //Don't bail here, we can't write to /media/internal on 1.4.5
  static const int BAIL_ON_FAILURE = 0; // /etc/fstab bug

  //Make sure rom dir exists
  int mode = S_IRWXU | S_IRWXG | S_IRWXO;
  int result = mkdir( SNES_HOME, mode );
  if ( BAIL_ON_FAILURE && result && ( errno != EEXIST ) )
  {
    fprintf( stderr, "Error creating directory %s!\n", SNES_HOME );
    exit( 1 );
  }
  result = mkdir( ROM_PATH, mode );
  if ( BAIL_ON_FAILURE && result && ( errno != EEXIST ) )
  {
    fprintf( stderr, "Error creating directory %s for roms!\n", ROM_PATH );
    exit( 1 );
  }

}

// Filter selector applied to candidate list of ROMs
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
      strcasecmp( extPtr, "sfc" ) &&
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

#endif // _ROM_SELECTOR_UTIL_H_
