/*
 * ===========================================================================
 *
 *       Filename:  Controller.h
 *
 *    Description:  Contains types and functions regarding controller skins
 *
 *        Version:  3.0
 *        Created:  01/22/2010 03:46:06 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "VBA.h"
#include "Options.h"

typedef struct
{
    int button1;
    int button2;
    char valid;
} controllerEvent;

struct controller_skin;
typedef struct controller_skin controller_skin;

struct controller_skin
{
char * name;
char * image_path;

int controller_screen_x_offset;
int controller_screen_y_offset;

int controller_screen_width;
int controller_screen_height;

int joy_x;
int joy_y;
int joy_radius;
int joy_dead;

int b_x;
int b_y;
int b_radius;

int a_x;
int a_y;
int a_radius;

int start_x;
int start_y;
int start_radius;

int select_x;
int select_y;
int select_radius;

int l_x;
int l_y;
int l_radius;

int r_x;
int r_y;
int r_radius;

int turbo_x;
int turbo_y;
int turbo_radius;

int capture_x;
int capture_y;
int capture_radius;

int ab_x;
int ab_y;
int ab_radius;

//circular linked list
controller_skin * next;

//end struct definition
};

extern controller_skin * skin;
extern int skin_index;
extern int skin_count;

extern controllerEvent controllerHitCheck( int x, int y );

extern void loadSkins();

extern void nextSkin();
extern char * getSkinName( controller_skin * skin );

#endif //_CONTROLLER_H_
