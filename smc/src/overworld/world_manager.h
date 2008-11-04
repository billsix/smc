/***************************************************************************
 * overworld.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2004 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_WORLD_MANAGER_H
#define SMC_WORLD_MANAGER_H

#include "../core/globals.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"

/* *** *** *** *** *** *** *** *** cOverworld_Manager *** *** *** *** *** *** *** *** *** */

class cOverworld;

class cOverworld_Manager : public CEGUI::XMLHandler, public cObject_Manager<cOverworld>
{
public:
	cOverworld_Manager( void );
	virtual ~cOverworld_Manager( void );

	/* Create a new world
	 * returns true if successful
	*/
	bool New( string name );

	// Load all overworlds
	void Load( void );
	/* Load overworlds from given directory
	 * user_dir : if set overrides game worlds
	*/
	void Load_Dir( string dir, bool user_dir = 0 );

	// Set Active Overworld from name or path
	bool Set_Active( string str );
	// Set Active Overworld
	bool Set_Active( cOverworld *world );

	// Reset to default world first Waypoint
	void Reset( void );

	// Get Overworld pointer
	cOverworld *Get( string str );
	// Get Overworld pointer from path
	cOverworld *Get_from_Path( string path );
	// Get Overworld pointer from name
	cOverworld *Get_from_Name( string name );

	// Return Overworld array number
	int Get_Array_Num( string path );

	// worlds descriptor filename
	string worlds_filename;

	// shows additional information
	bool debugmode;
	// draw the layer
	bool draw_layer;
	// map scrolling with the arrow keys
	bool cameramode;

	// world camera
	cCamera *camera;
private:
	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );
	// handles a world
    void handle_world( const CEGUI::XMLAttributes &attributes );

	// XML element Property list
	CEGUI::XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld information handler
extern cOverworld_Manager *pOverworld_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
