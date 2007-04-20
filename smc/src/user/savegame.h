/***************************************************************************
           savegame.h  -  Savegame Engine Header
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

#ifndef __SAVEGAME_H__
#define __SAVEGAME_H__

#include "../core/globals.h"
#include "../objects/sprite.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define SAVEGAME_VERSION 7
#define SAVEGAME_VERSION_UNSUPPORTED 5

/* *** *** *** *** *** *** *** cSave_Overworld_Waypoint *** *** *** *** *** *** *** *** *** *** */
/* Overworld Waypoint save data
 *
*/
class cSave_Overworld_Waypoint
{
public:
	cSave_Overworld_Waypoint( void );
	~cSave_Overworld_Waypoint( void );

	string destination;
	bool access;
};

typedef vector<cSave_Overworld_Waypoint *> Save_Overworld_WaypointList;


/* *** *** *** *** *** *** *** cSave_Overworld *** *** *** *** *** *** *** *** *** *** */
/* Overworld save data
 *
*/
class cSave_Overworld
{
public:
	cSave_Overworld( void );
	~cSave_Overworld( void );

	string name;

	// waypoint data
	Save_Overworld_WaypointList Waypoints;
};

typedef vector<cSave_Overworld *> Save_OverworldList;

/* *** *** *** *** *** *** *** cSave_Level_Object_Property *** *** *** *** *** *** *** *** *** *** */
/* Level object string property
*/
class cSave_Level_Object_Property
{
public:
	cSave_Level_Object_Property( string nName = "", string nValue = "" );

	string Name;
	string Value;
};

typedef vector<cSave_Level_Object_Property> Save_Level_Object_ProprtyList;

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */
/* Level object save data
 *
*/
class cSave_Level_Object
{
public:
	cSave_Level_Object( void );
	~cSave_Level_Object( void );

	// Returns the Value
	string Get_Value( string val_name );

	SpriteType type;

	// object properties
	Save_Level_Object_ProprtyList properties;
};

typedef vector<cSave_Level_Object *> Save_Level_ObjectList;

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */
/* Save data
 *
*/
class cSave
{
public:
	cSave( void );
	~cSave( void );

	// Set data to default/empty values
	void Clear( void );

	// ## Save Information
	// description
	string description;
	// savegame version
	int version;
	// time  ( seconds since 1970 )
	time_t save_time;

	// ## Player Information
	// lives
	unsigned int lives;
	// points
	unsigned int points;
	// goldpieces
	unsigned int goldpieces;
	// player state
	unsigned int player_state;
	// item in the itembox
	unsigned int itembox_item;

	// ## Level Information
	/* level name
	 * only saved if Player is in a Level
	*/
	string level_name;
	// level position
	float level_posx, level_posy;
	// objects data
	Save_Level_ObjectList level_objects;

	// ## Overworld Information
	// active Overworld
	string overworld_active;
	// current Waypoint
	unsigned int overworld_current_waypoint;

	// overworld data
	Save_OverworldList Overworlds;
};

/* *** *** *** *** *** *** *** cSavegame *** *** *** *** *** *** *** *** *** *** */

class cSavegame : public XMLHandler
{
public:
	cSavegame( void );
	~cSavegame( void );

	/* Load a Save
	 * Returns 0 if failed 
	 * 1 if ingame save
	 * 2 if Overworld save
	*/
	int Load_Game( unsigned int save_slot );
	// Save the game with the given description
	bool Save_Game( unsigned int save_slot, string description );

	// Load a Save
	cSave *Load( unsigned int save_slot );
	// Save a Save
	int Save( unsigned int save_slot, cSave *Save_info );

	// Returns only the Savegame Description
	string Get_Description( unsigned int save_slot, bool only_description = 0 );

	// Returns true if the Savegame is valid
	bool is_valid( unsigned int save_slot );

	// debug messages enabled
	bool debug;

	// current error count
	unsigned int error_count;

	// savegame directory
	string savegame_dir;
private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );

	void Handle_Level( const XMLAttributes &attributes );
	void Handle_Level_Object( const XMLAttributes &attributes );
	void Handle_Player( const XMLAttributes &attributes );
	void Handle_Overworld_Data( const XMLAttributes &attributes );
	void Handle_Overworld( const XMLAttributes &attributes );
	void Handle_Overworld_Level( const XMLAttributes &attributes );

	// XML element Item Tag list
	XMLAttributes xml_attributes;

	// current savegame overworld waypoints for overworld parsing
	Save_Overworld_WaypointList active_waypoints;
	// current savegame level objects for level parsing
	Save_Level_ObjectList level_objects;

	// temp Save used for loading
	cSave *save_temp;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Savegame Handler
extern cSavegame *pSavegame;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
