/***************************************************************************
			img_settings.h  -  header for the corresponding cpp file
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

#ifndef __IMG_SETTINGS_H__
#define __IMG_SETTINGS_H__

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Class: cImage_settings
// Image Settings Handler
// Inherits from <cFile_parser>
class cImage_settings : public cFile_parser
{
public:
	cImage_settings( void );
	virtual ~cImage_settings( void );

	// Apply the settings from the given file on the given image
	void ApplyTextureSettings( GL_Surface *image, string filename );

	// Handles the Image Settings Line
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );

	// the surface pointer used
	GL_Surface *surface;
};


#endif

