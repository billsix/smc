/***************************************************************************
 * level_exit.h  -  header for the corresponding cpp file
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

#ifndef SMC_LEVEL_EXIT_H
#define SMC_LEVEL_EXIT_H

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** Level Exit types *** *** *** *** *** *** *** *** *** *** */

enum Level_Exit_type
{
	LEVEL_EXIT_BEAM		= 0,	// no animation ( f.e. a door or hole )
	LEVEL_EXIT_WARP		= 1		// rotated player moves slowly into the destination direction
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Level exit
 * if a level filename it's the destination
*/
class cLevel_Exit : public cImageObjectSprite
{
public:
	// constructor
	cLevel_Exit( float x, float y );
	// create from stream
	cLevel_Exit( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cLevel_Exit( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevel_Exit *Copy( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );
    // Set direction
	void Set_Direction( ObjectDirection dir );

	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Activate
	void Activate( void );

	// Set the type
	void Set_Type( Level_Exit_type ltype );

	// Set the destination level
	void Set_Level( string filename );
	// Return the destination level
	string Get_Level( bool with_dir = 1, bool with_end = 1 );

	// Set the destination entry
	void Set_Entry( string entry_name );

	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	// level editor activation
	virtual void Editor_Activate( void );
	// level editor events
	bool Editor_Direction_Select( const CEGUI::EventArgs &event ); // editor direction option selected
	bool Editor_Destination_Level_Key( const CEGUI::EventArgs &event );  // editor destination level key up
	bool Editor_Destination_Entry_Key( const CEGUI::EventArgs &event );  // editor destination entry key up

	// level exit type
	Level_Exit_type exit_type;
	// destination level
	string dest_level;
	// destination entry ( only used if in same level )
	string dest_entry;

	// editor type color
	Color editor_color;
	// editor entry name text
	cGL_Surface *editor_entry_name;

private:
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
