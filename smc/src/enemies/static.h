/***************************************************************************
                static.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __STATIC_ENEMY_H__
#define __STATIC_ENEMY_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */

class cStaticEnemy : public cEnemy 
{
/* Static
 * Static Objects don't move but will
 * hit you if you touch them.
 * 
 * Todo : Should use Image Super-Class which is not implemented yet ;)
 * and the image should be a setting
 */
public:
	// constructor
	cStaticEnemy( float x, float y );
	// create from stream
	cStaticEnemy( XMLAttributes &attributes );
	// destructor
	virtual ~cStaticEnemy( void );

	// init defaults
	void Init( void );
	// copy
	virtual cStaticEnemy *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

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

	// default speed
	float speed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
