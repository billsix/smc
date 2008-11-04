/***************************************************************************
 * video.cpp  -  General video functions
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
#include "../core/math/utilities.h"
#include "../core/i18n.h"
#include "../core/math/size.h"
// CEGUI
#include "CEGUIDefaultResourceProvider.h"
#include "CEGUIDefaultLogger.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// png
#include <png.h>
#ifndef PNG_COLOR_TYPE_RGBA
	#define PNG_COLOR_TYPE_RGBA PNG_COLOR_TYPE_RGB_ALPHA
#endif

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
	red = static_cast<Uint8>(r * 255);
	green = static_cast<Uint8>(g * 255);
	blue = static_cast<Uint8>(b * 255);
	alpha = static_cast<Uint8>(a * 255);
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

CEGUI::colour Color :: Get_cegui_Color( void )
{
	return CEGUI::colour( static_cast<float>(red) / 255, static_cast<float>(green) / 255, static_cast<float>(blue) / 255, static_cast<float>(alpha) / 255 );
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

/* *** *** *** *** *** *** *** CEGUI renderer fake class *** *** *** *** *** *** *** *** *** *** */

cFake_Renderer :: cFake_Renderer( void )
{
	d_identifierString = "Fake Renderer";
}

cFake_Renderer :: ~cFake_Renderer( void )
{

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
	// default geometry detail is medium
	geometry_detail = 0.5f;
	// default texture detail is high
	texture_detail = 0.75f;

	initialised = 0;
}

cVideo :: ~cVideo( void )
{

}

void cVideo :: Init_CEGUI_Fake( void )
{
	// create fake Resource Provider
	CEGUI::DefaultResourceProvider *rp = new CEGUI::DefaultResourceProvider();
	// set Resource Provider directories
	if( CEGUI::System::getDefaultXMLParserName().compare( "XercesParser" ) == 0 )
	{
		// This is needed for Xerces to specify the schemas location
		rp->setResourceGroupDirectory( "schemas", DATA_DIR "/" GAME_SCHEMA_DIR "/" );
	}
	// get a directory to dump the CEGUI log
	string log_dump_dir;
#ifdef _WIN32
	log_dump_dir = Get_Temp_Directory() + "cegui.log";
#else
	log_dump_dir = "/dev/null";
#endif
	// create fake system and renderer
	pGuiSystem = new CEGUI::System( new cFake_Renderer(), rp, NULL, NULL, "", log_dump_dir );
}

void cVideo :: Delete_CEGUI_Fake( void )
{
	CEGUI::ResourceProvider *rp = pGuiSystem->getResourceProvider();
	CEGUI::Renderer *renderer = pGuiSystem->getRenderer();

	delete pGuiSystem;
	pGuiSystem = NULL;
	delete renderer;
	delete rp;
}

void cVideo :: Init_CEGUI( void )
{
	// create renderer
	try
	{
		pGuiRenderer = new CEGUI::OpenGLRenderer( 0, screen->w, screen->h );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "CEGUI Exception occurred : %s\n", ex.getMessage().c_str() );
		exit( EXIT_FAILURE );
	}

	/* create Resource Provider
	 * no need to destroy it later since it is handled by the CEGUI renderer
	*/
	CEGUI::DefaultResourceProvider *rp = static_cast<CEGUI::DefaultResourceProvider *>(pGuiRenderer->createResourceProvider());

	// set Resource Provider directories
	rp->setResourceGroupDirectory( "schemes", DATA_DIR "/" GUI_SCHEME_DIR "/" );
	rp->setResourceGroupDirectory( "imagesets", DATA_DIR "/" GUI_IMAGESET_DIR "/" );
	rp->setResourceGroupDirectory( "fonts", DATA_DIR "/" GUI_FONT_DIR "/" );
	rp->setResourceGroupDirectory( "looknfeels", DATA_DIR "/" GUI_LOOKNFEEL_DIR "/" );
	rp->setResourceGroupDirectory( "layouts", DATA_DIR "/" GUI_LAYOUT_DIR "/" );
	if( CEGUI::System::getDefaultXMLParserName().compare( "XercesParser" ) == 0 )
	{
		// Needed for Xerces to specify the schemas location
		rp->setResourceGroupDirectory( "schemas", DATA_DIR "/" GAME_SCHEMA_DIR "/" );
	}

	// create logger
	CEGUI::Logger *logger = new CEGUI::DefaultLogger();
	// set logging level
#ifdef _DEBUG
	logger->setLoggingLevel( CEGUI::Informative );
#else
	logger->setLoggingLevel( CEGUI::Errors );
#endif

	// create system
	try
	{
		pGuiSystem = new CEGUI::System( pGuiRenderer, rp, NULL, NULL, "", user_data_dir + "cegui.log" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "CEGUI Exception occurred : %s\n", ex.getMessage().c_str() );
		exit( EXIT_FAILURE );
	}
}

void cVideo :: Init_CEGUI_Data( void )
{
	// set the default resource groups to be used
	CEGUI::Scheme::setDefaultResourceGroup( "schemes" );
	CEGUI::Imageset::setDefaultResourceGroup( "imagesets" );
	CEGUI::Font::setDefaultResourceGroup( "fonts" );
	CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
	CEGUI::WindowManager::setDefaultResourceGroup( "layouts" );
	// only needed for Xerces
	if( CEGUI::System::getDefaultXMLParserName().compare( "XercesParser" ) == 0 )
	{
		//XercesParser::setSchemaDefaultResourceGroup( "schemas" );
	}

	// load the scheme file, which auto-loads the imageset
	try
	{
		CEGUI::SchemeManager::getSingleton().loadScheme( "TaharezLook.scheme" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "CEGUI Scheme Exception occurred : %s\n", ex.getMessage().c_str() );
		exit( EXIT_FAILURE );
	}

	// first font loaded automatically becomes the default font
	try
	{
		CEGUI::FontManager::getSingleton().createFont( "bluebold1024_medium.font" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "CEGUI Font Exception occurred : %s\n", ex.getMessage().c_str() );
		exit( EXIT_FAILURE );
	}

	// default mouse cursor
	pGuiSystem->setDefaultMouseCursor( "TaharezLook", "MouseArrow" );
	// force new mouse image
	CEGUI::MouseCursor::getSingleton().setImage( &CEGUI::ImagesetManager::getSingleton().getImageset( "TaharezLook" )->getImage( "MouseArrow" ) );
	// hide CEGUI mouse always because we render it manually
	CEGUI::MouseCursor::getSingleton().hide();
	// default tooltip
	pGuiSystem->setDefaultTooltip( "TaharezLook/Tooltip" );
	// create default root window
	CEGUI::Window *window_root = CEGUI::WindowManager::getSingleton().loadWindowLayout( "default.layout" );
	pGuiSystem->setGUISheet( window_root );
	window_root->activate();
}

void cVideo :: Init_SDL( void )
{
	if( SDL_Init( SDL_INIT_VIDEO ) == -1 )
	{
		printf( "Error : SDL initialization failed\nReason : %s\n", SDL_GetError() );
		exit( EXIT_FAILURE );
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

void cVideo :: Init_Video( bool reload_textures_from_file /* = 0 */, bool use_preferences /* = 1 */ )
{
	// set the video flags
	int flags = SDL_OPENGL | SDL_SWSURFACE;

	// only enter fullscreen if set in preferences
	if( use_preferences && pPreferences->video_fullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}

	int screen_w, screen_h, screen_bpp;

	// full initialization
	if( use_preferences )
	{
		screen_w = pPreferences->video_screen_w;
		screen_h = pPreferences->video_screen_h;
		screen_bpp = pPreferences->video_screen_bpp;
	}
	// initialization with SDL defaults
	else
	{
		screen_w = 800;
		screen_h = 600;
		screen_bpp = 16;
	}

	// first initialization
	if( !initialised )
	{
		// Set Caption
		SDL_WM_SetCaption( CAPTION, NULL );
		// Set Icon
		string filename_icon = DATA_DIR "/" GAME_ICON_DIR "/window_32.png";
		if( File_Exists( filename_icon ) )
		{
			SDL_Surface *icon = IMG_Load( filename_icon.c_str() );
			SDL_WM_SetIcon( icon, NULL );
			SDL_FreeSurface( icon );
		}
		else
		{
			printf( "Warning : Window icon %s does not exist\n", filename_icon.c_str() );
		}
	}

	// test screen mode
	int screen_test = Test_Video( screen_w, screen_h, screen_bpp, flags );

	// failed
	if( screen_test == 0 )
	{
		printf( "Warning : Video Resolution %dx%d is not supported\n", screen_w, screen_h );

		// set lowest available settings
		screen_w = 640;
		screen_h = 480;
		screen_bpp = 0;

		// overwrite user settings
		if( use_preferences )
		{
			pPreferences->video_screen_w = screen_w;
			pPreferences->video_screen_h = screen_h;
		}
	}
	// can not handle bits per pixel
	else if( screen_test > 1 && screen_bpp > 0 && screen_test < screen_bpp )
	{
		printf( "Warning : Video Bpp %d is not supported but %d is\n", screen_bpp, screen_test );
		// set closest supported bpp
		screen_bpp = screen_test;

		// overwrite user settings
		if( use_preferences )
		{
			pPreferences->video_screen_bpp = screen_bpp;
		}
	}

	int screen_rgb_size[3];

	// set bit per pixel sizes
	if( screen_bpp == 8 )
	{
		screen_rgb_size[0] = 3;
		screen_rgb_size[1] = 3;
		screen_rgb_size[2] = 2;
	}
	else if( screen_bpp == 15 )
	{
		screen_rgb_size[0] = 5;
		screen_rgb_size[1] = 5;
		screen_rgb_size[2] = 5;
	}
	else if( screen_bpp == 24 )
	{
		screen_rgb_size[0] = 8;
		screen_rgb_size[1] = 8;
		screen_rgb_size[2] = 8;
	}
	// same as 24...
	else if( screen_bpp == 32 )
	{
		screen_rgb_size[0] = 8;
		screen_rgb_size[1] = 8;
		screen_rgb_size[2] = 8;
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
	// if vertical synchronization is enabled
	if( use_preferences && pPreferences->video_vsync )
	{
		SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
	}

	// if reinitialization
	if( initialised )
	{
		// check if CEGUI is initialized
		bool cegui_initialized = pGuiSystem->getGUISheet() != NULL;

		// show loading screen
		if( cegui_initialized )
		{
			Loading_Screen_Init();
		}

		// save textures
		pImage_Manager->Grab_Textures( reload_textures_from_file, cegui_initialized );
		pFont->Grab_Textures();
		pGuiRenderer->grabTextures();
		pImage_Manager->Delete_Hardware_Textures();

		// exit loading screen
		if( cegui_initialized )
		{
			Loading_Screen_Exit();
		}
	}

	// SDL handles the screen surface memory management
	screen = SDL_SetVideoMode( screen_w, screen_h, screen_bpp, flags );

	if( !screen )
	{
		printf( "Error : Screen mode creation failed\nReason : %s\n", SDL_GetError() );
		exit( EXIT_FAILURE );
	}

	// check if fullscreen got set
	if( use_preferences && pPreferences->video_fullscreen )
	{
		int is_fullscreen = ( ( screen->flags & SDL_FULLSCREEN ) == SDL_FULLSCREEN );

		if( !is_fullscreen )
		{
			printf( "Warning : Fullscreen mode could not be set\n" );
		}
	}

	// check if double buffering got set
	int is_double_buffer;
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &is_double_buffer );
	double_buffer = is_double_buffer > 0;

	if( !double_buffer )
	{
		// only important on full initialization
		if( use_preferences )
		{
			printf( "Warning : Double Buffering could not be set\n" );
		}
	}

	// check if vertical synchronization got set
	if( use_preferences && pPreferences->video_vsync )
	{
		int is_vsync;
		// seems to return always true even if not available
		SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &is_vsync );

		if( !is_vsync )
		{
			printf( "Warning : VSync could not be set\n" );
		}
	}

	// get color bit size
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &rgb_size[0] );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &rgb_size[1] );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &rgb_size[2] );

	// check if color bit size is set as wanted
	if( use_preferences )
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

	// initialize opengl
	Init_OpenGL();

	// if reinitialization
	if( initialised )
	{
		// reset highest texture id
		pImage_Manager->high_texture_id = 0;

		/* restore GUI textures
		 * must be the first CEGUI call after the grabTextures function
		*/
		pGuiRenderer->restoreTextures();
		pFont->Restore_Textures();

		// send new size to CEGUI
		pGuiRenderer->setDisplaySize( CEGUI::Size( static_cast<float>(screen_w), static_cast<float>(screen_h) ) );

		// check if CEGUI is initialized
		bool cegui_initialized = pGuiSystem->getGUISheet() != NULL;

		// show loading screen
		if( cegui_initialized )
		{
			Loading_Screen_Init();
		}

		// initialize new image cache
		if( reload_textures_from_file )
		{
			Init_Image_Cache( 0, cegui_initialized );
		}

		// restore textures
		pImage_Manager->Restore_Textures( cegui_initialized );

		// exit loading screen
		if( cegui_initialized )
		{
			Loading_Screen_Exit();
		}
	}
	// finished first initialization
	else
	{
		// get opengl version
		opengl_version = static_cast<float>(atof( reinterpret_cast<const char*>(glGetString( GL_VERSION )) ));

		if( opengl_version < 1.4f )
		{
			printf( "Warning : OpenGL Version %f below optimal version 1.4 and higher\n", opengl_version );
		}

		initialised = 1;
	}
}

void cVideo :: Init_OpenGL( void )
{
    // viewport should cover the whole screen
    glViewport( 0, 0, pPreferences->video_screen_w, pPreferences->video_screen_h );

    // Camera projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	// Set up an Ortho Screen
	glOrtho( 0, static_cast<float>(pPreferences->video_screen_w), static_cast<float>(pPreferences->video_screen_h), 0, -1, 1 );
	
    // matrix operations
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Smooth Shading
	glShadeModel( GL_SMOOTH );

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

	// Geometry
	Init_Geometry();
	// texture detail
	Init_Texture_Detail();
	// Resolution Scale
	Init_Resolution_Scale();

	// Clear Screen
	Clear_Screen();

	SDL_GL_SwapBuffers();
}

void cVideo :: Init_Geometry( void )
{
	// Geometry Anti-Aliasing
	if( geometry_detail > 0.5f )
	{
		// Point
		glEnable( GL_POINT_SMOOTH );
		glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
		// Line
		glEnable( GL_LINE_SMOOTH );
		glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
		// Polygon - does not display correctly with open source ATi drivers ( 18.2.2008 )
		//glEnable( GL_POLYGON_SMOOTH );
		// Geforce 4 440 MX hangs if enabled
		//glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	}
	else
	{
		// Point
		glEnable( GL_POINT_SMOOTH );
		glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
		// Line
		glEnable( GL_LINE_SMOOTH );
		glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
	}

	/* Perspective Correction
	 * The quality of color, texture coordinate, and fog coordinate interpolation
	*/
	if( geometry_detail > 0.25f )
	{
		// high quality
		glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	}
	else
	{
		// low quality
		glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	}
}

void cVideo :: Init_Texture_Detail( void )
{
	/* filter quality of generated mipmap images
	 * only available if OpenGL version is 1.4 or greater
	*/
	if( opengl_version >= 1.4f )
	{
		if( texture_detail > 0.2f )
		{
			glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
		}
		else
		{
			glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
		}
	}
}

void cVideo :: Init_Resolution_Scale( void )
{
	// up scale
	global_upscalex = static_cast<float>(pPreferences->video_screen_w) / static_cast<float>(game_res_w);
	global_upscaley = static_cast<float>(pPreferences->video_screen_h) / static_cast<float>(game_res_h);
	// down scale
	global_downscalex = static_cast<float>(game_res_w) / static_cast<float>(pPreferences->video_screen_w);
	global_downscaley = static_cast<float>(game_res_h) / static_cast<float>(pPreferences->video_screen_h);
}

void cVideo :: Init_Image_Cache( bool recreate /* = 0 */, bool draw_gui /* = 0 */ )
{
	imgcache_dir = user_data_dir + USER_IMGCACHE_DIR;
	string imgcache_dir_active = imgcache_dir + "/" + int_to_string( pPreferences->video_screen_w ) + "x" + int_to_string( pPreferences->video_screen_h );

	// if cache is disabled
	if( !pPreferences->image_cache_enabled )
	{
		return;
	}

	// if not the same game version
	if( recreate || pPreferences->game_version != smc_version )
	{
		// delete all caches
		if( fs::exists( fs::path( imgcache_dir, fs::native ) ) )
		{
			try
			{
				fs::remove_all( fs::path( imgcache_dir, fs::native ) );
			}
			// could happen if a file is locked or no write rights
			catch( const std::exception &ex )
			{
				printf( "%s\n", ex.what() );

				if( draw_gui )
				{
					// caching failed
					Loading_Screen_Draw_Text( _("Caching Images failed : Could not remove old images") );
					SDL_Delay( 1000 );
				}
			}
		}
		
		fs::create_directory( fs::path( imgcache_dir, fs::native ) );
	}

	// no cache available
	if( !fs::exists( fs::path( imgcache_dir_active, fs::native ) ) )
	{
		fs::create_directories( fs::path( imgcache_dir_active + "/" GAME_PIXMAPS_DIR, fs::native ) );
	}
	// cache available
	else
	{
		imgcache_dir = imgcache_dir_active;
		return;
	}

	// texture detail should be maximum for caching
	float real_texture_detail = texture_detail;
	texture_detail = 1;

	CEGUI::ProgressBar *progress_bar = NULL;

	if( draw_gui )
	{
		// get progress bar
		progress_bar = static_cast<CEGUI::ProgressBar *>(CEGUI::WindowManager::getSingleton().getWindow( "progress_bar" ));
		progress_bar->setProgress( 0 );

		// set loading screen text
		Loading_Screen_Draw_Text( _("Caching Images") );
	}

	// get all files
	vector<string> image_files = Get_Directory_Files( DATA_DIR "/" GAME_PIXMAPS_DIR, ".settings", 1 );

	unsigned int loaded_files = 0;
	unsigned int file_count = image_files.size();

	// create directories, load images and save to cache
	for( vector<string>::iterator itr = image_files.begin(), itr_end = image_files.end(); itr != itr_end; ++itr )
	{
		// get filename
		string filename = (*itr);

		// remove data dir
		string cache_filename = filename.substr( strlen( DATA_DIR "/" ) );

		// if directory
		if( filename.rfind( "." ) == string::npos )
		{
			if( !fs::exists( fs::path( imgcache_dir_active + "/" + cache_filename, fs::native ) ) )
			{
				fs::create_directory( fs::path( imgcache_dir_active + "/" + cache_filename, fs::native ) );
			}

			loaded_files++;
			continue;
		}

		bool settings_file = 0;

		// Don't use .settings file type directly for image loading
		if( filename.rfind( ".settings" ) != string::npos )
		{
			settings_file = 1;
			filename.erase( filename.rfind( ".settings" ) );
			filename.insert( filename.length(), ".png" );
		}
		
		// load software image
		cSoftware_Image software_image = Load_Image( filename );
		SDL_Surface *sdl_surface = software_image.sdl_surface;
		cImage_settings_data *settings = software_image.settings;

		// failed to load image
		if( !sdl_surface )
		{
			continue;
		}

		// no settings
		if( !settings )
		{
			SDL_FreeSurface( sdl_surface );
			continue;
		}

		// create final image
		sdl_surface = Convert_To_Final_Software_Image( sdl_surface );

		// get final size for this resolution
		cSize_Float size = settings->Get_Surface_Size( sdl_surface );
		int new_width = static_cast<int>(size.m_width);
		int new_height = static_cast<int>(size.m_height);
		delete settings;

		// does not need to be sampled down
		if( new_width >= sdl_surface->w && new_height >= sdl_surface->h )
		{
			SDL_FreeSurface( sdl_surface );
			continue;
		}

		// calculate block reduction
		int reduce_block_x = sdl_surface->w / new_width;
		int reduce_block_y = sdl_surface->h / new_height;

		// create downsampled image
		unsigned int image_bpp = sdl_surface->format->BytesPerPixel;
		unsigned char *image_downsampled = new unsigned char[new_width * new_height * image_bpp];
		bool sampled = Downscale_Image( static_cast<unsigned char*>(sdl_surface->pixels), sdl_surface->w, sdl_surface->h, image_bpp, image_downsampled, reduce_block_x, reduce_block_y );
		SDL_FreeSurface( sdl_surface );
		
		// if image is available
		if( sampled )
		{
			// save as png
			if( settings_file )
			{
				cache_filename.insert( cache_filename.length(), ".png" );
			}

			// save image
			Save_Surface( imgcache_dir_active + "/" + cache_filename, image_downsampled, new_width, new_height, image_bpp );
		}

		delete image_downsampled;

		// count files
		loaded_files++;

		// draw
		if( draw_gui )
		{
			// update filename
			GL_Surface *surface_filename = pFont->Render_Text( pFont->font_small, filename, white );

			// update progress
			progress_bar->setProgress( static_cast<float>(loaded_files) / static_cast<float>(file_count) );

			// clear screen
			Clear_Screen();
			Draw_Rect( NULL, 0.01f, &black );

			// draw filename
			surface_filename->Blit( game_res_w * 0.2f, game_res_h * 0.8f, 0.1f );

			// Render
			pRenderer->Render();
			pGuiSystem->renderGUI();
			pRenderer_GUI->Render();
			SDL_GL_SwapBuffers();
			
			// delete
			delete surface_filename;
		}
	}

	// set back texture detail
	texture_detail = real_texture_detail;
	// set directory after surfaces got loaded from Load_GL_Surface()
	imgcache_dir = imgcache_dir_active;
}

int cVideo :: Test_Video( int width, int height, int bpp, int flags /* = 0 */ )
{
	// auto set the video flags
	if( !flags )
	{
		flags = SDL_OPENGL | SDL_SWSURFACE;

		// if fullscreen is set
		if( pPreferences->video_fullscreen )
		{
			flags |= SDL_FULLSCREEN;
		}
	}

	return SDL_VideoModeOK( width, height, bpp, flags );
}

vector<cSize_Int> cVideo :: Get_Supported_Resolutions( int flags /* = 0 */ )
{
	vector<cSize_Int> valid_resolutions;

	// auto set the video flags
	if( !flags )
	{
		// always set fullscreen
		flags = SDL_OPENGL | SDL_SWSURFACE | SDL_FULLSCREEN;
	}

	SDL_Rect** modes = SDL_ListModes( NULL, flags );
	bool create_default_list = 0;

	// no dimension is available
	if( modes == NULL )
	{
		create_default_list = 1;
	}
	// any dimension is allowed
	else if( modes == (SDL_Rect**)-1 )
	{
		create_default_list = 1;
	}
	else
	{
		for( int i = 0; modes[i]; ++i )
		{
			valid_resolutions.push_back( cSize_Int( modes[i]->w, modes[i]->h ) );
		}
	}

	if( create_default_list )
	{
		valid_resolutions.push_back( cSize_Int( 2048, 1536 ) );
		valid_resolutions.push_back( cSize_Int( 1600, 1200 ) );
		valid_resolutions.push_back( cSize_Int( 1280, 1024 ) );
		valid_resolutions.push_back( cSize_Int( 1024, 768 ) );
		valid_resolutions.push_back( cSize_Int( 800, 600 ) );
		valid_resolutions.push_back( cSize_Int( 640, 480 ) );
	}

	return valid_resolutions;
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
	pPreferences->video_fullscreen = !pPreferences->video_fullscreen;

	// save clear color
	GLclampf clear_color[4];
	glGetFloatv( GL_COLOR_CLEAR_VALUE, clear_color );

#ifdef WIN32
	// windows needs reinitialization
	Init_Video();
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
	if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos )
	{
		filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	}

	// check if already loaded
	GL_Surface *image = pImage_Manager->Get_Pointer( filename );
	// already loaded
	if( image )
	{
		return image;
	}
	
	// load new image
	image = Load_GL_Surface( filename, 1, print_errors );
	// add new image
	if( image )
	{
		pImage_Manager->Add( image );
	}
	
	return image;
}

cVideo::cSoftware_Image cVideo :: Load_Image( string filename, bool load_settings /* = 1 */, bool print_errors /* = 1 */ )
{
	// pixmaps dir must be given
	if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos ) 
	{
		filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	}

	cSoftware_Image software_image = cSoftware_Image();
	SDL_Surface *sdl_surface = NULL;
	cImage_settings_data *settings = NULL;

	// load settings if available
	if( load_settings )
	{
		string settings_file = filename;

		// if not already set
		if( settings_file.rfind( ".settings" ) == string::npos )
		{
			settings_file.erase( settings_file.rfind( "." ) + 1 );
			settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );
		}

		// if a settings file exists
		if( File_Exists( settings_file ) )
		{
			settings = pSettingsParser->Get( settings_file );

			// add cache dir and remove data dir
			string img_filename_cache = imgcache_dir + "/" + settings_file.substr( strlen( DATA_DIR "/" ) ) + ".png";

			// check if image cache file exists
			if( File_Exists( img_filename_cache ) )
			{
				sdl_surface = IMG_Load( img_filename_cache.c_str() );
			}
			// image given in base settings
			else if( !settings->m_base.empty() )
			{
				// use current directory
				string img_filename = filename.substr( 0, filename.rfind( "/" ) + 1 ) + settings->m_base;

				// not found
				if( !File_Exists( img_filename ) )
				{
					// use data dir
					img_filename = settings->m_base;

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
	if( !sdl_surface && File_Exists( filename ) && ( !settings || settings->m_base.empty() ) )
	{
		sdl_surface = IMG_Load( filename.c_str() );
	}

	if( !sdl_surface )
	{
		if( settings )
		{
			delete settings;
			settings = NULL;
		}

		if( print_errors )
		{
			printf( "Error loading image : %s\nReason : %s\n", filename.c_str(), SDL_GetError() );
		}

		return software_image;
	}

	software_image.sdl_surface = sdl_surface;
	software_image.settings = settings;
	return software_image;
}

GL_Surface *cVideo :: Load_GL_Surface( string filename, bool use_settings /* = 1 */, bool print_errors /* = 1 */ )
{
	// pixmaps dir must be given
	if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos ) 
	{
		filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	}

	// load software image
	cSoftware_Image software_image = Load_Image( filename, use_settings, print_errors );
	SDL_Surface *sdl_surface = software_image.sdl_surface;
	cImage_settings_data *settings = software_image.settings;

	// final surface
	GL_Surface *image = NULL;

	// with settings
	if( settings )
	{
		// get the size
		cSize_Float size = settings->Get_Surface_Size( sdl_surface );
		// get basic settings surface
		image = pVideo->Create_Texture( sdl_surface, settings->m_mipmap, static_cast<unsigned int>(size.m_width), static_cast<unsigned int>(size.m_height) );
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

SDL_Surface *cVideo :: Convert_To_Final_Software_Image( SDL_Surface *surface )
{
	// get power of two size
	unsigned int width = Get_Power_of_2( surface->w );
	unsigned int height = Get_Power_of_2( surface->h );

	// if it needs to be changed
	if( width != surface->w || height != surface->h || surface->format->BitsPerPixel != 32 )
	{
		// create power of 2 and 32 bits per pixel surface
		SDL_Surface *final = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32,
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff );
		#else
				0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );
		#endif

		// set the entire surface alpha to 0
		SDL_SetAlpha( surface, 0, SDL_ALPHA_TRANSPARENT );
		// blit to 32 bit surface
		SDL_BlitSurface( surface, NULL, final, NULL );
		// delete original surface
		SDL_FreeSurface( surface );
		// set new surface
		surface = final;
	}

	return surface;
}

GL_Surface *cVideo :: Create_Texture( SDL_Surface *surface, bool mipmap /* = 0 */, unsigned int force_width /* = 0 */, unsigned int force_height /* = 0 */ )
{
	if( !surface )
	{
		return NULL;
	}

	// create final image
	surface = Convert_To_Final_Software_Image( surface );

	// create one texture
	GLuint image_num = 0;
	glGenTextures( 1, &image_num );

	// if image id is 0 it failed
	if( !image_num )
	{
		printf( "Error : GL image generation failed\n" );
		SDL_FreeSurface( surface );
		return NULL;
	}
	
	// set highest texture id
	if( pImage_Manager->high_texture_id < image_num )
	{
		pImage_Manager->high_texture_id = image_num;
	}

	unsigned int width = surface->w;
	unsigned int height = surface->h;

	// new dimension is set
	if( force_width > 0 && force_height > 0 )
	{
		// get power of two size
		force_width = Get_Power_of_2( force_width );
		force_height = Get_Power_of_2( force_height );

		// scale
		if( force_width != width || force_height != height )
		{
			// create scaled image
			void *new_pixels = SDL_malloc( force_width * force_height * 4 );
			gluScaleImage( GL_RGBA, surface->w, surface->h, GL_UNSIGNED_BYTE, surface->pixels, force_width, force_height, GL_UNSIGNED_BYTE, new_pixels );
			SDL_free( surface->pixels );
			surface->pixels = new_pixels;

			// set new dimension
			width = force_width;
			height = force_height;
		}
		// set SDL_image pixel store mode
		else
		{
			glPixelStorei( GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel );
		}
	}

	// use the generated texture
	glBindTexture( GL_TEXTURE_2D, image_num );

	// set how this texture is drawn
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	// set texture magnification function
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	// upload to OpenGL texture
	Create_GL_Texture( width, height, surface->pixels, mipmap );
	// unset pixel store mode
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

	SDL_FreeSurface( surface );

	// create OpenGL surface class
	GL_Surface *image = new GL_Surface();
	image->image = image_num;
	image->tex_w = width;
	image->tex_h = height;
	image->start_w = static_cast<float>(width);
	image->start_h = static_cast<float>(height);
	image->w = image->start_w;
	image->h = image->start_h;
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
		// default texture minifying function
 		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		// copy the software bitmap into the opengl texture
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    }
}

Color cVideo :: Get_Pixel( int x, int y, GL_Surface *source /* = NULL */ )
{
	//glReadBuffer( GL_FRONT );

	float *pixel = new float[3];
	glLoadIdentity();
	glReadPixels( x, y, 1, 1, GL_RGB, GL_FLOAT, pixel );

	Color color = Color( pixel[0], pixel[1], pixel[2] );
	delete[] pixel;

	return color;
}

void cVideo :: Draw_Line( GL_line *line, float z, const Color *color, cLineRequest *request /* = NULL */ )
{
	if( !line )
	{
		return;
	}

	Draw_Line( line->x1, line->y1, line->x2, line->y2, z, color, request );
}

void cVideo :: Draw_Line( float x1, float y1, float x2, float y2, float z, const Color *color, cLineRequest *request /* = NULL */ )
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

void cVideo :: Draw_Rect( GL_rect *rect, float z, const Color *color, cRectRequest *request /* = NULL */ )
{
	if( !rect )
	{
		Draw_Rect( 0, 0, static_cast<float>(game_res_w), static_cast<float>(game_res_h), z, color, request );
	}
	else
	{
		Draw_Rect( rect->x, rect->y, rect->w, rect->h, z, color, request );
	}
}

void cVideo :: Draw_Rect( float x, float y, float width, float height, float z, const Color *color, cRectRequest *request /* = NULL */ )
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

void cVideo :: Draw_Gradient( GL_rect *rect, float z, const Color *color_1, const Color *color_2, ObjectDirection direction, cGradientRequest *request /* = NULL */ )
{
	if( !rect )
	{
		Draw_Gradient( 0, 0, static_cast<float>(game_res_w), static_cast<float>(game_res_h), z, color_1, color_2, direction, request );
	}
	else
	{
		Draw_Gradient( rect->x, rect->y, rect->w, rect->h, z, color_1, color_2, direction, request );
	}
}

void cVideo :: Draw_Gradient( float x, float y, float width, float height, float z, const Color *color_1, const Color *color_2, ObjectDirection direction, cGradientRequest *request /* = NULL */ )
{
	if( !color_1 || !color_2 || height == 0 || width == 0 )
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
	request->color_1 = *color_1;
	request->color_2 = *color_2;

	// direction
	request->dir = direction;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cVideo :: Draw_Circle( float x, float y, float radius, float z, Color *color, cCircleRequest *request /* = NULL */ )
{
	if( !color || radius <= 0 )
	{
		return;
	}

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cCircleRequest();
	}

	// position
	request->pos.x = x;
	request->pos.y = y;
	// radius
	request->radius = radius;

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

float cVideo :: Get_Scale( GL_Surface *image, float width, float height, bool only_downscale /* = 1 */ )
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

/* function from Jonathan Dummer
 * from image helper functions
 * MIT license
*/
bool cVideo :: Downscale_Image( const unsigned char* const orig, int width, int height, int channels, unsigned char* resampled, int block_size_x, int block_size_y )
{
	//	error check
	if( width <= 0 || height <= 0 || channels <= 0 || orig == NULL || resampled == NULL || block_size_x <= 0 || block_size_y <= 0 )
	{
		// invalid argument
		return 0;
	}

	int mip_width = width / block_size_x;
	int mip_height = height / block_size_y;

	// check size
	if( mip_width < 1 )
	{
		mip_width = 1;
	}
	if( mip_height < 1 )
	{
		mip_height = 1;
	}

	int j, i, c;

	for( j = 0; j < mip_height; ++j )
	{
		for( i = 0; i < mip_width; ++i )
		{
			for( c = 0; c < channels; ++c )
			{
				const int index = (j * block_size_y) * width * channels + (i * block_size_x) * channels + c;
				int sum_value;
				int u,v;
				int u_block = block_size_x;
				int v_block = block_size_y;
				int block_area;

				/* do a bit of checking so we don't over-run the boundaries
				 * necessary for non-square textures!
				 */
				if( block_size_x * (i + 1) > width )
				{
					u_block = width - i * block_size_y;
				}
				if( block_size_y * (j + 1) > height )
				{
					v_block = height - j * block_size_y;
				}
				block_area = u_block * v_block;

				/* for this pixel, see what the average
				 * of all the values in the block are.
				 * note: start the sum at the rounding value, not at 0
				 */
				sum_value = block_area >> 1;
				for( v = 0; v < v_block; ++v )
				{
					for( u = 0; u < u_block; ++u )
					{
						sum_value += orig[index + v * width * channels + u * channels];
					}
				}

				resampled[j * mip_width * channels + i * channels + c] = sum_value / block_area;
			}
		}
	}

	return 1;
}

void cVideo :: Save_Screenshot( void )
{
	string filename;
	
	for( unsigned int i = 1; i < 1000; i++ )
	{
		filename = user_data_dir + USER_SCREENSHOT_DIR "/" + int_to_string( i ) + ".png";

		if( !File_Exists( filename ) )
		{
			// create image data
			GLubyte *data = new GLubyte[pPreferences->video_screen_w * pPreferences->video_screen_h * 3];
			// read opengl screen
			glReadPixels( 0, 0, pPreferences->video_screen_w, pPreferences->video_screen_h, GL_RGB, GL_UNSIGNED_BYTE, static_cast<GLvoid *>(data) );
			// save
			Save_Surface( filename, data, pPreferences->video_screen_w, pPreferences->video_screen_h, 3, 1 );
			// clear data
			delete[] data;

			// show info
			debugdisplay->Set_Text( "Screenshot " + int_to_string( i ) + _(" saved"), speedfactor_fps * 2.5f );

			// finished
			return;
		}
	}
}

void cVideo :: Save_Surface( string filename, unsigned char *data, unsigned int width, unsigned int height, unsigned int bpp /* = 4 */, bool reverse_data /* = 0 */ )
{
	FILE *fp = NULL;

	fp = fopen( filename.c_str(), "wb" );

	if( !fp )
	{
		printf( "Could not save Surface\n" );
		return;
	}

	int png_color_type;

	if( bpp == 4 )
	{
		png_color_type = PNG_COLOR_TYPE_RGBA;
	}
	else if( bpp == 3 )
	{
		png_color_type = PNG_COLOR_TYPE_RGB;
	}
	else
	{
		printf( "Warning: cVideo :: Save_Surface : %s Unknown bytes per pixel %d\n", filename.c_str(), bpp );
		return;
	}

	png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	png_infop info_ptr = png_create_info_struct( png_ptr );

	png_init_io( png_ptr, fp );

	png_set_IHDR( png_ptr, info_ptr,
		width, height, 8 /* bit depth */, png_color_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

	png_write_info( png_ptr, info_ptr );

	png_uint_32 png_height = height;
	png_uint_32 row_bytes = width * bpp;

	png_byte *image = new png_byte[png_height * row_bytes];
	png_bytep *row_pointers = new png_bytep[png_height];

	// create image data
	int img_size = png_height * row_bytes;
	for( int i = 0; i < img_size; ++i )
	{
		image[i] = data[i];
	}

	// create row pointers
	if( reverse_data )
	{
		for( unsigned int i = 0; i < png_height; i++ )
		{
			// reverse direction because of opengl glReadPixels
			row_pointers[png_height - 1 - i] = image + (i * row_bytes);
		}
	}
	else
	{
		for( unsigned int i = 0; i < png_height; i++ )
		{
			row_pointers[i] = image + (i * row_bytes);
		}
	}

	png_write_image( png_ptr, row_pointers );
	png_write_end( png_ptr, info_ptr );
	png_destroy_write_struct( &png_ptr, &info_ptr );

	delete []image;
	delete []row_pointers;

	fclose( fp );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void Draw_Effect_Out( Effect_Fadeout effect /* = EFFECT_OUT_RANDOM */, float speed /* = 1 */ )
{
	if( effect == EFFECT_OUT_RANDOM )
	{
		effect = static_cast<Effect_Fadeout>( ( rand() % (EFFECT_OUT_AMOUNT - 1) ) + 1 );
	}

	switch( effect )
	{
	case EFFECT_OUT_BLACK:
	{
		Color color = static_cast<Uint8>(0);

		for( float i = 1; i > 0; i -= ( speed / 30 ) * pFramerate->speedfactor )
		{
			color.alpha = static_cast<Uint8>( 45 - ( 45 * i ) );

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &color, request );
			
			request->render_count = 2;

			// add request
			pRenderer->Add( request );

			pVideo->Render();

			pFramerate->Update();
			// maximum fps
			Correct_Frame_Time( 100 );
		}
		
		break;
	}
	case EFFECT_OUT_HORIZONTAL_VERTICAL:
	{
		float pos = 0;
		float size = 1;
		int hor = ( rand() % 2 ) - 1;
		Color color = Color( static_cast<Uint8>( rand() % 100 ), rand() % 100, rand() % 100, 150 );

		// horizontal
		if( hor )
		{
			pos = static_cast<float>(game_res_w);
		}
		// vertical
		else
		{
			pos = static_cast<float>(game_res_h);
		}

		while( pos > 0 )
		{
			// horizontal
			if( hor )
			{
				// left/top
				cRectRequest *rect_request = new cRectRequest();
				pVideo->Draw_Rect( pos, 0, size, static_cast<float>(game_res_h), 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				// down/right
				rect_request = new cRectRequest();
				pVideo->Draw_Rect( static_cast<float>(game_res_w) - pos - size, 0, size, static_cast<float>(game_res_h), 0.9f, &color, rect_request );
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
				pVideo->Draw_Rect( 0, pos, static_cast<float>(game_res_w), size, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				// down/right
				rect_request = new cRectRequest();
				pVideo->Draw_Rect( 0, static_cast<float>(game_res_h) - pos - size, static_cast<float>(game_res_w), size, 0.9f, &color, rect_request );
				rect_request->render_count = 2;
				// add request
				pRenderer->Add( rect_request );

				size = 15 * pFramerate->speedfactor;
				pos -= size;
			}

			pVideo->Render();

			pFramerate->Update();
			// maximum fps
			Correct_Frame_Time( 100 );
		}
		break;
	}
	case EFFECT_OUT_BIG_ITEM:
	{
		float f = 0.1f;
		GL_Surface *image = NULL;

		// item based on the camera x position
		if( pActive_Camera->x < 2000 )
		{
			image = pVideo->Get_Surface( "game/items/mushroom_red.png" );
		}
		else if( pActive_Camera->x < 5000 )
		{
			image = pVideo->Get_Surface( "game/items/fireplant.png" );
		}
		else if( pActive_Camera->x < 10000 )
		{
			image = pVideo->Get_Surface( "game/items/mushroom_green.png" );
		}
		else if( pActive_Camera->x < 20000 )
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

			color = Color( static_cast<Uint8>( 255 - ( f * 4 ) ), 255 - static_cast<Uint8>( f * 4 ), 255 - static_cast<Uint8>( f * 4 ), 200 - static_cast<Uint8>( f * 4 ) );

			// ## item
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			image->Blit( ( game_res_w / 2 ) - ( ( image->w * f ) / 2 ) , game_res_h / 2 - ( ( image->h * f ) / 2 ), 0.9f, request );

			request->blend_sfactor = GL_SRC_ALPHA;
			request->blend_dfactor = GL_ONE;

			request->color = color;

			// scale
			request->scale_x = f;
			request->scale_y = f;

			// add request
			pRenderer->Add( request );
			

			// ## additional black fadeout
			color = Color( 0, 0, 0, static_cast<Uint8>( 50 + ( f * 4 ) ) );

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

			rect.w = xwidth;
			rect.h = xwidth;

			for( unsigned int g = 0; g < 50; g++ )
			{
				rect.x = Get_Random_Float( 0, static_cast<float>(game_res_w) );
				rect.y = Get_Random_Float( 0, static_cast<float>(game_res_h) );

				Color pixel = black;

				// red
				if( rand_color == 0 )
				{
					pixel = Color( 1, static_cast<Uint8>( 45 ), static_cast<Uint8>( 45 ), static_cast<Uint8>( 5 * pFramerate->speedfactor ) );
				}
				// green
				else if( rand_color == 1 )
				{
					pixel = Color( static_cast<Uint8>( 45 ), 1, static_cast<Uint8>( 45 ), static_cast<Uint8>( 5 * pFramerate->speedfactor ) );
				}
				// blue
				else if( rand_color == 2 )
				{
					pixel = Color( static_cast<Uint8>( 45 ), static_cast<Uint8>( 45 ), 1, static_cast<Uint8>( 5 * pFramerate->speedfactor ) );
				}
				// yellow
				else
				{
					pixel = Color( 1, 1, static_cast<Uint8>( 45 ), static_cast<Uint8>( 6 * pFramerate->speedfactor ) );
				}

				// minimum alpha
				if( pixel.alpha < 5 )
				{
					pixel.alpha = 5;
				}
				
				// create request
				cRectRequest *request = new cRectRequest();
				pVideo->Draw_Rect( rect.x, rect.y, rect.w - (xwidth * 0.5f), rect.h - (xwidth * 0.5f), 0.9f, &pixel, request );
				
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
		GL_rect dest( 0, 0, static_cast<float>(game_res_w) / num_hor, static_cast<float>(game_res_h) / num_ver );
		
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
			Correct_Frame_Time( speedfactor_fps ); // correction needed
		}
		break;
	}
	case EFFECT_OUT_FIXED_COLORBOX:
	{
		Color color( static_cast<Uint8>(0), 0, 0, 20 );

		unsigned int rand_color = ( rand() % 3 );

		// green
		if( rand_color == 0 )
		{
			color = Color( static_cast<Uint8>(10), 55, 10 );
		}
		// blue
		else if( rand_color == 1 )
		{
			color = Color( static_cast<Uint8>(10), 10, 55 );
		}
		// red - yellow
		else
		{
			color = Color( static_cast<Uint8>(55), 20, 10 );
		}

		// position
		float pos_x = 0;
		float pos_y = 0;
		// size
		float rect_size = 20;
		// counter
		float random_counter = 0;
		// temp rect for drawing
		GL_rect rect;

		// animate while size did not reach the limit
		while( rect_size < 30 )
		{
			// add size
			rect_size += 0.3f * pFramerate->speedfactor;

			// change position and color
			if( random_counter < rect_size )
			{
				// color gets darker
				if( color.red > 1 )
				{
					color.red = static_cast<Uint8>(color.red * 0.7f);
					if( color.red < 1 )
					{
						color.red = 1;
					}
				}
				if( color.green > 1 )
				{
					color.green = static_cast<Uint8>(color.green * 0.7f);
					if( color.green < 1 )
					{
						color.green = 1;
					}
				}
				if( color.blue > 1 )
				{
					color.blue = static_cast<Uint8>(color.blue * 0.7f);
					if( color.blue < 1 )
					{
						color.blue = 1;
					}
				}

				// random position advances with size
				pos_x = Get_Random_Float( -(rect_size), 0 );
				pos_y = Get_Random_Float( -(rect_size), 0 );

				// set counter
				random_counter = rect_size + 2;
			}

			// draw rects as a net
			// horizontal
			for( rect.x = pos_x; rect.x < game_res_w; rect.x += random_counter )
			{
				// vertical
				for( rect.y = pos_y; rect.y < game_res_h; rect.y += random_counter )
				{
					rect.w = Get_Random_Float( 0, random_counter );
					rect.h = Get_Random_Float( 0, random_counter );

					Color pixel_color = Color( 5 + ( rand() % color.red ), 5 + ( rand() % color.green ), 5 + ( rand() % color.blue ), color.alpha );

					// create request
					cRectRequest *request = new cRectRequest();
					pVideo->Draw_Rect( &rect, 0.9f, &pixel_color, request );

					request->render_count = 2;

					request->blend_sfactor = GL_ONE_MINUS_SRC_ALPHA;
					request->blend_dfactor = GL_ONE_MINUS_SRC_COLOR;

					// add request
					pRenderer->Add( request );
				}
			}

			Color rect_color = color;
			rect_color.alpha = 120 - static_cast<Uint8>(rect_size * 3);
			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &rect_color, request );
			
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

void Draw_Effect_In( Effect_Fadein effect /* = EFFECT_IN_RANDOM */, float speed /* = 1 */ )
{
	// Clear render cache
	pRenderer->Clear( 1 );

	if( effect == EFFECT_IN_RANDOM )
	{
		effect = static_cast<Effect_Fadein>( ( rand() % (EFFECT_IN_AMOUNT - 1) ) + 1 );
	}

	switch( effect )
	{
	case EFFECT_IN_BLACK:
	{
		Color color = static_cast<Uint8>(0);

		for( float i = 1; i > 0; i -= ( speed / 30 ) * pFramerate->speedfactor )
		{
			color.alpha = static_cast<Uint8>( 255 * i );

			// create request
			cRectRequest *request = new cRectRequest();
			pVideo->Draw_Rect( NULL, 0.9f, &color, request );

			// add request
			pRenderer->Add( request );

			Draw_Game();

			pVideo->Render();

			pFramerate->Update();
			// maximum fps
			Correct_Frame_Time( 100 );
		}
		
		break;
	}
	default:
		break;  // be happy
	}
	
	pFramerate->Update();
}

void Loading_Screen_Init( void )
{
	if( CEGUI::WindowManager::getSingleton().isWindowPresent( "loading" ) )
	{
		printf( "Warning: Loading Screen already initialized." );
		return;
	}

	CEGUI::Window *guisheet = pGuiSystem->getGUISheet();

	// hide all windows
	for( unsigned int i = 0, gui_windows = guisheet->getChildCount(); i < gui_windows; i++ )
	{
		guisheet->getChildAtIdx( i )->hide();
	}

	// Create loading window
	CEGUI::Window *loading_window = CEGUI::WindowManager::getSingleton().loadWindowLayout( "loading.layout" );
	guisheet->addChildWindow( loading_window );

	// set info text
	CEGUI::Window *text_default = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_loading" ));
	text_default->setText( _("Loading") );
}

void Loading_Screen_Draw_Text( string str_info /* = "Loading" */ )
{
	// set info text
	CEGUI::Window *text_default = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_loading" ));
	if( !text_default )
	{
		printf( "Warning: Loading Screen not initialized." );
		return;
	}
	text_default->setText( reinterpret_cast<const CEGUI::utf8*>(str_info.c_str()) );

	// clear screen
	pVideo->Clear_Screen();
	pVideo->Draw_Rect( NULL, 0.01f, &black );

	// Render
	pRenderer->Render();
	pGuiSystem->renderGUI();
	pRenderer_GUI->Render();
	SDL_GL_SwapBuffers();
}

void Loading_Screen_Exit( void )
{
	CEGUI::Window *loading_window = CEGUI::WindowManager::getSingleton().getWindow( "loading" );

	// loading window is present
	if( loading_window )
	{
		CEGUI::Window *guisheet = pGuiSystem->getGUISheet();

		// delete loading window
		guisheet->removeChildWindow( loading_window );
		CEGUI::WindowManager::getSingleton().destroyWindow( loading_window );

		// show windows again
		// fixme : this should only show the hidden windows again
		for( unsigned int i = 0, gui_windows = guisheet->getChildCount(); i < gui_windows; i++ )
		{
			guisheet->getChildAtIdx( i )->show();
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cVideo *pVideo = NULL;

CEGUI::OpenGLRenderer *pGuiRenderer = NULL;
CEGUI::System *pGuiSystem = NULL;

SDL_Surface *screen = NULL;
