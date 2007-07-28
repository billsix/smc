/***************************************************************************
			keyboard.h  -  header for the corresponding cpp file
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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "../core/globals.h"

/* *** *** *** *** *** *** *** *** cKeyboard *** *** *** *** *** *** *** *** *** */

class cKeyboard
{
public:
	cKeyboard( void );
	~cKeyboard( void );

	// Reset all keys
	void Reset_keys( void );

	/* Function : CEGUI_Handle_KeyUp
	 * CEGUI Key Up handler
	 * returns true if CEGUI processed the given key up event
	*/
	bool CEGUI_Handle_Key_Up( SDLKey key );

	/* Function : KeyUp
	 * Gives player various cues depending on the key
	 * returns true if the event was processed
	*/
	bool Key_Up( SDLKey key );

	/* Function : CEGUI_Handle_KeyDown
	 * CEGUI Key Down handler
	 * returns true if CEGUI processed the given key down event
	*/
	bool CEGUI_Handle_Key_Down( SDLKey key );

	/* Function : KeyDown
	 * Key Down handler
	 * returns true if the event was processed
	*/
	bool Key_Down( SDLKey key );

	// Translate a SDLKey to the proper CEGUI::Key
	unsigned int SDLKey_to_CEGUIKey( SDLKey key );

	// Pressed keys
	Uint8 keys[SDLK_LAST];
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// global Keyboard pointer
extern cKeyboard *pKeyboard;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
