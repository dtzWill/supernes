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

#include "../getopt.h"

typedef struct
{
    char * name;
    int * value;
} vba_option;

bool writeOptions( char * cfgfile, vba_option * option_list, int option_count, bool useHex );
int readOptions( char * cfgfile, vba_option * option_list, int option_count, bool useHex );

extern struct option sdlOptions[];

extern void writeOptions();
extern void readOptions();

#endif //_OPTIONS_H_
