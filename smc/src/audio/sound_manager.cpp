/***************************************************************************
			sound_manager.cpp  -  Sound Handler/Manager
                              -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

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
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i]->filename.compare( path ) == 0 )
		{
			return items[i];	// return first found
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
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		delete items[i];
		items[i]= NULL;
	}
}

void cSoundManager :: Delete_All( void )
{
	Delete_Sounds();
	items.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSoundManager *pSoundManager = NULL;
