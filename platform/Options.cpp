/*
 * ===========================================================================
 *
 *       Filename:  Options.cpp
 *
 *    Description:  Options reading/parsing code
 *
 *        Version:  1.0
 *        Created:  08/12/2010 05:49:44 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "Options.h"
#include "VBA.h"
#include "GLUtil.h"
#include "Controller.h"

vba_option state_options[] =
{
    { "orientation", &orientation },
    { "sound", &soundMute },
    { "filter", &gl_filter },
    { "speed", &showSpeed },
    { "onscreen", &use_on_screen },
    { "autosave", &autosave },
    { "autoframeskip", &autoFrameSkip },
    { "skin", &skin_index },
    { "turbo_toggle", &turbo_toggle }
};

struct option sdlOptions[] = {
  { "agb-print", no_argument, &sdlAgbPrint, 1 },
  { "auto-frameskip", no_argument, &autoFrameSkip, 1 },  
  { "bios", required_argument, 0, 'b' },
  { "config", required_argument, 0, 'c' },
  { "debug", no_argument, 0, 'd' },
  { "filter", required_argument, 0, 'f' },
  { "filter-normal", no_argument, &filter, 0 },
  { "filter-tv-mode", no_argument, &filter, 1 },
  { "filter-2xsai", no_argument, &filter, 2 },
  { "filter-super-2xsai", no_argument, &filter, 3 },
  { "filter-super-eagle", no_argument, &filter, 4 },
  { "filter-pixelate", no_argument, &filter, 5 },
  { "filter-motion-blur", no_argument, &filter, 6 },
  { "filter-advmame", no_argument, &filter, 7 },
  { "filter-simple2x", no_argument, &filter, 8 },
  { "filter-bilinear", no_argument, &filter, 9 },
  { "filter-bilinear+", no_argument, &filter, 10 },
  { "filter-scanlines", no_argument, &filter, 11 },
  { "filter-hq2x", no_argument, &filter, 12 },
  { "filter-lq2x", no_argument, &filter, 13 },
  { "flash-size", required_argument, 0, 'S' },
  { "flash-64k", no_argument, &sdlFlashSize, 0 },
  { "flash-128k", no_argument, &sdlFlashSize, 1 },
  { "frameskip", required_argument, 0, 's' },
  { "fullscreen", no_argument, &fullscreen, 1 },
  { "gdb", required_argument, 0, 'G' },
  { "help", no_argument, &sdlPrintUsage, 1 },
  { "ifb-none", no_argument, &ifbType, 0 },
  { "ifb-motion-blur", no_argument, &ifbType, 1 },
  { "ifb-smart", no_argument, &ifbType, 2 },
  { "ips", required_argument, 0, 'i' },
  { "no-agb-print", no_argument, &sdlAgbPrint, 0 },
  { "no-auto-frameskip", no_argument, &autoFrameSkip, 0 },
  { "no-debug", no_argument, 0, 'N' },
  { "no-ips", no_argument, &sdlAutoIPS, 0 },
  { "no-mmx", no_argument, &disableMMX, 1 },
  { "no-pause-when-inactive", no_argument, &pauseWhenInactive, 0 },
  { "no-rtc", no_argument, &sdlRtcEnable, 0 },
  { "no-show-speed", no_argument, &showSpeed, 0 },
  { "no-throttle", no_argument, &throttle, 0 },
  { "pause-when-inactive", no_argument, &pauseWhenInactive, 1 },
  { "profile", optional_argument, 0, 'p' },
  { "rtc", no_argument, &sdlRtcEnable, 1 },
  { "save-type", required_argument, 0, 't' },
  { "save-auto", no_argument, &cpuSaveType, 0 },
  { "save-eeprom", no_argument, &cpuSaveType, 1 },
  { "save-sram", no_argument, &cpuSaveType, 2 },
  { "save-flash", no_argument, &cpuSaveType, 3 },
  { "save-sensor", no_argument, &cpuSaveType, 4 },
  { "save-none", no_argument, &cpuSaveType, 5 },
  { "show-speed-normal", no_argument, &showSpeed, 1 },
  { "show-speed-detailed", no_argument, &showSpeed, 2 },
  { "throttle", required_argument, 0, 'T' },
  { "verbose", required_argument, 0, 'v' },  
  { "video-1x", no_argument, &sizeOption, 0 },
  { "video-2x", no_argument, &sizeOption, 1 },
  { "video-3x", no_argument, &sizeOption, 2 },
  { "video-4x", no_argument, &sizeOption, 3 },
  { "yuv", required_argument, 0, 'Y' },
  { NULL, no_argument, NULL, 0 }
};


int atoiHex( char * s )
{
    int value;
    sscanf( s, "%x", &value );
    return value;
}

bool writeOptions( char * cfgfile, vba_option * option_list, int option_count, bool useHex )
{
   FILE * f = fopen( cfgfile, "w" );
   if ( !f )
   {
       perror( "Failed to read options!" );
       return false;
   }

   for ( int i = 0; i < option_count; i++ )
   {
       if ( useHex )
       {
       fprintf( f, "%s=%x\n", option_list[i].name, *option_list[i].value );
       }
       else
       {
           fprintf( f, "%s=%d\n", option_list[i].name, *option_list[i].value );
       }
   }

   return fclose( f ) == 0;
}

int readOptions( char * cfgfile, vba_option * option_list, int option_count, bool useHex )
{
    FILE * f = fopen( cfgfile, "r" );
    int options_read = 0;
    if ( !f )
    {
        perror( "Failed to read options!" );
        return -1;
    }

    char buffer[2048];

    while ( 1 )
    {
        char * s = fgets( buffer, 2048, f );

        //More to the file?
        if( s == NULL )
        {
            break;
        }

        //Ignore parts from '#' onwards
        char * p  = strchr(s, '#');

        if ( p )
        {
            *p = '\0';
        }

        char * token = strtok( s, " \t\n\r=" );

        if ( !token || strlen( token ) == 0 )
        {
            continue;
        }

        char * key = token;
        char * value = strtok( NULL, "\t\n\r" );

        if( value == NULL )
        {
            fprintf( stderr, "Empty value for key %s\n", key );
            continue;
        }

        //Okay now we have key/value pair.
        //match it to one of the ones we know about...
        char known = false;
        for ( int i = 0; i < option_count; i++ )
        {
            if ( !strcasecmp( key, option_list[i].name ) )
            {
                if ( useHex )
                {
                    *option_list[i].value = atoiHex( value );
                }
                else
                {
                    *option_list[i].value = atoi( value );
                }

                //Note that if there are duplicates, this will increase.  Ignoring that lameness for now. 
                //If someone really wants to make an invalid cfg, we'll just use it and when it doesn't work they can figure it out :)
                options_read++;
                known = true;
                break;
            }
        }
        if ( !known )
        {
            fprintf( stderr, "Unrecognized option %s\n", key );
        }
    }

    return fclose( f ) == 0 ? options_read : -1;
}

void writeOptions()
{
    writeOptions(
            OPTIONS_CFG,
            state_options,
            sizeof( state_options ) / sizeof( vba_option ),
            true );
}

void readOptions()
{
    readOptions(
            OPTIONS_CFG,
            state_options,
            sizeof( state_options ) / sizeof( vba_option ),
            true );
}

