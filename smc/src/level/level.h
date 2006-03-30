/***************************************************************************
                level.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Background Image Type definitions
typedef enum 
{
	BG_NONE = 0,			// no background type
	BG_IMG_LEFTRIGHT = 1,	// only to the horizontal
	BG_IMG_ALL = 2,			// into all directions
	BG_IMG_TOP = 3,			// is always on top
	BG_GR_VER = 103,		// Gradient Vertical
	BG_GR_HOR = 104			// Gradient Horizontal
} BackgroundType;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cBackground
{
public:
	cBackground( void );
	~cBackground( void );

	// Sets the type of Background
	void Set_type( BackgroundType ntype );

	// Sets the background color
	void Set_color_1( const Color &color );
	void Set_color_2( const Color &color );

	// Set the Background image
	void Set_image( string nimg_file_1 );
	// Set the Background Image scrolling speed
	void Set_image_scroll( double x = 1, double y = 1 );

	void Draw( void );
	void DrawGradient( void );

	// Returns the name of the current type
	string Get_Typename( void );

	// type
	BackgroundType type;

	// background image filename
	string img_file_1;
	// background image
	GL_Surface *img_1;

	// position
	double posx, posy;

	// background image scrolling speed
	double speedx, speedy;

	// background gradient colors
	Color color_1, color_2;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Level drawing Type definitions
typedef enum 
{
	LVL_UPDATE		= 1,	// updates and draws
	LVL_DRAW		= 0,	// only draw
	LVL_DRAW_EDITOR = 2,	// only draw but in the leveleditor order
	LVL_DRAW_NO_BG	= 3,	// only draws and without background gradient and image
	LVL_DRAW_BG		= 4		// only draws the backround gradient and image
} LevelDrawType;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cLevel : public cFile_parser
{
public:
	cLevel( void );
	~cLevel( void );

	// Creates a new level
	int New( string filename = "null" );
	// Loads a Level
	bool Load( string filename );
	// Unloads the current Level
	void Unload( void );
	// Saves the current Level
	void Save( void );

	// Draws all Level Objects ( Sprites, Enemies, Hud )
	// and updates them if update is true
	void Draw( LevelDrawType type = LVL_UPDATE );

	// Shows a big Text with the string and exits
	void Show_Error( string text );

	// Returns the number of Backgrounds
	unsigned int Get_Backgroundlayers( void );
	/* Returns a pointer to the requested Background
	 * If it doesn't exists returns NULL
	*/
	cBackground *Get_background( unsigned int num );

	// Returns the current Music filename with the given options
	string Get_Musicfile( bool with_dir = 1, bool with_end = 1 );
	// Sets the Musicfile
	void Set_Musicfile( string filename );
	/* Sets a new Levelfile name and automatically resaves the level
	 * if given also deletes the old level name
	*/
	void Set_Levelfile( string filename, bool delete_old = 1 );

	// Returns true if a level is loaded
	bool is_Loaded( void );

	// music filename
	string musicfile;
	// valid music to play
	bool valid_music;

	// level filename
	string levelfile;

	// levelengine version from the loaded Level
	int levelengine_version;

	// background layers
	typedef vector<cBackground *> BackgroundList;
	BackgroundList backgrounds;

	// Returns 0 if failed, 1 if ingame save, 2 if Overworld save
	int Load_Savegame( unsigned int save_slot );
	// saves the game with the given description
	bool Save_Savegame( unsigned int save_slot, string description );
private:
	// Handles the command
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );
	bool read_color_info(string *parts, unsigned int count, unsigned int line, Color &read_color );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
