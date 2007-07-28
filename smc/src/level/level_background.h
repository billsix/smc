/***************************************************************************
                level_background.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LEVEL_BACKGROUND_H__
#define __LEVEL_BACKGROUND_H__

#include "../core/globals.h"
#include "../video/video.h"

/* *** *** *** *** *** *** *** Background Image Type *** *** *** *** *** *** *** *** *** *** */

enum  BackgroundType
{
	BG_NONE = 0,			// no background type
	BG_IMG_LEFTRIGHT = 1,	// only to the horizontal
	BG_IMG_ALL = 2,			// into all directions
	BG_IMG_TOP = 3,			// is always on top
	BG_GR_VER = 103,		// Gradient Vertical
	BG_GR_HOR = 104			// Gradient Horizontal
};

/* *** *** *** *** *** *** *** Background class *** *** *** *** *** *** *** *** *** *** */

class cBackground
{
public:
	// default constructor
	cBackground( void );
	// create from stream
	cBackground( XMLAttributes &attributes );
	// destructor
	~cBackground( void );

	// Init defaults
	void Init( void );

	// create from stream
	void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	void Save_to_Stream( ofstream &file );

	// Sets the type of Background
	void Set_type( BackgroundType ntype );
	void Set_type( string ntype );

	// Sets the background color
	void Set_color_1( const Color &color );
	void Set_color_2( const Color &color );

	// Set the Background image
	void Set_image( string nimg_file_1 );
	// Set the Background Image scrolling speed
	void Set_scroll_speed( float x = 1, float y = 1 );
	// Set the Position Z
	void Set_PosZ( float val );

	void Draw( void );
	void DrawGradient( void );

	// Returns the name of the current type
	string Get_Typename( void );

	// type
	BackgroundType type;

	// background image filename
	string img_file_1;
	// background image
	GL_Surface *img_1;

	// position
	float posx, posy, posz;

	// background image scrolling speed
	float speedx, speedy;

	// background gradient colors
	Color color_1, color_2;
};

#endif
