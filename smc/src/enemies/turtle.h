/***************************************************************************
                turtle.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __TURTLE_H__
#define __TURTLE_H__

#include "../enemies/enemy.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

enum Turtle_state
{
	TURTLE_DEAD		= 0,
	TURTLE_WALK		= 1,
	TURTLE_SHELL_STAND	= 2,
	TURTLE_SHELL_RUN	= 3,
	TURTLE_FLY		= 4, // todo
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cTurtle : public cEnemy
{
public:
	// constructor
	cTurtle( float x, float y );
	// create from stream
	cTurtle( XMLAttributes &attributes );
	// destructor
	virtual ~cTurtle( void );

	// init defaults
	void Init( void );
	// copy
	virtual cTurtle *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Set Direction
	void Set_Direction( ObjectDirection dir, bool new_start_direction = 0 );
	// set color
	void Set_Color( DefaultColor col );

	/* Move into the opposite Direction
	 * if col_dir is given only turns around if the collision direction is in front
	 */
	virtual void Turn_Around( ObjectDirection col_dir = DIR_UNDEFINED );

	// die
	virtual void DownGrade( bool force = 0 );
	virtual void DieStep( void );

	// update
	virtual void Update( void );

	// if update is valid for the current state
	virtual bool is_Update_valid( void );

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

	// internal turtle state
	Turtle_state turtle_state;

	// default speed
	float speed_walk, speed_shell;

	/* If the player kicked the shell this counter is set.
	 * if this counter is higher than 0
	 * maryo cannot get killed by the shell
	 */
	float playercounter;

	// Color
	DefaultColor color_type;

private:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
