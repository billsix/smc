/***************************************************************************
 * video.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_VIDEO_H
#define SMC_VIDEO_H

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
	CEGUI::colour Get_cegui_Color( void );

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

/* *** *** *** *** *** *** *** CEGUI renderer fake class *** *** *** *** *** *** *** *** *** *** */

class cFake_Renderer : public CEGUI::Renderer
{
public:
	cFake_Renderer( void );
	virtual ~cFake_Renderer( void );

	virtual	void addQuad(const CEGUI::Rect& dest_rect, float z, const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect, const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode) {};
	virtual	void doRender(void) {};
	virtual	void clearRenderList(void) {};
	virtual void setQueueingEnabled(bool setting) {};
	virtual	CEGUI::Texture *createTexture(void) { return NULL; };
	virtual	CEGUI::Texture *createTexture(const CEGUI::String& filename, const CEGUI::String& resourceGroup) { return NULL; };
	virtual	CEGUI::Texture *createTexture(float size) { return NULL; };
	virtual	void destroyTexture(CEGUI::Texture* texture) {};
	virtual void destroyAllTextures(void) {};
	virtual bool isQueueingEnabled(void) const { return 0; };
	virtual float getWidth(void) const { return 0; };
	virtual float getHeight(void) const { return 0; };
	virtual CEGUI::Size getSize(void) const { return CEGUI::Size();};
	virtual CEGUI::Rect getRect(void) const { return CEGUI::Rect();};
	virtual	unsigned int getMaxTextureSize(void) const { return 0; };
	virtual	unsigned int getHorzScreenDPI(void) const { return 0; };
	virtual	unsigned int getVertScreenDPI(void) const { return 0; };
};

/* *** *** *** *** *** *** *** Video class *** *** *** *** *** *** *** *** *** *** */

class cVideo
{
public:
	cVideo( void );
	~cVideo( void );

	// Initialize CEGUI system and renderer fake for the preferences xml parser
	void Init_CEGUI_Fake( void );
	// Delete the CEGUI system and renderer fake
	void Delete_CEGUI_Fake( void );
	// Initialize the CEGUI System and Renderer
	void Init_CEGUI( void );
	// Initialize the basic CEGUI data and configuration
	void Init_CEGUI_Data( void );
	// Initialize all the SDL systems
	void Init_SDL( void );
	/* Initialize the screen surface
	 * reload_textures_from_file: if set reloads all textures from the original file
	 * use_preferences: if set use user preferences settings
	 * shows an error if failed and exits
	*/
	void Init_Video( bool reload_textures_from_file = 0, bool use_preferences = 1 );
	// Initialize OpenGL with current settings
	void Init_OpenGL( void );
	// Initialize Geometry with current settings
	void Init_Geometry( void );
	// Initialize Texture detail settings
	void Init_Texture_Detail( void );
	// initialize the up/down scaling value for the current resolution ( image/mouse scale )
	void Init_Resolution_Scale( void );
	/* Initialize the image cache and recreates cache if game version changed
	 * recreate : if set force cache recreation
	 * draw_gui : if set use the loading screen gui for drawing
	*/
	void Init_Image_Cache( bool recreate = 0, bool draw_gui = 0 );

	/* Test if the given resolution and bits per pixel are valid
	 * if flags aren't set they are auto set from the preferences
	 * returns 0 if the requested mode is not supported under any bit depth,
	 * or returns the bits-per-pixel of the closest available mode
	*/
	int Test_Video( int width, int height, int bpp, int flags = 0 );

	/* Get the supported resolutions for the current screen pixel format
	 * if flags aren't set they are set to the default
	*/
	vector<cSize_Int> Get_Supported_Resolutions( int flags = 0 );

	// Reset and clear the screen
	void Clear_Screen( void );

	// Render the Queue, GUI and Swap Buffers
	void Render( void );

	// Toggle fullscreen video mode ( new mode is set to preferences )
	void Toggle_Fullscreen( void );

	/* Check if the image was already loaded and returns a pointer to it else it will be loaded
	 * The returned image should not be deleted or modified.
	 */
	cGL_Surface *Get_Surface( string filename, bool print_errors = 1 );

	// Software image
	class cSoftware_Image
	{
	public:
		cSoftware_Image( void )
		{
			sdl_surface = NULL;
			settings = NULL;
		};

		SDL_Surface *sdl_surface;
		cImage_settings_data *settings;
	};

	/* Load and return the software image
	 * load_settings : enable file settings if set to 1
	 * print_errors : print errors if image couldn't be created or loaded
	 * The returned data in the software image should be deleted if not used anymore
	*/
	cSoftware_Image Load_Image( string filename, bool load_settings = 1, bool print_errors = 1 );

	/* Load and return the hardware image
	 * use_settings : enable file settings if set to 1
	 * print_errors : print errors if image couldn't be created or loaded
	 * The returned image should be deleted if not used anymore
	*/
	cGL_Surface *Load_GL_Surface( string filename, bool use_settings = 1, bool print_errors = 1 );

	/* Convert to a software image with a power of 2 size and 32 bits per pixel
	 * surface : the source image which gets converted if needed
	 * use the returned image after this conversion
	*/
	SDL_Surface *Convert_To_Final_Software_Image( SDL_Surface *surface );

	/* Create a GL image from a SDL_Surface
	 * mipmap : create texture mipmaps
	 * force_width/height : force the given width and height
	*/
	cGL_Surface *Create_Texture( SDL_Surface *surface, bool mipmap = 0, unsigned int force_width = 0, unsigned int force_height = 0 );

	/* Create a texture into the bound GL texture
	 * mipmap : create texture mipmaps
	*/
	void Create_GL_Texture( unsigned int width, unsigned int height, void *pixels, bool mipmap = 0 );

	// Get the Pixel color of a GL image
	Color Get_Pixel( int x, int y, cGL_Surface *source = NULL );

	// Draw a line
	void Draw_Line( GL_line *line, float z, const Color *color, cLineRequest *request = NULL );
	void Draw_Line( float x1, float y1, float x2, float y2, float z, const Color *color, cLineRequest *request = NULL );
	/* Draw a rectangle
	 * if request is NULL automatically creates the request
	*/
	void Draw_Rect( GL_rect *rect, float z, const Color *color, cRectRequest *request = NULL );
	void Draw_Rect( float x, float y, float width, float height, float z, const Color *color, cRectRequest *request = NULL );
	/* Draw a gradient
	 * if request is NULL automatically creates the request
	*/
	void Draw_Gradient( GL_rect *rect, float z, const Color *color_1, const Color *color_2, ObjectDirection direction, cGradientRequest *request = NULL );
	void Draw_Gradient( float x, float y, float width, float height, float z, const Color *color_1, const Color *color_2, ObjectDirection direction, cGradientRequest *request = NULL );
	// Draw a circle
	//void Draw_Circle( GL_circle *line, float z, Color *color, cCircleRequest *request = NULL );
	void Draw_Circle( float x, float y, float radius, float z, Color *color, cCircleRequest *request = NULL );

	/* Return the scale size if the image is bigger as the given size
	 * also upscales if only_downscale is set to 0
	*/
	float Get_Scale( cGL_Surface *image, float width, float height, bool only_downscale = 1 );

	/* Downscale an image
	 * Can be used for creating MIPmaps
	 * The incoming image should be a power-of-two sized
	*/
	bool Downscale_Image( const unsigned char* const orig, int width, int height, int channels, unsigned char* resampled, int block_size_x, int block_size_y );

	// Save an image of the current screen
	void Save_Screenshot( void );
	// Save data as png image
	void Save_Surface( string filename, unsigned char *data, unsigned int width, unsigned int height, unsigned int bpp = 4, bool reverse_data = 0 );

	// available OpenGL version
	float opengl_version;

	// using double buffering
	bool double_buffer;
	// using hardware surfaces
	bool hardware_surfaces;

	// screen red, green and blue color bit size
	int rgb_size[3];

	// default drawing buffer
	GLint default_buffer;

	// if audio initialization failed
	bool audio_init_failed;
	// if joystick initialization failed
	bool joy_init_failed;

	// active image cache directory
	string imgcache_dir;

	// geometry detail level 0.0 - 1.0
	float geometry_detail;
	// texture detail level 0.0 - 1.0
	float texture_detail;
private:
	// if set video is initialized successfully
	bool initialised;
};

/* Draw an Screen Fadeout Effect
 * if effect is RANDOM_EFFECT a random effect is selected
*/
void Draw_Effect_Out( Effect_Fadeout effect = EFFECT_OUT_RANDOM, float speed = 1 );

/* Draw an Screen Fadein Effect
 * if effect is RANDOM_EFFECT a random effect is selected
*/
void Draw_Effect_In( Effect_Fadein effect = EFFECT_IN_RANDOM, float speed = 1 );

// initialize loading screen
void Loading_Screen_Init( void );
// set the loading screen info string
void Loading_Screen_Draw_Text( string str_info = "Loading" );
// exit loading screen
void Loading_Screen_Exit( void );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Video Handler
extern cVideo *pVideo;

// GUI System
extern CEGUI::OpenGLRenderer *pGuiRenderer;
extern CEGUI::System *pGuiSystem;

// Screen
extern SDL_Surface *screen;
// Generic Colors
static const Color blue = Color( static_cast<Uint8>(150), 200, 225 );
static const Color darkblue = Color( static_cast<Uint8>(0), 0, 128 );
static const Color lightblue = Color( static_cast<Uint8>(41), 167, 255 );
static const Color black = Color( static_cast<Uint8>(0), 0, 0 );
static const Color blackalpha128 = Color( static_cast<Uint8>(0), 0, 0, 128 );
static const Color blackalpha192 = Color( static_cast<Uint8>(0), 0, 0, 192 );
static const Color white = Color( static_cast<Uint8>(255), 255, 255 );
static const Color whitealpha128 = Color( static_cast<Uint8>(255), 255, 255, 128 );
static const Color grey = Color( static_cast<Uint8>(128), 128, 128 );
static const Color lightgrey = Color( static_cast<Uint8>(64), 64, 64 );
static const Color lightgreyalpha64 = Color( static_cast<Uint8>(64), 64, 64, 64 );
static const Color green = Color( static_cast<Uint8>(0), 230, 0 );
static const Color lightgreen = Color( static_cast<Uint8>(20), 253, 20 );
static const Color lightgreenalpha64 = Color( static_cast<Uint8>(30), 230, 30, 64 );
static const Color yellow = Color( static_cast<Uint8>(255), 245, 10 );
static const Color greenyellow = Color( static_cast<Uint8>(154), 205, 50 );
static const Color darkgreen = Color( static_cast<Uint8>(1), 119, 34 );
static const Color red = Color( static_cast<Uint8>(250), 0, 0 );
static const Color lightred = Color( static_cast<Uint8>(255), 40, 20 );
static const Color lila = Color( static_cast<Uint8>(200), 0, 255 );
static const Color orange = Color( static_cast<Uint8>(248), 191, 38 );
static const Color lightorange = Color( static_cast<Uint8>(255), 220, 100 );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
