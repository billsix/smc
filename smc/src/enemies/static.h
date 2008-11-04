/***************************************************************************
 * static.h  -  headers for the corresponding cpp file
 *
 * Copyright (C) 2007 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_STATIC_ENEMY_H
#define SMC_STATIC_ENEMY_H

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */

class cStaticEnemy : public cEnemy 
{
/* Static enemies don't move but will
 * hit you if you touch them.
 */
public:
	// constructor
	cStaticEnemy( float x, float y );
	// create from stream
	cStaticEnemy( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cStaticEnemy( void );

	// init defaults
	void Init( void );
	// copy
	virtual cStaticEnemy *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set the static image
	void Set_Static_Image( string filename );
	// Set the rotation speed
	void Set_Rotation_Speed( float speed );

	/* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );

	// ignore onground check
	virtual void Check_on_Ground( void ) {};
	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Image_Key( const CEGUI::EventArgs &event );  // editor image key up
	bool Editor_Rotation_Speed_Key( const CEGUI::EventArgs &event );  // editor rotation speed key up

	// image filename
	string img_filename;
	// rotation speed
	float rotation_speed;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
