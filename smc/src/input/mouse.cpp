/***************************************************************************
 * mouse.cpp  -  mouse handling class
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

#include "../core/globals.h"
#include "../core/main.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../overworld/overworld.h"
#include "../overworld/world_player.h"
#include "../gui/menu.h"
#include "../core/camera.h"
#include "../level/level.h"
#include "../player/player.h"
#include "../user/preferences.h"
#include "../core/framerate.h"
#include "../objects/goldpiece.h"
#include "../objects/levelexit.h"
#include "../objects/bonusbox.h"
#include "../enemies/furball.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"

/* *** *** *** *** *** cSelectedObject *** *** *** *** *** *** *** *** *** *** *** *** */

cSelectedObject :: cSelectedObject( void )
{
	obj = NULL;

	mouse_h = 0;
	mouse_w = 0;

	user = 0;
}

cSelectedObject :: ~cSelectedObject( void )
{

}

/* *** *** *** *** *** cCopyObject *** *** *** *** *** *** *** *** *** *** *** *** */

cCopyObject :: cCopyObject( void )
{
	obj = NULL;
}

cCopyObject :: ~cCopyObject( void )
{
	if( obj )
	{
		delete obj;
	}
}

/* *** *** *** *** *** cMouseCursor *** *** *** *** *** *** *** *** *** *** *** *** */

cMouseCursor :: cMouseCursor( void )
: cMovingSprite()
{
	type = TYPE_MOUSECURSOR;

	clickcounter = 0;

	mode_selection = 0;
	selection_rect = GL_rect( 0, 0, 0, 0 );

	mouse_object = new cSelectedObject();
	active_object = NULL;

	mode_fastcopy = 0;
	mover_mode = 0;
	click_temp = NULL;

	Reset_Keys();

	SDL_GetMouseState( &x, &y );
	// scale to the virtual game size
	x = static_cast<int>( static_cast<float>(x) * global_downscalex );
	y = static_cast<int>( static_cast<float>(y) * global_downscaley );
}

cMouseCursor :: ~cMouseCursor( void )
{
	Clear_Copy_Objects();
	Clear_Selected_Objects();
	delete mouse_object;
}

void cMouseCursor :: Reset( bool clear_copy_buffer /* = 1 */ )
{
	// only clear copy buffer if requested
	if( clear_copy_buffer )
	{
		Clear_Copy_Objects();
	}
	Clear_Selected_Objects();
	Clear_Mouse_Object();
	Clear_Active_Object();

	// change to default cursor
	if( mover_mode )
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseArrow" );
	}

	mover_mode = 0;
	mode_selection = 0;
	mode_fastcopy = 0;

	Collisions_Clear();

	// show mouse
	if( editor_enabled )
	{
		visible = 1;
	}
}

void cMouseCursor :: Reset_Keys( void )
{
	left = 0;
	right = 0;
	middle = 0;
}

bool cMouseCursor :: Handle_Event( SDL_Event *ev )
{
	switch( ev->type )
	{
		case SDL_MOUSEMOTION:
		{
			pGuiSystem->injectMousePosition( static_cast<float>(ev->motion.x), static_cast<float>(ev->motion.y) );
			Update_Position();
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if( Handle_Mouse_Up( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if( Handle_Mouse_Down( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

bool cMouseCursor :: Handle_Mouse_Down( Uint8 button )
{
	switch( button )
	{
		// mouse buttons
		case SDL_BUTTON_LEFT:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::LeftButton ) )
			{
				return 1;
			}
			left = 1;
			break;
		}
		case SDL_BUTTON_MIDDLE:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::MiddleButton ) )
			{
				return 1;
			}
			middle = 1;
			break;
		}
		case SDL_BUTTON_RIGHT:
		{
			if( CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::RightButton ) )
			{
				return 1;
			}
			right = 1;
			break;
		}
		// mouse wheel
		case SDL_BUTTON_WHEELDOWN:
		{
			if( CEGUI::System::getSingleton().injectMouseWheelChange( -1 ) )
			{
				return 1;
			}
			break;
		}
		case SDL_BUTTON_WHEELUP:
		{
			if( CEGUI::System::getSingleton().injectMouseWheelChange( +1 ) )
			{
				return 1;
			}
			break;
		}
		default:
		{
			break;
		}
	}

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Mouse_Down( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Mouse_Down( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Mouse_Down( button ) )
		{
			return 1;
		}
	}

	return 0;
}

bool cMouseCursor :: Handle_Mouse_Up( Uint8 button )
{
	switch ( button )
	{
		case SDL_BUTTON_LEFT:
		{
			left = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::LeftButton ) )
			{
				return 1;
			}
		}
		break;
		case SDL_BUTTON_MIDDLE:
		{
			middle = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton ) )
			{
				return 1;
			}
		}
		break;
		case SDL_BUTTON_RIGHT:
		{
			right = 0;
			if( CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::RightButton ) )
			{
				return 1;
			}
		}
		break;
		default:
		{
			break;
		}
	}

	// handle button in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pActive_Level->Mouse_Up( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Mouse_Up( button ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Mouse_Up( button ) )
		{
			return 1;
		}
	}

	return 0;
}

bool cMouseCursor :: Collsion_Check( GL_rect *crect )
{
	if( !crect )
	{
		return 0;
	}

	if( x > crect->x + crect->w )
	{
		return 0;
	}
	if( x < crect->x )
	{
		return 0;
	}
	if( y > crect->y + crect->h )
	{
		return 0;
	}
	if( y < crect->y )
	{
		return 0;
	}

	return 1;
}

bool cMouseCursor :: Editor_Collsion_Check( float px /* = 0 */, float py /* = 0 */ )
{
	Collisions_Clear();

	if( mover_mode )
	{
		return 0;
	}

	// Get CEGUI Window containing the mouse
	CEGUI::Window *mouse_window = pGuiSystem->getWindowContainingMouse();

	// if mouse is over a blocking CEGUI window
	if( mouse_window && !mouse_window->isMousePassThroughEnabled() )
	{
		return 0;
	}

	// mouse rect
	GL_rect mouse_rect;

	// use given posx
	if( px != 0 )
	{
		mouse_rect.x = px;
	}
	else
	{
		mouse_rect.x = posx;
	}

	// use given posy
	if( py != 0 )
	{
		mouse_rect.y = py;
	}
	else
	{
		mouse_rect.y = posy;
	}

	mouse_rect.w = 1;
	mouse_rect.h = 1;

	if( Get_Collision( &mouse_rect ) )
	{
		return 1;
	}

	// no collisions
	return 0;
}

bool cMouseCursor :: Get_Collision( GL_rect *mouse_rect )
{
	SpriteList sprite_objects;
	pActive_Sprite_Manager->Get_Objects_sorted( sprite_objects, 1, 1 );

	// check objects
	for( SpriteList::reverse_iterator itr = sprite_objects.rbegin(), itr_end = sprite_objects.rend(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// don't check spawned objects
		if( obj->spawned )
		{
			continue;
		}

		if( Col_Box( mouse_rect, &obj->start_rect ) )
		{
			if( obj->type == TYPE_PLAYER )
			{
				Collision_Add( this, pActive_Player, 0, CO_PLAYER );
			}
			else
			{
				Collision_Add( this, obj, pActive_Sprite_Manager->Get_Array_Num( obj ), Get_Collision_Type( obj->sprite_array ) );
			}

			return 1;
		}
	}

	return 0;
}

void cMouseCursor :: Update( void )
{
	// only if editor is enabled
	if( !editor_enabled )
	{
		return;
	}

	Update_Doubleclick();
}

void cMouseCursor :: Draw( void )
{
	// only if editor is enabled
	if( !editor_enabled )
	{
		return;
	}

	Update_Selection();

	// if in Level or Overworld
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		// draw if not in mover mode
		if( !mover_mode )
		{
			Draw_Object_Rects();
		}
	}
}

void cMouseCursor :: Render( void )
{
	if( !visible )
	{
		return;
	}

	// Render CEGUI Mouse
	pGuiRenderer->setQueueingEnabled( 0 );
	CEGUI::MouseCursor *mouse = CEGUI::MouseCursor::getSingletonPtr();
	mouse->setVisible( 1 );
	mouse->draw();
	mouse->setVisible( 0 );
}

void cMouseCursor :: Update_Position( void )
{
	if( !mover_mode )
	{
		SDL_GetMouseState( &x, &y );
		// scale to the virtual game size
		x = static_cast<int>( static_cast<float>(x) * global_downscalex );
		y = static_cast<int>( static_cast<float>(y) * global_downscaley );
	}

	if( editor_enabled )
	{
		if( !mover_mode )
		{
			Set_Pos( x + pActive_Camera->x, y + pActive_Camera->y, 1 );
		}

		Update_Mouse_Object();
		Update_Selected_Objects();
	}
	else
	{
		Set_Pos( static_cast<float>(x), static_cast<float>(y), 1 );
	}
}

void cMouseCursor :: Update_Doubleclick( void )
{
	if( clickcounter )
	{
		clickcounter -= pFramerate->speedfactor;

		if( clickcounter < 0 )
		{
			clickcounter = 0;
		}
	}
}

void cMouseCursor :: Click( void )
{
	Editor_Collsion_Check();

	if( mover_mode )
	{
		return;
	}

	// no colliding sprite
	if( !mouse_object->obj )
	{
		cObjectCollision *col = Collision_Get_first();

		// collision with an object
		if( col )
		{
			cSprite *col_obj = NULL;

			// player
			if( col->type == CO_PLAYER )
			{
				col_obj = static_cast<cSprite *>(pActive_Player);
			}
			// object manager object
			else
			{
				col_obj = pActive_Sprite_Manager->Get_Pointer( col->number );
			}

			// if shift is pressed add object to the mouse objects
			if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
			{
				// add object
				if( !Add_Selected_Object( col_obj, 1 ) )
				{
					// if already added remove it
					Remove_Selected_Object( col_obj );
				}
			}
		}

		Start_Selection();
		clickcounter = 0;
		return;
	}

	if( clickcounter > 0 )
	{
		// if last clicked object was the same
		if( click_temp == mouse_object->obj )
		{
			Double_Click();
		}
		else
		{
			clickcounter = 0;
		}
	}
	else
	{
		// double click counter
		clickcounter = speedfactor_fps * 0.3f;
		// save last clicked object
		click_temp = mouse_object->obj;
	}
}

void cMouseCursor :: Double_Click( bool activate /* = 1 */ )
{
	Clear_Active_Object();

	// add new
	if( activate )
	{
		Set_Active_Object( mouse_object->obj );
	}

	clickcounter = 0;
}

void cMouseCursor :: Set_Mouse_Object( cSprite *sprite )
{
	// return if mouse object is the same or in mouse selection mode
	if( mouse_object->obj == sprite || ( sprite && ( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] || mode_selection ) ) )
	{
		return;
	}

	// remove old mouse_object from selected objects
	if( mouse_object->obj )
	{
		Remove_Selected_Object( mouse_object->obj, 1 );
	}

	// set new mouse object
	mouse_object->obj = sprite;
	// add new mouse object to selected objects
	Add_Selected_Object( sprite );
}

void cMouseCursor :: Update_Mouse_Object( void )
{
	if( ( !editor_enabled ) || !mouse_object->obj || ( mover_mode && ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) ) )
	{
		return;
	}

	// left mouse is pressed and no shift
	if( left )
	{
		// set new position
		mouse_object->obj->Set_Pos( static_cast<float>( static_cast<int>(posx) - mouse_object->mouse_w ), static_cast<float>( static_cast<int>(posy) - mouse_object->mouse_h ), 1 );

		// update object settings position
		if( active_object && active_object == mouse_object->obj )
		{
			active_object->Editor_Position_Update();
		}
	}
	// left mouse is not pressed
	else
	{
		// update mouse object positioning mouse width and height
		mouse_object->mouse_w = static_cast<int>(posx) - static_cast<int>(mouse_object->obj->startposx);
		mouse_object->mouse_h = static_cast<int>(posy) - static_cast<int>(mouse_object->obj->startposy);
	}
}

void cMouseCursor :: Clear_Mouse_Object( void )
{
	Set_Mouse_Object( NULL );
}

void cMouseCursor :: Add_Copy_Object( cSprite *sprite )
{
	if( !sprite )
	{
		return;
	}

	// check if not already added
	for( CopyObjectList::iterator itr = copy_objects.begin(), itr_end = copy_objects.end(); itr != itr_end; ++itr )
	{
		if( (*itr)->obj == sprite )
		{
			return;
		}
	}

	cSprite *copy = sprite->Copy();

	// can't be copied
	if( !copy )
	{
		return;
	}

	// insert object
	cCopyObject *copy_object = new cCopyObject();
	copy_object->obj = copy;
	copy_objects.push_back( copy_object );
}

void cMouseCursor :: Add_Copy_Objects( SpriteList spritelist )
{
	if( !spritelist.size() )
	{
		return;
	}

	// inserts all objects
	for( SpriteList::iterator itr = spritelist.begin(), itr_end = spritelist.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		Add_Copy_Object( obj );
	}
}

bool cMouseCursor :: Remove_Copy_Object( cSprite *sprite )
{
	if( !sprite )
	{
		return 0;
	}

	for( CopyObjectList::iterator itr = copy_objects.begin(), itr_end = copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		if( copy_obj->obj == sprite )
		{
			copy_objects.erase( itr );
			delete copy_obj;
			
			return 1;
		}
	}

	return 0;
}

void cMouseCursor :: Clear_Copy_Objects( void )
{
	for( CopyObjectList::iterator itr = copy_objects.begin(), itr_end = copy_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	copy_objects.clear();
}

GL_Vector cMouseCursor :: Get_Copy_Object_Base( float px, float py )
{
	GL_Vector vec = GL_Vector();

	if( !copy_objects.size() )
	{
		return vec;
	}

	// set a base
	float base_x = copy_objects[0]->obj->posx;
	float base_y = copy_objects[0]->obj->posy;

	for( CopyObjectList::iterator itr = copy_objects.begin(), itr_end = copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		// if more in top or left
		if( copy_obj->obj->posx < base_x || copy_obj->obj->posy < base_y )
		{
			base_x = copy_obj->obj->posx;
			base_y = copy_obj->obj->posy;
		}
	}

	vec.x = px - base_x;
	vec.y = py - base_y;

	return vec;
}

void cMouseCursor :: Paste_Copy_Objects( float px, float py )
{
	if( !copy_objects.size() )
	{
		return;
	}

	SpriteList new_objects;

	for( CopyObjectList::iterator itr = copy_objects.begin(), itr_end = copy_objects.end(); itr != itr_end; ++itr )
	{
		cCopyObject *copy_obj = (*itr);

		// set base vector
		GL_Vector base_vec = Get_Copy_Object_Base( copy_obj->obj->posx, copy_obj->obj->posy );

		// copy
		cSprite* new_object = Copy( copy_obj->obj, px + base_vec.x, py + base_vec.y );

		if( new_object )
		{
			new_objects.push_back( new_object );
		}
	}

	if( copy_objects.size() )
	{
		mouse_object->mouse_h = static_cast<int>( copy_objects[0]->obj->col_rect.h / 2 );
		mouse_object->mouse_w = static_cast<int>( copy_objects[0]->obj->col_rect.w / 2 );
	}

	// pasted objects are selected
	Clear_Selected_Objects();
	Add_Selected_Objects( new_objects, 1 );
}

bool cMouseCursor :: Add_Selected_Object( cSprite *sprite, bool from_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	// check if not already added
	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// overwrite user if given
			if( from_user && !sel_obj->user )
			{
				sel_obj->user = 1;
				return 1;
			}

			return 0;
		}
	}

	// insert object
	cSelectedObject *selected_object = new cSelectedObject();
	selected_object->obj = sprite;
	selected_object->user = from_user;
	selected_objects.push_back( selected_object );

	return 1;
}

void cMouseCursor :: Add_Selected_Objects( SpriteList spritelist, bool from_user /* = 0 */ )
{
	if( !spritelist.size() )
	{
		return;
	}

	// inserts all objects
	for( SpriteList::iterator itr = spritelist.begin(), itr_end = spritelist.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		Add_Selected_Object( obj, from_user );
	}
}

bool cMouseCursor :: Remove_Selected_Object( cSprite *sprite, bool no_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// don't delete user added selected object
			if( no_user && sel_obj->user )
			{
				return 0;
			}

			selected_objects.erase( itr );
			delete sel_obj;
			
			return 1;
		}
	}

	return 0;
}

SpriteList cMouseCursor :: Get_Selected_Objects( void )
{
	SpriteList spritelist;

	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *object = (*itr);

		spritelist.push_back( object->obj );
	}

	return spritelist;
}

void cMouseCursor :: Clear_Selected_Objects( void )
{
	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	selected_objects.clear();
}

void cMouseCursor :: Update_Selected_Objects( void )
{
	if( !editor_enabled || ( mover_mode && ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) ) )
	{
		return;
	}

	// update selected objects positioning mouse width and height
	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( !left || mode_selection )
		{
			sel_obj->mouse_w = static_cast<int>(posx) - static_cast<int>(sel_obj->obj->startposx);
			sel_obj->mouse_h = static_cast<int>(posy) - static_cast<int>(sel_obj->obj->startposy);
		}
	}

	// if a user selected object is not active
	if( !Is_Selected_Object( mouse_object->obj, 1 ) )
	{
		return;
	}

	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		// left mouse is pressed
		if( left )
		{
			// set new position
			sel_obj->obj->Set_Pos( static_cast<float>(static_cast<int>(posx) - sel_obj->mouse_w), static_cast<float>(static_cast<int>(posy) - sel_obj->mouse_h), 1 );

			// update object settings position
			if( active_object && active_object == sel_obj->obj )
			{
				active_object->Editor_Position_Update();
			}
		}
	}
}

bool cMouseCursor :: Is_Selected_Object( cSprite *sprite, bool only_user /* = 0 */ )
{
	if( !sprite )
	{
		return 0;
	}

	for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
	{
		cSelectedObject *sel_obj = (*itr);

		if( sel_obj->obj == sprite )
		{
			// if only user objects
			if( only_user && !sel_obj->user )
			{
				continue;
			}

			// found
			return 1;
		}
	}

	return 0;
}

void cMouseCursor :: Delete_Selected_Objects( void )
{
	for( int i = selected_objects.size() - 1; i >= 0; i-- )
	{
		Delete( selected_objects[i]->obj );
	}

	Clear_Selected_Objects();
}

unsigned int cMouseCursor :: Get_Selected_Object_Size( void )
{
	return selected_objects.size();
}

void cMouseCursor :: Set_Active_Object( cSprite *sprite )
{
	// clear existing
	Clear_Active_Object();

	if( sprite )
	{
		active_object = sprite;
		active_object->Editor_Activate();
	}
}

void cMouseCursor :: Clear_Active_Object( void )
{
	if( active_object )
	{
		active_object->Editor_Deactivate();
		active_object = NULL;
	}
}

cSprite *cMouseCursor :: Copy( cSprite *ncopy_object, float px, float py )
{
	if( !ncopy_object )
	{
		return NULL;
	}

	if( !editor_enabled )
	{
		printf( "Warning : No editor enabled for copy object: %s\n", ncopy_object->name.c_str() );
		return NULL;
	}

	cSprite *new_sprite = ncopy_object->Copy();

	if( !new_sprite )
	{
		printf( "Warning : Mouse object copying failed\n" );
		return NULL;
	}

	new_sprite->Set_Pos( px, py, 1 );

	if( new_sprite->sprite_array == ARRAY_MASSIVE || new_sprite->sprite_array == ARRAY_PASSIVE || new_sprite->sprite_array == ARRAY_FRONT_PASSIVE || new_sprite->sprite_array == ARRAY_ACTIVE || new_sprite->sprite_array == ARRAY_ENEMY || new_sprite->sprite_array == ARRAY_ANIM )
	{
		pActive_Sprite_Manager->Add( new_sprite );
	}
	else
	{
		printf( "Warning : cMouseCursor :: Copy object unknown array : %d\n", new_sprite->sprite_array );
		delete new_sprite;
		return NULL;
	}

	return new_sprite;
}

void cMouseCursor :: Delete( cSprite *sprite )
{
	if( !sprite )
	{
		return;
	}

	if( sprite->sprite_array == ARRAY_PASSIVE || sprite->sprite_array == ARRAY_FRONT_PASSIVE || sprite->sprite_array == ARRAY_MASSIVE || sprite->sprite_array == ARRAY_ACTIVE || sprite->sprite_array == ARRAY_ENEMY || sprite->sprite_array == ARRAY_ANIM )
	{
		// deactivate active object
		if( active_object == sprite )
		{
			Double_Click( 0 );
		}

		// remove if current player active item object
		if( pPlayer->active_object == sprite )
		{
			pPlayer->active_object = NULL;
		}

		// remove selected object
		Remove_Selected_Object( sprite );
		// remove copy object
		Remove_Copy_Object( sprite );

		// delete object
		if( editor_enabled )
		{
			// if overworld line point
			if( sprite->type == TYPE_OW_LINE_START || sprite->type == TYPE_OW_LINE_END )
			{
				cLayer_Line *line = static_cast<cLayer_Line *>(static_cast<cLayer_Line_Point *>(sprite)->line);

				// delete line
				pActive_Overworld->pLayer->Delete( line );
			}
			else
			{
				sprite->Destroy();
			}
		}

		sprite = NULL;
	}
}

void cMouseCursor :: Draw_Object_Rects( void )
{
	// draw mouse object
	GL_rect hover_rect;

	if( !left && mouse_object->obj )
	{
		hover_rect.x = mouse_object->obj->startposx - pActive_Camera->x;
		hover_rect.y = mouse_object->obj->startposy - pActive_Camera->y;
		hover_rect.w = mouse_object->obj->start_rect.w;
		hover_rect.h = mouse_object->obj->start_rect.h;

		// create request
		cRectRequest *request = new cRectRequest();

		if( mode_fastcopy )
		{
			pVideo->Draw_Rect( &hover_rect, 0.6f, &green, request );
		}
		else
		{
			pVideo->Draw_Rect( &hover_rect, 0.6f, &blue, request );
		}

		// not filled
		request->filled = 0;

		// add request
		pRenderer->Add( request );
	}


	// draw selected objects if not left mouse is pressed, shift selection or mouse selection
	if( !left || ( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) || mode_selection )
	{
		Color obj_color = white;

		for( SelectedObjectList::iterator itr = selected_objects.begin(), itr_end = selected_objects.end(); itr != itr_end; ++itr )
		{
			cSprite *object = (*itr)->obj;

			// check if visible on screen
			if( !object->Is_Visible_on_Screen() )
			{
				continue;
			}

			hover_rect.x = object->startposx - pActive_Camera->x;
			hover_rect.y = object->startposy - pActive_Camera->y;
			hover_rect.w = object->start_rect.w;
			hover_rect.h = object->start_rect.h;

			// object color
			obj_color = Get_Massive_Type_Color( object->massivetype );

			// create request
			cRectRequest *rect_request = new cRectRequest();
			pVideo->Draw_Rect( &hover_rect, 0.5f, &obj_color, rect_request );

			// not filled
			rect_request->filled = 0;

			// add request
			pRenderer->Add( rect_request );
		}
	}
}

void cMouseCursor :: Start_Selection( void )
{
	Clear_Mouse_Object();
	mode_selection = 1;
	selection_rect.x = x + pActive_Camera->x;
	selection_rect.y = y + pActive_Camera->y;
}

void cMouseCursor :: End_Selection( void )
{
	mode_selection = 0;

	Update_Position();
}

void cMouseCursor :: Update_Selection( void )
{
	if( !mode_selection )
	{
		return;
	}

	selection_rect.w = x - selection_rect.x + pActive_Camera->x;
	selection_rect.h = y - selection_rect.y + pActive_Camera->y;

	// create collision checking rect
	GL_rect rect = GL_rect( selection_rect.x, selection_rect.y, 0, 0 );

	// negative to positive
	if( selection_rect.w < 0 )
	{
		rect.x = selection_rect.x + selection_rect.w;
		rect.w = -selection_rect.w;
	}
	// positive
	else
	{
		rect.w = selection_rect.w;
	}

	// negative to positive
	if( selection_rect.h < 0 )
	{
		rect.y = selection_rect.y + selection_rect.h;
		rect.h = -selection_rect.h;
	}
	// positive
	else
	{
		rect.h = selection_rect.h;
	}

	// if rect is below minimum
	if( rect.w < 2 && rect.h < 2 )
	{
		return;
	}

	// only clear if not shift is pressed
	if( !( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) )
	{
		Clear_Selected_Objects();
	}

	// add selected objects
	for( SpriteList::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *object = (*itr);

		if( !object->spawned && Col_Box( &rect, &object->rect ) )
		{
			Add_Selected_Object( object, 1 );
		}
	}

	if( Col_Box( &rect, &pActive_Player->rect ) )
	{
		Add_Selected_Object( pActive_Player, 1 );
	}

	// ## inner rect
	cRectRequest *rect_request = new cRectRequest();
	pVideo->Draw_Rect( selection_rect.x - pActive_Camera->x + 0.5f, selection_rect.y - pActive_Camera->y + 0.5f, selection_rect.w, selection_rect.h, 0.509f, &white, rect_request );

	// not filled
	rect_request->filled = 0;
	// color
	rect_request->color.alpha = 128;

	// add request
	pRenderer->Add( rect_request );

	// ## outer rect
	// create request
	rect_request = new cRectRequest();
	pVideo->Draw_Rect( selection_rect.x - pActive_Camera->x, selection_rect.y - pActive_Camera->y, selection_rect.w, selection_rect.h, 0.51f, &lightblue, rect_request );

	// not filled
	rect_request->filled = 0;

	// add request
	pRenderer->Add( rect_request );
}

void cMouseCursor :: Toggle_Mover_Mode( void )
{
	mover_mode = !mover_mode;

	if( mover_mode )
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseMoveCursor" );
	}
	else
	{
		CEGUI::MouseCursor::getSingleton().setImage( "TaharezLook", "MouseArrow" );
	}
}

void cMouseCursor :: Mover_Update( Sint16 move_x, Sint16 move_y )
{
	if( !mover_mode )
	{
		return;
	}

	// mouse moves the camera
	pActive_Camera->Move( move_x, move_y );
	// keep mouse at it's position
	SDL_WarpMouse( static_cast<Uint16>(x * global_upscalex), static_cast<Uint16>(y * global_upscaley) );

	SDL_Event inEvent;
	SDL_PeepEvents( &inEvent, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK );

	while( SDL_PollEvent( &inEvent ) )
	{
		switch( inEvent.type )
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				if( inEvent.button.button == 2 )
				{
					mover_mode = 0;
				}
				break;
			}
			case SDL_KEYDOWN:
			{
				mover_mode = 0;
				pKeyboard->Key_Down( inEvent.key.keysym.sym );
				break;
			}
			case SDL_QUIT:
			{
				game_exit = 1;
				break;
			}
		}
	}
}

void cMouseCursor :: Editor_Update( void )
{
	if( !editor_enabled )
	{
		return;
	}

	bool is_col = Editor_Collsion_Check();
	cObjectCollision *col = Collision_Get_first();

	// valid object
	if( is_col && col )
	{
		// get colliding object
		cSprite *obj = NULL;

		// player
		if( col->type == CO_PLAYER )
		{
			obj = static_cast<cSprite *>(pActive_Player);
		}
		// object manager object
		else
		{
			obj = pActive_Sprite_Manager->Get_Pointer( col->number );
		}

		// mouse object type string
		string type_name;

		// set object data
		if( obj )
		{
			type_name = obj->name;

			if( !left || !mouse_object->obj )
			{
				Set_Mouse_Object( obj );
			}
		}

		// ## Set Massivetype Text
		if( col->type == CO_MASSIVE )
		{
			if( obj->massivetype == MASS_HALFMASSIVE )
			{
				type_name.insert( 0, _("Halfmassive - ") );
			}
			else
			{
				type_name.insert( 0, _("Massive - ") );
			}
		}
		else if( col->type == CO_PASSIVE )
		{
			if( obj->type == TYPE_OW_LINE_START || obj->type == TYPE_OW_LINE_END || obj->type == TYPE_OW_WAYPOINT )
			{
				// ignore
			}
			else if( obj->sprite_array == ARRAY_PASSIVE )
			{
				type_name.insert( 0, _("Passive - ") );
			}
			else
			{
				type_name.insert( 0, _("Front Passive - ") );
			}
		}
		else if( col->type == CO_ACTIVE )
		{
			if( obj->type == TYPE_HALFMASSIVE )
			{
				type_name.insert( 0, _("Halfmassive - ") );
			}
			else if( obj->type == TYPE_CLIMBABLE )
			{
				type_name.insert( 0, _("Climbable - ") );
			}
		}


		// if valid object draw position text
		if( col->type != CO_NOTHING && mouse_object->obj )
		{
			// position text
			string info = "X : " + int_to_string( static_cast<int>(mouse_object->obj->startposx) ) + "  Y : " + int_to_string( static_cast<int>(mouse_object->obj->startposy) );

			// if in debug mode draw editor Z position
			if( game_debug )
			{
				info.insert( info.length(), "  Z : " + float_to_string( mouse_object->obj->posz, 6 ) );

				// if also got editor z position
				if( obj->editor_posz != 0 )
				{
					info.insert( info.length(), _("  Editor Z : ") + float_to_string( mouse_object->obj->editor_posz, 6 ) );
				}
			}

			GL_Surface *position_info = pFont->Render_Text( pFont->font_small, info, white );

			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			position_info->Blit( static_cast<float>( x + 20 ), static_cast<float>( y + 35 ), 0.52f, request );
			request->delete_texture = 1;

			// shadow
			request->shadow_pos = 1;
			request->shadow_color = black;

			// add request
			pRenderer->Add( request );

			position_info->auto_del_img = 0;
			delete position_info;
		}

		if( debugdisplay->counter <= 0 )
		{
			debugdisplay->Set_Text( type_name, 1 );
		}
	}
	else
	{
		if( !left )
		{
			Clear_Mouse_Object();

			if( !col )
			{
				mouse_object->mouse_w = 0;
				mouse_object->mouse_h = 0;
			}
		}
		else
		{
			// no mouse object
			if( !mouse_object->obj )
			{
				// clear active object
				Double_Click( 0 );

				// clear selected objects if no shift is pressed
				if( !( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] ) )
				{
					Clear_Selected_Objects();
				}
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMouseCursor *pMouseCursor = NULL;
