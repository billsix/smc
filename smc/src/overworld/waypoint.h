/***************************************************************************
                waypoint.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
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

#include "../core/globals.h"
#include "../video/video.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** Waypoint types *** *** *** *** *** *** *** *** *** *** *** *** */

enum Waypoint_type
{
	WAYPOINT_NORMAL = 1,
	WAYPOINT_WORLD_LINK = 2 // Enters another World
};

/* *** *** *** *** *** *** cWaypoint *** *** *** *** *** *** *** *** *** *** *** */

class cWaypoint : public cSprite
{
public:
	// constructor
	cWaypoint( void );
	// create from stream
	cWaypoint( XMLAttributes &attributes );
	// destructor
	virtual ~cWaypoint( void );
	
	// Init defaults
	void Init( void );

	// copy this object
	virtual cWaypoint *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Set Access
	void Set_Access( bool enabled, bool new_start_access = 0 );

	// Set the Destination
	void Set_Destination( string str );
	// Returns the Destination
	string Get_Destination( bool with_dir = 0, bool with_end = 0 );

	// editor activation
	virtual void Editor_Activate( void );
	// editor events
	bool Editor_Destination_Key( const EventArgs &event );  // editor destination key up
	bool Editor_Access_Select( const EventArgs &event ); // editor access option selected
	bool Editor_Backward_Direction_Select( const EventArgs &event ); // editor direction backward option selected
	bool Editor_Forward_Direction_Select( const EventArgs &event ); // editor direction forward option selected

	/* backward and forward direction
	 */
	ObjectDirection direction_backward, direction_forward;

	/* The Waypoint type
	 * look at the definitions
	*/
	Waypoint_type waypoint_type;
	// destination
	string destination;

	// if this waypoint is accessible
	bool access;
	// the default access defined in the definition
	bool access_default;

	// identifier
	unsigned int ID;

	// color for the glim effect
	float gcolor;
	// glim effect type switch
	bool glim;


	// white arrow
	GL_Surface *arrow_white_l, *arrow_white_r, *arrow_white_u, *arrow_white_d;
	// blue arrow
	GL_Surface *arrow_blue_l, *arrow_blue_r, *arrow_blue_u, *arrow_blue_d;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
