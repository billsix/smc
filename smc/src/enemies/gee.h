/***************************************************************************
              gee.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __GEE_H__
#define __GEE_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** cGee *** *** *** *** *** *** *** *** *** *** *** *** */

/*
 * Shocks you with Electro, Lava or Gift
*/
class cGee : public cEnemy
{
public:
	// constructor
	cGee( float x, float y );
	// create from stream
	cGee( XMLAttributes &attributes );
	// destructor
	virtual ~cGee( void );

	// init defaults
	void Init( void );
	// copy
	virtual cGee *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// Set Max Distance
	void Set_Max_Distance( int nmax_distance );
	// set color
	void Set_Color( DefaultColor col );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Start Movement
	void Activate( void );
	// Stop Movement
	void Stop( void );

	// generate small cloud particles ( for moving )
	void Generate_Particles( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	// Check if out of max distance
	bool Check_Max_Distance( void );

	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Max_Distance_Key( const EventArgs &event );  // editor max distance key up
	bool Editor_Always_Fly_Select( const EventArgs &event ); // editor always fly option selected
	bool Editor_Wait_Time_Key( const EventArgs &event );  // editor wait time key up
	bool Editor_Fly_Distance_Key( const EventArgs &event );  // editor fly distance key up

	// color
	DefaultColor color_type;

	// destination direction velocity
	float dest_velx, dest_vely;
	// maximum distance from the startposition
	int max_distance;
	// always fly around
	bool always_fly;
	// time to wait until next movement
	float wait_time;
	// fly distance
	int fly_distance;

	// wait time counter
	float wait_time_counter;
	// fly distance counter
	float fly_distance_counter;
	// clouds particle counter
	float clouds_counter;

private:
	// Create the Name from the current settings
    void Create_Name( void );

};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
