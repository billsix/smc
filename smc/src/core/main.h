/***************************************************************************
 * main.h  -  header for the corresponding cpp file
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

#ifndef SMC_MAIN_H
#define SMC_MAIN_H

#include "../core/globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Initialize everything, called before everything else
// Seed random numbers, init SDL, OpenGL, CEGUI, load preferences, and create globals
void Init_Game( void );

// Delete globals. Also saves preferences and closes SDL
void Exit_Game( void );

/*
 * Top-level input function. Calls either KeyDown, KeyUp,
 * or passes control to pMouseCursor or pJoystick
 * Returns true if the event was handled.
*/ 
bool Handle_Input_Global( SDL_Event *ev );

// Called continuously from Game Loop. Update current game state
void Update_Game( void );

// Called continuously from Game Loop. Draw current game state
void Draw_Game( void );

#endif
