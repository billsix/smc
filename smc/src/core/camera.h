/***************************************************************************
                camera.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __CAMERA_H__
#define __CAMERA_H__

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
	void Set_PosX( float nx );
	void Set_PosY( float ny );
	// move the camera
	void Move( float move_x, float move_y );

	// update
	void Update( void );

	// center on the player with the given direction ( DIR_HORIZONTAL, DIR_VERTICAL and DIR_ALL )
	void Center( ObjectDirection direction = DIR_ALL );

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
	void Update_LimitX( float &nx );
	void Update_LimitY( float &ny );
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
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Camera class
extern cCamera *pCamera;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
