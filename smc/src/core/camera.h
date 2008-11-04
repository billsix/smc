/***************************************************************************
 * camera.h  -  header for the corresponding cpp file
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

#ifndef SMC_CAMERA_H
#define SMC_CAMERA_H

#include "../core/globals.h"
#include "../core/math/rect.h"

/* *** *** *** *** *** cCamera *** *** *** *** *** *** *** *** *** *** *** *** */

class cCamera
{
public:
	cCamera( void );
	~cCamera( void );

	// set camera position
	void Set_Pos( float nx, float ny );
	void Set_Pos_X( float nx );
	void Set_Pos_Y( float ny );
	// move the camera
	void Move( float move_x, float move_y );
	/* moves to the given position gradually
	 * returns 0 if reached nearest the possible position
	*/
	bool Move_to_Position_Gradually( float px, float py );

	// update
	void Update( void );

	// center on the player with the given direction ( DIR_HORIZONTAL, DIR_VERTICAL and DIR_ALL )
	void Center( ObjectDirection direction = DIR_ALL );
	// get centered player position x
	float Get_Center_Pos_X( void );
	// get centered player position y
	float Get_Center_Pos_Y( void );

	// reset limits
	void Reset_Limits( void );
	// set limits
	void Set_Limits( GL_rect rect );
	void Set_Limit_X( float val );
	void Set_Limit_Y( float val );
	void Set_Limit_W( float val );
	void Set_Limit_H( float val );
	// update limit with the given position
	void Update_Limit( float &nx, float &ny );
	void Update_Limit_X( float &nx );
	void Update_Limit_Y( float &ny );
	// update if position changed
	void Update_Position( void );

	// position
	float x, y;
	// additional position offset
	float x_offset, y_offset;
	// position offset speed
	float hor_offset_speed, ver_offset_speed;
	// limits
	GL_rect limit_rect;

	// fixed horizontal scrolling velocity
	float fixed_hor_vel;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
