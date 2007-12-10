/***************************************************************************
     level_settings.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LEVEL_SETTINGS_H__
#define __LEVEL_SETTINGS_H__

#include "../core/globals.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** *** Level Settings *** *** *** *** *** *** *** *** *** *** */
class cLevel_Settings
{
public:
	cLevel_Settings( void );
	~cLevel_Settings( void );

	// Initializes Text Sprites and Graphics
	void Init( void );
	// Enter the Level Settings
	void Enter( void );
	// Unload
	void Unload( void );

	// Update
	void Update( void );
	// Draw
	void Draw( void );

	/* handle key down event
	 * returns true if the key was processed
	*/
	bool Key_Down( SDLKey key );

	// Adds a Background image
	bool Add_Background_image( const EventArgs &event );
	// Delete the currently selected Background image
	bool Delete_Background_image( const EventArgs &event );
	// Set the current Background image
	bool Set_Background_image( const EventArgs &event );

	// Saves the Settings and exits
	bool Button_Save( const EventArgs &event );

	// Updates the Colors
	bool Update_BG_Colors( const EventArgs &event );
	// Loads the Background image List
	void Load_BG_imageList( void );

	// Set the Background image preview
	void Set_Background_image_Preview( string filename );
	// Updates the Background image
	bool Update_BG_image( const EventArgs &event );
	// Clears the background Layer fields
	void Clear_Layer_field( void );

	// Loads the global effect data
	void Load_global_effect( void );
	// Set the global effect image preview
	void Set_global_effect_image_Preview( string filename );
	// Updates the global effect image
	bool Update_global_effect_image( const EventArgs &event );

	// true if menu is active
	bool active;

	// Background colors
	Color bg_color_1, bg_color_2;

	// Gui
	Window *guiwindow;
	TabControl *tabcontrol;

	// background image preview
	cHudSprite *background_preview;

	// global effect image preview
	cHudSprite *global_effect_preview;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
