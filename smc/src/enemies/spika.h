/***************************************************************************
                spika.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SPIKA_H__
#define __SPIKA_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cSpika *** *** *** *** *** *** *** *** *** *** *** */

class cSpika : public cEnemy 
{
/* Spika
 * Will wait for you and wheel you 
 * if you can't run fast enough :D
 * 
 * Colors : Orange, Green and Grey
 */
public:
	// constructor
	cSpika( float x, float y );
	// create from stream
	cSpika( XMLAttributes &attributes );
	// destructor
	virtual ~cSpika( void );

	// init defaults
	void Init( void );
	// copy
	virtual cSpika *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// set color
	void Set_Color( DefaultColor col );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool is_Update_valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );

	// default speed
	float speed;
	// default detection size
	float detection_size;
	// Color
	DefaultColor color_type;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
