/***************************************************************************
 * moving_platform.h  -  header for the corresponding cpp file
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

#ifndef SMC_MOVING_PLATFORM_H
#define SMC_MOVING_PLATFORM_H

#include "../core/globals.h"
#include "../objects/objectsprite.h"


/* *** *** *** *** *** *** *** *** Moving Platform states *** *** *** *** *** *** *** *** *** */

enum Moving_Platform_State
{
    PLATFORM_STAY = 0,
    PLATFORM_TOUCHED = 1,
    PLATFORM_SHAKE = 2,
    PLATFORM_FALL = 3
};

/* *** *** *** *** *** *** cMoving_Platform *** *** *** *** *** *** *** *** *** *** *** */

/* todo : 
 * - platform type : static(current) and rotating
*/
class cMoving_Platform : public cImageObjectSprite
{
public:
	// constructor
	cMoving_Platform( float x, float y );
	// create from stream
	cMoving_Platform( CEGUI::XMLAttributes &attributes );
	// destructor
	~cMoving_Platform( void );
	
	// init defaults
	void Init( void );
	// copy
	virtual cMoving_Platform *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_to_Savegame( void );

	// Set Massive Type
	virtual void Set_Massive_Type( MassiveType mtype );
	// Set Direction
	void Set_Direction( ObjectDirection dir, bool new_start_direction = 0 );
	// Set Max Distance
	void Set_Max_Distance( int nmax_distance );
	// Set the Speed
	void Set_Speed( float val );
	// Set time when touched until it starts shaking
	void Set_Touch_Time( float val );
	// Set time it's shaking until falling
	void Set_Shake_Time( float val );
	/* Set time when touched until it starts moving
	 * if set to 0 automatically moves
	*/
	void Set_Touch_Move_Time( float val );

	// Set the middle image count
	void Set_Middle_Count( int val );
	// set image
	void Set_Image_Top_Left( GL_Surface *surface );
	void Set_Image_Top_Middle( GL_Surface *surface );
	void Set_Image_Top_Right( GL_Surface *surface );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request /* = NULL */ );

	// Update rect
	void Update_Rect( void );
	// Update velocity
	void Update_Velocity( void );

	// ignore onground check
	virtual void Check_on_Ground( void ) {};

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Max_Distance_Key( const CEGUI::EventArgs &event );  // editor max distance key up
	bool Editor_Speed_Key( const CEGUI::EventArgs &event );  // editor speed key up
	bool Editor_Hor_Middle_Count_Key( const CEGUI::EventArgs &event );  // editor horizontal middle image count key up
	bool Editor_Image_Top_Left_Key( const CEGUI::EventArgs &event );  // editor image top left key up
	bool Editor_Image_Top_Middle_Key( const CEGUI::EventArgs &event );  // editor image top middle key up
	bool Editor_Image_Top_Right_Key( const CEGUI::EventArgs &event );  // editor image top right key up
	bool Editor_Touch_Time_Key( const CEGUI::EventArgs &event );  // editor touch time key up
	bool Editor_Shake_Time_Key( const CEGUI::EventArgs &event );  // editor shaking time key up
	bool Editor_Touch_Move_Time_Key( const CEGUI::EventArgs &event );  // editor touch moving time key up

	// current direction
	ObjectDirection moving_direction;
	// moving speed
	float speed;
	// max moving distance
	int max_distance;
	// count the middle image is used
	unsigned int middle_count;

	// internal platform state
    Moving_Platform_State platform_state;

	// time when touched until it starts shaking
	float touch_time;
	// time since the platform got touched
	float touch_counter;
	// time it's shaking until falling
	float shake_time;
	// time since shaking started
	float shake_counter;
	// shaking direction counter
	float shake_dir_counter;
	// time when touched until it starts moving
	float touch_move_time;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
