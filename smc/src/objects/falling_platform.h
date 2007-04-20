/***************************************************************************
                falling_platform.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FALLING_PLATFORM_H__
#define __FALLING_PLATFORM_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** *** Falling Platform states *** *** *** *** *** *** *** *** *** */

enum Falling_Platform_Status
{
    PLATFORM_STAY = 0,
    PLATFORM_TOUCHED = 1,
    PLATFORM_SHAKE = 2,
    PLATFORM_FALL = 3
};

/* *** *** *** *** *** *** cFalling_Platform *** *** *** *** *** *** *** *** *** *** *** */

class cFalling_Platform : public cImageObjectSprite
{
public:
	// constructor
	cFalling_Platform( float x, float y );
	// create from stream
	cFalling_Platform( XMLAttributes &attributes );
	// destructor
	~cFalling_Platform( void );

	// init defaults
	void Init( void );

	// copy
	virtual cFalling_Platform *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Massive Type
	virtual void Set_Massivetype( MassiveType mtype );
	// Set the middle image count
	void Set_Middle_Count( int val );

	void Set_Time_Fall( float val );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request /* = NULL */ );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// set image
	void Set_image_top_left( GL_Surface *surface );
	void Set_image_top_middle( GL_Surface *surface );
	void Set_image_top_right( GL_Surface *surface );
	// update rect
	void Update_rect( void );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Time_Fall_Key( const EventArgs &event );  // editor speed key up
	bool Editor_Hor_Middle_Count_Key( const EventArgs &event );  // editor horizontal middle image count key up
	bool Editor_Image_Top_Left_Key( const EventArgs &event );  // editor image top left key up
	bool Editor_Image_Top_Middle_Key( const EventArgs &event );  // editor image top middle key up
	bool Editor_Image_Top_Right_Key( const EventArgs &event );  // editor image top right key up

    Falling_Platform_Status falling_platform_status;

	// falling speed
	float time_fall;
	// time since contact of the platform
	float contact_time;
	// shaking to indicate the platform will fall down
	bool shake_left;
	// time of shaking
	int shake_counter;
	// count the middle image is used
	unsigned int middle_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
