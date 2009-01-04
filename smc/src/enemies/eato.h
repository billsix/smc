/***************************************************************************
 * eato.h  -  headers for the corresponding cpp file
 *
 * Copyright (C) 2006 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_EATO_H
#define SMC_EATO_H

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
	cEato( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cEato( void );

	// init defaults
	void Init( void );
	// copy
	virtual cEato *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set the image directory
	void Set_Image_Dir( string filename );
	// Set direction
	void Set_Direction( ObjectDirection dir );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor image dir key up event
	bool Editor_Image_Dir_Key( const CEGUI::EventArgs &event );

	// image directory
	string img_dir;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
