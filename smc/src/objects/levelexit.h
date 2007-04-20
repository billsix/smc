/***************************************************************************
                levelexit.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LEVELEXIT_H__
#define __LEVELEXIT_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** Levelchange types *** *** *** *** *** *** *** *** *** *** */

enum Levelchange_type
{
	LVLCHANGE_BEAM		= 0,	// no special animation ( e.g. doors )
	LVLCHANGE_WARP		= 1		// the player slowly moves into the object ( e.g. pipes )
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* The Levelexit
 * if a level filename is given the given level is the destination
*/
class cLevelExit : public cImageObjectSprite
{
public:
	// constructor
	cLevelExit( float x, float y );
	// create from stream
	cLevelExit( XMLAttributes &attributes );
	// destructor
	virtual ~cLevelExit( void );

	// init defaults
	void Init( void );
	// copy this sprite
	virtual cLevelExit *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );
    // Set Direction
	void Set_Direction( ObjectDirection dir );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Activate the Levelexit
	void Activate( void );

	// Set the type
	void Set_Type( Levelchange_type ltype );

	// Sets the destination level
	void Set_Level( string filename );
	// Returns the destination level
	string Get_Level( bool with_dir = 1, bool with_end = 1 );

	// if draw is valid for the current state and position
	virtual bool is_Draw_valid( void );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Direction_Select( const EventArgs &event ); // editor direction option selected
	bool Editor_Destination_Level_Key( const EventArgs &event );  // editor destination level key up

	// levelchange type
	Levelchange_type levelchange_type;
	// destination level if type is not exit
	string levelname;

	// colors for the type
	Color col_exit, col_change;

private:
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
