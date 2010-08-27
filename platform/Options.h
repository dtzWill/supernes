/*
 * ===========================================================================
 *
 *       Filename:  Options.h
 *
 *    Description:  Options reading/parsing code
 *
 *        Version:  1.0
 *        Created:  08/12/2010 05:49:58 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

typedef struct
{
    char * name;
    int * value;
} game_option;

bool writeOptions( char * cfgfile, game_option * option_list, int option_count, bool useHex );
int readOptions( char * cfgfile, game_option * option_list, int option_count, bool useHex );

extern void writeOptions();
extern void readOptions();


/*-----------------------------------------------------------------------------
 *  Options
 *-----------------------------------------------------------------------------*/

extern int orientation;
extern int soundMute;
extern int gl_filter;
extern int showSpeed;
extern int autosave;
extern int UseTransparency;

#endif //_OPTIONS_H_
