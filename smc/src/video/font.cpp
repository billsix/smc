/***************************************************************************
 * font.cpp  -  internal font functions
 *
 * Copyright (C) 2006 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../video/font.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void Font_Delete_Ref( cGL_Surface *surface )
{
	pFont->Delete_Ref( surface );
}

/* *** *** *** *** *** *** *** Font Manager class *** *** *** *** *** *** *** *** *** *** */

cFont_Manager :: cFont_Manager( void )
{
	font_normal = NULL;
	font_small = NULL;
	font_very_small = NULL;
}

cFont_Manager :: ~cFont_Manager( void )
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

void cFont_Manager :: Init( void )
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
		exit( EXIT_FAILURE );
	}

	// open fonts
	font_normal = TTF_OpenFont( DATA_DIR "/" GUI_FONT_DIR "/default_bold.ttf", 18 );
	font_small = TTF_OpenFont( DATA_DIR "/" GUI_FONT_DIR "/default_bold.ttf", 11 );
	font_very_small = TTF_OpenFont( DATA_DIR "/" GUI_FONT_DIR "/default_bold.ttf", 9 );

	// if loading failed
	if( !font_normal || !font_small || !font_very_small )
	{
		printf( "Error : Font loading failed from directory %s\n", GUI_FONT_DIR );
		exit( EXIT_FAILURE );
	}
}

void cFont_Manager :: Add_Ref( cGL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	active_fonts.push_back( surface );
}

void cFont_Manager :: Delete_Ref( cGL_Surface *surface )
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

cGL_Surface *cFont_Manager :: Render_Text( TTF_Font *font, string text, Color color )
{
	// get SDL Color
	SDL_Color sdlcolor = color.Get_SDL_Color();
	// create text surface
	cGL_Surface *surface = pVideo->Create_Texture( TTF_RenderUTF8_Blended( font, text.c_str(), sdlcolor ) );

	if( !surface )
	{
		return NULL;
	}

	surface->filename = text;

	// set function if font gets deleted
	surface->Set_Destruction_Function( &Font_Delete_Ref );
	// add font to active fonts
	Add_Ref( surface );
	
	return surface;
}

void cFont_Manager :: Grab_Textures( void )
{
	// save to software memory
	for( ActiveFontList::iterator itr = active_fonts.begin(), itr_end = active_fonts.end(); itr != itr_end; ++itr )
	{
		cGL_Surface *obj = (*itr);

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

void cFont_Manager :: Restore_Textures( void )
{
	// load back into hardware textures
	for( Saved_Texture_List::iterator itr = software_textures.begin(), itr_end = software_textures.end(); itr != itr_end; ++itr )
	{
		// get saved texture
		cSaved_Texture *soft_tex = (*itr);
		// load it
		soft_tex->base->Load_Software_Texture( soft_tex );
	}

	// delete software textures
	for( Saved_Texture_List::iterator itr = software_textures.begin(), itr_end = software_textures.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	software_textures.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
namespace SMC
{ // work around conflicts caused by lack of namespace use in smc
cFont_Manager *pFont = NULL;
}
