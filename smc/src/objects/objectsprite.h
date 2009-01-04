/***************************************************************************
 * objectsprite.h  -  header for the corresponding cpp file
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

#ifndef SMC_OBJECTSPRITE_H
#define SMC_OBJECTSPRITE_H

#include "../objects/movingsprite.h"

/* *** *** *** *** *** *** *** cImageObjectSprite *** *** *** *** *** *** *** *** *** *** */

class cImageObjectSprite : public cMovingSprite
{
public:
	cImageObjectSprite( float x = 0, float y = 0 );
	virtual ~cImageObjectSprite( void );
	
	/* Sets the image for drawing
	 * if new_start_image is set the default start_image will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	virtual void Set_Image( int num, bool new_startimage = 0, bool del_img = 0 );
	// Get an array image
	cGL_Surface *Get_Image( unsigned int num );
	// Clears images list
	void Clear_Images( void );

	// Current image array number for drawing
	int curr_img;
	// Image list
	typedef vector<cGL_Surface *> ObjectImageList;
	ObjectImageList images;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif

