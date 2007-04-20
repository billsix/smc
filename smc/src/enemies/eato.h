/***************************************************************************
              eato.h  -  headers for the corresponding cpp file
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

#ifndef __EATO_H__
#define __EATO_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** cEato *** *** *** *** *** *** *** *** *** *** *** *** */

/*
 * Eats your Butt !
*/
class cEato : public cEnemy
{
public:
	// constructor
	cEato( float x, float y );
	// create from stream
	cEato( XMLAttributes &attributes );
	// destructor
	virtual ~cEato( void );

	// init defaults
	void Init( void );
	// copy
	virtual cEato *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set direction
	void Set_Direction( ObjectDirection dir );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
