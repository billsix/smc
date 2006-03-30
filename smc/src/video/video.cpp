/***************************************************************************
    video.cpp  -  internal video functions
                             -------------------
    copyright            : (C) 2003 - 2006 FluXy
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
	Parse_uint8( r, g, b, a );
};

Color :: Color( float r, float g, float b, float a /* = 1 */ )
{
	Parse_float( r, g, b, a );
}

Color :: Color( Uint8 grey )
{
	Parse_uint8( grey, grey, grey );
};

Color :: Color( Uint32 color )
{
	Parse( color );
};

Color :: Color( SDL_Color color )
{
	Parse( color );
	alpha = 255;
}

void Color :: Parse( Uint32 mapcolor )
{
	//SDL_GetRGBA( mapcolor, format, &red, &green, &blue, &alpha );
}

void Color :: Parse( SDL_Color color )
{
	red = color.r;
	green = color.g;
	blue = color.b;
}

void Color :: Parse_uint8( Uint8 r, Uint8 g, Uint8 b, Uint8 a /* = 255 */ )
{
	red = r;
	green = g;
	blue = b;
	alpha = a;
}

void Color :: Parse_float( float r, float g, float b, float a /* = 1 */ )
{
	red = (Uint8)(r * 255);
	green = (Uint8)(g * 255);
	blue = (Uint8)(b * 255);
	alpha = (Uint8)(a * 255);
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
	return colour( (float)(red / 255), (float)(green / 255), (float)(blue / 255), (float)(alpha / 255) );
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

/* *** *** *** *** *** *** *** *** GL_Surface *** *** *** *** *** *** *** *** *** */

GL_Surface :: GL_Surface( void )
{
	image = 0;

	int_x = 0;
	int_y = 0;
	w = 0;
	h = 0;
	tex_w = 0;
	tex_h = 0;

	// internal rotation data
	base_rotx = 0;
	base_roty = 0;
	base_rotz = 0;
	
	// collision data
	col_pos.x = 0;
	col_pos.y = 0;
	col_w = 0;
	col_h = 0;

	filename = "";

	auto_del_img = 1;
}

GL_Surface :: ~GL_Surface( void )
{
	if( auto_del_img && glIsTexture( image ) )
	{
		glDeleteTextures( 1, &image );
	}
}

void GL_Surface :: Blit( double x, double y, Color *shadow /* = NULL */, double rotx /* = 0 */, double roty /* = 0 */, double rotz /* = 0 */, double scale /* = 1 */, bool no_globalscale /* = 0 */ )
{
	if( shadow )
	{
		glColor4ub( shadow->red, shadow->green, shadow->blue, shadow->alpha );
		Blit( x + 2, y + 2, 0, rotx, roty, rotz, scale, no_globalscale );
		glColor4f( 1, 1, 1, 1 );
	}

	glLoadIdentity();
	
	// global scaling
	if( !no_globalscale && ( global_scalex != 1 || global_scaley != 1 ) )
	{
		glScaled( global_scalex, global_scaley, 0 );
	}

	// position
	glTranslated( x + ( ( w / 2 ) * scale ) + int_x, y + ( ( h / 2 ) * scale ) + int_y, 0 );

	// requested scaling
	if( scale != 1 )
	{
		glScaled( scale, scale, 1 );
	}

	// base image rotation
	if( base_rotx != 0 )
	{
		glRotated( base_rotx, 1, 0, 0 );
	}
	if( base_roty != 0 )
	{
		glRotated( base_roty, 0, 1, 0 );
	}
	if( base_rotz != 0 )
	{
		glRotated( base_rotz, 0, 0, 1 );
	}

	// requested rotation
	if( rotx != 0 )
	{
		glRotated( rotx, 1, 0, 0 );
	}
	if( roty != 0 )
	{
		glRotated( roty, 0, 1, 0 );
	}
	if( rotz != 0 )
	{
		glRotated( rotz, 0, 0, 1 );
	}

	glEnable( GL_TEXTURE_2D );
	// texture id
	glBindTexture( GL_TEXTURE_2D, image );

	// rectangle
	glBegin( GL_QUADS );
		// top left
		glTexCoord2f( 0, 0 );
		glVertex3d( -w / 2, -h / 2, 0 );
		// top right
		glTexCoord2f( 1, 0 );
		glVertex3d( w / 2, -h / 2, 0 );
		// bottom right
		glTexCoord2f( 1, 1 );
		glVertex3d( w / 2, h / 2, 0 );
		// bottom left
		glTexCoord2f( 0, 1 );
		glVertex3d( -w / 2, h / 2, 0 );
	glEnd();

	glDisable( GL_TEXTURE_2D );
}

GL_Surface *GL_Surface :: Copy( void )
{
	GL_Surface *new_surface = LoadTexture( filename, 0 );

	if( !new_surface )
	{
		printf( "Error : Couldn't load copy Texture\n" );
		return NULL;
	}

	if( !new_surface->image )
	{
		printf( "Error : Couldn't get GL image\n" );
		delete new_surface;
		return NULL;
	}

	new_surface->int_x = int_x;
	new_surface->int_y = int_y;
	new_surface->w = w;
	new_surface->h = h;
	new_surface->tex_h = tex_h;
	new_surface->tex_w = tex_w;
	new_surface->base_rotx = base_rotx;
	new_surface->base_roty = base_roty;
	new_surface->base_rotz = base_rotz;
	new_surface->col_pos = col_pos;
	new_surface->col_w = col_w;
	new_surface->col_h = col_h;

	return new_surface;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void Init_CEGUI( void )
{
	try
	{
		pGuiRenderer = new OpenGLRenderer( 0, GAME_RES_W, GAME_RES_H );
		pGuiSystem = new System( pGuiRenderer );
	}
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occured : %s", ex.getMessage().c_str() );
		exit( 0 );
	}
}

void Init_CEGUI_data( void )
{
#ifdef _DEBUG
	Logger::getSingleton().setLoggingLevel( Informative );
#else
	Logger::getSingleton().setLoggingLevel( Errors );
#endif

	// load the scheme file, which auto-loads the imageset
	try
	{
		SchemeManager::getSingleton().loadScheme( "data/gui/schemes/TaharezLookSkin.scheme" );
	}
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occured : %s", ex.getMessage().c_str() );
		exit( 0 );
	}

	// first font loaded automatically becomes the default font
	try
	{
		if( pPreferences->Screen_W >= 1024 ) // medium texture font
		{
			FontManager::getSingleton().createFont( "data/gui/font/bluebold1024_medium.font" );
			FontManager::getSingleton().createFont( "data/gui/font/bluebold1024_big.font" );
		}
		else // small texture font
		{
			FontManager::getSingleton().createFont( "data/gui/font/bluebold800_medium.font" );
			FontManager::getSingleton().createFont( "data/gui/font/bluebold800_big.font" );
		}
	}
	catch( Exception &ex )
	{
		fprintf( stderr, "CEGUI Exception occured : %s", ex.getMessage().c_str() );
		exit( 0 );
	}

	pGuiSystem->setDefaultMouseCursor( "TaharezLook", "MouseArrow" );
}

void Init_SDL( void )
{
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) == -1 )
	{
		printf( "Error : SDL initialization failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}
	
	if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) == -1 )
	{
		printf( "Warning : SDL Joystick initialization failed\nReason : %s\n", SDL_GetError() );
		pPreferences->Joy_enabled = 0;
	}

	if( SDL_InitSubSystem( SDL_INIT_AUDIO ) == -1 )
	{
		printf( "Warning : SDL Audio initialization failed\nReason : %s\n", SDL_GetError() );
		pPreferences->Sounds = 0;
		pPreferences->Music = 0;
	}

	SDL_EnableUNICODE( 1 );
}

void Init_Video( void )
{
	int flags = GetVideoflags();
	
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 ); // hangs on 16 bit
    //SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 ); // not yet needed
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	screen = SDL_SetVideoMode( pPreferences->Screen_W, pPreferences->Screen_H, pPreferences->Screen_Bpp, flags );

	if( !screen )
	{
		printf( "Error : Screen mode creation failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}
	
	SDL_WM_SetCaption( CAPTION, NULL );
}

void Init_OpenGL( void )
{
    // viewport should cover the whole screen
    glViewport( 0, 0, pPreferences->Screen_W, pPreferences->Screen_H );

    // Camera projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	// Set Up An Ortho Screen
	glOrtho( 0, (double)pPreferences->Screen_W, (double)pPreferences->Screen_H, 0, -100, 100 );
	
    // now matrix operations 
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	// Smooth Shading
	glShadeModel( GL_SMOOTH );
	// Line Smoothing
	glEnable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    // clear color
    glClearColor( 0, 0, 0, 1 );
	// Depth Buffer Setup
	//glClearDepth( 1.0 );

    // Z-Buffer
	//glEnable( GL_DEPTH_TEST );
	// depth mask
	glDepthMask( GL_TRUE );
	// Depth function
	//glDepthFunc( GL_LEQUAL );
	// Blending
	glEnable( GL_BLEND );
	// Blending function
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	// Alpha
	//glEnable( GL_ALPHA_TEST );
	// Alpha function
	//glAlphaFunc( GL_GREATER, 0.1f );
	// Perspective Calculations
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    // back-facing polygons
    glDisable( GL_CULL_FACE );

	Init_ImageScale();
}

void Init_BasicColors( void )
{
	std_bgcolor.Parse_uint8( 150, 200, 225 );
	blue.Parse_uint8( 150, 200, 225 );
	darkblue.Parse_uint8( 0, 0, 128 );
	lightblue.Parse_uint8( 41, 167, 255 );
	black.Parse_uint8( 0, 0, 0 );
	white.Parse_uint8( 255, 255, 255 );
	grey.Parse_uint8( 128, 128, 128 );
	lightgrey.Parse_uint8( 64, 64, 64 );
	lightgreyalpha.Parse_uint8( 64, 64, 64, 64 );
	green.Parse_uint8( 0, 230, 0 );
	lightgreenalpha.Parse_uint8( 30, 230, 30, 64 );
	greenyellow.Parse_uint8( 154, 205, 50 );
	darkgreen.Parse_uint8( 1, 119, 34 );
	red.Parse_uint8( 250, 0, 0 );
	lightred.Parse_uint8( 253, 22, 12 );
	lila.Parse_uint8( 200, 0, 255 );
	orange.Parse_uint8( 248, 191, 38 );
}

void Init_ImageScale( void )
{
	global_scalex = (double)pPreferences->Screen_W / (double)GAME_RES_W;
	global_scaley = (double)pPreferences->Screen_H / (double)GAME_RES_H;
}

int GetVideoflags( void )
{
	const SDL_VideoInfo *videoinfo = SDL_GetVideoInfo();

	int flags = SDL_HWPALETTE | SDL_OPENGL | SDL_GL_DOUBLEBUFFER;

   // check if surfaces can be stored in video memory
    if( videoinfo->hw_available )
	{
		flags |= SDL_HWSURFACE;
	}
    else
	{
		printf( "Warning : using Software surfaces\n" );
		flags |= SDL_SWSURFACE;
	}

    // check if hardware blits can be done
    if( videoinfo->blit_hw )
	{
		flags |= SDL_HWACCEL;
	}

	if( pPreferences->Fullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}

	return flags;
}

bool TestVideo( int width, int height, int bpp )
{
	int flags = GetVideoflags();

	if( SDL_VideoModeOK( width, height, bpp, flags ) == 0 )
	{
		return 0;
	}

	return 1;
}

void ResetScreen( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
}

void Toggle_Fullscreen( void )
{
#ifdef WIN32
	// Todo
#else
	SDL_WM_ToggleFullScreen( screen );	// this command doesn't work for non-X11 platforms
#endif
}

GL_Surface *GetImage( string filename )
{
	if( filename.find( PIXMAPS_DIR ) == string::npos ) 
	{
		filename.insert( 0, "/" );
		filename.insert( 0, PIXMAPS_DIR );
	}

	string filename_final = filename;

	GL_Surface *image = pImageManager->Get_Pointer( filename_final );

	if( image )
	{
		return image;
	}

	image = LoadTexture( filename );

	if( image )
	{
		pImageManager->Add( image );	
	}
	
	return image;
}

GL_Surface *LoadTexture( string filename, bool use_settings /* = 1 */ )
{
	if( filename.find( PIXMAPS_DIR ) == string::npos ) 
	{
		filename.insert( 0, "/" );
		filename.insert( 0, PIXMAPS_DIR );
	}

	SDL_Surface *surface = NULL;
	GL_Surface *image = NULL;
	bool surface_found = 0;

	if( valid_file( filename ) ) // does the image exist
	{
		surface = IMG_Load( filename.c_str() );

		if( surface ) // got the image loaded
		{
			surface_found = 1;
			image = CreateTexture( surface ); // create the texture
		}
	}

	if( use_settings )
	{
		string settings_file = filename;
		settings_file.erase( settings_file.rfind( "." ) + 1, 3 );
		settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );

		if( valid_file( settings_file ) )
		{
			if( !image ) // if the image is based on another image in the settings
			{
				image = new GL_Surface();
			}

			cImage_settings *SettingsParser = new cImage_settings();
			SettingsParser->ApplyTextureSettings( image, settings_file );
			delete SettingsParser;
		}
	}

	if( image )
	{
		image->filename = filename;
	}

	if( !surface_found && !image )
	{
		printf( "Error loading image : %s\nReason : %s\n", filename.c_str(), SDL_GetError() );
	}
	else if( !image )
	{
		printf( "Error creating GL image : %s\nReason : %s\n", filename.c_str(), SDL_GetError() );
	}

	return image;
}

GL_Surface *CreateTexture( SDL_Surface *surface )
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
	// use the generated texture
	glBindTexture( GL_TEXTURE_2D, image_num );

	// affect how this texture is drawn later on
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// copies the sdl surface into the opengl texture
    glPixelStorei( GL_UNPACK_ROW_LENGTH, final->pitch / final->format->BytesPerPixel );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, final->w, final->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, final->pixels );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

	SDL_FreeSurface( final );

	GL_Surface *image = new GL_Surface();
	image->image = image_num;
	image->tex_w = width;
	image->tex_h = height;
	image->w = width;
	image->h = height;
	image->col_w = image->w;
	image->col_h = image->h;

#ifdef _DEBUG
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "GL Error found : %s\n", gluErrorString( error ) );
	}
#endif
	return image;
}

unsigned int Get_GLsize( unsigned int size )
{
	unsigned int value = 1;

	while( value < size )
	{
		value <<= 1;
	}

	return value;
}

Color Get_Color( Uint8 r, Uint8 g, Uint8 b, Uint8 a /* = 255 */ )
{
	return Color( r, g, b, a );
}

Color GetPixel( int x, int y, GL_Surface *source /* = NULL */ )
{
	//glReadBuffer( GL_FRONT );

	Color pixel = (Uint8)0;
	glLoadIdentity();
	//glReadPixels( x, y, 1, 1, GL_RGB, GL_UNSIGNED_INT, &pixel );

	return pixel;
}

void DrawPixel( double x, double y, Color *color, GL_Surface *target /* = NULL */ )
{
	DrawRect( x, y, 1, 1, color, target );
}

void DrawLine( GL_line *line, Color *color, GL_Surface *target /* = NULL */ )
{
	if( !line )
	{
		return;
	}

	DrawLine( line->x1, line->y1, line->x2, line->y2, color, target );
}

void DrawLine( double x1, double y1, double x2, double y2, Color *color, GL_Surface *target /* = NULL */ )
{
	if( !color )
	{
		return;
	}

	// TODO : target drawing support

	glLoadIdentity();

	// global scaling
	if( global_scalex != 1 || global_scaley != 1 )
	{
		glScaled( global_scalex, global_scaley, 0 );
	}

	if( target )
	{
		// draw on the target !!
	}

	glColor4ub( color->red, color->green, color->blue, color->alpha );

	glBegin( GL_LINES );
		glVertex2d( x1, y1 );
		glVertex2d( x2, y2 );
	glEnd();

	glColor4ub( 255, 255, 255, 255 );
}

void DrawRect( GL_rect *rect, Color *color, GL_Surface *target /* = NULL */ )
{
	if( !rect )
	{
		if( !target )
		{
			DrawRect( 0, 0, GAME_RES_W, GAME_RES_H, color );
		}
		else
		{
			DrawRect( 0, 0, target->w, target->h, color, target );
		}

		return;
	}

	DrawRect( rect->x, rect->y, rect->w, rect->h, color, target );
}

void DrawRect( double x, double y, double width, double height, Color *color, GL_Surface *target /* = NULL */ )
{
	if( !color || height == 0 || width == 0 )
	{
		return;
	}

	// TODO : target drawing support

	glLoadIdentity();

	// global image scaling
	if( global_scalex != 1 || global_scaley != 1 )
	{
		glScaled( global_scalex, global_scaley, 0 );
	}

	if( target )
	{
		// draw on the target !!
	}

	glColor4ub( color->red, color->green, color->blue, color->alpha );

	glBegin( GL_POLYGON );
		glVertex2d( x, y );
		glVertex2d( x + width, y );
		glVertex2d( x + width, y + height );
		glVertex2d( x, y + height );
	glEnd();

	glColor4ub( 255, 255, 255, 255 );
}

GL_Surface *RenderText( TTF_Font *font, string text, Color color )
{
	SDL_Color sdlcolor = color.Get_SDL_Color();

	return CreateTexture( TTF_RenderText_Blended( font, text.c_str(), sdlcolor ) );
}


double Get_scale( GL_Surface *image, double height, double width, bool only_downscale /* = 1 */ )
{
	if( !image )
	{
		return NULL;
	}

	if( !( only_downscale && image->h <= height && image->w <= width ) ) // size
	{
		double zoom = width / image->w;

		if( height / image->h < zoom ) // if height is smaller
		{
			zoom = height / image->h;
		}

		return zoom;
	}

	return 1;
}

void DrawEffect( FadeoutEffect effect, double speed )
{
	if( effect == RANDOM_EFFECT )
	{
		effect = (FadeoutEffect)( ( rand() % (TOTAL_FADE_EFFECTS - 1) ) + 1 );
	}

	// remember buffer mode
	int buffer = GL_BACK;
	glGetIntegerv( GL_DRAW_BUFFER, &buffer );

	switch( effect )
	{
	case BLACK_FADEOUT:
	{
		// switch to single buffer
		glDrawBuffer( GL_FRONT );

		Color color = (Uint8)0;

		for( double i = 1; i > 0; i -= ( speed / 30 ) * Framerate.speedfactor )
		{
			color.alpha = (Uint8)( ( 25 - ( 25 * i ) ) * Framerate.speedfactor );
			DrawRect( NULL, &color );
			
			glFlush();
			Framerate.Update();
		}
		break;
	}
	case HORIZONTAL_VERTICAL_FADE:
	{
		// switch to single buffer
		glDrawBuffer( GL_FRONT );

		double pos = 0;
		double size = 1;
		int hor = ( rand() % 2 ) - 1;
		Color color = Get_Color( rand() % 100, rand() % 100, rand() % 100, 150 );

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
			if( hor )
			{
				DrawRect( pos, 0, size, GAME_RES_H, &color ); // left/top
				DrawRect( GAME_RES_W - pos - size, 0, size, GAME_RES_H, &color ); // down/right

				size = 20 * Framerate.speedfactor;
				pos -= size;
			}
			else
			{
				DrawRect( 0, pos, GAME_RES_W, size, &color ); // left/top
				DrawRect( 0, GAME_RES_H - pos - size, GAME_RES_W, size, &color ); // down/right

				size = 15 * Framerate.speedfactor;
				pos -= size;
			}

			glFlush();

			Framerate.Update();
			CorrectFrameTime( DESIRED_FPS * 2 ); // correction needed
		}
		break;
	}
	case BIG_MUSHROOM:
	{
		double f = 0;
		GL_Surface *image = GetImage( "game/items/mushroom_red.png" );

		while( f < 50 )
		{
			f += 0.9 * Framerate.speedfactor;

			Color color = Get_Color( 255 - (int)( f * 4 ), 255 - (int)( f * 4 ), 255 - (int)( f * 4 ) );

			glColor3ub( color.red, color.green, color.blue );
			image->Blit( ( GAME_RES_W / 2 ) - ( ( image->w * f ) / 2 ) , GAME_RES_H / 2 - ( ( image->h * f ) / 2 ), NULL, 0, 0, 0, f );
			glColor3ub( 255, 255, 255 );

			SDL_GL_SwapBuffers();
			Framerate.Update();
		}

		break;
	}
	case RANDOM_COLOR_BOOST:
	{
		// switch to single buffer
		glDrawBuffer( GL_FRONT );

		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR );

		double f = ( ( GAME_RES_W * GAME_RES_H ) / 11000 );
		unsigned int rand_color = ( rand() % 4 );
		
		double xwidth = 8;

		GL_rect Rct;
		
		while( f > 0 )
		{
			xwidth += 1.1 * Framerate.speedfactor;

			for( unsigned int g = 0; g < 2000; g++ )
			{
				Rct.x = rand() % ( GAME_RES_W );
				Rct.y = rand() % ( GAME_RES_H );
				
				Rct.w = xwidth;
				Rct.h = xwidth;

				Color pixel = (Uint8)0;

				if( rand_color == 0 ) // red
				{
					pixel = Get_Color( 1, (Uint8)( Rct.y / 4 ), (Uint8)( Rct.x / 4 ), (Uint8)( 12 * Framerate.speedfactor ) );
				}
				else if( rand_color == 1 ) // green
				{
					pixel = Get_Color( (Uint8)( Rct.x/4 ), 1, (Uint8)( Rct.y/4 ), (Uint8)( 12 * Framerate.speedfactor ) );
				}
				else if( rand_color == 2 ) // blue
				{
					pixel = Get_Color( (Uint8)( Rct.x/4 ), (Uint8)( Rct.y/4 ), 1, (Uint8)( 12 * Framerate.speedfactor ) );
				}
				else // yellow
				{
					pixel = Get_Color( 1, 1, (Uint8)( Rct.y/4 ), (Uint8)( 15 * Framerate.speedfactor ) );
				}

				Rct.x -= xwidth / 2;
				Rct.y -= xwidth / 2;
				
				DrawRect( &Rct, &pixel );
			}
			
			f -= Framerate.speedfactor;

			glFlush();

			Framerate.Update();
		}
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;
	}
	case TILE_PIXELATION:
	{
		// switch to single buffer
		glDrawBuffer( GL_FRONT );

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
			
			Color color = (Uint8)0;
			DrawRect( &dest, &color );

			glFlush();
			Framerate.Update();
			CorrectFrameTime( DESIRED_FPS ); // correction needed
		}
		break;
	}
	case FIXED_COLORBOX_FADEOUT:
	{
		// switch to single buffer
		glDrawBuffer( GL_FRONT );

		//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR );
		glBlendFunc( GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA );

		double xsize = 4.5;
		int xtemp = 0;
		Uint16 xpos = 0;
		Uint16 ypos = 0;

		GL_rect Rct;
		Color color = (Uint8)0;

		unsigned int rand_color = ( rand() % 4 );

		if( rand_color == 1 ) // green
		{
			color = Get_Color( 10, 50, 10 );
		}
		else if( rand_color == 2 ) // blue
		{
			color = Get_Color( 10, 10, 50 );
		}
		else // yellow
		{
			color = Get_Color( 50, 50, 10 );
		}

		while( xsize < 90 )
		{
			xsize += 2.2 * Framerate.speedfactor;

			if( xtemp < (int)xsize )
			{
				xpos = (Uint16)( rand() % (unsigned int)xsize ) + 1;
				ypos = (Uint16)( rand() % (unsigned int)xsize ) + 1;
				xtemp = (int)xsize + 1;
			}

			for( unsigned int g = xpos; g < GAME_RES_W; g += (unsigned int)xtemp )
			{
				for( unsigned int y = ypos; y < GAME_RES_H; y += (unsigned int)xtemp )
				{
					Rct.x = g;
					Rct.y = y;
					
					Rct.w = 10 + ( rand() % ( 5 + ( xtemp / 5 ) ) );
					Rct.h = 10 + ( rand() % ( 5 + ( xtemp / 5 ) ) );

					Color pixel = Get_Color( rand() % color.red, rand() % color.green, rand() % color.blue, ( rand() % 3 ) + 2 );
					
					Rct.x -= 2 + ( xtemp / 2 );
					Rct.y -= 2 + ( xtemp / 2 );
					
					DrawRect( &Rct, &pixel );
					pixel.alpha += (Uint8)( xsize / 5 );
				}
			}

			DrawRect( NULL, &Get_Color( 0, 0, 0, 10 ) );

			glFlush();
			Framerate.Update();
		}
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;
	}
	default:
		break;  // be happy
	}
	
	glDrawBuffer( buffer ); // reset draw buffer
	Framerate.Update();
}

void DrawLoadingScreen( void )
{
	Window *window_1 = WindowManager::getSingleton().loadWindowLayout( "data/gui/layout/loading.layout" );
	pGuiSystem->setGUISheet( window_1 );
	cSprite *loading_img_1 = new cSprite( GetImage( "menu/extra_1.png" ), GAME_RES_W / 7, (double)( GAME_RES_H / 1.5 ) );
	cSprite *loading_img_2 = new cSprite( GetImage( "menu/extra_2.png" ), GAME_RES_W / 1.4, (double)( GAME_RES_H / 10 ) );

	glClearColor( 1, 1, 1, 1 ); // white
	ResetScreen();
	loading_img_1->Draw();
	loading_img_2->Draw();
	pGuiSystem->renderGUI();
	SDL_GL_SwapBuffers();

	delete loading_img_1;
	delete loading_img_2;
	WindowManager::getSingleton().destroyWindow( window_1 );
}
