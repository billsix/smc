/***************************************************************************
    font.cpp  -  internal font functions
                             -------------------
    copyright            : (C) 2006 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 
#include "../video/font.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void Font_Delete_Ref( GL_Surface *surface )
{
	pFont->Delete_Ref( surface );
}

/* *** *** *** *** *** *** *** Font class *** *** *** *** *** *** *** *** *** *** */

cFont :: cFont( void )
{
	font_normal = NULL;
	font_small = NULL;
	font_very_small = NULL;
}

cFont :: ~cFont( void )
{
	if( TTF_WasInit() )
	{
		if( font_normal )
		{
			TTF_CloseFont( font_normal );
			font_normal = NULL;
		}

		if( font_small )
		{
			TTF_CloseFont( font_small );
			font_small = NULL;
		}

		if( font_very_small )
		{
			TTF_CloseFont( font_very_small );
			font_very_small = NULL;
		}

		TTF_Quit();
	}
}

void cFont :: Init( void )
{
	// if already initialised
	if( TTF_WasInit() )
	{
		return;
	}
	
	// init ttf
	if( TTF_Init() == -1 )
	{
		printf( "Error : SDL_TTF initialization failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}

	// open fonts
	font_normal = TTF_OpenFont( GUI_FONT_DIR "/default_bold.ttf", 18 );
	font_small = TTF_OpenFont( GUI_FONT_DIR "/default_bold.ttf", 11 );
	font_very_small = TTF_OpenFont( GUI_FONT_DIR "/default_bold.ttf", 9 );

	// if loading failed
	if( !font_normal || !font_small || !font_very_small )
	{
		printf( "Error : Font loading failed from directory %s\n", GUI_FONT_DIR );
		exit( 0 );
	}
}

void cFont :: Add_Ref( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	active_fonts.push_back( surface );
}

void cFont :: Delete_Ref( GL_Surface *surface )
{
	for( unsigned int i = 0; i < active_fonts.size(); i++ )
	{
		// delete reference if found
		if( active_fonts[i] == surface )
		{
			active_fonts.erase( active_fonts.begin() + i );
			return;
		}
	}
}

GL_Surface *cFont :: RenderText( TTF_Font *font, string text, Color color )
{
	// get SDL Color
	SDL_Color sdlcolor = color.Get_SDL_Color();
	// create text surface
	GL_Surface *surface = pVideo->CreateTexture( TTF_RenderText_Blended( font, text.c_str(), sdlcolor ) );

	if( !surface )
	{
		return NULL;
	}

	surface->filename = text;

	// set function if font gets deleted
	surface->Set_destruction_function( &Font_Delete_Ref );
	// add font to active fonts
	Add_Ref( surface );
	
	return surface;
}

void cFont :: Grab_Textures( void )
{
	// save to software memory
	for( unsigned int i = 0; i < active_fonts.size(); i++ ) 
	{
		GL_Surface *obj = active_fonts[i];

		// get software texture and save it
		software_textures.push_back( obj->Get_Software_Texture() );
		// delete hardware texture
		if( glIsTexture( obj->image ) )
		{
			glDeleteTextures( 1, &obj->image );
		}
		obj->image = 0;
	}
}

void cFont :: Restore_Textures( void )
{
	// load back into hardware textures
	for( unsigned int i = 0; i < software_textures.size(); i++ )
	{
		// get saved texture
		cSoftware_texture *soft_tex = software_textures[i];
		// load it
		soft_tex->base->Load_Software_Texture( soft_tex );
	}

	// delete software surfaces
	for( unsigned int i = 0; i < software_textures.size(); i++ )
	{
		delete software_textures[i];
	}

	software_textures.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFont *pFont = NULL;
