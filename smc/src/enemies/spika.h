/***************************************************************************
                spika.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

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

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// default speed
	float speed;
	// default detection size
	float detection_size;
	// Color
	DefaultColor color_type;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
