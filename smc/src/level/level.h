/***************************************************************************
                level.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "../core/globals.h"
#include "../level/global_effect.h"
#include "../level/level_background.h"
#include "../core/file_parser.h"

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

/* *** *** *** *** *** cLevel  *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel : public cFile_parser, public XMLHandler
{
public:
	cLevel( void );
	~cLevel( void );

	// Creates a new level
	int New( string filename = "null" );
	/* Loads a Level
	 * if delayed is given the level will be loaded on the next update
	*/
	bool Load( string filename, bool delayed = 0 );
	/* Unloads the current Level
	 * if delayed is given unloads the on the next update
	*/
	void Unload( bool delayed = 0 );
	// Save the Level
	void Save( void );

	// update level changes
	void Update( void );

	// Draws all Layer 1 Level Objects ( Passive/Massive Sprites, Enemies, Animations )
	// and updates them if update is true
	void Draw_Layer1( LevelDrawType type = LVL_DRAW );
	// Draws all Layer 2 Level Objects ( Front Passive Sprites, Editor Objects, Editor Hud )
	// and updates them if update is true
	void Draw_Layer2( LevelDrawType type = LVL_DRAW );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	/* handle key down event
	 * returns true if the key was processed
	*/
	bool Key_Down( SDLKey key );
	/* handle key up event
	 * returns true if the key was processed
	*/
	bool Key_Up( SDLKey key );

	// Returns the number of Backgrounds
	unsigned int Get_Backgroundlayers( void );
	/* Returns a pointer to the requested Background
	 * If it doesn't exists returns NULL
	*/
	cBackground *Get_background( unsigned int num );

	/* Returns the current Music filename with the given options
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
	// Set the Level Author
	void Set_Author( string name );
	// Set the Level Version
	void Set_Version( string level_version );

	// Returns true if a level is loaded
	bool is_Loaded( void );

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

	// background layers
	typedef vector<cBackground *> BackgroundList;
	BackgroundList backgrounds;

	// global effect
	cGlobal_effect *pGlobal_effect;
private:
	// Handles the old level format command
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );
	bool read_color_info( string *parts, unsigned int count, unsigned int line, Color &read_color );

	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );

	// XML element Item Tag list
	XMLAttributes xml_attributes;
};

// Returns a Level Object if element name is available else NULL
cSprite *Get_Level_Object( const String &element, XMLAttributes &attributes );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level
extern cLevel *pLevel;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
