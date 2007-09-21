/***************************************************************************
           preferences.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include "../core/globals.h"

/* *** *** *** *** *** cPreferences *** *** *** *** *** *** *** *** *** *** *** *** */

class cPreferences : public XMLHandler
{
public:
	cPreferences( void );
	virtual ~cPreferences( void );
	
	// Load the preferences from a file
	bool Load( string filename = "" );
	// Save the preferences to a file
	void Save( void );

	// Reset the settings
	void Default( void );
	 // Gets the settings from current in-game settings
	void Update( void );
	// Sets the settings to current in-game settings
	void Apply( void );

	/* *** *** *** Game Settings *** *** *** *** */

	// Game
	float camera_hor_speed, camera_ver_speed;

	// Audio
	bool Music, Sounds;
	Uint8 music_volume, sound_volume;

	// Video
	bool Fullscreen, joy_enabled;
	Uint16 Screen_W, Screen_H;
	Uint8 Screen_Bpp;
	bool vsync;

	// Keyboard key definitions
	SDLKey key_up, key_down, key_left, key_right, key_jump, key_shoot, key_action;
	// Joypad key definitions
	int joy_jump, joy_shoot, joy_item, joy_action, joy_exit;

	// Special
	bool always_run;
	string joy_name;
	bool joy_analog_jump;
	bool editor_mouse_auto_hide, background_images;

	/* *** *** *** *** *** *** *** */

	string config_filename;

private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );
	// handles an item
    void handle_item( const XMLAttributes& attributes );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Preferences
extern cPreferences *pPreferences;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
