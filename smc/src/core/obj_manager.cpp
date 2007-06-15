/***************************************************************************
			obj_manager.cpp  -  Object Handler/Manager
                              -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../core/obj_manager.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../input/mouse.h"
#include "../overworld/ow_player.h"

/* *** *** *** *** *** *** cObjectManager *** *** *** *** *** *** *** *** *** *** *** */

cObjectManager :: cObjectManager( unsigned int reserve_items /* = 2000 */, unsigned int zpos_items /* = 100 */ )
{
	items.reserve( reserve_items );

	for( unsigned int i = 0; i < zpos_items; i++ )
	{
		zposdata.push_back( 0.0f );
		zposdata_editor.push_back( 0.0f );
	}
}

cObjectManager :: ~cObjectManager( void )
{
	Delete_All();
}

void cObjectManager :: Add( cSprite *sprite )
{
	// empty object
	if( !sprite )
	{
		return;
	}

	Set_Z( sprite );

	items.push_back( sprite );
}

cSprite *cObjectManager :: Copy( unsigned int identifier )
{
	if( identifier >= items.size() )
	{
		return NULL;
	}

	return Copy_Object( items[identifier], items[identifier]->posx, items[identifier]->posy );
}

void cObjectManager :: Delete( unsigned int array_num, bool delete_data /* = 1 */ )
{
	// out of array
	if( array_num >= items.size() )
	{
		return;
	}

	if( delete_data )
	{
		delete items[array_num];
	}

	items.erase( items.begin() + array_num );
}

bool cObjectManager :: Delete( cSprite *obj, bool delete_data /* = 1 */ ) 
{
	// empty object
	if( !obj )
	{
		return 0;
	}

	int array_num = Get_Array_num( obj );

	// not found
	if( array_num < 0 )
	{
		return 0;
	}

	Delete( array_num, delete_data );
	
	// successful
	return 1;
}

void cObjectManager :: Delete_Objects( bool delayed /* = 0 */ )
{
	// clear z position data
	for( unsigned int i = 0; i < zposdata.size(); i++ )
	{
		zposdata[i] = 0.0f;
		zposdata_editor[i] = 0.0f;
	}

	// delete objects
	for( SpriteList::iterator itr = items.begin(); itr != items.end(); )
	{
		// get object pointer
		cSprite *obj = (*itr);

		// delete only on update
		if( delayed )
		{
			obj->Destroy();
			obj->Collisions_Clear();
			++itr;
		}
		// delete
		else
		{
			itr = items.erase( itr );
			delete obj;
		}
	}

	// clear collision data
	if( pPlayer )
	{
		pPlayer->Collisions_Clear();
		pPlayer->Reset_onGround();
	}
	if( pMouseCursor )
	{
		pMouseCursor->Collisions_Clear();
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

cSprite *cObjectManager :: Get_Pointer( unsigned int identifier )
{
	if( identifier >= items.size() )
	{
		// out of array
		return NULL;
	}

	// available
	return items[identifier];
}

cSprite *cObjectManager :: Get_First( SpriteType type )
{
	cSprite *first = NULL;

	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( obj->type == type && ( !first || obj->posz < first->posz ) )
		{
			first = obj;
		}
	}

	// return result
	return first;
}

cSprite *cObjectManager :: Get_Last( SpriteType type )
{
	cSprite *last = NULL;

	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( obj->type == type && ( !last || obj->posz > last->posz ) )
		{
			last = obj;
		}
	}

	// return result
	return last;
}

cSprite *cObjectManager :: Get_from_Position( int posx, int posy, SpriteType type /* = TYPE_UNDEFINED */ )
{
	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( (int)obj->startposx != posx || (int)obj->startposy != posy )
		{
			continue;
		}

		// if type is given
		if( type != TYPE_UNDEFINED )
		{
			// skip invalid type
			if( obj->type != type )
			{
				continue;
			}
		}

		// found
		return obj;
	}

	return NULL;
}

void cObjectManager :: Get_Objects_sorted( SpriteList &objects, bool editor_sort /* = 0 */, bool with_player /* = 0 */ )
{
	objects = items;

	if( with_player )
	{
		if( Game_Mode == MODE_LEVEL )
		{
			objects.push_back( pPlayer );
		}
		else if( Game_Mode == MODE_OVERWORLD )
		{
			objects.push_back( pOverworld_Player );
		}
	}

	// z position sort
	if( !editor_sort )
	{
		// default
		sort( objects.begin(), objects.end(), zpos_sort() );
	}
	else
	{
		// editor
		sort( objects.begin(), objects.end(), editor_zpos_sort() );
	}
}

int cObjectManager :: Get_Array_num( cSprite *obj ) 
{
	// invalid
	if( !obj )
	{
		return -1;
	}

	for( unsigned int i = 0; i < items.size(); i++ )
	{
		// if the same
		if( items[i] == obj )
		{
			return i;
		}
	}

	// not found
	return -1;
}

void cObjectManager :: Update_items_valid_draw( void )
{
	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update_valid_draw();
	}
}

void cObjectManager :: Update_items( void )
{
	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update();
	}
}

void cObjectManager :: Draw_items( void )
{
	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cObjectManager :: Handle_Collision_items( void )
{
	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// collision and movement handling
		(*itr)->Collide_Move();
		// handle found collisions
		(*itr)->Handle_Collisions();
	}
}

void cObjectManager :: Update( void )
{
	for( SpriteList::iterator itr = items.begin(); itr != items.end(); )
	{
		// get object pointer
		cSprite *obj = (*itr);

		// if destroy is set
		if( obj->destroy )
		{
			itr = items.erase( itr );
			delete obj;
		}
		// increment
		else
		{
			++itr;
		}
	}
}

unsigned int cObjectManager :: size( void )
{
	return items.size();
}

unsigned int cObjectManager :: Get_Size_array( ArrayType sprite_array )
{
	unsigned int count = 0;

	for( SpriteList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		if( (*itr)->sprite_array == sprite_array )
		{
			count++;
		}
	}

	return count;
}

void cObjectManager :: Switch( cSprite *obj1, cSprite *obj2 )
{
	// empty object
	if( !obj1 || !obj2 )
	{
		return;
	}

	// if the same
	if( obj1 == obj2 )
	{
		return;
	}

	int obj1_pos = Get_Array_num( obj1 );
	int obj2_pos = Get_Array_num( obj2 );
	
	// not found
	if( obj1_pos < 0 || obj2_pos < 0 )
	{
		return;
	}

	items[obj1_pos] = obj2;
	items[obj2_pos] = obj1;
}

void cObjectManager :: Set_Z( cSprite *sprite )
{
	// set new z position if unset
	if( sprite->posz <= zposdata[sprite->type] )
	{
		sprite->posz = zposdata[sprite->type] + 0.000001f;
	}
	// if editor Z position is given
	if( sprite->editor_posz != 0 )
	{
		if( sprite->editor_posz <= zposdata_editor[sprite->type] )
		{
			sprite->editor_posz = zposdata_editor[sprite->type] + 0.000001f;
		}
	}


	// update z position
	if( sprite->posz > zposdata[sprite->type] )
	{
		zposdata[sprite->type] = sprite->posz;
	}
	// if editor Z position is given
	if( sprite->editor_posz != 0 )
	{
		if( sprite->editor_posz > zposdata_editor[sprite->type] )
		{
			zposdata_editor[sprite->type] = sprite->editor_posz;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cObjectManager *pObjManager = NULL;
