/***************************************************************************
              rokko.h  -  headers for the corresponding cpp file
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

#ifndef __ROKKO_H__
#define __ROKKO_H__

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
	cRokko( XMLAttributes &attributes );
	// destructor
	virtual ~cRokko( void );

	// init defaults
	void Init( void );
	// copy
	virtual cRokko *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// set flying speed
	void Set_Speed( float nspeed );
	// activate
	void Activate( void );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Generates Smoke Particles
	void Generate_Smoke( unsigned int quota = 10 );

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
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Speed_Key( const EventArgs &event );  // editor speed key up

	// smoke particle counter
	float smoke_counter;
	// flying speed
	float speed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
