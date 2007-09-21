/***************************************************************************
                sound_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "../audio/audio.h"
#include "../core/obj_manager.h"

/* *** *** *** *** *** *** cSoundManager *** *** *** *** *** *** *** *** *** *** *** */

/*  Keeps track of all the sounds in memory
 *
 * Operators:
 * - cSoundManager [path]
 * - cSoundManager [ID]
*/
class cSoundManager : public cObject_Manager<cAudio_Sound>
{
public:
	cSoundManager( void );
	virtual ~cSoundManager( void );

	// Return the Sound by Path
	virtual cAudio_Sound *Get_Pointer( string path );

	// Return the Sound Path in a string
	string Get_Path( unsigned int identifier );

	/* Add a Sound
	 * Should always have the path set
	 */
	void Add( cAudio_Sound *item );

	cAudio_Sound *operator [] ( unsigned int identifier )
	{
		return cObject_Manager<cAudio_Sound>::Get_Pointer( identifier );
	}
	
	cAudio_Sound *operator [] ( string path )
	{
		return Get_Pointer( path );
	}

	/* Free all Sounds, but keep object vector entries
	*/
	void Delete_Sounds( void );

private:
	// sounds loaded since initialization
	unsigned int load_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Sound Manager
extern cSoundManager *pSoundManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
