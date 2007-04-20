/***************************************************************************
           goldpiece.h  -  header for the corresponding cpp file
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

#ifndef __GOLDPIECE_H__
#define __GOLDPIECE_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** cGoldPiece *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldpiece : public cImageObjectSprite
{
public:
	// constructor
	cGoldpiece( float x, float y );
	// create from stream
	cGoldpiece( XMLAttributes &attributes );
	// destructor
	virtual ~cGoldpiece( void );
	
	// init defaults
	void Init( void );

	// copy
	virtual cGoldpiece *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set the gold color
	virtual void Set_Goldcolor( DefaultColor ncolor );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// the player gets the goldpiece
	void Activate( void );

	// goldpiece rotation
	float counter;
	// gold color
	DefaultColor color_type;
};

/* *** *** *** *** *** cJGoldpiece *** *** *** *** *** *** *** *** *** *** *** *** */

/* Jumping Goldpiece
 * used by Goldbox
*/
class cJGoldpiece : public cGoldpiece
{
public:
	cJGoldpiece( float x, float y );
	virtual ~cJGoldpiece( void );
	
	// update
	virtual void Update( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};
};

/* *** *** *** *** *** cFGoldpiece *** *** *** *** *** *** *** *** *** *** *** *** */

/* Falling Goldpiece
 * if direction is nothing it will be a random direction
*/
class cFGoldpiece : public cGoldpiece
{
public:
	cFGoldpiece( float x, float y, ObjectDirection dir = DIR_UNDEFINED );
	virtual ~cFGoldpiece( void );
	
	// Set the gold color
	virtual void Set_Goldcolor( DefaultColor ncolor );

	// update
	virtual void Update( void );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
