/***************************************************************************
			video.cpp  -  General video functions
                             -------------------
    copyright            : (C) 2005 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../video/video.h"
#include "../gui/hud.h"
#include "../user/preferences.h"
#include "../core/framerate.h"
#include "../video/font.h"
#include "../core/game_core.h"
#include "../video/img_settings.h"
#include "../core/camera.h"
#include "../input/mouse.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../core/main.h"

#include "CEGUIDefaultResourceProvider.h"

/* *** *** *** *** *** *** *** *** Color struct *** *** *** *** *** *** *** *** *** */

Color :: Color( void )
{
	red = 0;
	green = 0;
	blue = 0;
	alpha = 255;
};

Color :: Color( Uint8 r, Uint8 g, Uint8 b, Uint8 a /* = 255 */ )
{
	red = r;
	green = g;
	blue = b;
	alpha = a;
};

Color :: Color( float r, float g, float b, float a /* = 1 */ )
{
	red = (Uint8)(r * 255);
	green = (Uint8)(g * 255);
	blue = (Uint8)(b * 255);
	alpha = (Uint8)(a * 255);
}

Color :: Color( Uint8 grey )
{
	red = grey;
	green = grey;
	blue = grey;
	alpha = 255;
};

Color :: Color( Uint32 color )
{
	//SDL_GetRGBA( mapcolor, format, &red, &green, &blue, &alpha );
};

Color :: Color( SDL_Color color )
{
	red = color.r;
	green = color.g;
	blue = color.b;
	alpha = 255;
}

SDL_Color Color :: Get_SDL_Color( void )
{
	SDL_Color color;
	color.r = red;
	color.g = green;
	color.b = blue;
	return color;
}

colour Color :: Get_cegui_colour( void )
{
	return colour( (float)red / 255, (float)green / 255, (float)blue / 255, (float)alpha / 255 );
}

bool Color :: Compare( const Color color )
{
	return red == color.red && green == color.green && blue == color.blue && alpha == color.alpha;
}

bool Color :: operator == ( const Color color ) const
{
	return red == color.red && green == color.green && blue == color.blue && alpha == color.alpha;
}

bool Color :: operator == ( const SDL_Color color ) const
{
	return red == color.r && green == color.g && blue == color.b;
}

/* *** *** *** *** *** *** *** Video class *** *** *** *** *** *** *** *** *** *** */

cVideo :: cVideo( void )
{
	opengl_version = 0;

	double_buffer = 0;
	hardware_surfaces = 0;

	rgb_size[0] = 0;
	rgb_size[1] = 0;
	rgb_size[2] = 0;

	default_buffer = GL_BACK;
	
	audio_init_failed = 0;
	joy_init_failed = 0;
	initialised = 0;
}

cVideo :: ~cVideo( void )
{

}

void cVideo :: Init_CEGUI( void )
{
	try
	{
		pGuiRenderer = new OpenGLRenderer( 0, GAME_RES_W, GAME_RES_H );
		pGuiSystem = new System( pGuiRenderer, NULL, NULL, NULL, "", user_data_dir + "/cegui.log" );
	}
	// catch CEGUI Exceptions
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occurred : %s", ex.getMessage().c_str() );
		exit( 0 );
	}
}

void cVideo :: Init_CEGUI_data( void )
{
#ifdef _DEBUG
	Logger::getSingleton().setLoggingLevel( Informative );
#else
	Logger::getSingleton().setLoggingLevel( Errors );
#endif

	// initialise the required Resource Provider directories
	DefaultResourceProvider *rp = (DefaultResourceProvider *)System::getSingleton().getResourceProvider();

	rp->setResourceGroupDirectory( "schemes", DATA_DIR "/" GUI_SCHEME_DIR "/" );
	rp->setResourceGroupDirectory( "imagesets", DATA_DIR "/" GUI_IMAGESET_DIR "/" );
	rp->setResourceGroupDirectory( "fonts", DATA_DIR "/" GUI_FONT_DIR "/" );
	rp->setResourceGroupDirectory( "looknfeels", DATA_DIR "/" GUI_LOOKNFEEL_DIR "/" );
	rp->setResourceGroupDirectory( "layouts", DATA_DIR "/" GUI_LAYOUT_DIR "/" );
#ifdef CEGUI_WITH_XERCES
	// This is needed for Xerces to specify the schemas location
	rp->setResourceGroupDirectory( "schemas", SCHEMA_DIR "/" );
#endif

	// set the default resource groups to be used
	Scheme::setDefaultResourceGroup( "schemes" );
	Imageset::setDefaultResourceGroup( "imagesets" );
	Font::setDefaultResourceGroup( "fonts" );
	WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
	WindowManager::setDefaultResourceGroup( "layouts" );
	// only needed for Xerces
#if defined( CEGUI_WITH_XERCES ) && ( CEGUI_DEFAULT_XMLPARSER == XercesParser )
	XercesParser::setSchemaDefaultResourceGroup( "schemas" );
#endif

	// load the scheme file, which auto-loads the imageset
	try
	{
		SchemeManager::getSingleton().loadScheme( "TaharezLook.scheme" );
	}
	// catch CEGUI Exceptions
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occurred : %s", ex.getMessage().c_str() );
		exit( 0 );
	}

	// first font loaded automatically becomes the default font
	try
	{
		// load best fitting texture font
		if( pPreferences->Screen_W >= 1024 )
		{
			FontManager::getSingleton().createFont( "bluebold1024_medium.font" );
			FontManager::getSingleton().createFont( "bluebold1024_big.font" );
		}
		else if( pPreferences->Screen_W >= 800 )
		{
			FontManager::getSingleton().createFont( "bluebold800_medium.font" );
			FontManager::getSingleton().createFont( "bluebold800_big.font" );
		}
		else
		{
			FontManager::getSingleton().createFont( "bluebold640_medium.font" );
			FontManager::getSingleton().createFont( "bluebold640_big.font" );
		}
	}
	// catch CEGUI Exceptions
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occurred : %s", ex.getMessage().c_str() );
		exit( 0 );
	}

	// default mouse cursor
	pGuiSystem->setDefaultMouseCursor( "TaharezLook", "MouseArrow" );
	// force new mouse image
	MouseCursor::getSingleton().setImage( &ImagesetManager::getSingleton().getImageset( "TaharezLook" )->getImage( "MouseArrow" ) );
	// hide CEGUI mouse always because we render the mouse manually
	MouseCursor::getSingleton().hide();
	// default tooltip
	pGuiSystem->setDefaultTooltip( "TaharezLook/Tooltip" );
	// create default root window
	Window *window_root = WindowManager::getSingleton().loadWindowLayout( "default.layout" );
	pGuiSystem->setGUISheet( window_root );
}

void cVideo :: Init_SDL( void )
{
	if( SDL_Init( SDL_INIT_VIDEO ) == -1 )
	{
		printf( "Error : SDL initialization failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}

	atexit( SDL_Quit );

	if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) == -1 )
	{
		printf( "Warning : SDL Joystick initialization failed\nReason : %s\n", SDL_GetError() );
		joy_init_failed = 1;
	}
	else
	{
		joy_init_failed = 0;
	}

	if( SDL_InitSubSystem( SDL_INIT_AUDIO ) == -1 )
	{
		printf( "Warning : SDL Audio initialization failed\nReason : %s\n", SDL_GetError() );
		audio_init_failed = 1;
	}
	else
	{
		audio_init_failed = 0;
	}

	SDL_EnableUNICODE( 1 );
	// hide by default
	SDL_ShowCursor( SDL_DISABLE );
}

void cVideo :: Init_Video( void )
{
	// set the video flags
	int flags = SDL_OPENGL | SDL_SWSURFACE;

	// only enter fullscreen if already initialized
	if( initialised && pPreferences->Fullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}

	int screen_w = pPreferences->Screen_W;
	int screen_h = pPreferences->Screen_H;
	int screen_bpp = pPreferences->Screen_Bpp;
	int screen_rgb_size[3];

	// first initialization with SDL defaults
	if( !initialised )
	{
		screen_w = 800;
		screen_h = 600;
		screen_bpp = 0;

		// Set Caption
		SDL_WM_SetCaption( CAPTION, NULL );
		// Set Icon
		SDL_Surface *icon = IMG_Load( DATA_DIR "/" GAME_ICON_DIR "/window_32.png" );
		SDL_WM_SetIcon( icon, NULL );
		SDL_FreeSurface( icon );
	}

	// set bits per pixel sizes
	if( screen_bpp == 8 )
	{
		screen_rgb_size[0] = 3;
		screen_rgb_size[1] = 3;
		screen_rgb_size[2] = 2;
	}
	else if( screen_bpp == 32 )
	{
		screen_rgb_size[0] = 8;
		screen_rgb_size[1] = 8;
		screen_rgb_size[2] = 8;
	}
	else if( screen_bpp == 15 )
	{
		screen_rgb_size[0] = 5;
		screen_rgb_size[1] = 5;
		screen_rgb_size[2] = 5;
	}
	else // 16 and default
	{
		screen_rgb_size[0] = 5;
		screen_rgb_size[1] = 6;
		screen_rgb_size[2] = 5;
	}

	// request settings
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, screen_rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, screen_rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, screen_rgb_size[2] );
	// hangs on 16 bit
	//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 ); 
	// not yet needed
	//SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	// if VSync is enabled
	if( initialised && pPreferences->vsync )
	{
		SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
	}

	// if reinitialization
	if( initialised )
	{
		// save textures
		pImageManager->Grab_Textures();
		pFont->Grab_Textures();
		pGuiRenderer->grabTextures();
		pImageManager->Delete_Hardware_Textures();
	}

	// SDL handles the screen surface memory management
	screen = SDL_SetVideoMode( screen_w, screen_h, screen_bpp, flags );

	if( !screen )
	{
		printf( "Error : Screen mode creation failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}

	// check fullscreen
	if( initialised && pPreferences->Fullscreen )
	{
		int is_fullscreen = ( ( screen->flags & SDL_FULLSCREEN ) == SDL_FULLSCREEN );

		if( !is_fullscreen )
		{
			printf( "Warning : Fullscreen mode not set\n" );
		}
	}

	// check Double Buffering
	int is_double_buffer;
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &is_double_buffer );

	if( is_double_buffer )
	{
		double_buffer = 1;
	}
	else
	{
		double_buffer = 0;

		if( initialised )
		{
			printf( "Warning : not using Double Buffering\n" );
		}
	}

	// check VSync
	if( pPreferences->vsync && initialised )
	{
		int is_vsync;
		// seems to return always true even if not available
		SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &is_vsync );

		if( !is_vsync )
		{
			printf( "Warning : not using VSync\n" );
		}
	}

	// get color bit size
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &rgb_size[0] );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &rgb_size[1] );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &rgb_size[2] );

	// check color bit size
	if( initialised )
	{
		if( rgb_size[0] < screen_rgb_size[0] )
		{
			printf( "Warning : smaller red bit size %d as requested %d\n", rgb_size[0], screen_rgb_size[0] );
		}

		if( rgb_size[1] < screen_rgb_size[1] )
		{
			printf( "Warning : smaller green bit size %d as requested %d\n", rgb_size[1], screen_rgb_size[1] );
		}

		if( rgb_size[2] < screen_rgb_size[2] )
		{
			printf( "Warning : smaller blue bit size %d as requested %d\n", rgb_size[2], screen_rgb_size[2] );
		}
	}

	// remember default buffer
	glGetIntegerv( GL_DRAW_BUFFER, &default_buffer );

	/* check if accelerated visual
	int accelerated = 0;
	SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &accelerated );
	printf( "accel %d\n", accelerated );*/

	// if reinitialization
	if( initialised )
	{
		// reset highest texture id
		pImageManager->high_texture_id = 0;

		// reinit opengl
		Init_OpenGL();

		// restore textures
		pGuiRenderer->restoreTextures();
		pImageManager->Restore_Textures();
		pFont->Restore_Textures();
	}
	// finished first initialization
	else
	{
		// get opengl version
		opengl_version = (float)atof( reinterpret_cast<const char*>(glGetString( GL_VERSION )) );

		if( opengl_version < 1.4f )
		{
			printf( "Warning : OpenGL Version %f below optimal version 1.4 and higher\n", opengl_version );
		}
	}

	initialised = 1;
}

void cVideo :: Init_OpenGL( void )
{
    // viewport should cover the whole screen
    glViewport( 0, 0, pPreferences->Screen_W, pPreferences->Screen_H );

    // Camera projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	// Set up an Ortho Screen
	glOrtho( 0, (float)pPreferences->Screen_W, (float)pPreferences->Screen_H, 0, -1, 1 );
	
    // matrix operations
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Smooth Shading
	glShadeModel( GL_SMOOTH );
	// Line Smoothing
	glEnable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    // clear color
    glClearColor( 0, 0, 0, 1 );

    // Z-Buffer
	glEnable( GL_DEPTH_TEST );

	// Depth function
	glDepthFunc( GL_LEQUAL );
	// Depth Buffer Setup
	glClearDepth( 1 );

	// Blending
	glEnable( GL_BLEND );
	// Blending function
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Alpha
	glEnable( GL_ALPHA_TEST );
	// Alpha function
	glAlphaFunc( GL_GREATER, 0.01f );

	// Perspective Calculations
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    // back-facing polygons
    //glDisable( GL_CULL_FACE );

	Init_ImageScale();
}

void cVideo :: Init_BasicColors( void )
{
	blue = Color( (Uint8)150, 200, 225 );
	darkblue = Color( (Uint8)0, 0, 128 );
	lightblue = Color( (Uint8)41, 167, 255 );
	black = Color( (Uint8)0, 0, 0 );
	blackalpha128 = Color( (Uint8)0, 0, 0, 128 );
	blackalpha192 = Color( (Uint8)0, 0, 0, 192 );
	white = Color( (Uint8)255, 255, 255 );
	whitealpha128 = Color( (Uint8)255, 255, 255, 128 );
	grey = Color( (Uint8)128, 128, 128 );
	lightgrey = Color( (Uint8)64, 64, 64 );
	lightgreyalpha64 = Color( (Uint8)64, 64, 64, 64 );
	green = Color( (Uint8)0, 230, 0 );
	lightgreen = Color( (Uint8)20, 253, 20 );
	lightgreenalpha64 = Color( (Uint8)30, 230, 30, 64 );
	yellow = Color( (Uint8)255, 245, 10 );
	greenyellow = Color( (Uint8)154, 205, 50 );
	darkgreen = Color( (Uint8)1, 119, 34 );
	red = Color( (Uint8)250, 0, 0 );
	lightred = Color( (Uint8)255, 40, 20 );
	lila = Color( (Uint8)200, 0, 255 );
	orange = Color( (Uint8)248, 191, 38 );
	lightorange = Color( (Uint8)255, 220, 100 );
}

void cVideo :: Init_ImageScale( void )
{
	global_scalex = (float)pPreferences->Screen_W / (float)GAME_RES_W;
	global_scaley = (float)pPreferences->Screen_H / (float)GAME_RES_H;
}

bool cVideo :: Test_Video( int width, int height, int bpp )
{
	// set the video flags
	int flags = SDL_OPENGL;

	// only in fullscreen if already initialized
	if( pPreferences->Fullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}

	if( SDL_VideoModeOK( width, height, bpp, flags ) == 0 )
	{
		return 0;
	}

	return 1;
}

void cVideo :: Clear_Screen( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
}

void cVideo :: Render( void )
{
	pRenderer->Render();
	pGuiSystem->renderGUI();
	pRenderer_GUI->Render();
	pMouseCursor->Render();
	SDL_GL_SwapBuffers();
}

void cVideo :: Toggle_Fullscreen( void )
{
	// toggle fullscreen
	pPreferences->Fullscreen = !pPreferences->Fullscreen;

	// save clear color
	GLclampf clear_color[4];
	glGetFloatv( GL_COLOR_CLEAR_VALUE, clear_color );

#ifdef WIN32
	// windows needs reinitialization
	pVideo->Init_Video();
#else
	// works only for X11 platforms
	SDL_WM_ToggleFullScreen( screen );
#endif

	// set back clear color
	glClearColor( clear_color[0], clear_color[1], clear_color[2], clear_color[3] );
}

GL_Surface *cVideo :: Get_Surface( string filename, bool print_errors /* = 1 */ )
{
	// .settings file type can't be used directly
	if( filename.find( ".settings" ) != string::npos )
	{
		filename.erase( filename.find( ".settings" ) );
		filename.insert( filename.length(), ".png" );
	}

	// pixmaps dir must be given
	if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR ) == string::npos )
	{
		filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	}

	// check if already loaded
	GL_Surface *image = pImageManager->Get_Pointer( filename );
	// already loaded
	if( image )
	{
		return image;
	}
	
	// load new image
	image = Load_Surface( filename, 1, print_errors );
	// add new image
	if( image )
	{
		pImageManager->Add( image );
	}
	
	return image;
}

GL_Surface *cVideo :: Load_Surface( string filename, bool use_settings /* = 1 */, bool print_errors /* = 1 */ )
{
	// pixmaps dir must be given
	if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos ) 
	{
		filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	}

	cImage_settings_data *settings = NULL;
	SDL_Surface *sdl_surface = NULL;

	// load settings if available
	if( use_settings )
	{
		string settings_file = filename;

		// if not already set
		if( settings_file.rfind( ".settings" ) == string::npos )
		{
			settings_file.erase( settings_file.rfind( "." ) + 1 );
			settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );
		}

		// if a settings file exists
		if( file_exists( settings_file ) )
		{
			settings = pSettingsParser->Get( settings_file );

			// image given in base settings
			if( !settings->base.empty() )
			{
				// use current directory
				string img_filename = filename.substr( 0, filename.rfind( "/" ) + 1 ) + settings->base;

				// not found
				if( !file_exists( img_filename ) )
				{
					// use data dir
					img_filename = settings->base;

					// pixmaps dir must be given
					if( img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos ) 
					{
						img_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
					}
				}

				sdl_surface = IMG_Load( img_filename.c_str() );
			}
		}
	}

	// if not set in image settings and file exists
	if( ( !settings || settings->base.empty() ) && file_exists( filename ) )
	{
		sdl_surface = IMG_Load( filename.c_str() );
	}

	if( !sdl_surface )
	{
		if( print_errors )
		{
			printf( "Error loading image : %s\nReason : %s\n", filename.c_str(), SDL_GetError() );
		}

		return NULL;
	}

	// create GL image
	GL_Surface *image = NULL;

	// with settings
	if( settings )
	{
		// check if texture needs to get downscaled
		float new_w = (float)Get_GLsize( sdl_surface->w ), new_h = (float)Get_GLsize( sdl_surface->h );

		// if image settings dimension
		if( settings->width > 0 && settings->height > 0 )
		{
			/* downscale to fit best for the current resolution
			 * uses the first dimension which is not smaller as the default scale
			*/
			while( new_w * 0.5f > settings->width * global_scalex && new_h * 0.5f > settings->height * global_scaley )
			{
				new_w *= 0.5f;
				new_h *= 0.5f;
			}
		}

		image = Create_Texture( sdl_surface, settings->mipmap, (unsigned int)new_w, (unsigned int)new_h );

		// apply settings
		settings->Apply( image );
		delete settings;
	}
	// without settings
	else
	{
		image = Create_Texture( sdl_surface );
	}

	// set filename
	if( image )
	{
		image->filename = filename;
	}
	// print error
	else if( print_errors )
	{
		printf( "Error loading image : %s\nReason : %s\n", filename.c_str(), SDL_GetError() );
	}

	return image;
}

GL_Surface *cVideo :: Create_Texture( SDL_Surface *surface, bool mipmap /* = 0 */, unsigned int force_width /* = 0 */, unsigned int force_height /* = 0 */ )
{
	if( !surface )
	{
		return NULL;
	}

	unsigned int image_num = 0;
	unsigned int width = Get_GLsize( surface->w );
	unsigned int height = Get_GLsize( surface->h );

	SDL_Surface *final = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32,
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff );
	#else
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );
	#endif

	SDL_SetAlpha( surface, 0, SDL_ALPHA_TRANSPARENT );
	SDL_BlitSurface( surface, NULL, final, NULL );
	SDL_FreeSurface( surface );
	surface = NULL;

	// create one texture
	glGenTextures( 1, &image_num );

	if( !image_num )
	{
		printf( "Error : GL image generation failed\n" );
		SDL_FreeSurface( final );
		return NULL;
	}
	
	// set highest texture id
	if( pImageManager->high_texture_id < image_num )
	{
		pImageManager->high_texture_id = image_num;
	}

	// new dimension is set
	if( force_width > 0 && force_height > 0 )
	{
		force_width = Get_GLsize( force_width );
		force_height = Get_GLsize( force_height );

		// scale
		if( force_width != width || force_height != height )
		{
			// create scaled image
			void *new_pixels = SDL_malloc( force_width * force_height * 4 );
			gluScaleImage( GL_RGBA, final->w, final->h, GL_UNSIGNED_BYTE, final->pixels, force_width, force_height, GL_UNSIGNED_BYTE, new_pixels );
			SDL_free( final->pixels );
			final->pixels = new_pixels;

			// set new dimension
			width = force_width;
			height = force_height;
		}
		// set SDL_image pixel store mode
		else
		{
			glPixelStorei( GL_UNPACK_ROW_LENGTH, final->pitch / final->format->BytesPerPixel );
		}
	}

	// use the generated texture
	glBindTexture( GL_TEXTURE_2D, image_num );

	// how this texture is drawn
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	Create_GL_Texture( width, height, final->pixels, mipmap );
	
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

	SDL_FreeSurface( final );

	GL_Surface *image = new GL_Surface();
	image->image = image_num;
	image->tex_w = width;
	image->tex_h = height;
	image->w = (float)width;
	image->h = (float)height;
	image->col_w = image->w;
	image->col_h = image->h;

	// if debug build check for errors
#ifdef _DEBUG
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "CreateTexture : GL Error found : %s\n", gluErrorString( error ) );
	}
#endif

	return image;
}

void cVideo :: Create_GL_Texture( unsigned int width, unsigned int height, void *pixels, bool mipmap /* = 0 */ )
{
	// create mipmaps
    if( mipmap )
    {
		// enable mipmap filter
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

		// if OpenGL 1.4 or higher
        if( opengl_version >= 1.4f )
		{
			// use glTexImage2D to create Mipmaps
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1 );
			// copy the software bitmap into the opengl texture
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
        }
		// OpenGL below 1.4
		else
		{
			// use glu to create Mipmaps
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		}
    }
	// no mipmaps
    else
    {
		// default filter
 		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		// copy the software bitmap into the opengl texture
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    }
}

Color cVideo :: GetPixel( int x, int y, GL_Surface *source /* = NULL */ )
{
	//glReadBuffer( GL_FRONT );

	Color color;
	float pixel[3];
	glLoadIdentity();
	glReadPixels( x, y, 1, 1, GL_RGB, GL_FLOAT, &pixel );

	color = Color( pixel[0], pixel[1], pixel[2] );

	return color;
}

void cVideo :: DrawLine( GL_line *line, float z, Color *color, cLineRequest *request /* = NULL */ )
{
	if( !line )
	{
		return;
	}

	DrawLine( line->x1, line->y1, line->x2, line->y2, z, color, request );
}

void cVideo :: DrawLine( float x1, float y1, float x2, float y2, float z, Color *color, cLineRequest *request /* = NULL */ )
{
	if( !color )
	{
		return;
	}

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cLineRequest();
	}

	// line
	request->line.x1 = x1;
	request->line.y1 = y1;
	request->line.x2 = x2;
	request->line.y2 = y2;

	// z position
	request->pos_z = z;

	// color
	request->color = *color;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cVideo :: Draw_Rect( GL_rect *rect, float z, Color *color, cRectRequest *request /* = NULL */ )
{
	if( !rect )
	{
		Draw_Rect( 0, 0, GAME_RES_W, GAME_RES_H, z, color, request );
	}
	else
	{
		Draw_Rect( rect->x, rect->y, rect->w, rect->h, z, color, request );
	}
}

void cVideo :: Draw_Rect( float x, float y, float width, float height, float z, Color *color, cRectRequest *request /* = NULL */ )
{
	if( !color || height == 0 || width == 0 )
	{
		return;
	}

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cRectRequest();
	}

	// rect
	request->rect.x = x;
	request->rect.y = y;
	request->rect.w = width;
	request->rect.h = height;

	// z position
	request->pos_z = z;

	// color
	request->color = *color;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cVideo :: Draw_Gradient( GL_rect *rect, float z, Color *color1, Color *color2, ObjectDirection direction, cGradientRequest *request /* = NULL */ )
{
	if( !rect )
	{
		Draw_Gradient( 0, 0, GAME_RES_W, GAME_RES_H, z, color1, color2, direction, request );
	}
	else
	{
		Draw_Gradient( rect->x, rect->y, rect->w, rect->h, z, color1, color2, direction, request );
	}
}

void cVideo :: Draw_Gradient( float x, float y, float width, float height, float z, Color *color1, Color *color2, ObjectDirection direction, cGradientRequest *request /* = NULL */ )
{
	if( !color1 || !color2 || height == 0 || width == 0 )
	{
		return;
	}

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cGradientRequest();
	}

	// rect
	request->rect.x = x;
	request->rect.y = y;
	request->rect.w = width;
	request->rect.h = height;

	// z position
	request->pos_z = z;

	// color
	request->color_1 = *color1;
	request->color_2 = *color2;

	// direction
	request->dir = direction;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

float cVideo :: Get_scale( GL_Surface *image, float height, float width, bool only_downscale /* = 1 */ )
{
	if( !image )
	{
		return 0;
	}

	// change size
	if( !( only_downscale && image->h <= height && image->w <= width ) )
	{
		float zoom = width / image->w;

		if( height / image->h < zoom ) // if height is smaller
		{
			zoom = height / image->h;
		}

		return zoom;
	}

	return 1;
}

void cVideo :: Save_Screenshot( void )
{
	string filename;
	
	for( unsigned int i = 1; i < 1000; i++ )
	{
		filename = user_data_dir + USER_SCREENSHOT_DIR "/" + int_to_string( i ) + ".bmp";

		if( !file_exists( filename ) )
		{
			// Todo : save it as png

			// create RGB image data
			GLubyte *data = new GLubyte[pPreferences->Screen_W * pPreferences->Screen_H * 3];

			// read gl screen
			glReadPixels( 0, 0, pPreferences->Screen_W, pPreferences->Screen_H, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)data );

			// create SDL surface
			SDL_Surface *temp = SDL_CreateRGBSurface( SDL_SWSURFACE, pPreferences->Screen_W, pPreferences->Screen_H, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );

			if( SDL_MUSTLOCK( temp ) )
			{
				SDL_LockSurface( temp );
			}

			Uint32 *pdata = (Uint32 *)temp->pixels;

			// copy into SDL surface
			for( int y = temp->h - 1, y2 = 0; y >= 0 && y2 < pPreferences->Screen_H; --y, ++y2 )
			{
				for( int x = temp->w -1; x >= 0; --x )
				{
					pdata[y * temp->pitch/4 + x] =
						SDL_MapRGBA( temp->format,
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 0 ],
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 1 ],
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 2 ],
							255 );
				}
			}

			if( SDL_MUSTLOCK( temp ) )
			{
				SDL_UnlockSurface( temp );
			}

			// save the surface
			SDL_SaveBMP( temp, filename.c_str() );
			
			// delete screen data
			SDL_FreeSurface( temp );
			delete[] data;

			debugdisplay->Set_Text( "Screenshot " + int_to_string( i ) + " saved", DESIRED_FPS * 2.5 );

			return;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned int Get_GLsize( unsigned int size )
{
	unsigned int value = 1;

	while( value < size )
	{
		value <<= 1;
	}

	return value;
}

void Draw_Effect_out( Effect_Fadeout effect /* = EFFECT_OUT_RANDOM */, float speed /* = 1 */ )
{
	if( effect == EFFECT_OUT_RANDOM )
	{
		effect = (Effect_Fadeout)( ( rand() % (EFFECT_OUT_AMOUNT - 1) ) + 1 );
	}

	switch( effect )
	{
	case EFFECT_OUT_BLACK:
	{
		Color color = (Uint8)0;

		for( float i = 1; i > 0; i -= ( speed / 30 ) * pFramerate->speedfactor )
		{
			color.alpha = (Uint8)( 45 - ( 45 * i ) );

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &color, request );
			
			request->render_count = 2;

			// add request
			pRenderer->Add( request );

			pVideo->Render();

			pFramerate->Update();
			// maximum 90 fps
			CorrectFrameTime( 90 );
		}
		
		break;
	}
	case EFFECT_OUT_HORIZONTAL_VERTICAL:
	{
		float pos = 0;
		float size = 1;
		int hor = ( rand() % 2 ) - 1;
		Color color = Color( (Uint8)( rand() % 100 ), rand() % 100, rand() % 100, 150 );

		if( hor ) // horizontal
		{
			pos = GAME_RES_W;
		}
		else // vertical
		{
			pos = GAME_RES_H;
		}

		while( pos > 0 )
		{
			// horizontal
			if( hor )
			{
				// left/top
				cRectRequest *rect_request = new cRectRequest();
				pVideo->Draw_Rect( pos, 0, size, GAME_RES_H, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				// down/right
				rect_request = new cRectRequest();
				pVideo->Draw_Rect( GAME_RES_W - pos - size, 0, size, GAME_RES_H, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				size = 20 * pFramerate->speedfactor;
				pos -= size;
			}
			// vertical
			else
			{
				// left/top
				cRectRequest *rect_request = new cRectRequest();
				pVideo->Draw_Rect( 0, pos, GAME_RES_W, size, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				// down/right
				rect_request = new cRectRequest();
				pVideo->Draw_Rect( 0, GAME_RES_H - pos - size, GAME_RES_W, size, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				size = 15 * pFramerate->speedfactor;
				pos -= size;
			}

			pVideo->Render();

			pFramerate->Update();
			// maximum 90 fps
			CorrectFrameTime( 90 );
		}
		break;
	}
	case EFFECT_OUT_BIG_ITEM:
	{
		float f = 0.1f;
		GL_Surface *image = NULL;

		// item based on the camera x position
		if( pCamera->x < 2000 )
		{
			image = pVideo->Get_Surface( "game/items/mushroom_red.png" );
		}
		else if( pCamera->x < 5000 )
		{
			image = pVideo->Get_Surface( "game/items/fire_flower.png" );
		}
		else if( pCamera->x < 10000 )
		{
			image = pVideo->Get_Surface( "game/items/mushroom_green.png" );
		}
		else if( pCamera->x < 20000 )
		{
			image = pVideo->Get_Surface( "game/items/star.png" );
		}
		else
		{
			image = pVideo->Get_Surface( "game/items/moon_1.png" );
		}

		Color color = white;

		while( f < 50 )
		{
			Draw_Game();

			f += 0.9f * pFramerate->speedfactor * speed * ( f / 7 );

			color = Color( (Uint8)( 255 - ( f * 4 ) ), 255 - (Uint8)( f * 4 ), 255 - (Uint8)( f * 4 ), 200 - (Uint8)( f * 4 ) );

			// ## item
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			image->Blit( ( GAME_RES_W / 2 ) - ( ( image->w * f ) / 2 ) , GAME_RES_H / 2 - ( ( image->h * f ) / 2 ), 0.9f, request );

			request->blend_sfactor = GL_SRC_ALPHA;
			request->blend_dfactor = GL_ONE;

			request->color = color;

			// scale
			request->scale_x = f;
			request->scale_y = f;

			// add request
			pRenderer->Add( request );
			

			// ## additional black fadeout
			color = Color( 0, 0, 0, (Uint8)( 50 + ( f * 4 ) ) );

			// create request
			cRectRequest *rect_request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.901f, &color, rect_request );

			// add request
			pRenderer->Add( rect_request );

			pVideo->Render();
			pFramerate->Update();
		}

		break;
	}
	case EFFECT_OUT_RANDOM_COLOR_BOOST:
	{
		float f = 50;
		unsigned int rand_color = ( rand() % 4 );

		float xwidth = 45;

		GL_rect rect;
		
		while( f > 0 )
		{
			xwidth -= 0.1f + ( xwidth / 50 ) * pFramerate->speedfactor;

			for( unsigned int g = 0; g < 50; g++ )
			{
				rect.x = (float)( rand() % ( GAME_RES_W ) );
				rect.y = (float)( rand() % ( GAME_RES_H ) );
				
				rect.w = xwidth;
				rect.h = xwidth;

				Color pixel = black;

				// red
				if( rand_color == 0 )
				{
					pixel = Color( 1, (Uint8)( rect.y / 4 ), (Uint8)( rect.x / 4 ), (Uint8)( 5 * pFramerate->speedfactor ) );
				}
				// green
				else if( rand_color == 1 )
				{
					pixel = Color( (Uint8)( rect.x / 4 ), 1, (Uint8)( rect.y / 4 ), (Uint8)( 5 * pFramerate->speedfactor ) );
				}
				// blue
				else if( rand_color == 2 )
				{
					pixel = Color( (Uint8)( rect.x / 4 ), (Uint8)( rect.y / 4 ), 1, (Uint8)( 5 * pFramerate->speedfactor ) );
				}
				// yellow
				else
				{
					pixel = Color( 1, 1, (Uint8)( rect.y / 4 ), (Uint8)( 6 * pFramerate->speedfactor ) );
				}

				// fix alpha
				if( pixel.alpha < 5 )
				{
					pixel.alpha = 5;
				}

				rect.x -= xwidth / 2;
				rect.y -= xwidth / 2;
				
				// create request
				cRectRequest *request = new cRectRequest();
				pVideo->Draw_Rect( &rect, 0.9f, &pixel, request );
				
				request->render_count = 2;

				request->blend_sfactor = GL_SRC_ALPHA;
				request->blend_dfactor = GL_ONE_MINUS_SRC_COLOR;

				// add request
				pRenderer->Add( request );
			}
			
			f -= pFramerate->speedfactor;

			pVideo->Render();
			pFramerate->Update();
		}
		break;
	}
	case EFFECT_OUT_TILE_PIXELATION:
	{
		const unsigned int num_hor = 8;
		const unsigned int num_ver = 6;
		unsigned int num = num_hor * num_ver;

		bool grid[num_ver][num_hor];

		for( unsigned int i = 0; i < num_ver; i++ )
		{
			for( unsigned int j = 0; j < num_hor; j++ )
			{
				grid[i][j] = 1;
			}
		}
		
		unsigned int select_x = 0, select_y = 0;
		unsigned int temp;
		GL_rect dest( 0, 0, GAME_RES_W / num_hor, GAME_RES_H / num_ver );
		
		for( unsigned int i = 0; i < num; i++ )
		{
			while( grid[select_y][select_x] == 0 ) // find a unused rect
			{
				temp = rand()%( num );

				select_y = temp / num_hor;
				select_x = temp % num_hor;
			}

			grid[select_y][select_x] = 0;
			dest.x = select_x * dest.w;
			dest.y = select_y * dest.h;
			
			Color color = black;

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( &dest, 0.9f, &color, request );
			
			request->render_count = 2;

			// add request
			pRenderer->Add( request );
			

			pVideo->Render();
			pFramerate->Update();
			CorrectFrameTime( DESIRED_FPS ); // correction needed
		}
		break;
	}
	case EFFECT_OUT_FIXED_COLORBOX:
	{
		float xsize = 10;
		int xtemp = 0;
		Uint16 xpos = 0;
		Uint16 ypos = 0;

		GL_rect rect;
		Color color = black;

		unsigned int rand_color = ( rand() % 3 );

		// green
		if( rand_color == 0 )
		{
			color = Color( (Uint8)10, 55, 10 );
		}
		// blue
		else if( rand_color == 1 )
		{
			color = Color( (Uint8)10, 10, 55 );
		}
		// yellow
		else
		{
			color = Color( (Uint8)55, 55, 10 );
		}

		color.alpha = 1;

		while( xsize < 20 )
		{
			xsize += 0.3f * pFramerate->speedfactor;

			if( xtemp < (int)xsize )
			{
				if( xtemp < 10 && xsize > 10 )
				{
					// todo : float color class ?
					color.red *= 0.1f;
					color.green *= 0.1f;
					color.blue *= 0.1f;
				}

				xpos = (Uint16)( rand() % (unsigned int)xsize ) + 2;
				ypos = (Uint16)( rand() % (unsigned int)xsize ) + 2;
				xtemp = (int)xsize + 2;
			}

			for( unsigned int g = xpos; g < GAME_RES_W; g += (unsigned int)xtemp + 5 )
			{
				for( unsigned int y = ypos; y < GAME_RES_H; y += (unsigned int)xtemp + 5 )
				{
					rect.x = (float)g;
					rect.y = (float)y;
					
					rect.w = (float)( 5 + ( rand() % ( (unsigned int)xsize + ( xtemp / 2 ) ) ) );
					rect.h = (float)( 5 + ( rand() % ( (unsigned int)xsize + ( xtemp / 2 ) ) ) );

					Color pixel = Color( ( rand() % color.red ) + 5, ( rand() % color.green ) + 5, ( rand() % color.blue ) + 5, (Uint8)color.alpha );
					
					rect.x -= 2 + ( xtemp / 2 );
					rect.y -= 2 + ( xtemp / 2 );

					// create request
					cRectRequest *request = new cRectRequest();
					pVideo->Draw_Rect( &rect, 0.9f, &pixel, request );

					request->render_count = 2;

					request->blend_sfactor = GL_ONE_MINUS_SRC_ALPHA;
					request->blend_dfactor = GL_ONE_MINUS_SRC_ALPHA;

					// add request
					pRenderer->Add( request );
				}
			}

			if( xsize > 15 )
			{
				color.alpha = xsize * 3.5f;
			}
			else if( xsize > 10 )
			{
				color.alpha = xsize * 1.5f;
			}
			else
			{
				color.alpha = xsize * 0.5f;
			}

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &color, request );
			
			request->render_count = 2;

			request->blend_sfactor = GL_SRC_ALPHA;
			request->blend_dfactor = GL_ONE_MINUS_SRC_COLOR;

			// add request
			pRenderer->Add( request );

			pVideo->Render();
			pFramerate->Update();
		}
		break;
	}
	default:
		break;  // be happy
	}
	
	pFramerate->Update();
}

void Draw_Effect_in( Effect_Fadein effect /* = EFFECT_IN_RANDOM */, float speed /* = 1 */ )
{
	// Clear render cache
	pRenderer->Clear( 1 );

	if( effect == EFFECT_IN_RANDOM )
	{
		effect = (Effect_Fadein)( ( rand() % (EFFECT_IN_AMOUNT - 1) ) + 1 );
	}

	switch( effect )
	{
	case EFFECT_IN_BLACK:
	{
		Color color = (Uint8)0;

		for( float i = 1; i > 0; i -= ( speed / 30 ) * pFramerate->speedfactor )
		{
			color.alpha = (Uint8)( 255 * i );

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &color, request );

			// add request
			pRenderer->Add( request );

			Draw_Game();

			pVideo->Render();

			pFramerate->Update();
			// maximum 90 fps
			CorrectFrameTime( 90 );
		}
		
		break;
	}
	default:
		break;  // be happy
	}
	
	pFramerate->Update();
}

void Draw_Loading_Screen( void )
{
	bool mouse_hide = 0;

	// get loading window
	Window *window_1 = WindowManager::getSingleton().loadWindowLayout( "loading.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( window_1 );
	// loading images
	cHudSprite *loading_img_1 = new cHudSprite( pVideo->Get_Surface( "menu/extra_1.png" ), GAME_RES_W * 0.08f, GAME_RES_H * 0.75f );
	cHudSprite *loading_img_2 = new cHudSprite( pVideo->Get_Surface( "menu/extra_2.png" ), GAME_RES_W * 0.6f, GAME_RES_H * 0.1f );

	// white background
	glClearColor( 1, 1, 1, 1 );
	pVideo->Clear_Screen();

	// Gradient
	Color col1 = Color( 0.7f, 0.93f, 1 );
	Color col2 = Color( 0.26f, 0.8f, 1 );
	pVideo->Draw_Gradient( NULL, 0.08f, &col1, &col2, DIR_VERTICAL );
	// Loading Images
	loading_img_1->Draw();
	loading_img_2->Draw();

	pRenderer->Render();
	pGuiSystem->renderGUI();
	pRenderer_GUI->Render();
	SDL_GL_SwapBuffers();

	// delete
	delete loading_img_1;
	delete loading_img_2;
	pGuiSystem->getGUISheet()->removeChildWindow( window_1 );
	WindowManager::getSingleton().destroyWindow( window_1 );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cVideo *pVideo = NULL;

OpenGLRenderer *pGuiRenderer = NULL;
System *pGuiSystem = NULL;

SDL_Surface *screen = NULL;
Color blue, darkblue, lightblue, black, blackalpha128, blackalpha192, white, whitealpha128, grey, lightgrey, lightgreyalpha64, green, lightgreen, lightgreenalpha64, yellow, greenyellow, darkgreen, red, lightred, lila, orange, lightorange;
