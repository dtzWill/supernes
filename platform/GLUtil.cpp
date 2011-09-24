/*
 * ===========================================================================
 *
 *       Filename:  GLUtil.cpp
 *
 *    Description:  GL utilities and definitions
 *
 *        Version:  1.0
 *        Created:  08/12/2010 06:06:54 PM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#include "GLUtil.h"
#include "Types.h"
#include "esFunc.h"
#include "pdl.h"
#include <assert.h>
#include "Controller.h"
#include "Options.h"
#include "RegionTimer.h"
#include <SDL_image.h>

/*-----------------------------------------------------------------------------
 *  GL variables
 *-----------------------------------------------------------------------------*/
GLuint texture = 0;
GLuint controller_tex = 0;

// Handle to a program object
GLuint programObject;

// Attribute locations
GLint  positionLoc;
GLint  texCoordLoc;

// Sampler location
GLint samplerLoc;

SDL_Surface *surface = NULL;
SDL_Overlay *overlay = NULL;
SDL_Rect overlay_rect;

int srcWidth = -1;
int srcHeight = -1;

int destWidth = -1;
int destHeight = -1;

//#define DEBUG_GL

#ifdef DEBUG_GL
void checkError()
{
    /* Check for error conditions. */
    GLenum gl_error = glGetError( );

    if( gl_error != GL_NO_ERROR ) {
        fprintf( stderr, "SNES9X: OpenGL error: %x\n", gl_error );
        while(1);
        exit( 1 );
    }

    const char * sdl_error = SDL_GetError( );

    if( sdl_error[0] != '\0' ) {
        fprintf(stderr, "SNES9X: SDL error '%s'\n", sdl_error);
        while(1);
        exit( 2 );
    }
}
#else
#define checkError()
#endif

/*-----------------------------------------------------------------------------
 *  Vertex coordinates for various orientations.
 *-----------------------------------------------------------------------------*/

//Current coords;
float vertexCoords[8];

//Landscape, keyboard on left.
float land_l_vertexCoords[] =
{
    -1, -1,
    1, -1,
    -1, 1,
    1, 1
};

//Landscape, keyboard on right.
float land_r_vertexCoords[] =
{
    1, 1,
    -1, 1,
    1, -1,
    -1, -1
};
//Portrait
float portrait_vertexCoords[] =
{
    -1, 1,
    -1, -1,
    1, 1,
    1, -1
};

float * controller_coords = land_r_vertexCoords;

float texCoords[] =
{
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    1.0, 1.0
};

GLushort indices[] = { 0, 1, 2, 1, 2, 3 };

void GL_Init()
{

    assert( !SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 ) );
    //assert( !SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 0 ) );
    //assert( !SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 ) );
    assert( !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ) );

    surface = SDL_SetVideoMode( 0, 0, 32,
        SDL_OPENGL);
    destWidth = surface->w;
    destHeight = surface->h;

    if(surface == NULL) {
      //systemMessage(0, "Failed to set video mode");
      SDL_Quit();
      exit(-1);
    }

    // setup 2D gl environment
    checkError();
    checkError();
    // Black background
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    checkError();

    // Remove unnecessary operations..
    glDepthFunc(GL_ALWAYS);
    checkError();
    glDisable(GL_DEPTH_TEST);
    checkError();
    glDisable(GL_STENCIL_TEST);
    checkError();
    glDisable(GL_CULL_FACE);
    checkError();

    //Enable alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    GLbyte vShaderStr[] =  
        "attribute vec4 a_position;   \n"
        "attribute vec2 a_texCoord;   \n"
        "varying vec2 v_texCoord;     \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = a_position; \n"
        "   v_texCoord = a_texCoord;  \n"
        "}                            \n";

    GLbyte fShaderStr[] =  
        "precision mediump float;                            \n"
        "varying vec2 v_texCoord;                            \n"
        "uniform sampler2D s_texture;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
        "}                                                   \n";

    // Load the shaders and get a linked program object
    programObject = esLoadProgram ( (char *)vShaderStr, (char *)fShaderStr );
    checkError();

    // Get the attribute locations
    positionLoc = glGetAttribLocation ( programObject, "a_position" );
    checkError();
    texCoordLoc = glGetAttribLocation ( programObject, "a_texCoord" );
    checkError();

    // Get the sampler location
    samplerLoc = glGetUniformLocation ( programObject, "s_texture" );
    checkError();

}


void GL_InitTexture(int _srcWidth, int _srcHeight)
{
    srcWidth = _srcWidth;
    srcHeight = _srcHeight;

    //delete it if we already have one
    if ( texture )
    {
        glDeleteTextures( 1, &texture );
        texture = 0;
    }
    if ( controller_tex )
    {
        glDeleteTextures( 1, &controller_tex );
        controller_tex = 0;
    }

    glGenTextures(1, &texture);
    checkError();
    glBindTexture(GL_TEXTURE_2D, texture);
    checkError();
    
    //sanity check
    int num;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &num );
    assert( num == texture );
    glGetIntegerv( GL_ACTIVE_TEXTURE, &num );
    assert( num == GL_TEXTURE0 );
    checkError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter );
    checkError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter );
    checkError();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    checkError();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    checkError();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, srcWidth, srcHeight, 0, GL_RGB,
            GL_UNSIGNED_SHORT_5_6_5, NULL );
    checkError();

    if( !skin )
    {
        printf( "No skins found! Running without one...\n" );
        return;
    }

    //Load controller
    SDL_Surface * initial_surface = IMG_Load( skin->image_path );
    if ( !initial_surface )
    {
        printf( "No controller image found!  Running without one...\n" );
        return;
    }

    //Make sure the surface is the right format...
    SDL_Surface * controller_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, initial_surface->w, initial_surface->h, 32,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_SetAlpha(initial_surface, 0, 0);
    SDL_BlitSurface( initial_surface, NULL, controller_surface, NULL );

    glGenTextures(1, &controller_tex );
    glBindTexture( GL_TEXTURE_2D, controller_tex );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter );
    checkError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter );
    checkError();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    checkError();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    checkError();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, controller_surface->w, controller_surface->h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, controller_surface->pixels );
    checkError();

    SDL_FreeSurface( initial_surface );
    SDL_FreeSurface( controller_surface );
}

void updateOrientation()
{
    //XXX: This function is a beast, make it less crazy.
    //
    float screenAspect = (float)destWidth/(float)destHeight;
    float emulatedAspect = (float)srcWidth/(float)srcHeight;
    
    //XXX: Hack until skins support setting their orientation
    if ( use_on_screen && skin )
      orientation = ORIENTATION_LANDSCAPE_R;
    
    switch( orientation )
    {
        case ORIENTATION_LANDSCAPE_R:
            memcpy( vertexCoords, land_r_vertexCoords, 8*sizeof(float) );
            break;
        case ORIENTATION_LANDSCAPE_L:
            memcpy( vertexCoords, land_l_vertexCoords, 8*sizeof(float) );
            break;
        default:
            printf( "Unsupported orientation: %d!\n", orientation );
            printf( "Defaulting to portrait orientation\n" );
            //fall through
            orientation = ORIENTATION_PORTRAIT;
        case ORIENTATION_PORTRAIT:
            memcpy( vertexCoords, portrait_vertexCoords, 8*sizeof(float) );
            break;
    }
    if ( orientation != ORIENTATION_PORTRAIT )
    {
        emulatedAspect = 1/emulatedAspect;//landscape has reversed aspect ratio
    }

    if (!fullscreen)
    {
      for ( int i = 0; i < 4; i++ )
      {
        vertexCoords[2*i+1] *= screenAspect / emulatedAspect;
      }

      if ( use_on_screen && skin )
      {
        float csw = skin->controller_screen_width,
              csh = skin->controller_screen_height;
        float controller_aspect = csw / csh;
        int x_offset_center = 0, y_offset_center = 0;

        // Forcing b/c I know aspect ratios > 1 :(
        float effectiveHeight = destWidth;
        float effectiveWidth = effectiveHeight / emulatedAspect;

        // So, given these, we need to scale to max csw and csh.
        float scale_max_csw = csw/effectiveWidth;
        float scale_max_csh = csh/effectiveHeight;

        // Pick the leser, and use that to calculate centering offsets
        float scale_factor;
        if ( scale_max_csw < scale_max_csh )
        {
          scale_factor = scale_max_csw;
          //We're scaling to the width, so we need to center the height:
          y_offset_center = (effectiveHeight * scale_max_csh - effectiveHeight * scale_max_csw) / 2;
        }
        else
        {
          scale_factor = scale_max_csh;
          //We're scaling to the height, so we need to center the width:
          x_offset_center = (effectiveWidth * scale_max_csw - effectiveWidth * scale_max_csh) / 2;
        }

        for ( int i = 0; i < 4; i++ )
        {
          //scale
          vertexCoords[2*i] *= scale_factor;
          vertexCoords[2*i+1] *= scale_factor;
        }

        float y_offset = 1.0 - vertexCoords[0];
        float x_offset = 1.0 - vertexCoords[1];

        //push the screen to the coordinates indicated
        y_offset -= ( (float)skin->controller_screen_y_offset / (float)destWidth ) * 2;
        x_offset -= ( (float)skin->controller_screen_x_offset / (float)destHeight ) * 2;

        //And account for centering...
        y_offset -= ( y_offset_center / (float)destWidth ) * 2;
        x_offset -= ( x_offset_center / (float)destHeight ) * 2;

        for ( int i = 0; i < 4; i++ )
        {
          //translate
          vertexCoords[2*i] += y_offset;
          vertexCoords[2*i+1] += x_offset;
        }
      }
    }

    int notification_direction;
    switch ( orientation )
    {
        case ORIENTATION_PORTRAIT:
            notification_direction = PDL_ORIENTATION_BOTTOM;
            break;
        case ORIENTATION_LANDSCAPE_L:
            notification_direction = PDL_ORIENTATION_RIGHT;
            break;
        case ORIENTATION_LANDSCAPE_R:
            notification_direction = PDL_ORIENTATION_LEFT;
            break;
        default:
            //o_O invalid orientation.
            notification_direction = PDL_ORIENTATION_BOTTOM;
            break;
    }
    //PDL_SetOrientation( notification_direction );
}

void drawSkin()
{
  // Use the program object
  glUseProgram ( programObject );
  checkError();

  glVertexAttribPointer( positionLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), controller_coords );
  checkError();
  glVertexAttribPointer( texCoordLoc, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), texCoords );

  checkError();

  glEnableVertexAttribArray( positionLoc );
  checkError();
  glEnableVertexAttribArray( texCoordLoc );
  checkError();

  checkError();

  //sampler texture unit to 0
  glBindTexture(GL_TEXTURE_2D, controller_tex);
  glUniform1i( samplerLoc, 0 );
  checkError();

  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
  checkError();

}


void GL_RenderPix(u8 * pix,int w, int h)
{
    // Update the texture dimensions/scaling depending on the rendered image size.
    if ( srcWidth != w || srcHeight != h )
    {
      srcWidth = w; srcHeight = h;
      GL_InitTexture(w,h);
      updateOrientation();
    }

    glClear( GL_COLOR_BUFFER_BIT );
    checkError();

    /*-----------------------------------------------------------------------------
     *  Background Skin
     *-----------------------------------------------------------------------------*/
    if ( use_on_screen && skin && !skin->transparent)
    {
      drawSkin();
    }

    /*-----------------------------------------------------------------------------
     *  Draw the frame of the snes
     *-----------------------------------------------------------------------------*/

    // Use the program object
    glUseProgram ( programObject );
    checkError();

    glVertexAttribPointer( positionLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertexCoords );
    checkError();
    glVertexAttribPointer( texCoordLoc, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), texCoords );

    checkError();

    glEnableVertexAttribArray( positionLoc );
    checkError();
    glEnableVertexAttribArray( texCoordLoc );
    checkError();

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D( GL_TEXTURE_2D,0,
            0,0, srcWidth,srcHeight,
            GL_RGB,GL_UNSIGNED_SHORT_5_6_5,pix);

    checkError();

    //sampler texture unit to 0
    glUniform1i( samplerLoc, 0 );
    checkError();

    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
    checkError();

    /*-----------------------------------------------------------------------------
     *  Skin Overlay
     *-----------------------------------------------------------------------------*/
    if ( use_on_screen && skin && skin->transparent)
    {
      drawSkin();
    }


    //Push to screen
    SDL_GL_SwapBuffers();
    checkError();

}

void SDL_DrawSurfaceAsGLTexture( SDL_Surface * s, float * coords )
{
  /*-----------------------------------------------------------------------------
   *  Convert the surface to a texture, and upload it
   *-----------------------------------------------------------------------------*/
  RegionTimer T("DrawSurfaceAsGLTexture");
  static GLuint surface_tex;
  static int w = -1, h = -1;

  if ( (w != s->w) || (h != s->h) ) {


    // Remove the texture
    if (surface_tex)
      glDeleteTextures( 1, &surface_tex );

    glGenTextures( 1, &surface_tex );
    checkError();
    glBindTexture( GL_TEXTURE_2D, surface_tex );
    checkError();

    int num;
    glGetIntegerv( GL_ACTIVE_TEXTURE, &num );
    assert( num == GL_TEXTURE0 );
    checkError();


    // XXX: We assume things about the surface's format that could be incorrect.
    // Add some kinda assertion

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter );
    checkError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter );
    checkError();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    checkError();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    checkError();

    w = s->w; h = s->h;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB,
      GL_UNSIGNED_BYTE, s->pixels );
  checkError();


  /*-----------------------------------------------------------------------------
   *  Now actually render it
   *-----------------------------------------------------------------------------*/

  glUseProgram ( programObject );
  checkError();

  glVertexAttribPointer( positionLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), coords );
  checkError();
  glVertexAttribPointer( texCoordLoc, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), texCoords );

  checkError();

  glEnableVertexAttribArray( positionLoc );
  checkError();
  glEnableVertexAttribArray( texCoordLoc );
  checkError();

  checkError();

  //sampler texture unit to 0
  glBindTexture(GL_TEXTURE_2D, surface_tex);
  glUniform1i( samplerLoc, 0 );
  checkError();

  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
  checkError();

  //Push to screen
  SDL_GL_SwapBuffers();
  checkError();
}

int GL_GetNativeWidth()
{
  return destWidth;
}

int GL_GetNativeHeight()
{
  return destHeight;
}
