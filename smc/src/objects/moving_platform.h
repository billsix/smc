/***************************************************************************
                moving_platform.h  -  header for the corresponding cpp file
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

#ifndef __MOVING_PLATFORM_H__
#define __MOVING_PLATFORM_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"


/* todo : 
 * - platform type : static(current) and rotating
 * - also add new bool falling
*/


/* *** *** *** *** *** *** cMoving_Platform *** *** *** *** *** *** *** *** *** *** *** */

class cMoving_Platform : public cImageObjectSprite
{
public:
	// constructor
	cMoving_Platform( float x, float y );
	// create from stream
	cMoving_Platform( XMLAttributes &attributes );
	// destructor
	~cMoving_Platform( void );
	
	// init defaults
	void Init( void );
	// copy
	virtual cMoving_Platform *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Massive Type
	virtual void Set_Massivetype( MassiveType mtype );
	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// Set Max Distance
	void Set_Max_Distance( int nmax_distance );
	// Set the Speed
	void Set_Speed( float val );
	// Set the middle image count
	void Set_Middle_Count( int val );
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

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Max_Distance_Key( const EventArgs &event );  // editor max distance key up
	bool Editor_Speed_Key( const EventArgs &event );  // editor speed key up
	bool Editor_Hor_Middle_Count_Key( const EventArgs &event );  // editor horizontal middle image count key up
	bool Editor_Image_Top_Left_Key( const EventArgs &event );  // editor image top left key up
	bool Editor_Image_Top_Middle_Key( const EventArgs &event );  // editor image top middle key up
	bool Editor_Image_Top_Right_Key( const EventArgs &event );  // editor image top right key up

	// current direction
	ObjectDirection moving_direction;
	// moving speed
	float speed;
	// max moving distance
	int max_distance;
	// count the middle image is used
	unsigned int middle_count;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
