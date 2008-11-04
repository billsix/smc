/***************************************************************************
 * ow_player.h  -  header for the corresponding cpp file
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

#ifndef SMC_WORLD_PLAYER_H
#define SMC_WORLD_PLAYER_H

#include "../core/globals.h"
#include "../gui/hud.h"
#include "../overworld/world_layer.h"
#include "../player/player.h"
#include "../overworld/world_waypoint.h"

/* *** *** *** *** *** *** *** cOverworld_Player *** *** *** *** *** *** *** *** *** *** */

class cOverworld_Player : public cImageObjectSprite
{
public:
	cOverworld_Player( void );
	virtual ~cOverworld_Player( void );

	// Load images
	void Load_Images( void );
	// Unload images
	void Unload_Images( void );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( cSurfaceRequest *request = NULL );
	// Draw debug text
	void Draw_Debug_Text( void );

	// Reset
	void Reset( void );

	// General input interact event
	void Action_Interact( input_identifier key_type );
	// Stop Interacting event
	void Action_Stop_Interact( input_identifier key_type );

	// Set the velocity for the current direction
	void Update_Vel( void );

	// Activates the current Waypoint
	void Activate_Waypoint( void );

	/* Start walking into the given direction
	 * returns 0 if the next level is not accessible or not available
	*/
	bool Start_Walk( ObjectDirection new_direction );
	/* Updates walking
	 * checks for new line contacts and updates the Animation
	*/
	void Update_Walk( void );
	// Start fixed walking into the given Waypoint
	void Start_Waypoint_Walk( unsigned int new_waypoint );
	/* Fixed walking into the Waypoint
	 * moves maryo smoothly into the found Waypoint
	*/
	void Update_Waypoint_Walk( void );

	// Set Maryo to the given Waypoint position
	bool Set_Waypoint( unsigned int waypoint, bool new_startpos = 0 );
	// Get current Waypoint
	cWaypoint *Get_Waypoint( void );

	// Returns the current Waypoint front line
	cLayer_Line *Get_Front_Line( ObjectDirection dir );

	/* Checks how much Maryo can walk into all directions
	 * and sets the size into the path directions
	*/
	void Update_Path_Diff( unsigned int csize = 25 );

	// Maryo changes direction if a near path is found or turns around 
	void Change_Direction( void );

	/* Automatically corrects Maryo's position to the walkable Layer Line
	 * with the given size and the minimal distance to not-walkable area
	*/
	void Auto_Pos_Correction( float size = 1.7f, float min_distance = 5 );

	// state
	Maryo_type maryo_state;
	// current waypoint
	int current_waypoint;
	// line waypoint
	unsigned int line_waypoint;
	// current line ( if walking )
	int current_line;

	// Animation counter
	float anim_counter;
	// Animation Speed
	float anim_speed;
	// last image available for the animation
	unsigned int anim_max;

	// if touched a Waypoint use fixed walking
	bool fixed_walking;

	// valid path detection lines
	cLine_collision line_hor, line_ver;

private:
	// Debug Last Set Data
	int debug_current_line_last;
	int debug_lines_last;
	int debug_current_waypoint_last;

	// Debug Text Sprites
	cHudSprite *debug_current_waypoint;
	cHudSprite *debug_lines;
	cHudSprite *debug_current_line;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld Player
extern cOverworld_Player *pOverworld_Player;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
