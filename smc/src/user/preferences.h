/***************************************************************************
           preferences.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cPreferences : public XMLHandler
{
public:
	cPreferences( void );
	virtual ~cPreferences( void );
	
	// Load the Preferences from a file
	bool Load( string filename = "" );
	// Save the Preferences to a file
	void Save( void );

	// Reset the Settings
	void Default( void );
	 // Gets the settings from Current in-game Settings
	void Update( void );
	// Sets the settings to Current in-game Settings
	void Apply( void );

	/* *** *** *** Game Settings *** *** *** *** */

	// Audio
	bool Music, Sounds;
	Uint8 music_volume, sound_volume;

	// Video
	bool Fullscreen, Joy_enabled;
	Uint16 Screen_W, Screen_H;
	Uint8 Screen_Bpp;

	// Keyboard key definitions
	SDLKey Key_up, Key_down, Key_left, Key_right, Key_shoot;
	// Joypad key definitions
	int Joy_shoot, Joy_jump, Joy_exit, Joy_item;

	// Special
	bool Lvleditor_automousehide, Backgroundimages_disabled;

	// directories
	string level_dir;

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

#endif
