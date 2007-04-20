/***************************************************************************
                main.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __MAIN_H__
#define __MAIN_H__

#include "../core/globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Function : InitGame
// Initialize everything, called before everything else
// Seed random numbers, init SDL, OpenGL, CEGUI, load preferences, and create globals
void InitGame( void );

// Function : ExitGame
// Delete globals. Also saves preferences and closes SDL
void ExitGame( void );

/* Function : Handle_Input_Global
 * Top-level input function. Calls either KeyDown, KeyUp,
 * or passes control to pMouseCursor or pJoystick
 * Returns true if the event was handled.
*/ 
bool Handle_Input_Global( SDL_Event *ev );

// Function : Update_Game
// Called continuously from Game Loop. Update current game state
void Update_Game( void );

// Function : Draw_Game
// Called continuously from Game Loop. Draw current game state
void Draw_Game( void );

#endif
