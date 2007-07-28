/***************************************************************************
                overworld.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __WORLDS_H__
#define __WORLDS_H__

#include "../core/globals.h"
#include "../overworld/overworld.h"


/* *** *** *** *** *** *** *** *** cOverworld_manager *** *** *** *** *** *** *** *** *** */

class cOverworld;
typedef vector<cOverworld *> OverworldList;

class cOverworld_manager : public XMLHandler
{
public:
	cOverworld_manager( void );
	virtual ~cOverworld_manager( void );

	// Load Overworlds
	void Load( void );
	// Unload Overworlds
	void Unload( void );

	// Set Active Overworld
	bool Set_Active( string name );
	bool Set_Active( cOverworld *world );

	// Reset to default world first Waypoint
	void Reset( void );

	// Returns Overworld pointer
	cOverworld *Get( string path );
	cOverworld *Get_from_Name( string name );
	cOverworld *Get( unsigned int num );

	// Returns Overworld array num
	int Get_array_num( string path );

	// worlds
	OverworldList worlds;

	// worlds descriptor filename
	string worlds_filename;

	// shows additional information
	bool debugmode;
	// map scrolling with the arrow keys
	bool cameramode;
private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );
	// handles a world
    void handle_world( const XMLAttributes &attributes );

	// XML element Property list
	XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld information handler
extern cOverworld_manager *pOverworld_manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
