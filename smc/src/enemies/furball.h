/***************************************************************************
 * furball.h  -  headers for the corresponding cpp file
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

#ifndef SMC_FURBALL_H
#define SMC_FURBALL_H

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** cFurball *** *** *** *** *** *** *** *** *** *** *** */

class cFurball : public cEnemy 
{
/* Furball
 * Secret attacks: Furball's lull you into a false sense of security so that you will
 * forget to do anything and just stupidly run into them.
 * Don't fall for it! Take them very seriously! Pay attention!
 */
public:
	// constructor
	cFurball( float x, float y );
	// create from stream
	cFurball( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cFurball( void );

	// init defaults
	void Init( void );
	// copy
	virtual cFurball *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );

	// Set Direction
	void Set_Direction( ObjectDirection dir );
	// set color ( Brown or Blue )
	void Set_Color( DefaultColor col );

	/* Move into the opposite Direction
	 * if col_dir is given only turns around if the collision direction is in front
	 */
	virtual void Turn_Around( ObjectDirection col_dir = DIR_UNDEFINED );

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
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );

	// Color
	DefaultColor color_type;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
