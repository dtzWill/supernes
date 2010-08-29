/*
 * ===========================================================================
 *
 *       Filename:  Controller.cpp
 *
 *    Description:  Controller/Skin code
 *
 *        Version:  1.0
 *        Created:  08/12/2010 05:00:11 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "snes9x.h"

#include "Controller.h"
#include "GLUtil.h"

//current skin
controller_skin * skin = NULL;
int skin_index = 0;
int skin_count;

//No need to pull in a math library for this...
#define SIN_30 ( 0.5f )
#define SIN_60 ( 0.866025404f )
#define COS_30 SIN_60
#define COS_60 SIN_30

/*-----------------------------------------------------------------------------
 *  Some helpful macros
 *-----------------------------------------------------------------------------*/
static bool hit( int x, int y, int c_x, int c_y, int c_radius )
{
    return
        ( x - c_x ) * ( x - c_x ) +
        ( y - c_y ) * ( y - c_y )
        <
        c_radius * c_radius;
}

static bool hit_y( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->y_x, skin->y_y, skin->y_radius );
}

static bool hit_x( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->x_x, skin->x_y, skin->x_radius );
}

static bool hit_b( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->b_x, skin->b_y, skin->b_radius );
}

static bool hit_a( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->a_x, skin->a_y, skin->a_radius );
}

static bool hit_start( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->start_x, skin->start_y, skin->start_radius );
}

static bool hit_select( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->select_x, skin->select_y, skin->select_radius );
}

static bool hit_l( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->l_x, skin->l_y, skin->l_radius );
}

static bool hit_r( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->r_x, skin->r_y, skin->r_radius );
}

static bool hit_turbo( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->turbo_x, skin->turbo_y, skin->turbo_radius );
}

static bool hit_capture( controller_skin * skin, int x, int y )
{
    return hit( x, y, skin->capture_x, skin->capture_y, skin->capture_radius );
}

//D-pad
static bool hit_joy( controller_skin * skin, int x, int y )
{
    //We hit the joy if we're within the larger radius,
    //but outside the dead zone circle
    return
        hit( x, y, skin->joy_x, skin->joy_y, skin->joy_radius ) &&
        ! hit( x, y, skin->joy_x, skin->joy_y, skin->joy_dead );
}

//Landscape_r mode...
static bool hit_up( controller_skin * skin, int x, int y )
{
    int relx = ( x - skin->joy_x );
    if ( relx < 0 ) relx = -relx;
    return
        hit_joy( skin, x, y ) &&
        ( y < skin->joy_y - relx * SIN_30 );
}
static bool hit_down( controller_skin * skin, int x, int y )
{
    int relx = ( x - skin->joy_x );
    if ( relx < 0 ) relx = -relx;
    return
        hit_joy( skin, x, y ) &&
        ( y > skin->joy_y + relx * SIN_30 );
}
static bool hit_left( controller_skin * skin, int x, int y )
{
    int rely = ( y - skin->joy_y );
    if ( rely < 0 ) rely = -rely;
    return
        hit_joy( skin, x, y ) &&
        ( x < skin->joy_x - rely * COS_30 );
}
static bool hit_right( controller_skin * skin, int x, int y )
{
    int rely = ( y - skin->joy_y );
    if ( rely < 0 ) rely = -rely;
    return
        hit_joy( skin, x, y ) &&
        ( x > skin->joy_x + rely * COS_30 );
}

controller_skin tmp_skin;

game_option controller_options[] =
{
    //Controller stuff
    { "touch_screen_x_offset", &tmp_skin.controller_screen_x_offset },
    { "touch_screen_y_offset", &tmp_skin.controller_screen_y_offset },
    { "touch_screen_width", &tmp_skin.controller_screen_width },
    { "touch_screen_height", &tmp_skin.controller_screen_height },
    { "touch_joy_x", &tmp_skin.joy_x },
    { "touch_joy_y", &tmp_skin.joy_y },
    { "touch_joy_radius", &tmp_skin.joy_radius },
    { "touch_joy_deadzone", &tmp_skin.joy_dead },
    { "touch_y_x", &tmp_skin.b_x },
    { "touch_y_y", &tmp_skin.b_y },
    { "touch_y_radius", &tmp_skin.b_radius },
    { "touch_x_x", &tmp_skin.b_x },
    { "touch_x_y", &tmp_skin.b_y },
    { "touch_x_radius", &tmp_skin.b_radius },
    { "touch_b_x", &tmp_skin.b_x },
    { "touch_b_y", &tmp_skin.b_y },
    { "touch_b_radius", &tmp_skin.b_radius },
    { "touch_a_x", &tmp_skin.a_x },
    { "touch_a_y", &tmp_skin.a_y },
    { "touch_a_radius", &tmp_skin.a_radius },
    { "touch_start_x", &tmp_skin.start_x },
    { "touch_start_y", &tmp_skin.start_y },
    { "touch_start_radius", &tmp_skin.start_radius },
    { "touch_select_x", &tmp_skin.select_x },
    { "touch_select_y", &tmp_skin.select_y },
    { "touch_select_radius", &tmp_skin.select_radius },
    { "touch_l_x", &tmp_skin.l_x },
    { "touch_l_y", &tmp_skin.l_y },
    { "touch_l_radius", &tmp_skin.l_radius },
    { "touch_r_x", &tmp_skin.r_x },
    { "touch_r_y", &tmp_skin.r_y },
    { "touch_r_radius", &tmp_skin.r_radius },
    { "touch_turbo_x", &tmp_skin.turbo_x },
    { "touch_turbo_y", &tmp_skin.turbo_y },
    { "touch_turbo_radius", &tmp_skin.turbo_radius },
    { "touch_capture_x", &tmp_skin.capture_x },
    { "touch_capture_y", &tmp_skin.capture_y },
    { "touch_capture_radius", &tmp_skin.capture_radius }
};

/*-----------------------------------------------------------------------------
 *  Skin loading
 *-----------------------------------------------------------------------------*/

//Returns the skin name w/o the leading period, doesn't allocate anything
char * strip_leading_period( char * str )
{
    char * ret = str; 

    if( ret && *ret == '.' ) ret++;

    return ret;
}

char * getSkinName( controller_skin * skin )
{
  return strip_leading_period( skin->name );
}

void load_skin( char * skin_cfg, char * skin_img, char * skin_name, char * skin_folder, controller_skin ** skin )
{
    bool success = false;

    if ( !skin_cfg || !skin_img || !skin_name || !skin_folder || !skin )
    {
        //Come on now, call this correctly :)
        return;
    }

    char full_skin_cfg[strlen( skin_folder ) + strlen( skin_cfg ) + 2];
    char full_skin_img[strlen( skin_folder ) + strlen( skin_img ) + 2];
    strcpy( full_skin_cfg, skin_folder );
    strcpy( full_skin_cfg + strlen( skin_folder ), "/" );
    strcpy( full_skin_cfg + strlen( skin_folder ) + 1, skin_cfg );

    strcpy( full_skin_img, skin_folder );
    strcpy( full_skin_img + strlen( skin_folder ), "/" );
    strcpy( full_skin_img + strlen( skin_folder ) + 1, skin_img );

    int all_options = sizeof( controller_options ) / sizeof ( game_option );
    int options_read = readOptions(
            full_skin_cfg,
            controller_options,
            all_options,
            false );
    if ( options_read != all_options )
    {
        fprintf( stderr, "Error reading in %s! Read %d/%d\n", full_skin_cfg, options_read, all_options );
        return;
    }

    //Before adding to list of skins, verify we don't have this one already
    if ( *skin )
    {
        controller_skin * cur = *skin;
        do
        {
            bool duplicate = !strcmp(
                strip_leading_period(cur->name),
                strip_leading_period(skin_name));
            if ( duplicate )
            {
                fprintf( stderr, "Duplicate skin \"%s\" found, skipping!\n", skin_name );
                return;
            }
            cur = cur->next;
        } while (cur != *skin);
    }

    //Well we read it in, let's add it to the list.
    controller_skin * newskin = (controller_skin *)malloc( sizeof( controller_skin ) );
    memcpy( newskin, &tmp_skin, sizeof( controller_skin ) );
    //populate non-option fields...
    newskin->name = strdup( skin_name );
    newskin->image_path = strdup( full_skin_img );
    if ( *skin == NULL )
    {
        //Okay so our skin list is empty.
        *skin = newskin;
        //loop back, circular linked list
        (*skin)->next = newskin;
    }
    else
    {
        //We have a list of length >= 1
        //So we insert this one at the end, and set the 'skin' pointer to the end of the list.
        controller_skin * next = (*skin)->next;
        (*skin)->next = newskin;
        newskin->next = next;
        *skin = newskin;
    }

    printf( "Loaded skin %s\n", skin_name );
}

/* ===========================================================================
 * controllerHitCheck
 *   
 *  Description:  Determines which on-screen controls were hit for the given x,y
 * =========================================================================*/
int controllerHitCheck( int x, int y )
{
    int buttonMask = 0;

    // If there are no skins loaded, then no buttons can be hit.
    if ( !skin ) return 0;

    // D-pad
    if ( hit_up( skin, x, y ) ) buttonMask |= SNES_UP_MASK;
    if ( hit_down( skin, x, y ) ) buttonMask |= SNES_DOWN_MASK;
    if ( hit_left( skin, x, y ) ) buttonMask |= SNES_LEFT_MASK;
    if ( hit_right( skin, x, y ) ) buttonMask |= SNES_RIGHT_MASK;

    //Buttons
    if ( hit_y( skin, x, y ) ) buttonMask |= SNES_Y_MASK;
    if ( hit_x( skin, x, y ) ) buttonMask |= SNES_X_MASK;
    if ( hit_b( skin, x, y ) ) buttonMask |= SNES_B_MASK;
    if ( hit_a( skin, x, y ) ) buttonMask |= SNES_A_MASK;

    //Triggers/shoulder
    if ( hit_l( skin, x, y ) ) buttonMask |= SNES_TL_MASK;
    if ( hit_r( skin, x, y ) ) buttonMask |= SNES_TR_MASK;

    //Start/select
    if ( hit_start( skin, x, y ) ) buttonMask |= SNES_START_MASK;
    if ( hit_select( skin, x, y ) ) buttonMask |= SNES_SELECT_MASK;

    //XXX: Support touch-screen turbo/screenshot/misc others?

    return buttonMask;
}


/*-----------------------------------------------------------------------------
 *  Skin loading logic
 *-----------------------------------------------------------------------------*/
void scanForSkins( char * folder )
{
    fprintf( stderr, "Scanning \"%s\" for skins...\n", folder );
    DIR * d = opendir( folder );
    if ( !d )
    {
        perror( "Failed to open skin path!\n" );
        return;
    }
    //Note: this code isn't super portable, mostly because I don't care right now :).
    //Should work on most/all linux boxes...
    //(including of course the pre :))

    struct dirent * dp;
    //For each entry in this directory..
    while ( dp = readdir( d ) )
    {
        //If this is a directory (and not "." or ".." )
        if ( dp->d_type == DT_DIR &&
                strcmp( dp->d_name, "." ) &&
                strcmp( dp->d_name, ".." ) )

        {
            char * skin_name = dp->d_name;
            int folderlen = strlen( skin_name ) + strlen( folder ) + 2;

            //build full path
            char skin_folder[folderlen];
            strcpy( skin_folder, folder );
            strcpy( skin_folder + strlen( folder ), "/" );
            strcpy( skin_folder + strlen( folder ) + 1, skin_name );

            load_skin( SKIN_CFG_NAME, SKIN_IMG_NAME, skin_name, skin_folder, &skin );
        }
    }

    closedir( d );
}

void loadSkins()
{
    skin = NULL;

    // XXX: I *want* to use the appropriate PDK_get* method, but seems like
    // 1.4.5 bug prevents that.  So, for now, just look for skins relative to us.

    scanForSkins( "skins" );


    // Look for skins on the /media/internal path for user-loaded skins
    scanForSkins( SKIN_PATH );


    //As we build the list, the 'skin' always point to the last one in the list.
    //Here we just wrap around to the first (circule ll) so we default to the 'first' one.
    if ( skin )
    {
        skin = skin->next;

        controller_skin * first = skin;
        skin_count = 1;
        while ( skin->next != first )
        {
            skin_count++;
            skin = skin->next;
        }
        skin = first;
        
        //count 'skin_index' skins, this is the one we used last time.
        //Nope, not very robust to adding/removing skins, but whatcha gonna do.
        //Not sure it's worth implementing string support for in the cfg files.
        for ( int i = 0; i < skin_index; i++ )
        {
          skin = skin->next;
        }
    }
    else
    {
        skin_count = 0;
    }

}

void nextSkin()
{
  if ( skin_count > 0 )
  {
    skin = skin->next;
    //So next time we know which one
    skin_index = ( skin_index + 1 ) % skin_count;
  }

  //XXX: This doesn't really belong here.. what does the controller know about
  //the screen size....
  GL_InitTexture(IMAGE_WIDTH, IMAGE_HEIGHT);
  updateOrientation();
}
