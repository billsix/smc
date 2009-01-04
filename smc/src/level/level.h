/***************************************************************************
 * level.h  -  header for the corresponding cpp file
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

#ifndef SMC_LEVEL_H
#define SMC_LEVEL_H

#include "../core/globals.h"
#include "../level/global_effect.h"
#include "../level/level_background.h"
#include "../core/file_parser.h"
#include "../level/level_manager.h"
#include "../objects/level_entry.h"
#include "../audio/random_sound.h"


/* *** *** *** *** *** cLevel *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel : public cFile_parser, public CEGUI::XMLHandler
{
public:
	cLevel( void );
	virtual ~cLevel( void );

	/* Create a new level
	 * returns true if successful
	*/
	bool New( string filename );
	/* Load a Level
	 * if delayed is given the level will be loaded on the next update
	*/
	bool Load( string filename, bool delayed = 0 );
	/* Unload the current Level
	 * if delayed is given unloads the on the next update
	*/
	void Unload( bool delayed = 0 );
	// Save the Level
	void Save( void );
	// Delete and Unload Level
	void Delete( void );

	// update level changes
	void Update( void );

	// Draw Layer 1 ( Backgrounds, Level objects and Animations )
	void Draw_Layer_1( LevelDrawType type = LVL_DRAW );
	// Draw Layer 2 ( Global Effects )
	void Draw_Layer_2( LevelDrawType type = LVL_DRAW );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	/* handle key down event
	 * returns true processed
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

	/* Return the current Music filename with the given options
	 * if with_dir is set to 0 the whole directory is cut
	 * if set to 1 the music directory is cut out
	 * if set to 1 the full directory will be returned
	*/
	string Get_Musicfile( int with_dir = 2, bool with_end = 1 );
	// Set the Music filename
	void Set_Musicfile( string filename );
	/* Set a new Level filename name and automatically re-save the level
	 * delete_old : if set delete the old level name
	*/
	void Set_Levelfile( string filename, bool delete_old = 1 );
	/* Return the level path if level is valid else empty
	 * check_only_user_dir : only check user directory for the level
	*/
	bool Get_Path( string &filename, bool check_only_user_dir = 0 );
	// Set the Level Author
	void Set_Author( string name );
	// Set the Level Version
	void Set_Version( string level_version );

	// Get entry with the given name
	cLevel_Entry *Get_Entry( string name );

	// Return true if a level is loaded
	bool Is_Loaded( void );

	// music filename
	string musicfile;
	// valid music to play
	bool valid_music;

	// level filename
	string levelfile;
	// if a new level should be loaded this is the next level filename
	string next_levelfile;

	// unload the level on the next update
	bool delayed_unload;

	// level engine version
	float engine_version;
	// last save time
	time_t last_saved;
	// level author
	string author;
	// level version
	string version;

	// background manager
	cBackground_Manager *background_manager;
	// global effect
	cGlobal_effect *pGlobal_effect;
	// sprite manager
	cSprite_Manager *pSprite_Manager;

private:
	// Handles the old txt level format command
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );
	bool Read_Color_Data( string *parts, unsigned int count, unsigned int line, Color &read_color );

	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );

	// XML element Item Tag list
	CEGUI::XMLAttributes xml_attributes;
};

// Return the Level Object if element name is available else NULL
cSprite *Get_Level_Object( const CEGUI::String &xml_element, CEGUI::XMLAttributes &attributes, float engine_version = level_engine_version );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level
extern cLevel *pActive_Level;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
