/***************************************************************************
 * rokko.h  -  headers for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_ROKKO_H
#define SMC_ROKKO_H

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cRokko *** *** *** *** *** *** *** *** *** *** *** */

/*	A giant, slow-moving bullet
 */
class cRokko : public cEnemy 
{
public:
	// constructor
	cRokko( float x, float y );
	// create from stream
	cRokko( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cRokko( void );

	// init defaults
	void Init( void );
	// copy
	virtual cRokko *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// set flying speed
	void Set_Speed( float nspeed );
	// Set max detection distance for the front
	void Set_Max_Distance_Front( float distance );
	// Set max detection distance for the sides
	void Set_Max_Distance_Sides( float distance );
	// activate
	void Activate( void );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// update the distance rect
	void Update_Distance_rect( void );
	// Get the final distance rect
	GL_rect Get_Final_Distance_Rect( void );

	// Generates Smoke Particles
	void Generate_Smoke( unsigned int quota = 10 );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// ignore onground check
	virtual void Check_on_Ground( void ) {};
	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_out_of_Level( ObjectDirection dir );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Speed_Key( const CEGUI::EventArgs &event );  // editor speed key up

	// smoke particle counter
	float smoke_counter;
	// flying speed
	float speed;
	// maximum detection distance for the front
	float max_distance_front;
	// maximum detection distance for the sides
	float max_distance_sides;
	// detection distance rect
	GL_rect distance_rect;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
