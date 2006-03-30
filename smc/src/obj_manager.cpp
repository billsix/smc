/***************************************************************************
			obj_manager.cpp  -  Object Handler/Manager
                              -------------------
    copyright            :	(C) 2005 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "globals.h"

cObjectManager :: cObjectManager( void )
{
	items.reserve( 2000 );
}

cObjectManager :: ~cObjectManager( void )
{
	Delete_All();
}

cSprite *cObjectManager :: Get_Pointer( unsigned int identifier )
{
	if( Get_Size() > identifier )
	{
		return items[identifier];
	}

	return NULL; // not found
}

cSprite *cObjectManager :: Copy( unsigned int identifier )
{
	if( identifier > items.size() )
	{
		return NULL;
	}

	if( items[identifier] )
	{
		return Copy_Object( items[identifier], items[identifier]->posx, items[identifier]->posy );
	}

	return NULL; // not found
}

void cObjectManager :: Update( void )
{
	for( int i = Get_Size() - 1; i >= 0; i-- )
	{
		if( items[i]->destroy )
		{
			delete items[i];
			items.erase( items.begin() + i );
		}
	}
}

unsigned int cObjectManager :: Get_Size( void )
{
	return (unsigned int)items.size();
}

unsigned int cObjectManager :: Get_Size_array( unsigned int array )
{
	unsigned int count = 0;

	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( items[i] && items[i]->array == array )
		{
			count++;
		}
	}

	return count;
}

void cObjectManager :: Add( cSprite *sprite )
{
	if( !sprite )
	{
		return;
	}

	items.push_back( sprite );
}

void cObjectManager :: Delete( unsigned int identifier, bool delete_data /* = 1 */ )
{
	if( identifier >= items.size() )
	{
		return;
	}

	if( items[identifier] && delete_data )
	{
		delete items[identifier];
	}

	items.erase( items.begin() + identifier );
}

void cObjectManager :: Delete_Objects( bool delayed /* = 0 */ )
{
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i] )
		{
			if( delayed ) // delete only on update
			{
				items[i]->destroy = 1;
				items[i]->ClearCollisions();
			}
			else
			{
				delete items[i];
				items.erase( items.begin() + i );
			}
		}
	}

	// clear collision data
	if( pPlayer )
	{
		pPlayer->ClearCollisions();
		pPlayer->Reset_onGround();
	}
	if( pMouseCursor )
	{
		pMouseCursor->ClearCollisions();
	}
}

void cObjectManager :: Delete_All( bool delayed /* = 0 */ )
{
	Delete_Objects( delayed );

	if( !delayed )
	{
		items.clear();
	}
}
