/***************************************************************************
			sound_manager.cpp  -  Sound Handler/Manager
                              -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../audio/sound_manager.h"

/* *** *** *** *** *** *** cSoundManager *** *** *** *** *** *** *** *** *** *** *** */

cSoundManager :: cSoundManager( void )
{
	load_count = 0;
}

cSoundManager :: ~cSoundManager( void )
{
	Delete_All();
}

cAudio_Sound *cSoundManager :: Get_Pointer( string path )
{
	for( SoundList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		cAudio_Sound *obj = (*itr);

		if( obj->filename.compare( path ) == 0 )
		{
			return obj;	// return first found
		}
	}

	return NULL; // not found
}

cAudio_Sound *cSoundManager :: Get_Pointer_array( unsigned int identifier )
{
	if( items.size() > identifier )
	{
		return items[identifier];
	}

	return NULL; // not found
}

unsigned int cSoundManager :: Get_Size( void )
{
	return (unsigned int)items.size();
}

void cSoundManager :: Add( cAudio_Sound *sound )
{
	load_count++;
	items.push_back( sound );
}

void cSoundManager :: Delete_Sounds( void )
{
	for( SoundList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		cAudio_Sound *obj = (*itr);

		delete obj;
		obj = NULL;
	}
}

void cSoundManager :: Delete_All( void )
{
	Delete_Sounds();
	items.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSoundManager *pSoundManager = NULL;
