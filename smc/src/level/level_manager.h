/***************************************************************************
                level_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LEVEL_MANAGER_H__
#define __LEVEL_MANAGER_H__

#include "../core/globals.h"
#include "../core/obj_manager.h"

// engine version
#define LEVEL_ENGINE_VERSION 2.3f
// default files for levels
#define LEVEL_DEFAULT_MUSIC "land/land_5.ogg"
#define LEVEL_DEFAULT_BACKGROUND "game/background/green_junglehills.png"

/* *** *** *** *** *** Level drawing Types *** *** *** *** *** *** *** *** *** *** *** *** */

enum LevelDrawType
{
	LVL_DRAW		= 0,	// only draw
	LVL_DRAW_NO_BG	= 1,	// only draws and without background gradient and image
	LVL_DRAW_BG		= 2		// only draws the background gradient and image
};

/* *** *** *** *** *** cLevel_manager  *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel;

class cLevel_manager : public cObject_Manager<cLevel>
{
public:
	cLevel_manager( void );
	virtual ~cLevel_manager( void );

	// Load level descriptions
	void Init( void );
	// Unload
	void Unload( void );

	/* Load new Level
	 * delayed : if set the level will be loaded on the next update
	 * sub_level : if set previous level isn't unloaded
	*/
	bool Load( string filename, bool delayed = 0, bool sub_level = 0 );

};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level Manager
extern cLevel_manager *pLevel_manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
