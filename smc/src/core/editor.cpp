/***************************************************************************
				editor.cpp  -  class for the basic editor
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../core/editor.h"
#include "../core/game_core.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../audio/audio.h"
#include "../video/font.h"
#include "../video/animation.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../level/level.h"
#include "../player/player.h"
#include "../overworld/worlds.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"
// boost filesystem
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

/* *** *** *** *** *** *** *** *** cEditor_Item_Object *** *** *** *** *** *** *** *** *** */

cEditor_Item_Object :: cEditor_Item_Object( string text )
: ListboxItem( "" )
{
	list_text = new ListboxTextItem( text.c_str() );

	sprite_obj = NULL;
	preview_scale = 1;
}

cEditor_Item_Object :: ~cEditor_Item_Object( void )
{
	delete list_text;

	if( sprite_obj )
	{
		delete sprite_obj;
	}
}

void cEditor_Item_Object :: Init( void )
{
	// CEGUI settings
	list_text->setTextColours( Get_Massivetype_Color( sprite_obj->massivetype ).Get_cegui_colour() );
	list_text->setSelectionColours( colour( 0.33f, 0.33f, 0.33f ) );
	list_text->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );

	// image dimension text
	// string size_text = int_to_string( (int)image->w ) + "x" + int_to_string( (int)image->h );
	preview_scale = pVideo->Get_scale( sprite_obj->image, 50, 50 );
}

// Return the rendered pixel size of this list box item.
Size cEditor_Item_Object :: getPixelSize( void ) const
{
	Size tmp = list_text->getPixelSize();
	tmp.d_height += 60;

	return tmp;
}

void cEditor_Item_Object :: draw( const Vector3 &position, float alpha, const Rect &clipper ) const
{
	// draw text
	list_text->draw( position, alpha, clipper );
	// update image position
	sprite_obj->Set_Pos( position.d_x + 20, position.d_y + 50 );
}

void cEditor_Item_Object :: draw( RenderCache &cache, const Rect &targetRect, float zBase, float alpha, const Rect *clipper ) const
{
	// draw text
	list_text->draw( cache, targetRect, zBase, alpha, clipper );
	// update image position
	sprite_obj->Set_Pos( targetRect.d_left + 20, targetRect.d_top + 50 );
}

void cEditor_Item_Object :: Draw_Image( void )
{
	// no image available to blit
	if( !sprite_obj->start_image )
	{
		return;
	}

	const Listbox *owner = static_cast<const Listbox *>( getOwnerWindow() );

	// if item is not visible
	if( !owner->isVisible() )
	{
		return;
	}

	// create request
	cSurfaceRequest *request = new cSurfaceRequest();
	sprite_obj->start_image->Blit( sprite_obj->posx, sprite_obj->posy, 0.9f, request );

	request->scale_x = preview_scale;
	request->scale_y = preview_scale;

	request->rotx += sprite_obj->start_rotx;
	request->roty += sprite_obj->start_roty;
	request->rotz += sprite_obj->start_rotz;

	// add request
	pRenderer_GUI->Add( request );
}

/* *** *** *** *** *** *** *** *** cEditor_Menu_Object *** *** *** *** *** *** *** *** *** */

cEditor_Menu_Object :: cEditor_Menu_Object( string text )
: ListboxTextItem( text.c_str() )
{
	bfunction = 0;
	header = 0;
}

cEditor_Menu_Object :: ~cEditor_Menu_Object( void )
{

}

void cEditor_Menu_Object :: Init( void )
{
	setSelectionColours( colour( 0.33f, 0.33f, 0.33f ) );
	setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
}

/* *** *** *** *** *** *** *** cEditor *** *** *** *** *** *** *** *** *** *** */

cEditor :: cEditor( void )
{
	enabled = 0;

	camera_speed = 35;
	menu_timer = 0;
    show_editor_help = 0;
	editor_window = NULL;
}

cEditor :: ~cEditor( void )
{
	Unload();
}

void cEditor :: Init( void )
{
	// already loaded
	if( editor_window )
	{
		return;
	}

	// Create Editor CEGUI Window
	editor_window = WindowManager::getSingleton().loadWindowLayout( "editor.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( editor_window );

	// Get TabControl
	TabControl *tabcontrol_menu = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_editor" ));
	// TabControl Menu Tab Events
	tabcontrol_menu->getTabContents( "editor_tab_menu" )->subscribeEvent( Window::EventMouseEnters, Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );
	// TabControl Items Tab Events
	tabcontrol_menu->getTabContents( "editor_tab_items" )->subscribeEvent( Window::EventMouseEnters, Event::Subscriber( &cEditor::Editor_Mouse_Enter, this ) );

	// Get Menu Listbox
	Listbox *listbox_menu = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_menu" ));
	// Menu Listbox events
	listbox_menu->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cEditor::Menu_Select, this ) );
	// Get Items Listbox
	Listbox *listbox_items = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));
	// Items Listbox events
	listbox_items->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cEditor::Item_Select, this ) );

	// Get Items
	if( !valid_file( items_filename ) )
	{
		printf( "Error : Editor Loading : No Item file found : %s\n", items_filename.c_str() );
		return;
	}
	// Parse Items
	System::getSingleton().getXMLParser()->parseXMLFile( *this, items_filename.c_str(), SCHEMA_DIR "/Editor_Items.xsd", "" );

	// Get all image items
	Load_Image_Items( PIXMAPS_DIR );

	// Get Menu
	if( !valid_file( menu_filename ) )
	{
		printf( "Error : Editor Loading : No Menu file found : %s\n", menu_filename.c_str() );
		return;
	}
	// Parse Menu
	System::getSingleton().getXMLParser()->parseXMLFile( *this, menu_filename.c_str(), SCHEMA_DIR "/Editor_Menu.xsd", "" );
}

void cEditor :: Unload( void )
{
	// Unload Items
	Unload_Item_Menu();

	// if editor window is loaded
	if( editor_window )
	{
		pGuiSystem->getGUISheet()->removeChildWindow( editor_window );
		WindowManager::getSingleton().destroyWindow( editor_window );
		editor_window = NULL;
	}

	// Tagged Items
	for( TaggedItemObjectsList::iterator itr = tagged_item_objects.begin(), itr_end = tagged_item_objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}
	tagged_item_objects.clear();
	// Tagged Image Settings
	for( TaggedItemImageSettingsList::iterator itr = tagged_item_images.begin(), itr_end = tagged_item_images.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}
	tagged_item_images.clear();


	// Help Sprites
	for( HudSpriteList::iterator itr = help_sprites.begin(), itr_end = help_sprites.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	help_sprites.clear();
}

void cEditor :: Toggle( void )
{
	// enable
	if( !enabled )
	{
		Enable();
	}
	// disable
	else if( enabled )
	{
		Disable();
	}
}

void cEditor :: Enable( void )
{
	// already enabled
	if( enabled )
	{
		return;
	}

	// Draw Loading Text
	Draw_StaticText( "Loading", &orange, &lightgreyalpha64, &grey, &black, 0 );

	// Basic Initialize
	Init();

	pAudio->PlaySound( "editor/enter.ogg" );
	debugdisplay->Set_Text( "Editor enabled" );
	pAnimationManager->Delete_All();

	pMouseCursor->visible = 1;

	// object manager
	cObjectManager *obj_manager = NULL;

	if( Game_Mode == MODE_LEVEL )
	{
		obj_manager = pObjManager;

		// player
		pPlayer->Update_Position_Rect();
	}
	else
	{
		obj_manager = pActive_Overworld->object_manager;

		// player
		pOverworld_Player->Update_Position_Rect();
	}

	for( SpriteList::iterator itr = obj_manager->items.begin(), itr_end = obj_manager->items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update_Position_Rect();
	}

	pCamera->Update_Position();
	enabled = 1;
}

void cEditor :: Disable( void )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	Unload();

	pAudio->PlaySound( "editor/leave.ogg" );
	debugdisplay->Set_Text( "Editor disabled" );

	// object manager
	cObjectManager *obj_manager = NULL;

	if( Game_Mode == MODE_LEVEL )
	{
		obj_manager = pObjManager;

		// player
		pPlayer->Update_Position_Rect();
	}
	else
	{
		obj_manager = pActive_Overworld->object_manager;

		// player
		pOverworld_Player->Update_Position_Rect();
	}

	for( SpriteList::iterator itr = obj_manager->items.begin(), itr_end = obj_manager->items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update_Position_Rect();
	}

	pCamera->Center();
	pMouseCursor->Reset( 0 );
	pMouseCursor->Set_Visible( 0 );

	// disable help screen
	show_editor_help = 0;

	enabled = 0;

	// if level should be saved
	if( Box_Question( "Save Level ?" ) )
	{
		Function_Save();
	}
}

void cEditor :: Update( void )
{
	if( !enabled )
	{
		return;
	}


	// if timed out
	if( menu_timer >= DESIRED_FPS * 2 )
	{
		// fade out
		if( editor_window->getAlpha() > 0 )
		{
			float new_alpha = editor_window->getAlpha() - pFramerate->speedfactor * 0.05f;

			if( new_alpha <= 0 )
			{
				new_alpha = 1;
				// hide editor window
				editor_window->setXPosition( UDim( -0.19f, 0 ) );
				// Hide Listbox
				Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_menu" ));
				listbox->hide();
				listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));
				listbox->hide();
				menu_timer = 0;
			}

			editor_window->setAlpha( new_alpha );
		}
	}
	// active
	else
	{
		// fade in
		if( editor_window->getAlpha() < 1 )
		{
			float new_alpha = editor_window->getAlpha() + pFramerate->speedfactor * 0.1f;

			if( new_alpha > 1 )
			{
				new_alpha = 1;
			}

			editor_window->setAlpha( new_alpha );
		}
		// inactive counter
		else if( editor_window->getXPosition().asRelative( 1 ) == 0 )
		{
			// if mouse is over the window
			if( MouseCursor::getSingletonPtr()->getDisplayIndependantPosition().d_x < editor_window->getXPosition().asRelative( 1 ) + 0.2f )
			{
				menu_timer = 0;
			}
			// inactive
			else
			{
				menu_timer += pFramerate->speedfactor;
			}
		}
	}

	pMouseCursor->Editor_Update();
}

void cEditor :: Draw( void )
{
	if( !enabled )
	{
		return;
	}

	Color color = (Uint8)0;

	// Camera limit bottom line
	if( pCamera->y > pCamera->limit_rect.y && pCamera->y + pCamera->limit_rect.y < GAME_RES_H )
	{
		int start_x = 0;

		if( pCamera->x < 0 )
		{
			start_x = -(int)pCamera->x;
		}

		Color color = Color( (Uint8)0, 0, 100, 192 );
		pVideo->DrawLine( (float)start_x, (float)(GAME_RES_H - (int)pCamera->y), (float)GAME_RES_W, (float)( GAME_RES_H - (int)pCamera->y ), 0.124f, &color );
	}
	// Camera limit top line
	if( pCamera->y + GAME_RES_H > pCamera->limit_rect.y + pCamera->limit_rect.h && pCamera->y < pCamera->limit_rect.y + pCamera->limit_rect.h )
	{
		int start_x = 0;

		if( pCamera->x < pCamera->limit_rect.x )
		{
			start_x = -(int)pCamera->x;
		}

		Color color = Color( (Uint8)20, 20, 150, 192 );
		pVideo->DrawLine( (float)start_x, ( pCamera->limit_rect.y + pCamera->limit_rect.h ) - pCamera->y, (float)GAME_RES_W, ( pCamera->limit_rect.y + pCamera->limit_rect.h ) - pCamera->y, 0.124f, &color );
	}

	// Camera limit left line
	if( pCamera->x < 0 && pCamera->x > -GAME_RES_W )
	{
		int start_y = GAME_RES_H;

		if( pCamera->y < GAME_RES_H )
		{
			start_y = GAME_RES_H - (int)pCamera->y;
		}

		Color color = Color( (Uint8)0, 100, 0, 192 );
		pVideo->DrawLine( (float)( 0 - pCamera->x ), (float)start_y, (float)( -pCamera->x ), 0, 0.124f, &color );
	}
	// Camera limit right line
	if( pCamera->x < pCamera->limit_rect.x + pCamera->limit_rect.w && pCamera->x > -GAME_RES_W + pCamera->limit_rect.x + pCamera->limit_rect.w )
	{
		int start_y = GAME_RES_H;

		if( pCamera->y < GAME_RES_H )
		{
			start_y = GAME_RES_H - (int)pCamera->y;
		}

		Color color = Color( (Uint8)20, 150, 20, 192 );
		pVideo->DrawLine( (float)( pCamera->limit_rect.x + pCamera->limit_rect.w - pCamera->x ), (float)start_y, (float)( pCamera->limit_rect.x + pCamera->limit_rect.w - pCamera->x ), 0, 0.124f, &color );
	}

	// Get Items Listbox
	Listbox *listbox_items = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));

	// Vertical ScrollBar Position
	float scroll_pos = listbox_items->getVertScrollbar()->getScrollPosition();

	// if editor window is active
	if( editor_window->getXPosition().asRelative( 1 ) >= 0 )
	{
		// draw the visible items
		for( unsigned int i = 0; i < listbox_items->getItemCount(); i++ )
		{
			cEditor_Item_Object *item = static_cast<cEditor_Item_Object *>( listbox_items->getListboxItemFromIndex( i ) );

			float item_pos = item->getPixelSize().d_height * ( i + 1 );

			// not visible
			if( item_pos > listbox_items->getListRenderArea().getHeight() + scroll_pos || item_pos - scroll_pos < 50 )
			{
				continue;
			}

			item->Draw_Image();
		}
	}

	if( show_editor_help )
	{
	    Draw_Editor_Help();
	}
}

void cEditor :: Process_Input( void )
{
	if( !enabled )
	{
		return;
	}

	// Drag Delete
	if( ( pKeyboard->keys[SDLK_RCTRL] || pKeyboard->keys[SDLK_LCTRL] ) && pMouseCursor->right && pMouseCursor->Editor_Collsion_Check() )
	{
		pMouseCursor->Delete( static_cast<cMovingSprite *>(pMouseCursor->mouse_object->obj) );
	}

	// Camera Movement
	if( pKeyboard->keys[SDLK_RIGHT] || pJoystick->right )
	{
	    if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
	        pCamera->Move( camera_speed * pFramerate->speedfactor * 3, 0 );
	    }
	    else
		{
            pCamera->Move( camera_speed * pFramerate->speedfactor, 0 );
	    }
	}
	else if( pKeyboard->keys[SDLK_LEFT] || pJoystick->left )
	{
	    if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
	        pCamera->Move( -( camera_speed * pFramerate->speedfactor * 3 ), 0 );
	    }
	    else
		{
            pCamera->Move( -( camera_speed * pFramerate->speedfactor ), 0 );
	    }
	}
	if( pKeyboard->keys[SDLK_UP] || pJoystick->up )
	{
	    if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
	        pCamera->Move( 0, -( camera_speed * pFramerate->speedfactor * 3 ) );
	    }
	    else
		{
            pCamera->Move( 0, -( camera_speed * pFramerate->speedfactor ) );
	    }
	}
	else if( pKeyboard->keys[SDLK_DOWN] || pJoystick->down )
	{
	    if( pKeyboard->keys[SDLK_RSHIFT] || pKeyboard->keys[SDLK_LSHIFT] )
		{
	        pCamera->Move( 0, camera_speed * pFramerate->speedfactor * 3 );
	    }
	    else
		{
            pCamera->Move( 0, camera_speed * pFramerate->speedfactor );
	    }
	}
}

bool cEditor :: Handle_Event( SDL_Event *ev )
{
	if( !enabled )
	{
		return 0;
	}

	switch( ev->type )
	{
	case SDL_MOUSEMOTION:
	{
		if( pMouseCursor->mover_mode )
		{
			pMouseCursor->Mover_Update( input_event.motion.xrel, input_event.motion.yrel );
		}

		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		// left
		if( ev->button.button == 1 )
		{
			pMouseCursor->Click();
		}
		// middle
		else if( ev->button.button == 2 )
		{
			// Activate fast copy mode
			if( pMouseCursor->mouse_object->obj )
			{
				pMouseCursor->mode_fastcopy = 1;
				return 1;
			}
			// Mover mode
			else
			{
				pMouseCursor->Toggle_Mover_mode();
				return 1;
			}
		}
		// right
		else if( ev->button.button == 3 )
		{
			if( !pMouseCursor->left )
			{
				pMouseCursor->Delete( static_cast<cMovingSprite *>(pMouseCursor->mouse_object->obj) );
				return 1;
			}
		}

		break;
	}
	case SDL_MOUSEBUTTONUP:
	{
		// left
		if( ev->button.button == 1 )
		{
			pMouseCursor->End_selection();
		}
		// middle
		else if( ev->button.button == 2 )
		{
			pMouseCursor->mode_fastcopy = 0;
		}
		break;
	}
	default: // other events
	{
		break;
	}
	}

	return 0;
}

bool cEditor :: Key_Down( SDLKey key )
{
	if( !enabled )
	{
		return 0;
	}

	if( key == SDLK_F1 )
	{
		show_editor_help = !show_editor_help;
	}
	// focus level start
	else if( key == SDLK_HOME )
	{
		pCamera->Set_Pos( 0, 0 );
	}
	// move the screen size to the right
	else if( key == SDLK_n )
	{
		pCamera->Move( GAME_RES_W, 0 );
	}
	// move the screen size to the left
	else if( key == SDLK_p )
	{
		pCamera->Move( -GAME_RES_W, 0 );
	}
	// push selected objects into the front
	else if( key == SDLK_KP_PLUS )
	{
		for( unsigned int i = 0; i < pMouseCursor->selected_objects.size(); i++ )
		{
			Change_DrawPosition( static_cast<cMovingSprite *>(pMouseCursor->selected_objects[i]->obj), 0 );
		}
	}
	// push selected objects into the back
	else if( key == SDLK_KP_MINUS )
	{
		for( unsigned int i = 0; i < pMouseCursor->selected_objects.size(); i++ )
		{
			Change_DrawPosition( static_cast<cMovingSprite *>(pMouseCursor->selected_objects[i]->obj), 1 );
		}
	}
	// ## only with mouse_object functions
	// Precise Pixel-Positioning or copy into direction
	if( ( key == SDLK_KP2 || key == SDLK_KP4 || key == SDLK_KP6 || key == SDLK_KP8 ) && pMouseCursor->mouse_object->obj )
	{
		if( pMouseCursor->mode_fastcopy )
		{
			ObjectDirection dir = DIR_UNDEFINED;

			// down
			if( key == SDLK_KP2 )
			{
				dir = DIR_DOWN;
			}
			// left
			else if( key == SDLK_KP4 )
			{
				dir = DIR_LEFT;
			}
			// right
			else if( key == SDLK_KP6 )
			{
				dir = DIR_RIGHT;
			}
			// up
			else if( key == SDLK_KP8 )
			{
				dir = DIR_UP;
			}

			// get currently selected objects
			SpriteList objects = pMouseCursor->Get_selected_objects();
			// copy objects
			SpriteList new_objects = Copy_Direction( objects, dir );

			for( unsigned int i = 0; i < objects.size(); i++ )
			{
				// add new object
				pMouseCursor->Add_selected_object( new_objects[i], 1 );
				// deselect old object
				pMouseCursor->Remove_selected_object( objects[i] );
			}
		}
		else
		{
			// down
			if( key == SDLK_KP2 )
			{
				pCamera->Move( 0, 1 );
			}
			// left
			else if( key == SDLK_KP4 )
			{
				pCamera->Move( -1, 0 );
			}
			// right
			else if( key == SDLK_KP6 )
			{
				pCamera->Move( 1, 0 );
			}
			// up
			else if( key == SDLK_KP8 )
			{
				pCamera->Move( 0, -1 );
			}
		}
	}
	// select all Sprites
	else if( key == SDLK_a && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pMouseCursor->Clear_selected_objects();

		cObjectManager *obj_manager = NULL;

		if( editor_level_enabled )
		{
			obj_manager = pObjManager;

			// player
			pMouseCursor->Add_selected_object( pPlayer, 1 );
		}
		else
		{
			obj_manager = pActive_Overworld->object_manager;

			// player
			pMouseCursor->Add_selected_object( pOverworld_Player, 1 );
		}

		// all level objects
		for( SpriteList::iterator itr = obj_manager->items.begin(), itr_end = obj_manager->items.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			pMouseCursor->Add_selected_object( obj, 1 );
		}
	}
	// Paste copy buffer objects
	else if( key == SDLK_INSERT || ( key == SDLK_v && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) ) )
	{
		pMouseCursor->Paste_copy_objects( pMouseCursor->posx, pMouseCursor->posy );
	}
	// Cut selected Sprites to the copy buffer
	else if( key == SDLK_x && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pMouseCursor->Clear_copy_objects();

		for( unsigned int i = 0; i < pMouseCursor->selected_objects.size(); i++ )
		{
			pMouseCursor->Add_copy_object( pMouseCursor->selected_objects[i]->obj );
		}

		pMouseCursor->Delete_selected_objects();
	}
	// Add selected Sprites to the copy buffer
	else if( key == SDLK_c && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pMouseCursor->Clear_copy_objects();

		for( unsigned int i = 0; i < pMouseCursor->selected_objects.size(); i++ )
		{
			pMouseCursor->Add_copy_object( pMouseCursor->selected_objects[i]->obj );
		}
	}
	// Delete mouse object
	else if( key == SDLK_DELETE && pMouseCursor->mouse_object->obj )
	{
		pMouseCursor->Delete( static_cast<cMovingSprite *>(pMouseCursor->mouse_object->obj) );
	}
	// if shift got pressed remove mouse object for possible mouse selection
	else if( ( key == SDLK_RSHIFT || key == SDLK_LSHIFT ) && pMouseCursor->mouse_object->obj )
	{
		pMouseCursor->Clear_mouse_object();
	}
	// Delete selected objects
	else if( key == SDLK_DELETE )
	{
		pMouseCursor->Delete_selected_objects();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cEditor :: Add_Object( cSprite *object )
{
	// virtual
	delete object;
}

void cEditor :: Add_Menu_Object( string name, string tags, colour normal_color /* = white.Get_cegui_colour */ )
{
	// Create Menu Object
	cEditor_Menu_Object *new_menu = new cEditor_Menu_Object( name );

	// if function
	if( tags.find( "function:" ) == 0 )
	{
		new_menu->bfunction = 1;
		// cut out the function identifier
		tags.erase( 0, 9 );
	}

	// if header
	if( tags.find( "header" ) == 0 )
	{
		new_menu->header = 1;
		// cut out the function identifier
		tags.erase( 0, 6 );

		// header color rect
		new_menu->setTextColours( normal_color, normal_color, grey.Get_cegui_colour(),grey.Get_cegui_colour() );
		// not selectable
		new_menu->setDisabled( 1 );
		// set tooltip
		new_menu->setTooltipText( "Header " + name );
	}
	// if not a header
	else
	{
		new_menu->setTextColours( normal_color );
	}

	// if default items menu
	if( !new_menu->bfunction && !new_menu->header )
	{
		// set tooltip
		new_menu->setTooltipText( "Tags used " + tags );
	}

	new_menu->tags = tags;
	new_menu->Init();

	// Get Listbox
	Listbox *listbox_editor_menu = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_menu" ));
	// Add Listbox item
	listbox_editor_menu->addItem( new_menu );
}

void cEditor :: Activate_Menu( cEditor_Menu_Object *entry )
{
	// Function
	if( entry->bfunction )
	{
		if( entry->tags.compare( "exit" ) == 0 )
		{
			Function_Exit();
		}
		else
		{
			printf( "Unknown Function %s\n", entry->tags.c_str() );
		}
	}
	// Header
	else if( entry->header )
	{
		return;
	}
	// Item Menu
	else
	{
		if( Load_Item_Menu( entry->tags ) )
		{
			// Get TabControl
			TabControl *tabcontrol_menu = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_editor" ));
			// Select Items Tab
			tabcontrol_menu->setSelectedTab( "editor_tab_items" );
		}
		// failed
		else
		{
			printf( "Unknown Menu Type %s\n", entry->tags.c_str() );
		}
	}
}

bool cEditor :: Load_Item_Menu( string item_tags )
{
	if( item_tags.empty() )
	{
		return 0;
	}

	Unload_Item_Menu();

	// Convert to Array Tags
	vector<string> array_tags;

	// Convert
	while( item_tags.length() )
	{
		unsigned int pos = item_tags.find( ";" );

		// last item
		if( pos == string::npos )
		{
			array_tags.push_back( item_tags );
			item_tags.clear();
			break;
		}

		// add tag
		array_tags.push_back( item_tags.substr( 0, pos ) );
		// remove tag
		item_tags.erase( 0, pos + 1 );
	}

	unsigned int tag_pos = 0;

	// Get all Images with the Tags
	for( TaggedItemImageSettingsList::iterator itr = tagged_item_images.begin(), itr_end = tagged_item_images.end(); itr != itr_end; ++itr )
	{
		cImage_settings_data *settings = (*itr);

		// search
		while( is_tag_available( settings->editor_tags, array_tags[tag_pos] ) )
		{
			tag_pos++;

			// found all tags
			if( tag_pos == array_tags.size() )
			{
				GL_Surface *image = pVideo->Get_Surface( settings->base );
				// Create Sprite
				cMovingSprite *new_sprite = new cMovingSprite( image );
				// default massivetype
				new_sprite->Set_Sprite_Type( (SpriteType)image->type );
				// Add new Sprite
				Add_Item_Object( new_sprite );

				break;
			}
		}

		tag_pos = 0;
	}

	// Get all Objects with the Tags
	for( TaggedItemObjectsList::iterator itr = tagged_item_objects.begin(), itr_end = tagged_item_objects.end(); itr != itr_end; ++itr )
	{
		cSprite *object = (*itr);

		// search
		while( is_tag_available( object->editor_tags, array_tags[tag_pos] ) )
		{
			tag_pos++;

			// found all tags
			if( tag_pos == array_tags.size() )
			{
				// Add Objects
				Add_Item_Object( object->Copy() );

				break;
			}
		}

		tag_pos = 0;
	}

	return 1;
}

void cEditor :: Unload_Item_Menu( void )
{
	// already unloaded
	if( !WindowManager::getSingleton().isWindowPresent( "editor_items" ) )
	{
		return;
	}

	// Get Items Listbox
	Listbox *listbox_items = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));
	// Clear Listbox
	listbox_items->resetList();
}


void cEditor :: Add_Item_Object( cSprite *sprite, string new_name /* = "" */, GL_Surface *image /* = NULL */ )
{
	// if invalid
	if( !sprite )
	{
		printf( "Warning : Invalid Editor Item\n" );
		return;
	}

	// set correct array if not given
	if( sprite->sprite_array == ARRAY_UNDEFINED )
	{
		if( sprite->massivetype == MASS_PASSIVE )
		{
			sprite->sprite_array = ARRAY_PASSIVE;
		}
		else if( sprite->massivetype == MASS_MASSIVE )
		{
			sprite->sprite_array = ARRAY_MASSIVE;
		}
		else if( sprite->massivetype == MASS_HALFMASSIVE )
		{
			sprite->sprite_array = ARRAY_ACTIVE;
		}
	}

	// set correct type if not given
	if( sprite->type == TYPE_UNDEFINED )
	{
		if( sprite->massivetype == MASS_PASSIVE )
		{
			sprite->type = TYPE_PASSIVE;
		}
		else if( sprite->massivetype == MASS_MASSIVE )
		{
			sprite->type = TYPE_MASSIVE;
		}
		else if( sprite->massivetype == MASS_HALFMASSIVE )
		{
			sprite->type = TYPE_HALFMASSIVE;
		}
		else if( sprite->massivetype == MASS_CLIMBABLE )
		{
			sprite->type = TYPE_CLIMBABLE;
		}
	}

	// if no image is given use the sprite start image
	if( !image )
	{
		image = sprite->start_image;
	}

	// set object name
	string obj_name;

	if( new_name.length() )
	{
		obj_name = new_name;
	}
	else if( sprite->name.length() )
	{
	    obj_name = sprite->name;
	}
	// no object name available
	else
	{
		if( image )
		{
			obj_name = image->Get_filename( 0, 0 );
		}

		// Warn if using filename
		printf( "Warning : editor object %s with no name given\n", obj_name.c_str() );
	}

	// check if name is fitting
	if( obj_name.length() > 25 )
	{
		obj_name.erase( 25 );
		obj_name += "|";
	}

	cEditor_Item_Object *new_item = new cEditor_Item_Object( obj_name );

	// object pointer
	new_item->sprite_obj = sprite;

	// Initialize
	new_item->Init();

	// Get Items Listbox
	Listbox *listbox_items = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));
	// Add Item
	listbox_items->addItem( new_item );
}

void cEditor :: Load_Image_Items( string dir )
{
	fs::path full_path( dir );
	fs::directory_iterator end_iter;

	// load all available objects
	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			// if directory
			if( fs::is_directory( *dir_itr ) )
			{
				// ignore hidden directories
				if( dir_itr->leaf().find( "." ) == 0 )
				{
					continue;
				}

				// load all items from the sub-directory
				Load_Image_Items( dir + "/" + dir_itr->leaf() );
			}
			// valid file
			else if( dir_itr->leaf().rfind( ".settings" ) != string::npos )
			{
				// load settings
				string settings_filename = dir + "/" + dir_itr->leaf();
				cImage_settings_data *settings = pSettingsParser->Get( settings_filename );

				// if settings are available
				if( settings )
				{
					// if required editor tag is available
					if( settings->editor_tags.find( editor_item_tag ) != string::npos )
					{
						// set base to the filename
						settings->base = dir + "/" + dir_itr->leaf();
						// add real image
						tagged_item_images.push_back( settings );
					}
				}
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->leaf().c_str(), ex.what() );
		}
	}
}

void cEditor :: Activate_Item( cEditor_Item_Object *entry )
{
	// invalid
	if( !entry )
	{
		printf( "Error : Invalid Editor Item\n" );
		return;
	}

	// create copy from editor item
	cSprite *new_sprite = Copy_Object( entry->sprite_obj, pMouseCursor->posx, pMouseCursor->posy );

	// if copying failed
	if( !new_sprite )
	{
		printf( "Error : Editor Sprite %s copy failed\n", entry->sprite_obj->name.c_str() );
		return;
	}

	// hide editor window
	editor_window->setXPosition( UDim( -0.19f, 0 ) );

	Add_Object( new_sprite );

	// Set Mouse Objects
	pMouseCursor->left = 1;
	pMouseCursor->mouse_object->mouse_h = (int)( new_sprite->col_rect.h / 2 );
	pMouseCursor->mouse_object->mouse_w = (int)( new_sprite->col_rect.w / 2 );
	pMouseCursor->Set_mouse_object( new_sprite );
}

bool cEditor :: is_tag_available( string str, string tag, unsigned int search_pos /* = 0 */ )
{
	// found tag position
	unsigned int pos = str.find( tag, search_pos );

	// not found
	if( pos == string::npos )
	{
		return 0;
	}

	// tag end position
	unsigned int end_pos = pos + tag.length();

	// if tag starting position is valid
	if( pos == 0 || str.substr( pos - 1, 1 ).compare( ";" ) == 0  )
	{
		// if tag ending position is valid
		if( end_pos == str.length() || str.substr( end_pos, 1 ).compare( ";" ) == 0  )
		{
			return 1;
		}
	}

	// not valid - continue search
	return is_tag_available( str, tag, end_pos );
}

void cEditor :: Draw_Editor_Help( void )
{
	// Help Window Background Rect
	pVideo->Draw_Rect( 50, 5, 700, 550, 0.58f, &blackalpha192 );

	// no help text set
	if( !help_sprites.size() )
	{
		// Add/Create the Help Text
		Add_Help_Line( "Editor Help", "", 5, 300 );
		Add_Help_Line( "F1", "Toggle this Help Window" );
		Add_Help_Line( "F8", "Open / Close the Level editor" );
		Add_Help_Line( "F10", "Toggle sound effects" );
		Add_Help_Line( "F11", "Toggle music play" );
		Add_Help_Line( "Home", "Focus level start" );
		Add_Help_Line( "End", "Focus last level exit" );
		Add_Help_Line( "L", "Load a Level" );
		Add_Help_Line( "W", "Load an Overworld" );
		Add_Help_Line( "N", "Step one screen to the right ( Next Screen )" );
		Add_Help_Line( "P", "Step one screen to the left ( Previous Screen )" );
		Add_Help_Line( "M", "Cycle through massive types (use for object groups)" );
		Add_Help_Line( "Massive types (with hover color):" );
		Add_Help_Line( "Massive ->   Halfmassive ->   Climbable ->   Passive ->   Front Passive", "" , 0, 80 );
		Add_Help_Line( "Ctrl + S", "Quicksave current Level" );
		Add_Help_Line( "Ctrl + D", "Toggle debug mode" );
		Add_Help_Line( "Ctrl + A", "Select all objects" );
		Add_Help_Line( "Ctrl + X", "Cut currently selected objects" );
		Add_Help_Line( "Ctrl + C", "Copy currently selected objects" );
		Add_Help_Line( "Ctrl + V", "Paste current copied / cutted objects" );
		Add_Help_Line( "Insert", "Paste current copied / cutted objects" );
		Add_Help_Line( "Del", "If Mouse is over an object: Delete current object" );
		Add_Help_Line( "Del", "If Mouse has nothing selected: Delete selected objects" );
		Add_Help_Line( "Numpad:" );
		Add_Help_Line( " +", "Bring object to front" );
		Add_Help_Line( " -", "Send object to back" );
		Add_Help_Line( " 2/4/6/8", "Move selected tile in direction ( 1 Pixel )" );
		Add_Help_Line( "Mouse:" );
		Add_Help_Line( " Left (Hold)", "Drag objects" );
		Add_Help_Line( " Left (Click)", "With shift to select / deselect single objects" );
		Add_Help_Line( " Right", "Delete intersecting Object" );
		Add_Help_Line( " Middle", "Switch to Mover Mode" );
		Add_Help_Line( "Arrow keys:" );
		Add_Help_Line( " Use arrow keys to move around. Press shift for faster movement" );
	}

	// draw
	for( HudSpriteList::iterator itr = help_sprites.begin(), itr_end = help_sprites.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}

	// Draw Massivetype colors
	pVideo->Draw_Rect( 70, 218, 10, 10, 0.581f, &Get_Massivetype_Color( MASS_MASSIVE ) );
	pVideo->Draw_Rect( 132, 218, 10, 10, 0.581f, &Get_Massivetype_Color( MASS_HALFMASSIVE ) );
	pVideo->Draw_Rect( 213, 218, 10, 10, 0.581f, &Get_Massivetype_Color( MASS_CLIMBABLE ) );
    pVideo->Draw_Rect( 284, 218, 10, 10, 0.581f, &Get_Massivetype_Color( MASS_PASSIVE ) );
    pVideo->Draw_Rect( 343, 218, 10, 10, 0.581f, &Get_Massivetype_Color( MASS_PASSIVE ) );
}

void cEditor :: Add_Help_Line( string key_text, string text /* = "" */, float spacing /* = 0 */, float pos_x /* = 60 */ )
{
	// create help sprites
	cHudSprite *help_sprite_key_text = new cHudSprite();
	cHudSprite *help_sprite_text = new cHudSprite();
	// with shadow
	help_sprite_key_text->Set_Shadow( black, 0.5f );
	help_sprite_text->Set_Shadow( black, 0.5f );
	// position in front
	help_sprite_key_text->posz = 0.591f;
	help_sprite_text->posz = 0.59f;

	// Set Y position
	float pos_y = spacing;
	// if not the first help sprite use the last position
	if( help_sprites.size() )
	{
		// get last help sprite
		cHudSprite *last_hud_sprite = help_sprites[ help_sprites.size() - 1 ];
		// set correct position
		pos_y += last_hud_sprite->posy + last_hud_sprite->rect.h;
	}
	// first item
	else
	{
		pos_y += 5;
	}

	// text must be filled with something to get created by RenderText
	if( key_text.empty() )
	{
		key_text = " ";
	}
	if( text.empty() )
	{
		text = " ";
	}

	// set key text
	help_sprite_key_text->Set_Image( pFont->RenderText( pFont->font_very_small, key_text, lightorange ), 0, 1 );
	help_sprite_key_text->Set_Pos( pos_x, pos_y, 1 );
	// set text
	help_sprite_text->Set_Image( pFont->RenderText( pFont->font_very_small, text, white ), 0, 1 );
	help_sprite_text->Set_Pos( pos_x + 90, pos_y, 1 );

	// add to array
	help_sprites.push_back( help_sprite_key_text );
	help_sprites.push_back( help_sprite_text );
}

void cEditor :: Change_DrawPosition( cSprite *obj, bool push_back )
{
	// empty object
	if( !obj )
	{
		return;
	}

	// check if valid object
	if( !( obj->sprite_array == ARRAY_MASSIVE || obj->sprite_array == ARRAY_PASSIVE || obj->sprite_array == ARRAY_FRONT_PASSIVE || obj->sprite_array == ARRAY_ACTIVE ) )
	{
		return;
	}

	cObjectManager *obj_manager = NULL;

	if( editor_level_enabled )
	{
		obj_manager = pObjManager;
	}
	else
	{
		obj_manager = pActive_Overworld->object_manager;
	}

	cSprite *temp = NULL;

	// push front
	if( !push_back )
	{
		temp = obj_manager->items.back();

		// if already in front
		if( obj == temp )
		{
			return;
		}

		obj_manager->Delete( obj, 0 );
		obj_manager->items.back() = obj;
		obj_manager->items.insert( obj_manager->items.end() - 1, temp );

		obj->posz = obj_manager->Get_Last( obj->type )->posz + 0.000001f;
	}
	// push back
	else
	{
		temp = obj_manager->items.front();

		// if already in back
		if( obj == temp )
		{
			return;
		}

		obj_manager->Delete( obj, 0 );
		obj_manager->items.front() = obj;
		obj_manager->items.insert( obj_manager->items.begin() + 1, temp );

		obj->posz = obj_manager->Get_First( obj->type )->posz - 0.000001f;
	}
}

SpriteList cEditor :: Copy_Direction( SpriteList objects, ObjectDirection dir )
{
	// additional direction objects offset
	unsigned int offset = 0;

	// get the objects difference offset
	if( dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		// first object
		cSprite *first = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->startposx < first->startposx )
			{
				first = objects[i];
			}
		}

		// last object
		cSprite *last = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->startposx + objects[i]->start_rect.w > last->startposx + last->start_rect.w )
			{
				last = objects[i];
			}
		}

		// Set X offset
		offset = (int)( last->startposx - first->startposx + last->start_rect.w );
	}
	else if( dir == DIR_UP || dir == DIR_DOWN )
	{
		// first object
		cSprite *first = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->startposy < first->startposy )
			{
				first = objects[i];
			}
		}

		// last object
		cSprite *last = objects[0];

		for( unsigned int i = 1; i < objects.size(); i++ )
		{
			if( objects[i]->startposy + objects[i]->start_rect.h > last->startposy + last->start_rect.h )
			{
				last = objects[i];
			}
		}

		// Set Y offset
		offset = (int)( last->startposy - first->startposy + last->start_rect.h );
	}

	// new copied objects
	SpriteList new_objects;

	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		new_objects.push_back( Copy_Direction( objects[i], dir, offset ) );
	}

	// return only new objects
	return new_objects;
}

cSprite *cEditor :: Copy_Direction( cSprite *obj, ObjectDirection dir, int offset /* = 0 */ )
{
	float w = 0, h = 0;

	if( dir == DIR_LEFT )
	{
		if( offset )
		{
			w = -(float)offset;
		}
		else
		{
			w = -obj->start_rect.w;
		}
	}
	else if( dir == DIR_RIGHT )
	{
		if( offset )
		{
			w = (float)offset;
		}
		else
		{
			w = obj->start_rect.w;
		}
	}
	else if( dir == DIR_UP )
	{
		if( offset )
		{
			h = -(float)offset;
		}
		else
		{
			h = -obj->start_rect.h;
		}
	}
	else if( dir == DIR_DOWN )
	{
		if( offset )
		{
			h = (float)offset;
		}
		else
		{
			h = obj->start_rect.h;
		}
	}

	// only move camera if obj is the mouse object
	if( pMouseCursor->mouse_object->obj == obj )
	{
		pCamera->Move( w, h );
	}

	return pMouseCursor->Copy( obj, obj->startposx + w, obj->startposy + h );
}

bool cEditor :: Editor_Mouse_Enter( const EventArgs &event )
{
	// ignore if a button is pressed
	if( pMouseCursor->left || pMouseCursor->middle || pMouseCursor->right )
	{
		return 1;
	}

	// if not active fade in
	if( editor_window->getXPosition().asRelative( 1 ) != 0 )
	{
		editor_window->setXPosition( UDim( 0, 0 ) );
		editor_window->setAlpha( 0 );

		// Show Listbox
		Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_menu" ));
		listbox->show();
		listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "editor_items" ));
		listbox->show();
	}
	// if active but fading out
	else if( editor_window->getAlpha() < 1 )
	{
		editor_window->setAlpha( 1 );
		menu_timer = 0;
	}

	return 1;
}

bool cEditor :: Menu_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// set item
	if( item )
	{
		Activate_Menu( (cEditor_Menu_Object *)item );
	}
	// clear ?
	else
	{
		// todo : clear
	}

	return 1;
}

bool cEditor :: Item_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// activate item
	if( item )
	{
		Activate_Item( (cEditor_Item_Object *)item );
	}

	return 1;
}

void cEditor :: Function_Exit( void )
{
	pKeyboard->Key_Down( SDLK_F8 );
}

// XML element start
void cEditor :: elementStart( const String &element, const XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
    }
}

// XML element end
void cEditor :: elementEnd( const String &element )
{
	if( element != "Property" )
	{
		if( element == "Item" )
		{
			// Menu Item
			if( xml_attributes.getValueAsString( "tags" ).length() )
			{
				Handle_Menu( xml_attributes );
			}
			// Items Item
			else
			{
				Handle_Item( xml_attributes );
			}
		}
		else if( element == "Items" || element == "Menu" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Editor Unknown Item Element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = XMLAttributes();
	}
}

void cEditor :: Handle_Item( const XMLAttributes &attributes )
{
	// element name must be given
	String name = xml_attributes.getValueAsString( "object_name" );
	String tags = xml_attributes.getValueAsString( "object_tags" );

	// create
	cSprite *object = NULL;

	// Level object
	if( editor_level_enabled )
	{
		object = Get_Level_Object( name, xml_attributes );
	}
	// Overworld object
	else
	{
		object = Get_World_Object( name, xml_attributes );
	}


	// if creation failed
	if( !object )
	{
		return;
	}

	// set editor tags
	object->editor_tags = tags.c_str();

	// Add Item Object
	tagged_item_objects.push_back( object );
}

void cEditor :: Handle_Menu( const XMLAttributes &attributes )
{
	string name = xml_attributes.getValueAsString( "name" ).c_str();
	string tags = xml_attributes.getValueAsString( "tags" ).c_str();

	Add_Menu_Object( name, tags, PropertyHelper::stringToColour( xml_attributes.getValueAsString( "color", "FFFFFFFF" ) ) );
}
