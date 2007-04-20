/***************************************************************************
                gumba.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __GUMBA_H__
#define __GUMBA_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cGumba *** *** *** *** *** *** *** *** *** *** *** */

class cGumba : public cEnemy 
{
/* Gumba
 * Secret attacks: Gumba's lull you into a false sense of security so that you will
 * forget to do anything and just stupidly run into them.
 * Don't fall for it! Take them very seriously! Pay attention!
 * Colors : Brown and Red
 */
public:
	// constructor
	cGumba( float x, float y );
	// create from stream
	cGumba( XMLAttributes &attributes );
	// destructor
	virtual ~cGumba( void );

	// init defaults
	void Init( void );
	// copy
	virtual cGumba *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
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

	// Color
	DefaultColor color_type;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
