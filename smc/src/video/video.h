/***************************************************************************
			video.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../globals.h"

#ifndef __VIDEO_H__
#define __VIDEO_H__

/* *** *** *** *** *** *** *** *** Primitive types *** *** *** *** *** *** *** *** *** */

struct GL_line
{
public:
	GL_line( void )
	: x1( 0 ), y1( 0 ), x2( 0 ), y2( 0 ) {}

	GL_line( double x1, double y1, double x2, double y2 )
	: x1( x1 ), y1( y1 ), x2( x2 ), y2( y2 ) {}

	double x1, y1;
	double x2, y2;
};

struct GL_point
{
public:
	GL_point( void )
	: x( 0 ), y( 0 ) {}
	
	GL_point( double x, double y )
	: x( x ), y( y ) {}
	
	GL_point( const GL_point &p )
	: x( p.x ), y( p.y ) {}

	// Returns the distance to another point
	double distance( const GL_point &p ) const
	{
    	return sqrt( ( x - p.x ) * ( x- p.x ) + ( y - p.y ) * ( y - p.y ) );
	}

	// Translate point
	GL_point &operator += ( const GL_point &p )
	{
		x += p.x; y += p.y; 
		return *this; 
	}
	
	// Translate point negatively
	GL_point &operator -= ( const GL_point &p )
	{
		x -= p.x; y -= p.y;
		return *this; 
	}
	
	// Point + Point operator
	GL_point operator + ( const GL_point &p ) const
	{
		return GL_point( x + p.x, y + p.y );
	}

	// Point - Point operator
	GL_point operator - ( const GL_point &p ) const
	{
		return GL_point( x - p.x, y - p.y ); 
	}

	// Point == Point operator
	bool operator == ( const GL_point &p ) const
	{
		return ( x == p.x ) && ( y == p.y ); 
	}

	// Point != Point operator
	bool operator != ( const GL_point &p ) const
	{
		return ( x != p.x ) || ( y != p.y ); 
	}

	double x, y;
};

struct GL_rect
{
public:
	GL_rect( void )
	: x( 0 ), y( 0 ), w( 0 ), h( 0 ) {}

	GL_rect( double x, double y, double w, double h )
	: x( x ), y( y ), w( w ), h( h ) {}

	// returns a SDL_Rect
	SDL_Rect Get_Rect( void )
	{
		SDL_Rect rect;
		rect.x = (Sint16)x;
		rect.y = (Sint16)y;
		rect.w = (Uint16)w;
		rect.h = (Uint16)h;

		return rect;
	}

	// returns a SDL_Rect with the given position
	SDL_Rect Get_Rect_pos( double posx, double posy )
	{
		SDL_Rect rect;
		rect.x = (Sint16)( x + posx );
		rect.y = (Sint16)( y + posy );
		rect.w = (Uint16)w;
		rect.h = (Uint16)h;

		return rect;
	}

	// returns the point in the middle of the rect
	GL_point Get_pos_middle( void )
	{
		return GL_point( x + ( w / 2 ), y + ( h / 2 ) );
	}

	// clears the data
	void clear( void )
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	double x, y;
	double w, h;
};

/* *** *** *** *** *** *** *** *** Color struct *** *** *** *** *** *** *** *** *** */

struct Color
{
	Color( void );
	Color( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 );
	Color( float r, float g, float b, float a = 1 );
	Color( Uint8 grey );
	Color( Uint32 color );
	Color( SDL_Color color );

	// Parses the given color value
	void Parse( Uint32 mapcolor );
	void Parse( SDL_Color color );
	void Parse_uint8( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 );
	void Parse_float( float r, float g, float b, float a = 1 );

	// Returns a matching Color value
	SDL_Color Get_SDL_Color( void );
	colour Get_cegui_colour( void );

	// compares colors
	bool Compare( const Color c );

	bool operator == ( const Color c ) const;
	bool operator == ( const SDL_Color c ) const;

	Uint8 red, green, blue, alpha;
};

/* *** *** *** *** *** *** *** *** OpenGL Surface *** *** *** *** *** *** *** *** *** */

struct GL_Surface
{
public:
	GL_Surface( void );
	~GL_Surface( void );

	/* Blit the surface on the given position
	* can optional have rotation, scaling and shadowing
	*/
	void Blit( double x, double y, Color *shadow = NULL, double rotx = 0, double roty = 0, double rotz = 0, double scale = 1, bool no_globalscale = 0 );

	/* copies an image and returns it
	 * works only for images
	*/
	GL_Surface *Copy( void );

	// GL texture
	unsigned int image;
	// internal drawing offset
	double int_x, int_y;
	// drawing dimension
	double w, h;
	// texture dimension
	unsigned int tex_w, tex_h;
	// internal rotation
	double base_rotx, base_roty, base_rotz;
	// image collision data
	GL_point col_pos;
	double col_w, col_h;

	// origin if created from a file
	string filename;
	// should the image be deleted
	bool auto_del_img;
};

/* *** *** *** *** *** *** *** Effect types *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	RANDOM_EFFECT,
	BLACK_FADEOUT, // fade out the screen to black
	HORIZONTAL_VERTICAL_FADE, // random color boxes from left/right or up/down
	BIG_MUSHROOM, // a big mushroom eats the whole screen
	RANDOM_COLOR_BOOST, // boosts a random color in a cool fadeout
	TILE_PIXELATION, // big black boxes pup up random on a tile grid
	FIXED_COLORBOX_FADEOUT, // small fast random alpha color boxes fades out the screen
	TOTAL_FADE_EFFECTS
} FadeoutEffect;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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


// Returns the current Video flags
int GetVideoflags( void );

// Tests if the given resolution and bits per pixel are working/valid
bool TestVideo( int width, int height, int bpp );

// Resets and clears the screen
void ResetScreen( void );

// Toggle fullscreen video mode with windowed
void Toggle_Fullscreen( void );

/* Checks if the image was already loaded and returns a pointer to it
 * else it will be loaded
 * The returned image should not be deleted or modified.
 */
GL_Surface *GetImage( string filename );

/* Loads and returns the image
 * and sets the file settings by default
 * The returned image should be deleted if not used anymore
*/
GL_Surface *LoadTexture( string filename, bool use_settings = 1 );

/* Creates a GL image from a SDL_Surface
*/
GL_Surface *CreateTexture( SDL_Surface *surface );

// sets the next fitting size for an texture dimension
unsigned int Get_GLsize( unsigned int size );

// Returns the values as a color class
Color Get_Color( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 );

// Gets the Pixel color of an GL image
Color GetPixel( int x, int y, GL_Surface *source = NULL );
// Puts a pixel with the color on a GL image
void DrawPixel( double x, double y, Color *color, GL_Surface *target = NULL );
// Draws a line
void DrawLine( GL_line *line, Color *color, GL_Surface *target = NULL );
void DrawLine( double x1, double y1, double x2, double y2, Color *color, GL_Surface *target = NULL );
// Draws a rectangle
void DrawRect( GL_rect *rect, Color *color, GL_Surface *target = NULL );
void DrawRect( double x, double y, double width, double height, Color *color, GL_Surface *target = NULL );
// Renders the given text into a new surface
GL_Surface *RenderText( TTF_Font *font, string text, Color color = (Uint8)0 );

/* returns the scale size if the image is bigger as the given size
 * - also upscales if only_downscale is set to 0
*/
double Get_scale( GL_Surface *image, double height, double width, bool only_downscale = 1 );

// Draws a defined or if effect is RANDOM_EFFECT a random effect will be selected
void DrawEffect( FadeoutEffect effect = RANDOM_EFFECT, double speed = 1 );

// draws the loading screen
void DrawLoadingScreen( void );

#endif
