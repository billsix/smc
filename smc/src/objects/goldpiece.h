/***************************************************************************
 * goldpiece.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_GOLDPIECE_H
#define SMC_GOLDPIECE_H

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** cGoldpiece *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldpiece : public cImageObjectSprite
{
public:
	// constructor
	cGoldpiece( float x, float y );
	// create from stream
	cGoldpiece( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cGoldpiece( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );
	// init defaults
	void Init( void );

	// copy
	virtual cGoldpiece *Copy( void );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_to_Savegame( void );

	// Set the gold color
	virtual void Set_Gold_Color( DefaultColor ncolor );

	// the player gets the goldpiece
	void Activate( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	// ignore onground check if goldpiece
	virtual void Check_on_Ground( void );

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

	// ignore ground check
	virtual void Check_on_Ground( void ) {};

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
};

/* *** *** *** *** *** cFGoldpiece *** *** *** *** *** *** *** *** *** *** *** *** */

/* Falling Goldpiece
 * if direction is undefined it moves into a random direction
*/
class cFGoldpiece : public cGoldpiece
{
public:
	cFGoldpiece( float x, float y, ObjectDirection dir = DIR_UNDEFINED );
	virtual ~cFGoldpiece( void );
	
	// Set the gold color
	virtual void Set_Gold_Color( DefaultColor ncolor );

	// update
	virtual void Update( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
