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

enum Falling_Platform_State
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
	// Set time until falling
	void Set_Time_Fall( float val );

	// set image
	void Set_image_top_left( GL_Surface *surface );
	void Set_image_top_middle( GL_Surface *surface );
	void Set_image_top_right( GL_Surface *surface );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request /* = NULL */ );

	// update rect
	void Update_rect( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// if update is valid for the current state
	virtual bool is_Update_valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
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

	// internal platform state
    Falling_Platform_State platform_state;

	// time until falling
	float time_fall;
	// time since contact of the platform
	float contact_time;
	// shaking direction counter
	float shake_dir_counter;
	// time of shaking
	float shake_counter;
	// count the middle image is used
	unsigned int middle_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
