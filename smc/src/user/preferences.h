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
	bool audio_music, audio_sound;
	Uint8 audio_music_volume, audio_sound_volume;
	unsigned int audio_hz;

	// Video
	bool video_fullscreen;
	Uint16 video_screen_w, video_screen_h;
	Uint8 video_screen_bpp;
	bool video_vsync;

	// Keyboard key definitions
	SDLKey key_up, key_down, key_left, key_right, key_jump, key_shoot, key_action;
	// Joypad
	bool joy_enabled;
	// active joy name
	string joy_name;
	// jump with upwards
	bool joy_analog_jump;
	// hor/ver axis used
	int joy_axis_hor, joy_axis_ver;
	// axis threshold
	Sint16 joy_axis_threshold;
	// button definitions
	int joy_button_jump, joy_button_shoot, joy_button_item, joy_button_action, joy_button_exit;


	// Special
	bool always_run;
	bool editor_mouse_auto_hide, level_background_images;

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
