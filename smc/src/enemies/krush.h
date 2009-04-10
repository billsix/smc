/***************************************************************************
 * krush.h  -  headers for the corresponding cpp file
 *
 * Copyright (C) 2004 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_KRUSH_H
#define SMC_KRUSH_H

#include "../enemies/enemy.h"

namespace SMC
{

/* *** *** *** *** *** Krush states *** *** *** *** *** *** *** *** *** *** *** *** */

enum Krush_state
{
	KRUSH_DEAD		= 0,
	KRUSH_WALK		= 1,
	KRUSH_SMALL		= 2
};

/* *** *** *** *** *** cKrush *** *** *** *** *** *** *** *** *** *** *** *** */

class cKrush : public cEnemy
{
/* Krush
 * Walking all Day and doesn't stop even if you hit him the first time :O
 * but the second time he's out.
 * Colors : Red
 */
public:
	// constructor
	cKrush( float x, float y );
	// create from stream
	cKrush( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cKrush( void );

	// init defaults
	void Init( void );
	// copy
	virtual cKrush *Copy( void );

	// create from stream
	virtual void Create_From_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_To_Stream( ofstream &file );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	// Set Direction
	void Set_Direction( const ObjectDirection dir );

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
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual Col_Valid_Type Validate_Collision( cSprite *obj );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );

	// Krush state
	Krush_state krush_state;

	// speed when small
	float m_small_speed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
