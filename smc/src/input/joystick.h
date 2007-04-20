/***************************************************************************
                joystick.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

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

	// Handles the Joystick events
	bool Handle_Event( SDL_Event *ev );
	// Handles the Joystick motion
	void Handle_Motion( SDL_Event *ev );
	// Handles the Joystick Button Events
	void Handle_Button_Event( SDL_Event *ev );

	// Returns the current Joystick name
	string Get_Name( void );
	// Returns all available Joystick names
	vector<string> Get_Names( void );

	// Sets the given button state
	void Set_Button( unsigned int button, bool pressed );

	// check if the analog direction is pressed
	bool Left( int nThreshold = 3000 );
	bool Right( int nThreshold = 3000 );
	bool Up( int nThreshold = 3000 );
	bool Down( int nThreshold = 3000 );
	// check if the given button is pushed
	bool Button( unsigned int button );

	// current joystick pointer
	SDL_Joystick *joystick;
	// current Joystick name
	string joy_name;

	// button state array
	typedef vector<int> ButtonList;
	ButtonList buttons;
	
	// analog directions
	bool left, right, up, down;

	// analog threshold
	int Threshold;
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
