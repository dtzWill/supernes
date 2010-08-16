/*
 * ===========================================================================
 *
 *       Filename:  pdl.h
 *
 *    Description:  Parts of PDL that I use
 *
 *        Version:  1.0
 *        Created:  01/26/2010 01:32:23 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

//XXX due to work elsewhere there are better headers to be using, and I should use them.

#define PDL_ORIENTATION_BOTTOM 0
#define PDL_ORIENTATION_RIGHT 1
#define PDL_ORIENTATION_TOP 2
#define PDL_ORIENTATION_LEFT 3

extern "C" void PDL_SetOrientation( int direction );

extern "C" void PDL_LaunchBrowser( char * url );
