/***************************************************************************
                waypoint.h  -  header for the corresponding cpp file
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

#ifndef __OVERWORLD_WAYPOINT_H__
#define __OVERWORLD_WAYPOINT_H__

#include "../globals.h"

/* *** *** *** *** *** Waypoint types *** *** *** *** *** *** *** *** *** *** *** *** */
typedef enum
{
	WAYPOINT_NORMAL = 1,
	WAYPOINT_SPECIAL = 2, // Bonus Map
	WAYPOINT_SECRET = 3, // Extra Path if maryo found a secret door/pipe

} Waypoint_type;

/* *** *** *** *** *** *** Waypoint class *** *** *** *** *** *** *** *** *** *** *** */
class cWaypoint
{
public:
	cWaypoint( void );
	~cWaypoint( void );
	
	// Draws the Waypoint
	void Draw( void );

	// Returns the Levelfile
	string Get_Levelfile( bool with_dir = 1, bool with_end = 1 );

	GL_rect rect;

	/* back and forward direction
	 * 100 = First
	 * 101 = Last
	 */ 
	ObjectDirection direction_back, direction_forward;

	// white arrow
	GL_Surface *arrow_white_l, *arrow_white_r, *arrow_white_u, *arrow_white_d;
	// blue arrow
	GL_Surface *arrow_blue_l, *arrow_blue_r, *arrow_blue_u, *arrow_blue_d;
	// The waypoint image
	GL_Surface *image;

	/* The Waypoint type
	 * look at the definitions
	*/
	Waypoint_type type;
	// The Levelname
	string levelname;

	// if this waypoint is accessable
	bool access;
	// the default access defined in the definition
	bool access_default;

	// identifier
	unsigned int ID;

	// color for the glim effect
	double gcolor;
	// glim effect type switch
	bool glim;
};

#endif
