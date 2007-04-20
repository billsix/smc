/***************************************************************************
                sound_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "../audio/audio.h"

/* *** *** *** *** *** *** cSoundManager *** *** *** *** *** *** *** *** *** *** *** */

//  Keeps track of all the sounds in memory
//
// Operators:
//  - cSoundManager [path]
//  - cSoundManager [ID]
class cSoundManager
{
public:
	cSoundManager( void );
	~cSoundManager( void );

	// Function: Get_Pointer
	// Returns the Sound by Path
	cAudio_Sound *Get_Pointer( string path );

	// Function: Get_Pointer
	// Returns the Sound by ID
	cAudio_Sound *Get_Pointer( unsigned int identifier );

	// Function: Get_Pointer_array
	// Returns an array by ID
	cAudio_Sound *Get_Pointer_array( unsigned int identifier );

	// Function: Get_Path
	// Returns the Sound Path in a string
	string Get_Path( unsigned int identifier );

	/*	Function: Get_Size
	 *	Returns the Current size
	 */
	unsigned int Get_Size( void );

	/* Function: Add
	 *  Adds a Sound
	 *	Should always have a Path
	 */
	void Add( cAudio_Sound *item );

	cAudio_Sound *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}
	
	cAudio_Sound *operator [] ( string path )
	{
		return Get_Pointer( path );
	}

	/*
	* Function: Delete_Sounds
	* Frees all Sounds, but keep items vector entries
	*/
	void Delete_Sounds( void );

	/*
	* Function: Delete_All
	* Deletes all Items completely.
	*/
	void Delete_All( void );

private:
	// sounds loaded since initialization
	unsigned int load_count;
	// sound items array
	SoundList items;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Sound Manager
extern cSoundManager *pSoundManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
