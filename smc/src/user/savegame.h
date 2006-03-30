/***************************************************************************
           savegame.h  -  Savegame Engine Header
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

#ifndef __SAVEGAME_H__
#define __SAVEGAME_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define SAVEGAME_VERSION "2.1"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cSavegame_data
{
public:
	cSavegame_data( void );
	~cSavegame_data( void );

	void Init( void );

	string description;			// Level Description
	int version;				// Level Savegame Version
	string levelname;			// Level Name
	string timestamp;			// Time

	// Player Information
	unsigned int lives;
	unsigned int points;
	unsigned int goldpieces;
	unsigned int state;			// 1 = small, 2 = big, 3 = Fire ...
	unsigned int itembox_item;	// The Item in the Itembox
	int posx, posy;				// Position

	// OverWorld information
	unsigned int Overworld_save;			// is this an Overworld save
	unsigned int Overworld_nlevel;			// accessed Overworld normal levels
	unsigned int Overworld_slevel;			// accessed Overworld secret levels
	unsigned int Overworld_curr_waypoint;	// Current Waypoint
	unsigned int Overworld_world;			// Current Overworld
};


class cSavegame : public cFile_parser
{
public:
	cSavegame( void );
	~cSavegame( void );

	// Loads the Savegame
	cSavegame_data Load( unsigned int save_slot );
	// Saves the Savegame
	int Save( unsigned int save_slot, cSavegame_data Save_info );

	// Returns only the Savegame Description
	string Get_Description( unsigned int save_slot, bool only_description = 0 );

	// Print Savegame Debug information
	void Debug_Print( cSavegame_data savegame );

	// Returns true if the Savegame is valid
	bool is_valid( unsigned int save_slot );

	// savegame debug messages
	bool debug;

private:
	// Handles the command
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );

	// for Savegame loading
	cSavegame_data savegame_temp;
};
#endif
