/***************************************************************************
              jpiranha.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __JPIRANHA_H__
#define __JPIRANHA_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** cjPiranha *** *** *** *** *** *** *** *** *** *** *** *** */

/* Jumping Piranha
 * No Description.
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

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// Set Max Distance
	void Set_Max_Distance( float nmax_distance );
	// Set the Speed
	void Set_Speed( float val );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// update destination velocity from the speed
	void Update_dest_vel( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
	
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
