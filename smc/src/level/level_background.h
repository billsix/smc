/***************************************************************************
 * level_background.h  -  header for the corresponding cpp file
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

#ifndef SMC_LEVEL_BACKGROUND_H
#define SMC_LEVEL_BACKGROUND_H

#include "../core/globals.h"
#include "../video/video.h"
#include "../core/obj_manager.h"

/* *** *** *** *** *** *** *** Background Image Type *** *** *** *** *** *** *** *** *** *** */

enum BackgroundType
{
	BG_NONE = 0,			// no background type
	BG_IMG_BOTTOM = 1,		// only horizontal on the bottom
	BG_IMG_ALL = 2,			// into all directions
	BG_IMG_TOP = 3,			// only horizontal on top
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
	cBackground( CEGUI::XMLAttributes &attributes );
	// destructor
	~cBackground( void );

	// Init defaults
	void Init( void );

	// create from stream
	void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	void Save_to_Stream( ofstream &file );

	// Sets the type of Background
	void Set_Type( BackgroundType ntype );
	void Set_Type( string ntype );

	// Sets the background color
	void Set_Color_1( const Color &color );
	void Set_Color_2( const Color &color );

	// Set the Position
	void Set_Pos( float x, float y );
	// Set the Position z
	void Set_Pos_Z( float val );
	// Set the Background image
	void Set_Image( string nimg_file_1 );
	// Set the Background Image scrolling speed
	void Set_Scroll_Speed( float x = 1, float y = 1 );

	// Set constant x velocity
	void Set_Const_Velocity_X( float vel );
	// Set constant y velocity
	void Set_Const_Velocity_Y( float vel );

	// Update
	void Update( void );
	// draw
	void Draw( void );
	// draw gradient
	void Draw_Gradient( void );

	// Returns the name of the current type
	string Get_Type_Name( void );

	// type
	BackgroundType type;
	// position
	float posx, posy, posz;

	// - background image settings
	// image
	GL_Surface *img_1;
	// image filename
	string img_file_1;
	// scrolling speed
	float speedx, speedy;
	// constant velocity
	float const_velx, const_vely;

	// - background gradient settings
	// colors
	Color color_1, color_2;
};

/* *** *** *** *** *** cBackground_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

class cBackground_Manager : public cObject_Manager<cBackground>
{
public:
	cBackground_Manager( void );
	virtual ~cBackground_Manager( void );
};

#endif
