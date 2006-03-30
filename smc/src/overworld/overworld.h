/***************************************************************************
                overworld.h  -  header for the corresponding cpp file
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

#ifndef __OVERWORLD_H__
#define __OVERWORLD_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** Overworld handler *** *** *** *** *** *** *** *** *** */

class cOverworld_world
{
public:
	cOverworld_world( void );
	~cOverworld_world( void );

	string Get_WorldFile( void );

	string name;
	string dir;
};

typedef vector<cOverworld_world *> Overworld_worldList;

class cOverworld_worlds : public XMLHandler
{
public:
	cOverworld_worlds( void );
	~cOverworld_worlds( void );

	// Load Overworld descriptions
	void Load( void );
	// Unload Overworld descriptions
	void Unload( void );

	// Returns Overworld description
	cOverworld_world *Get( string name );

	Overworld_worldList worlds;

	// world descriptor filename
	string world_filename;

private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );
	// handles a world
    void handle_world( const XMLAttributes& attributes );
};

/* *** *** *** *** *** *** *** *** OverWorld *** *** *** *** *** *** *** *** *** */


typedef vector<cSprite *> MapObjectList;
typedef vector<cWaypoint *> WaypointList;

class cOverWorld : public XMLHandler
{
public:
	cOverWorld( void );
	~cOverWorld( void );

	 // Enters the Overworld
	void Enter( void );
	// handles keydown events
	void KeyDown_OW( SDLKey key );

	 // Loads an Overworld
	bool Load( string name );
	// Unloads the Overworld
	void Unload( void );
	// Todo : Loads a map from the current OverWorld
	void Load_Map( void );

	// Draws the Overworld
	void Draw( void );
	// Draws all Map Objects
	void Draw_MapObjects( void );
	// Draw all Waypoints
	void Draw_Waypoints( void );
	// Draws the HUD
	void Draw_HUD( void );
	// Draws the Debug Stuff ( if Debugmode )
	void Draw_Debug( void );

	// Updates the Overworld
	void Update( void );
	// Updates the Camera
	void Update_Camera( void );

	// Sets the current Map progress
	void Set_Progress( unsigned int normal_level, unsigned int secret_level );

	// Returns a pointer to the Waypoint if in index else NULL
	cWaypoint *Get_Waypoint( unsigned int num );
	// Checks if the rect collides with a waypoint returns -1 if no collisions
	int Get_Waypoint_Collision( GL_rect *rect_2 );
	// returns the last accessable waypoint
	int Get_LastValidWaypoint( void );

	// Enable the next Level and walk into the forward direction
	bool GotoNextLevel( void );
	// Resets the Waypoint access to the default
	void Reset_Waypoints( void );

	// Current Overworld Map
	unsigned int Map;

	// Waypoints on the map
	WaypointList Waypoints;
	
	// current Overworld
	int current_world;
	// Overworld information handler
	cOverworld_worlds *world_handler;

	// Map Objects
	MapObjectList MapObjects;

	// current Layer for collision checking
	cLayer *Layer;
	// Maryo sprite
	cWorld_Maryo *Maryo;
	// HUD
	cSprite *hud_level;

	// if the overworld should be left ( 1 = quit, 2 = Enter Main Menu )
	unsigned int entered;
	// shows additional information
	bool debugmode;
	// map scrolling with the arrow keys
	bool cameramode;
	// enables world editing
	bool editormode;
private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );
	// handles a Sprite
    void handle_sprite( const XMLAttributes& attributes );
	// handles a Waypoint
    void handle_waypoint( const XMLAttributes& attributes );
	// handles a Layer
    void handle_layer( const XMLAttributes& attributes );
	// handles the Player
    void handle_player( const XMLAttributes& attributes );

	// Adds a Sprite Map Object
	void AddMapObject( cSprite *object );
 
	// Debug Texts
	GL_Surface *debugimage;
	GL_Surface *debug_curr_waypoint;
	GL_Surface *debug_curr_line;
	GL_Surface *debug_nlevel;
};

#endif
