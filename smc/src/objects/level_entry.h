/***************************************************************************
 * level_entry.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2007 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_LEVEL_ENTRY_H
#define SMC_LEVEL_ENTRY_H

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** Level Entry types *** *** *** *** *** *** *** *** *** *** */

enum Level_Entry_type
{
	LEVEL_ENTRY_BEAM		= 0,	// no animation ( f.e. a door or hole )
	LEVEL_ENTRY_WARP		= 1		// rotated player moves slowly into the destination direction
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Level Entry
 * 
*/
class cLevel_Entry : public cImageObjectSprite
{
public:
	// constructor
	cLevel_Entry( float x, float y );
	// create from stream
	cLevel_Entry( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cLevel_Entry( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevel_Entry *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );
    // Set direction
	void Set_Direction( ObjectDirection dir );

	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Get centered player position y
	float Get_Player_Pos_X( void );
	// Get centered player position y
	float Get_Player_Pos_Y( void );

	// Activate
	void Activate( void );

	// Set the type
	void Set_Type( Level_Entry_type ltype );
	// Set the name
	void Set_Name( string str_name );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// editor activation
	virtual void Editor_Activate( void );
	// editor direction option selected event
	bool Editor_Direction_Select( const CEGUI::EventArgs &event );
	// editor name key up event
	bool Editor_Name_Key( const CEGUI::EventArgs &event );

	// level entry type
	Level_Entry_type entry_type;
	// identification name
	string entry_name;

	// editor type color
	Color editor_color;
	// editor entry name text
	cGL_Surface *editor_entry_name;

private:
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
