/***************************************************************************
 * jpiranha.h  -  headers for the corresponding cpp file
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

#ifndef SMC_JPIRANHA_H
#define SMC_JPIRANHA_H

#include "../enemies/enemy.h"

/* *** *** *** *** *** cjPiranha *** *** *** *** *** *** *** *** *** *** *** *** */

/* Jumping Piranha
 * Eats your butt
*/
class cjPiranha : public cEnemy 
{
public:
	// constructor
	cjPiranha( float x, float y );
	// create from stream
	cjPiranha( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cjPiranha( void );

	// initialize defaults
	void Init( void );
	// copy
	virtual cjPiranha *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_to_Savegame( void );

	// Set the image directory
	void Set_Image_Dir( string filename );
	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// Set Max Distance
	void Set_Max_Distance( float nmax_distance );
	// Set the Speed
	void Set_Speed( float val );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Returns the distance to the end position
	float Get_End_Distance( void );
	// update destination velocity from the speed
	void Update_Dest_Vel( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	
	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor image dir key up event
	bool Editor_Image_Dir_Key( const CEGUI::EventArgs &event );
	// editor max distance key up
	bool Editor_Max_Distance_Key( const CEGUI::EventArgs &event );
	// editor speed key up
	bool Editor_Speed_Key( const CEGUI::EventArgs &event );

	// image directory
	string img_dir;

	// moving speed
	float speed;
	// destination direction velocity
	float dest_velx, dest_vely;
	// maximum distance from the startposition
	float max_distance;

	// time to wait until next approach
	float wait_time;
	// moving back to the original position
	bool move_back;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
