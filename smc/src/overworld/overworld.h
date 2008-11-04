/***************************************************************************
 * overworld.h  -  header for the corresponding cpp file
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

#ifndef SMC_OVERWORLD_H
#define SMC_OVERWORLD_H

#include "../overworld/world_manager.h"
#include "../overworld/world_waypoint.h"
#include "../overworld/world_layer.h"
#include "../overworld/world_player.h"
#include "../gui/hud.h"
#include "../audio/random_sound.h"

/* *** *** *** *** *** *** *** *** cOverworld_description *** *** *** *** *** *** *** *** *** */

class cOverworld_description : public CEGUI::XMLHandler
{
public:
	cOverworld_description( void );
	virtual ~cOverworld_description( void );

	// Load
	void Load( void );

	// Save
	void Save( void );

	// returns the full path
	string Get_Full_Path( void );

	// directory name
	string path;
	// world name
	string name;
	// is visible on selection
	bool visible;
	// from user directory
	bool user;

	// world comment
	string comment;
private:
	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );
	// handle world description
    void handle_world( const CEGUI::XMLAttributes& attributes );

	// XML element Property list
	CEGUI::XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** cOverworld *** *** *** *** *** *** *** *** *** */

typedef vector<cWaypoint *> WaypointList;

class cOverworld : public CEGUI::XMLHandler
{
public:
	cOverworld( void );
	virtual ~cOverworld( void );

	/* Enter
	* if delayed is set this overworld will be entered on the next game update
	*/
	void Enter( bool delayed = 0 );

	/* handle key down event
	 * returns true if processed
	*/
	bool Key_Down( SDLKey key );
	/* handle key up event
	 * returns true if processed
	*/
	bool Key_Up( SDLKey key );
	/* handle mouse button down event
	 * returns true if processed
	*/
	bool Mouse_Down( Uint8 button );
	/* handle mouse button up event
	 * returns true if processed
	*/
	bool Mouse_Up( Uint8 button );
	/* handle joystick button down event
	 * returns true if processed
	*/
	bool Joy_Button_Down( Uint8 button );
	/* handle joystick button up event
	 * returns true if processed
	*/
	bool Joy_Button_Up( Uint8 button );

	// Load
	bool Load( void );
	// Unload
	void Unload( void );
	// Save
	void Save( void );

	// Draw
	void Draw( void );
	// Draws the HUD
	void Draw_HUD( void );

	// Update
	void Update( void );
	// Updates the Camera
	void Update_Camera( void );

	// Add Map Object
	void Add_Map_Object( cSprite *object );

	/* Sets the current Waypoint progress
	 * if force is set already accessible waypoints will be unset
	*/
	void Set_Progress( unsigned int normal_level, bool force = 1 );

	/* Returns a pointer to the Waypoint
	 * if not found returns NULL
	*/
	cWaypoint *Get_Waypoint( string name );
	cWaypoint *Get_Waypoint( unsigned int num );
	/* Returns the Waypoint array number if the level name matches
	 * if not found returns -1
	*/
	int Get_Level_Waypoint_Num( string level_name );
	/* Returns the Waypoint array number if the destination name matches
	 * if not found returns -1
	*/
	int Get_Waypoint_Num( string world_name );

	/* Check if the rect collides with a Waypoint
	 * if no collision found returns -1
	*/
	int Get_Waypoint_Collision( GL_rect *rect_2 );
	// returns the last accessible Waypoint
	int Get_Last_Valid_Waypoint( void );
	// update the Waypoint text
	void Update_Waypoint_text( void );

	// Enable the next Level and walk into the forward direction
	bool Goto_Next_Level( void );
	// Resets the Waypoint access to the default
	void Reset_Waypoints( void );

	// description
	cOverworld_description *description;

	// Map objects
	cSprite_Manager *sprite_manager;
	// Waypoints
	WaypointList waypoints;

	// current Layer for collision checking
	cLayer *pLayer;
	// Background Color
	Color background_color;
	// music filename
	string musicfile;
	// HUD world name
	cHudSprite *hud_world_name;
	// HUD current level name
	cHudSprite *hud_level_name;

	// goto next level on overworld enter
	bool next_level;

	// Settings
	unsigned int player_start_waypoint;
	Moving_state player_moving_state;

private:
	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );
 
	// XML element Property list
	CEGUI::XMLAttributes xml_attributes;
};

// Returns a World Object if element name is available else NULL
cSprite *Get_World_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Overworld
extern cOverworld *pActive_Overworld;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
