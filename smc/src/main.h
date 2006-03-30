/***************************************************************************
                main.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
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

#include "globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Function: InitGame
// Initialize everything, called before everything else.
// Seed random numbers, init CEGUI, SDL, OpenGL, load preferences, and create globals
void InitGame( void );

// Function: ExitGame
// Delete globals. Also saves preferences and closes SDL.
void ExitGame( void );

// Function: HandleInput
// Top-level input function. Calls either <KeyDown>, <KeyUp>,
// or passes control to pMouseCursor or pJoystick
void HandleInput( void );

// Function: KeyUp
// Gives player various cues depending on the key.
void KeyUp( SDLKey key );

// Function: KeyDown
// Key Down handler.
void KeyDown( SDLKey key );

// Function: KeyDown_Level
// Level Key Down handler.
void KeyDown_Level( SDLKey key );

// Function: HandleInput_Level
// Called continuously from Game Loop. Level input handler.
void HandleInput_Level( void );

// Function: ProcessInput_Level
// Called continuoutsly from Game Loop. Level static input handler.
void ProcessInput_Level( void );

// Function: Save_Screenshot
// Save bitmap of current screen. (not implemented)
void Save_Screenshot( void );

// Function: UpdateGame
// Called continuoutsly from Game Loop. Updates globals, draws, and swaps.
void UpdateGame( void );


#endif
