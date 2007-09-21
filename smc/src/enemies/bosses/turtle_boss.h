/***************************************************************************
                turtle_boss.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
							(C) 2006 Tobias Maasland
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __TURTLEBOSS_H__
#define __TURTLEBOSS_H__

#include "../../enemies/enemy.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

enum TurtleBoss_state
{
	TURTLEBOSS_DEAD		= 0,
	TURTLEBOSS_WALK		= 1,
	TURTLEBOSS_SHELL_STAND	= 2,
	TURTLEBOSS_SHELL_RUN	= 3,
	TURTLEBOSS_STAND_ANGRY = 4
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cTurtleBoss : public cEnemy
{
public:
	// constructor
	cTurtleBoss( float x, float y );
	// create from stream
	cTurtleBoss( XMLAttributes &attributes );
	// destructor
	virtual ~cTurtleBoss( void );

	// init defaults
	void Init( void );

	// copy
	virtual cTurtleBoss *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	void Set_Max_Hits( int nmax_hits );
	void Set_Max_Downgrade_Counts( int nmax_downgrade_count );
	void Set_Max_Downgrade_Time( float nmax_downgrade_time );

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

	// Throw Fireballs upwards
    void Throw_Fireballs( unsigned int amount = 6 );

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

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Max_Hits_Key( const EventArgs &event );  // editor max hits key up
	bool Editor_Max_Downgrade_Counts_Key( const EventArgs &event );  // editor max downgrades key up
	bool Editor_Max_Downgrade_Time_Key( const EventArgs &event );  // editor max downgrades time key up

	// internal turtle state
	TurtleBoss_state turtle_state;

	// default speed
	float speed_walk, speed_shell;

	/* If the player kicked the shell this counter starts.
	 * if this counter is higher than 0
	 * maryo cannot get killed by the shell
	 */
	float playercounter;

	// Color
	DefaultColor color_type;

private:
	// Activates walking Turtle
    void Activate_Turtle( void );
	// Create the Name from the current settings
    void Create_Name( void );

    // How many times has the turtle boss been hit since the last count / downgrade?
    int hits;
    // How many times did the turtle downgrade?
    int downgrade_counts;
	// maximum times hit-able
    int max_hits;
	// maximum times downgrade-able
    int max_downgrade_counts;
	
    float max_downgrade_time;

    float run_time;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
