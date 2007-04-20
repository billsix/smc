/***************************************************************************
                rex.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            : (C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __REX_H__
#define __REX_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** Rex states *** *** *** *** *** *** *** *** *** *** *** *** */

enum Rex_state
{
	REX_DEAD		= 0,
	REX_WALK		= 1,
	REX_MINI		= 2,
};

/* *** *** *** *** *** cRex *** *** *** *** *** *** *** *** *** *** *** *** */

class cRex : public cEnemy
{
/* Rex
 * Walking all Day and doesn't stops even
 * if you hit him for the first time but
 * the second time he's out :O
 * Colors : Blue
 */
public:
	// constructor
	cRex( float x, float y );
	// create from stream
	cRex( XMLAttributes &attributes );
	// destructor
	virtual ~cRex( void );

	// init defaults
	void Init( void );
	// copy
	virtual cRex *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir );

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

	// The rex state
	Rex_state rex_state;

	// The speed
	float speed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
