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
#include "VBA.h"
#include "pdl.h"
#include "Controller.h"

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

//#define DEBUG_GL

#ifdef DEBUG_GL
void checkError()
{
    /* Check for error conditions. */
    GLenum gl_error = glGetError( );

    if( gl_error != GL_NO_ERROR ) {
        fprintf( stderr, "VBA: OpenGL error: %x\n", gl_error );
        while(1);
        exit( 1 );
    }

    char * sdl_error = SDL_GetError( );

    if( sdl_error[0] != '\0' ) {
        fprintf(stderr, "VBA: SDL error '%s'\n", sdl_error);
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

    surface = SDL_SetVideoMode( 320, 480, 32,
        SDL_OPENGL|
        (fullscreen ? SDL_FULLSCREEN : 0));

    if(surface == NULL) {
      systemMessage(0, "Failed to set video mode");
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
        "  vec4 color = texture2D( s_texture, v_texCoord );  \n"
        "  gl_FragColor.r = color.b;                         \n"
        "  gl_FragColor.g = color.g;                         \n"
        "  gl_FragColor.b = color.r;                         \n"
        "  gl_FragColor.a = 1.0;                             \n"
        "}                                                   \n";

    // Load the shaders and get a linked program object
    programObject = esLoadProgram ( ( char *)vShaderStr, (char *)fShaderStr );
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


void GL_InitTexture()
{
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcWidth, srcHeight, 0, GL_RGBA,
            GL_UNSIGNED_SHORT_5_5_5_1, NULL );
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
    //Create RGB surface and copy controller into it
    SDL_Surface * controller_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, initial_surface->w, initial_surface->h, 24,
            0xff0000, 0x00ff00, 0x0000ff, 0);
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, controller_surface->w, controller_surface->h, 0, GL_RGB,
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
    
    //XXX: 'orientation' is invariant as far the rendering loop goes; move
    //the corresponding invariant results (vertexCoords, etc)
    //to be calculated outside this method
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

    for ( int i = 0; i < 4; i++ )
    {
        vertexCoords[2*i+1] *= screenAspect / emulatedAspect;
    }

    if ( use_on_screen && orientation == ORIENTATION_LANDSCAPE_R && skin )
    {
        float controller_aspect = (float)skin->controller_screen_width / (float)skin->controller_screen_height;
        float scale_factor;
        if ( (float)srcHeight * controller_aspect  > (float)skin->controller_screen_height )
        {
            //width is limiting factor
            scale_factor = ( (float)skin->controller_screen_height / (float)destWidth );
        }
        else
        {
            //height is limiting factor
            //'effectiveWidth' b/c we already scaled previously
            //and we don't fill the screen due to aspect ratio
            float effectiveWidth = (float)destWidth / emulatedAspect;
            scale_factor = ( (float)skin->controller_screen_width / effectiveWidth );
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

        for ( int i = 0; i < 4; i++ )
        {
            //translate
            vertexCoords[2*i] += y_offset;
            vertexCoords[2*i+1] += x_offset;
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
    PDL_SetOrientation( notification_direction );
}

void GL_RenderPix(u8 * pix)
{
    glClear( GL_COLOR_BUFFER_BIT );
    checkError();

    /*-----------------------------------------------------------------------------
     *  Overlay
     *-----------------------------------------------------------------------------*/
    if ( use_on_screen && orientation == ORIENTATION_LANDSCAPE_R && skin )
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


    /*-----------------------------------------------------------------------------
     *  Draw the frame of the gb(c/a)
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
            GL_RGBA,GL_UNSIGNED_SHORT_5_5_5_1,pix);

    checkError();

    //sampler texture unit to 0
    glUniform1i( samplerLoc, 0 );
    checkError();

    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
    checkError();

    //Push to screen
    SDL_GL_SwapBuffers();
    checkError();

}

void SDL_DrawSurfaceAsGLTexture( SDL_Surface * s, float * coords )
{

  /*-----------------------------------------------------------------------------
   *  Convert the surface to a texture, and upload it
   *-----------------------------------------------------------------------------*/
  static GLuint surface_tex;
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

  // Remove the texture
  glDeleteTextures( 1, &surface_tex );
}


