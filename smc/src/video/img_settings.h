/***************************************************************************
			img_settings.h  -  header for the corresponding cpp file
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

#ifndef __IMG_SETTINGS_H__
#define __IMG_SETTINGS_H__

#include "../core/globals.h"
#include "../core/file_parser.h"
#include "../video/gl_surface.h"
#include "../core/math/rect.h"


/* *** *** *** *** *** *** cImage_settings_data *** *** *** *** *** *** *** *** *** *** *** */

class cImage_settings_data
{
public:
	cImage_settings_data( void );
	~cImage_settings_data( void );

	// Apply settings to an image
	void Apply( GL_Surface *image );
	// Apply base settings
	void Apply_base( cImage_settings_data *base_settings_data );

	// base image/settings path
	string base;
	// inherit base settings
	bool base_settings;

	// internal drawing offset
	int int_x, int_y;
	// collision rect
	GL_rect col_rect;
	// width
	int width;
	// height
	int height;
	// rotation
	int rotation_x, rotation_y, rotation_z;
	// texture mipmapping
	bool mipmap;

	// editor tags
	string editor_tags;
	// name
	string name;
	// sprite type
	int type;
	// ground type
	GroundType ground_type;
	// obsolete
	bool obsolete;
};

/* *** *** *** *** *** *** cImage_settings *** *** *** *** *** *** *** *** *** *** *** */

// Image Settings Handler
// Inherits from cFile_parser
class cImage_settings : public cFile_parser
{
public:
	cImage_settings( void );
	virtual ~cImage_settings( void );

	/* Returns the settings from the given file
	 * load_base_settings : if set will overwrite settings with all base settings if available
	*/
	cImage_settings_data *Get( string &filename, bool load_base_settings = 1 );

	// Handles the Image Settings Line
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );

	// the surface pointer used
	cImage_settings_data *settings;
	// load base settings
	bool load_base;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Image Settings Handler class
extern cImage_settings *pSettingsParser;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
