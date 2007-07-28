/***************************************************************************
                level_entry.h  -  header for the corresponding cpp file
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

#ifndef __LEVEL_ENTRY_H__
#define __LEVEL_ENTRY_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

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
	cLevel_Entry( XMLAttributes &attributes );
	// destructor
	virtual ~cLevel_Entry( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevel_Entry *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );
    // Set direction
	void Set_Direction( ObjectDirection dir );

	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Activate
	void Activate( void );

	// Set the name
	void Set_Name( string str_name );

	// if draw is valid for the current state and position
	virtual bool is_Draw_valid( void );

	// level editor activation
	virtual void Editor_Activate( void );
	// level editor events
	bool Editor_Direction_Select( const EventArgs &event ); // editor direction option selected
	bool Editor_Name_Key( const EventArgs &event );  // editor name key up

	// identification name
	string entry_name;

	// editor type color
	Color editor_color;
	// editor entry name text
	GL_Surface *editor_entry_name;

private:
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
