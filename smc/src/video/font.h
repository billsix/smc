/***************************************************************************
 * font.h  -  header for the corresponding cpp file
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

#ifndef SMC_FONT_H
#define SMC_FONT_H

#include "../core/globals.h"
#include "../video/img_manager.h"

/* *** *** *** *** *** *** *** Font Manager class *** *** *** *** *** *** *** *** *** *** */

// Deletes an active Font Surface
void Font_Delete_Ref( cGL_Surface *surface );

class cFont_Manager
{
public:
	cFont_Manager( void );
	~cFont_Manager( void );

	// initialization
	void Init( void );

	// Adds an allocated Font surface
	void Add_Ref( cGL_Surface *surface );
	// Deletes an active Font Surface
	void Delete_Ref( cGL_Surface *surface );

	// Renders the given text into a new surface
	cGL_Surface *Render_Text( TTF_Font *font, string text, Color color = static_cast<Uint8>(0) );

	/* Saves hardware textures in software memory
	*/
	void Grab_Textures( void );

	/* Loads the saved software textures back into hardware textures
	*/
	void Restore_Textures( void );

	// TTF loaded fonts
	TTF_Font *font_normal, *font_small, *font_very_small;

	// current active loaded font list
	typedef vector<cGL_Surface *> ActiveFontList;
	ActiveFontList active_fonts;

	// saved software textures only used for reloading
	Saved_Texture_List software_textures;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Font Handler
namespace SMC
{ // work around conflicts caused by lack of namespace use in smc
extern cFont_Manager *pFont;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
using namespace SMC;

#endif
