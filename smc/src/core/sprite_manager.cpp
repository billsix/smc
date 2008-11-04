/***************************************************************************
 * sprite_manager.cpp  -  Sprite Manager
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/sprite_manager.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../input/mouse.h"
#include "../overworld/world_player.h"

/* *** *** *** *** *** *** cSprite_Manager *** *** *** *** *** *** *** *** *** *** *** */

cSprite_Manager :: cSprite_Manager( unsigned int reserve_items /* = 2000 */, unsigned int zpos_items /* = 100 */ )
: cObject_Manager<cSprite>()
{
	objects.reserve( reserve_items );

	for( unsigned int i = 0; i < zpos_items; i++ )
	{
		zposdata.push_back( 0.0f );
		zposdata_editor.push_back( 0.0f );
	}
}

cSprite_Manager :: ~cSprite_Manager( void )
{
	Delete_All();
}

void cSprite_Manager :: Add( cSprite *sprite )
{
	// empty object
	if( !sprite )
	{
		return;
	}

	Set_Z( sprite );

	// Check if an destroyed object can be replaced
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		// if destroy is set
		if( obj->destroy )
		{
			// set new object
			*itr = sprite;
			// delete old
			delete obj;

			return;
		}
	}

	cObject_Manager<cSprite>::Add( sprite );
}

cSprite *cSprite_Manager :: Copy( unsigned int identifier )
{
	if( identifier >= objects.size() )
	{
		return NULL;
	}

	return objects[identifier]->Copy();
}

void cSprite_Manager :: Delete_All( bool delayed /* = 0 */ )
{
	// clear z position data
	for( unsigned int i = 0; i < zposdata.size(); i++ )
	{
		zposdata[i] = 0.0f;
		zposdata_editor[i] = 0.0f;
	}

	// delayed
	if( delayed )
	{
		for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			obj->Destroy();
			obj->Collisions_Clear();
		}
	}
	// instant
	else
	{
		cObject_Manager<cSprite>::Delete_All();
	}

	// todo : this should be elsewhere...
	// clear collision data
	if( pPlayer )
	{
		pPlayer->Collisions_Clear();
		pPlayer->Reset_on_Ground();
	}
	if( pMouseCursor )
	{
		pMouseCursor->Collisions_Clear();
	}
}

cSprite *cSprite_Manager :: Get_First( SpriteType type )
{
	cSprite *first = NULL;

	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
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

cSprite *cSprite_Manager :: Get_Last( SpriteType type )
{
	cSprite *last = NULL;

	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
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

cSprite *cSprite_Manager :: Get_from_Position( int posx, int posy, SpriteType type /* = TYPE_UNDEFINED */ )
{
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *obj = (*itr);

		if( static_cast<int>(obj->startposx) != posx || static_cast<int>(obj->startposy) != posy )
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

void cSprite_Manager :: Get_Objects_sorted( SpriteList &new_objects, bool editor_sort /* = 0 */, bool with_player /* = 0 */ )
{
	new_objects = objects;

	if( with_player )
	{
		new_objects.push_back( pActive_Player );
	}

	// z position sort
	if( !editor_sort )
	{
		// default
		sort( new_objects.begin(), new_objects.end(), zpos_sort() );
	}
	else
	{
		// editor
		sort( new_objects.begin(), new_objects.end(), editor_zpos_sort() );
	}
}

void cSprite_Manager :: Update_Items_Valid_Draw( void )
{
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update_Valid_Draw();
	}
}

void cSprite_Manager :: Update_Items( void )
{
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update();
	}
}

void cSprite_Manager :: Draw_Items( void )
{
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cSprite_Manager :: Handle_Collision_Items( void )
{
	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// collision and movement handling
		(*itr)->Collide_Move();
		// handle found collisions
		(*itr)->Handle_Collisions();
	}
}

unsigned int cSprite_Manager :: Get_Size_Array( ArrayType sprite_array )
{
	unsigned int count = 0;

	for( SpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		if( (*itr)->sprite_array == sprite_array )
		{
			count++;
		}
	}

	return count;
}

void cSprite_Manager :: Set_Z( cSprite *sprite )
{
	// don't set animation z position
	if( sprite->sprite_array == ARRAY_ANIM )
	{
		return;
	}

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
