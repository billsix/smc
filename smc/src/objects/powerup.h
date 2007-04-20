/***************************************************************************
                powerup.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __POWERUP_H__
#define __POWERUP_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** cPowerUp *** *** *** *** *** *** *** *** *** *** *** *** */

class cPowerUp : public cImageObjectSprite
{
public:
	// constructor
	cPowerUp( float x = 0, float y = 0 );
	// destructor
	virtual ~cPowerUp( void );

	/* draw
	 * by default spawned powerups don't draw in leveleditor mode
	*/
	virtual void Draw( cSurfaceRequest *request = NULL );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	float counter;
};

/* *** *** *** *** *** cMushroom *** *** *** *** *** *** *** *** *** *** *** *** */

class cMushroom : public cPowerUp
{
public:
	// constructor
	cMushroom( float x, float y );
	// create from stream
	cMushroom( XMLAttributes &attributes );
	// destructor
	virtual ~cMushroom( void );

	// init defaults
	void Init( void );

	// copy
	virtual cMushroom *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set the Mushroom Type
	void Set_Type( SpriteType ntype );

	// Activates the item
	virtual void Activate( void );

	// update
	virtual void Update( void );

	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );

	// glim animation modifier
	bool glim_mod;
};

/* *** *** *** *** *** cFirePlant *** *** *** *** *** *** *** *** *** *** *** *** */

class cFirePlant : public cPowerUp
{
public:
	// constructor
	cFirePlant( float x, float y );
	// destructor
	virtual ~cFirePlant( void );

	// copy
	virtual cFirePlant *Copy( void );

	// update
	virtual void Update( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
};

/* *** *** *** *** *** cMoon *** *** *** *** *** *** *** *** *** *** *** *** */

class cMoon : public cPowerUp
{
public:
	// constructor
	cMoon( float x, float y );
	// create from stream
	cMoon( XMLAttributes &attributes );
	// destructor
	virtual ~cMoon( void );

	// init defaults
	void Init( void );

	// copy
	virtual cMoon *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// update
	virtual void Update( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	float particle_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
