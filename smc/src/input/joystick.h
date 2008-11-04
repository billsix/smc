/***************************************************************************
 * joystick.h  -  header for the corresponding cpp file
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

#ifndef SMC_JOYSTICK_H
#define SMC_JOYSTICK_H

#include "../core/globals.h"

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

class cJoystick
{
public:
	cJoystick( void );
	~cJoystick( void );

	// Initializes the Joystick system
	int Init( void );
	// Closes the current Joystick
	void Close( void );

	// Opens the specified Joystick
	bool Stick_Open( unsigned int index );
	// Closes the Stick
	void Stick_Close( void );

	// Resets all Buttons and modifiers
	void Reset_keys( void );

	// Handles the Joystick motion
	void Handle_Motion( SDL_Event *ev );
	// Handle Joystick Button down event
	bool Handle_Button_Down_Event( SDL_Event *ev );
	// Handle Joystick Button up event
	bool Handle_Button_Up_Event( SDL_Event *ev );

	// Returns the current Joystick name
	string Get_Name( void );
	// Returns all available Joystick names
	vector<string> Get_Names( void );

	// Sets the given button state
	void Set_Button( Uint8 button, bool pressed );

	// Get the assigned shortcut
	Uint8 *Get_Shortcut( input_identifier shortcut_id );
	// Assign a shortcut
	void Assign_Shortcut( input_identifier shortcut_id, Uint8 new_button );

	// check if the analog direction is pressed
	bool Left( void );
	bool Right( void );
	bool Up( void );
	bool Down( void );
	// check if the given button is pushed
	bool Button( Uint8 button );

	// current joystick pointer
	SDL_Joystick *joystick;
	// current Joystick name
	string joy_name;

	// button state array
	typedef vector<bool> ButtonList;
	ButtonList buttons;
	
	// analog directions
	bool left, right, up, down;

	// current opened joystick
	int cur_stick;
	// if true the current joystick is available/loaded
	bool stick_open;
	
	// available buttons
	unsigned int num_buttons;
	// available axes
	unsigned int num_axes;
	// available balls
	unsigned int num_balls;

	// if true print debug output
	bool debug;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// global Joystick pointer
extern cJoystick *pJoystick;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
