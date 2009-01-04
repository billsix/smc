/***************************************************************************
 * preferences.h  -  header for the corresponding cpp file
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

#ifndef SMC_PREFERENCES_H
#define SMC_PREFERENCES_H

#include "../core/globals.h"

/* *** *** *** *** *** cPreferences *** *** *** *** *** *** *** *** *** *** *** *** */

class cPreferences : public CEGUI::XMLHandler
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
	 // Get settings from current game settings
	void Update( void );
	// Set settings to current game settings
	void Apply( void );
	// Set new video settings
	void Apply_Video( Uint16 screen_w, Uint16 screen_h, Uint8 screen_bpp, bool fullscreen, bool vsync, float geometry_detail, float texture_detail );
	// Set new audio settings
	void Apply_Audio( bool sound, bool music );

	/* *** *** *** Game Settings *** *** *** *** */

	// Game
	// default language
	string language;
	// last version of smc which saved the preferences file
	float game_version;
	// force the given user data directory
	string force_user_data_dir;
	// smart camera speed
	float camera_hor_speed, camera_ver_speed;

	// Audio
	bool audio_music, audio_sound;
	unsigned int audio_hz;

	// Video
	bool video_fullscreen;
	Uint16 video_screen_w, video_screen_h;
	Uint8 video_screen_bpp;
	bool video_vsync;

	// Keyboard
	// key definitions
	SDLKey key_up, key_down, key_left, key_right, key_jump, key_shoot, key_action;
	// scroll speed
	float scroll_speed;
	// Joystick
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
	Uint8 joy_button_jump, joy_button_shoot, joy_button_item, joy_button_action, joy_button_exit;


	// Special
	// player always runs
	bool always_run;
	// level background images enabled
	bool level_background_images;
	// image cache enabled
	bool image_cache_enabled;

	// Editor
	// hide mouse if clicked on an object
	bool editor_mouse_auto_hide;
	// show item images
	bool editor_show_item_images;
	// size of the item images
	unsigned int editor_item_image_size;
	/* *** *** *** *** *** *** *** */

	// configuration filename
	string config_filename;

private:
	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );
	// handles an item
    void handle_item( const CEGUI::XMLAttributes& attributes );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Preferences
extern cPreferences *pPreferences;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
