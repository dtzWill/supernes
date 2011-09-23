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
#include "GLUtil.h"
#include "snes9x.h"

/*-----------------------------------------------------------------------------
 *  Option variable definitions
 *-----------------------------------------------------------------------------*/
int orientation = ORIENTATION_PORTRAIT;
int gl_filter = GL_LINEAR;
int soundMute = false;
int showSpeed = false;
int autosave = true;
int UseTransparency = true;
int use_on_screen = true;
int fullscreen = false;

game_option state_options[] =
{
    { "orientation", &orientation },
    { "sound", &soundMute },
    { "filter", &gl_filter },
    { "speed", &showSpeed },
    { "autosave", &autosave },
    { "transparency", &UseTransparency },
    { "onscreen", &use_on_screen },
    { "skin", &skin_index },
    { "fullscreen", &fullscreen }
};

int atoiHex( const char * s )
{
    int value;
    sscanf( s, "%x", &value );
    return value;
}

bool writeOptions( const char * cfgfile, game_option * option_list, int option_count, bool useHex )
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

int readOptions( const char * cfgfile, game_option * option_list, int option_count, bool useHex )
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
            sizeof( state_options ) / sizeof( game_option ),
            true );
}

void readOptions()
{
    readOptions(
            OPTIONS_CFG,
            state_options,
            sizeof( state_options ) / sizeof( game_option ),
            true );
}

