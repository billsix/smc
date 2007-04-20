/***************************************************************************
			font.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FONT_H__
#define __FONT_H__

#include "../core/globals.h"
#include "../video/img_manager.h"

/* *** *** *** *** *** *** *** Font class *** *** *** *** *** *** *** *** *** *** */

// Deletes an active Font Surface
void Font_Delete_Ref( GL_Surface *surface );

class cFont
{
public:
	cFont( void );
	~cFont( void );

	// initialization
	void Init( void );

	// Adds an allocated Font surface
	void Add_Ref( GL_Surface *surface );
	// Deletes an active Font Surface
	void Delete_Ref( GL_Surface *surface );

	// Renders the given text into a new surface
	GL_Surface *RenderText( TTF_Font *font, string text, Color color = (Uint8)0 );

	/*
	* Function : Grab_Textures
	* saves hardware textures in software memory
	*/
	void Grab_Textures( void );

	/*
	* Function : Reload_Textures
	* Loads the saved software textures back into hardware textures
	*/
	void Restore_Textures( void );

	// TTF loaded fonts
	TTF_Font *font_normal, *font_small, *font_very_small;

	// current active loaded font list
	typedef vector<GL_Surface *> ActiveFontList;
	ActiveFontList active_fonts;

	// saved software textures for reloading
	SoftwareTextureList software_textures;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Font Handler
extern cFont *pFont;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
