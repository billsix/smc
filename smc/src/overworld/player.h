/***************************************************************************
                player.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __OVERWORLD_PLAYER_H__
#define __OVERWORLD_PLAYER_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** World Maryo *** *** *** *** *** *** *** *** *** *** */

typedef vector<GL_Surface *> MaryoImageList;

class cWorld_Maryo : public cSprite
{
public:
	cWorld_Maryo( void );
	~cWorld_Maryo( void );

	// Loads the Images
	void Load_Images( void );
	// Unloads all Images
	void Unload_Images( void );

	// Draw Maryo
	void Draw( void );
	// Update Maryo
	void Update( void );

	// Set the velocity for the current direction
	void Update_Vel( void );

	/* Start walking into the given direction 
	 * returns 0 if the next level is not accessable or not available
	*/
	bool Start_Walk( ObjectDirection direction );
	/* Updates Maryos walking
	 * checks for new line contacts and updates the Animation
	*/
	void Update_Walk( void );
	// Start fixed walking into the given Waypoint
	void Start_WaypointWalk( unsigned int new_waypoint );
	/* Fixed walking into the Waypoint
	 * moves maryo smoothly into the found Waypoint
	*/
	void Update_WaypointWalk( void );

	// Set Maryo to the given Waypoint position
	bool Set_Waypoint( unsigned int waypoint );

	/* Checks how much Maryo can walk into all directions
	 * and sets the size into the path directions
	*/
	void Update_PathDiff( unsigned int csize = 25 );

	// Maryo changes direction if a near path is found or turns around 
	void Change_Direction( void );

	// Returns the foot rect for walking collision checking
	GL_rect Get_ColRect( void );

	/* Automatically corrects Maryos position to the walkable Layer Line
	 * with the given size and the minimal distance to unwalkable area
	*/
	void Auto_Pos_correct( double size = 1.7, double min_distance = 5 );

	// Maryo images
	MaryoImageList images;

	// current state
	Maryo_type maryo_state;
	// current Waypoint
	int current_waypoint;
	// current line ( if walking )
	int current_line;

	// Animation counter
	double anim_counter;
	// Animation Speed
	double anim_speed;
	// last image available for the animation
	unsigned int anim_max;

	// if Maryo touches a Waypoint use fixed walking
	bool fixed_walking;

	// valid path detection lines
	cLine_collision line_hor, line_ver;
};

#endif
