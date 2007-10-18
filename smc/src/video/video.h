/***************************************************************************
			video.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "../core/globals.h"

/* *** *** *** *** *** *** *** *** Primitive types *** *** *** *** *** *** *** *** *** */

class GL_line
{
public:
	GL_line( void )
	: x1( 0 ), y1( 0 ), x2( 0 ), y2( 0 ) {}

	GL_line( float x1, float y1, float x2, float y2 )
	: x1( x1 ), y1( y1 ), x2( x2 ), y2( y2 ) {}

	float x1, y1;
	float x2, y2;
};

/* *** *** *** *** *** *** *** *** Color struct *** *** *** *** *** *** *** *** *** */

class Color
{
public:
	Color( void );
	Color( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 );
	Color( float r, float g, float b, float a = 1 );
	Color( Uint8 grey );
	Color( Uint32 color );
	Color( SDL_Color color );

	// Returns a matching Color value
	SDL_Color Get_SDL_Color( void );
	colour Get_cegui_colour( void );

	// compares colors
	bool Compare( const Color c );

	bool operator == ( const Color c ) const;
	bool operator == ( const SDL_Color c ) const;

	Uint8 red, green, blue, alpha;
};

/* *** *** *** *** *** *** *** Effect types *** *** *** *** *** *** *** *** *** *** */

enum Effect_Fadeout
{
	EFFECT_OUT_RANDOM,
	EFFECT_OUT_BLACK, // fade out the screen to black
	EFFECT_OUT_HORIZONTAL_VERTICAL, // random color boxes from left/right or up/down
	EFFECT_OUT_BIG_ITEM, // a big item eats the whole screen
	EFFECT_OUT_RANDOM_COLOR_BOOST, // boosts a random color in a cool fadeout
	EFFECT_OUT_TILE_PIXELATION, // big black boxes pup up random on a tile grid
	EFFECT_OUT_FIXED_COLORBOX, // small fast random alpha color boxes fades out the screen
	EFFECT_OUT_AMOUNT
};

enum Effect_Fadein
{
	EFFECT_IN_RANDOM,
	EFFECT_IN_BLACK, // fade in the screen from black
	EFFECT_IN_AMOUNT
};

/* *** *** *** *** *** *** *** Video class *** *** *** *** *** *** *** *** *** *** */

class cVideo
{
public:
	cVideo( void );
	~cVideo( void );

	/* Initializes the CEGUI System and Renderer
	*/
	void Init_CEGUI( void );
	// Initializes the basic CEGUI data and configuration
	void Init_CEGUI_data( void );
	/* Initializes all the SDL systems
	*/
	void Init_SDL( void );
	/* Initializes the screen surface
	 * shows an error if failed and exits
	*/
	void Init_Video( void );
	// Initializes the OpenGL settings
	void Init_OpenGL( void );
	// Initializes the Basic color set
	void Init_BasicColors( void );
	// initializes the scaling value for all images for the current resolution
	void Init_ImageScale( void );

	// Tests if the given resolution and bits per pixel are working/valid
	bool Test_Video( int width, int height, int bpp );

	// Resets and clears the screen
	void Clear_Screen( void );

	// Renders the Queue, GUI and Swaps Buffers
	void Render( void );

	// Toggle fullscreen video mode with windowed
	void Toggle_Fullscreen( void );

	/* Checks if the image was already loaded and returns a pointer to it else it will be loaded
	 * The returned image should not be deleted or modified.
	 */
	GL_Surface *Get_Surface( string filename, bool print_errors = 1 );

	/* Loads and returns the image
	 * use_settings : enable file settings if set to 1
	 * print_errors : print errors if image couldn't be created or loaded
	 * The returned image should be deleted if not used anymore
	*/
	GL_Surface *Load_Surface( string filename, bool use_settings = 1, bool print_errors = 1 );

	/* Creates a GL image from a SDL_Surface
	 * mipmap : create texture mipmaps
	 * force_width/height : force the given width and height
	*/
	GL_Surface *Create_Texture( SDL_Surface *surface, bool mipmap = 0, unsigned int force_width = 0, unsigned int force_height = 0 );

	/* Creates a texture into the bound GL texture
	 * mipmap : create texture mipmaps
	*/
	void Create_GL_Texture( unsigned int width, unsigned int height, void *pixels, bool mipmap = 0 );

	// Gets the Pixel color of a GL image
	Color GetPixel( int x, int y, GL_Surface *source = NULL );

	// Draws a line
	void DrawLine( GL_line *line, float z, Color *color, cLineRequest *request = NULL );
	void DrawLine( float x1, float y1, float x2, float y2, float z, Color *color, cLineRequest *request = NULL );
	/* Draw a rectangle
	* if request is NULL automatically creates the request
	*/
	void Draw_Rect( GL_rect *rect, float z, Color *color, cRectRequest *request = NULL );
	void Draw_Rect( float x, float y, float width, float height, float z, Color *color, cRectRequest *request = NULL );
	/* Draw a gradient
	* if request is NULL automatically creates the request
	*/
	void Draw_Gradient( GL_rect *rect, float z, Color *color1, Color *color2, ObjectDirection direction, cGradientRequest *request = NULL );
	void Draw_Gradient( float x, float y, float width, float height, float z, Color *color1, Color *color2, ObjectDirection direction, cGradientRequest *request = NULL );

	/* returns the scale size if the image is bigger as the given size
	 * - also upscales if only_downscale is set to 0
	*/
	float Get_scale( GL_Surface *image, float height, float width, bool only_downscale = 1 );

	// Function: Save_Screenshot
	// Save bitmap of current screen.
	void Save_Screenshot( void );

	// available OpenGL version
	float opengl_version;

	// using double buffering
	bool double_buffer;
	// using hardware surfaces
	bool hardware_surfaces;

	// screen red, green and blue color bit size
	int rgb_size[3];

	// default drawing buffer
	int default_buffer;

	// if audio initialization failed
	bool audio_init_failed;
	// if joystick initialization failed
	bool joy_init_failed;

private:
	bool initialised;
};

// sets the next fitting size for an texture dimension
unsigned int Get_GLsize( unsigned int size );

/* Draw an Screen Fadeout Effect
 * if effect is RANDOM_EFFECT a random effect is selected
*/
void Draw_Effect_out( Effect_Fadeout effect = EFFECT_OUT_RANDOM, float speed = 1 );

/* Draw an Screen Fadein Effect
 * if effect is RANDOM_EFFECT a random effect is selected
*/
void Draw_Effect_in( Effect_Fadein effect = EFFECT_IN_RANDOM, float speed = 1 );

// draws the loading screen
void Draw_Loading_Screen( void );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Video Handler
extern cVideo *pVideo;

// GUI System
extern OpenGLRenderer *pGuiRenderer;
extern System *pGuiSystem;

// Screen
extern SDL_Surface *screen;
// Default Colors
extern Color blue, darkblue, lightblue, black, blackalpha128, blackalpha192, white, whitealpha128, grey, lightgrey, lightgreyalpha64, green, lightgreen, lightgreenalpha64, yellow, greenyellow, darkgreen, red, lightred, lila, orange, lightorange;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
