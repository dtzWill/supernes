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

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <SDL_ttf.h>

#ifndef _ROM_SELECTOR_UTIL_H_
#define _ROM_SELECTOR_UTIL_H_

extern TTF_Font * font_small;
extern TTF_Font * font_normal;
extern TTF_Font * font_large;

static const SDL_Color textColor = { 255, 255, 255 };
static const SDL_Color hiColor = { 255, 200, 200 };
static const SDL_Color linkColor = { 200, 200, 255 };

// Here we remove everything in '()'s or '[]'s
// which is usually annoying release information, etc
char * strip_rom_name( const char * rom_name )
{
  char buffer[300];
  const char * src = rom_name;
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
