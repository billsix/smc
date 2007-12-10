/***************************************************************************
              jpiranha.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __JPIRANHA_H__
#define __JPIRANHA_H__

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
	cjPiranha( XMLAttributes &attributes );
	// destructor
	virtual ~cjPiranha( void );

	// init defaults
	void Init( void );
	// copy
	virtual cjPiranha *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_to_Savegame( void );

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
	void Update_dest_vel( void );

	// if update is valid for the current state
	virtual bool is_Update_valid( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
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
	bool Editor_Max_Distance_Key( const EventArgs &event );  // editor max distance key up
	bool Editor_Speed_Key( const EventArgs &event );  // editor speed key up

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
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
