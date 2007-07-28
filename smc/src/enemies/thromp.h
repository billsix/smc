/***************************************************************************
              thromp.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __THROMP_H__
#define __THROMP_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** cThromp *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thromp
 * The falling Stone
*/
class cThromp : public cEnemy
{
public:
	// constructor
	cThromp( float x, float y );
	// create from stream
	cThromp( XMLAttributes &attributes );
	// destructor
	virtual ~cThromp( void );

	// init defaults
	void Init( void );
	// copy
	virtual cThromp *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// Set Max Distance
	void Set_Max_Distance( int nmax_distance );
	// Set the Speed
	void Set_Speed( float val );

	// Jump out
	void Activate( void );
	// Moves back to the original position
	void Move_Back( void );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// update destination velocity from the speed
	void Update_dest_vel( void );
	// returns the distance rect
	GL_rect Get_Distance_rect( void );

	// Generates Smoke Particles
	void Generate_Smoke( unsigned int power = 10 );

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
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Max_Distance_Key( const EventArgs &event );  // editor max distance key up
	bool Editor_Speed_Key( const EventArgs &event );  // editor speed key up

	// speed
	float speed;
	// destination direction velocity
	float dest_velx, dest_vely;
	// maximum distance from the startposition
	int max_distance;

	// moving back to the original position
	bool move_back;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
