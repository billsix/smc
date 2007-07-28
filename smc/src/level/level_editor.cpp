/***************************************************************************
			level_editor.cpp  -  class for the Level Editor
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
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
#include "../level/level_editor.h"
#include "../level/level.h"
#include "../level/level_settings.h"
#include "../core/game_core.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"
#include "../user/preferences.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"

/* *** *** *** *** *** *** *** cEditor_Level *** *** *** *** *** *** *** *** *** *** */

cEditor_Level :: cEditor_Level( void )
: cEditor()
{
	menu_filename = EDITOR_DIR "/level_menu.xml";
	items_filename = EDITOR_DIR "/level_items.xml";

	editor_item_tag = "level";
}

cEditor_Level :: ~cEditor_Level( void )
{
	//
}

void cEditor_Level :: Init( void )
{
	// already loaded
	if( editor_window )
	{
		return;
	}

	// nothing

	cEditor::Init();
}

void cEditor_Level :: Enable( void )
{
	// already enabled
	if( enabled )
	{
		return;
	}

	editor_level_enabled = 1;

	if( Game_Mode == MODE_LEVEL )
	{
		editor_enabled = 1;
	}

	cEditor::Enable();
}

void cEditor_Level :: Disable( void )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	editor_level_enabled = 0;

	if( Game_Mode == MODE_LEVEL )
	{
		editor_enabled = 0;
	}
	
	cEditor::Disable();
}

bool cEditor_Level :: Key_Down( SDLKey key )
{
	if( !enabled )
	{
		return 0;
	}


	// check basic editor events
	if( cEditor::Key_Down( key ) )
	{
		return 1;
	}
	// save level
	else if( key == SDLK_s && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pLevel->Save();
	}
	// focus last levelexit
	else if( key == SDLK_END )
	{
		float new_cameraposx = 0;
		float new_cameraposy = 0;

		for( SpriteList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			if( obj->sprite_array != ARRAY_ACTIVE )
			{
				continue;
			}

			if( obj->type == TYPE_LEVELEXIT && new_cameraposx < obj->posx )
			{
				new_cameraposx = obj->posx;
				new_cameraposy = obj->posy;
			}
		}

		if( new_cameraposx != 0 || new_cameraposy != 0 )
		{
			pCamera->Set_Pos( new_cameraposx - ( GAME_RES_W / 2 ), new_cameraposy - ( GAME_RES_H / 2 ) );
		}
	}
	// modify selected objects state
	else if( key == SDLK_m )
	{
		if( pMouseCursor->selected_objects.size() )
		{
			// change state of the base object
			if( pLevel_Editor->Switch_ObjectState( pMouseCursor->selected_objects[0]->obj ) )
			{
				// change all object states to the base object state
				for( SelectedObjectList::iterator itr = pMouseCursor->selected_objects.begin(), itr_end = pMouseCursor->selected_objects.end(); itr != itr_end; ++itr )
				{
					cSprite *obj = (*itr)->obj;

					// skip base object
					if( obj == pMouseCursor->selected_objects[0]->obj )
					{
						continue;
					}

					// sprites need additional data
					if( obj->type == TYPE_PASSIVE || obj->type == TYPE_FRONT_PASSIVE || obj->type == TYPE_MASSIVE || obj->type == TYPE_CLIMBABLE || obj->type == TYPE_HALFMASSIVE )
					{
						obj->type = pMouseCursor->selected_objects[0]->obj->type;
						obj->sprite_array = pMouseCursor->selected_objects[0]->obj->sprite_array;
						obj->can_be_ground = pMouseCursor->selected_objects[0]->obj->can_be_ground;
					}
					
					// set state
					obj->Set_Massivetype( pMouseCursor->selected_objects[0]->obj->massivetype );
				}
			}
		}
	}
	// modify mouse object state
	else if( key == SDLK_m && pMouseCursor->mouse_object->obj )
	{
		Switch_ObjectState( pMouseCursor->mouse_object->obj );
		pMouseCursor->Clear_mouse_object();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cEditor_Level :: Add_Object( cSprite *object )
{
	// add to level object manager
	pObjManager->Add( object );
}

void cEditor_Level :: Activate_Menu( cEditor_Menu_Object *entry )
{
	// If Function
	if( entry->bfunction )
	{
		if( entry->tags.compare( "new" ) == 0 )
		{
			Function_New();
		}
		else if( entry->tags.compare( "load" ) == 0 )
		{
			Function_Load();
		}
		else if( entry->tags.compare( "save" ) == 0 )
		{
			Function_Save();
		}
		else if( entry->tags.compare( "save_as" ) == 0 )
		{
			Function_Save_as();
		}
		else if( entry->tags.compare( "reload" ) == 0 )
		{
			Function_Reload();
		}
		else if( entry->tags.compare( "clear" ) == 0 )
		{
			Function_Clear();
		}
		else if( entry->tags.compare( "settings" ) == 0 )
		{
			Function_Settings();
		}
		// unknown level function
		else
		{
			cEditor::Activate_Menu( entry );
		}
	}
	// unknown level function
	else
	{
		cEditor::Activate_Menu( entry );
	}
}

bool cEditor_Level :: Switch_ObjectState( cSprite *obj )
{
	// empty object
	if( !obj )
	{
		return 0;
	}

	// from Passive to Front Passive
	if( obj->type == TYPE_PASSIVE )
	{
		obj->Set_Sprite_Type( TYPE_FRONT_PASSIVE );
	}
	// from Front Passive to Massive
	else if( obj->type == TYPE_FRONT_PASSIVE )
	{
		obj->Set_Sprite_Type( TYPE_MASSIVE );
	}
	// from Massive to Halfmassive
	else if( obj->type == TYPE_MASSIVE )
	{
		obj->Set_Sprite_Type( TYPE_HALFMASSIVE );
	}
	// from Halfmassive to Climbable
	else if( obj->type == TYPE_HALFMASSIVE )
	{
		obj->Set_Sprite_Type( TYPE_CLIMBABLE );
	}
	// from Climbable to Passive
	else if( obj->type == TYPE_CLIMBABLE )
	{
		obj->Set_Sprite_Type( TYPE_PASSIVE );
	}
	// moving platform
	else if( obj->type == TYPE_MOVING_PLATFORM )
	{
		if( obj->massivetype == MASS_PASSIVE )
		{
			obj->Set_Massivetype( MASS_MASSIVE );
		}
		else if( obj->massivetype == MASS_MASSIVE )
		{
			obj->Set_Massivetype( MASS_HALFMASSIVE );
		}
		else if( obj->massivetype == MASS_HALFMASSIVE )
		{
			obj->Set_Massivetype( MASS_CLIMBABLE );
		}
		else if( obj->massivetype == MASS_CLIMBABLE )
		{
			obj->Set_Massivetype( MASS_PASSIVE );
		}
	}
	// invalid object type
	else
	{
		return 0;
	}

	return 1;
}

void cEditor_Level :: Function_New( void )
{
	string levelname = Box_Text_Input( "Create a new Level", "Name" );

	// invalid
	if( levelname.empty() )
	{
		return;
	}

	if( pLevel->New( levelname ) )
	{
		debugdisplay->Set_Text( "Created " + levelname );
	}
	else
	{
		debugdisplay->Set_Text( "Level already exist " + levelname );
	}
}

void cEditor_Level :: Function_Load( void )
{
	pKeyboard->Key_Down( SDLK_l );
}

void cEditor_Level :: Function_Save( void )
{
	pLevel->Save();
}

void cEditor_Level :: Function_Save_as( void )
{
	string levelname = Box_Text_Input( "Save Level as", "New name", 1 );

	// invalid
	if( levelname.empty() )
	{
		return;
	}

	pLevel->Set_Levelfile( levelname, 0 );
	pLevel->Save();
}

void cEditor_Level :: Function_Reload( void )
{
	// if denied
	if( !Box_Question( "Reload Level ?" ) )
	{
		return;
	}

	pLevel->Save();
	pLevel->Load( Get_filename( pLevel->data_file, 0 ) );
}

void cEditor_Level :: Function_Clear( void )
{
	// if denied
	if( !Box_Question( "Clear Level ?" ) )
	{
		return;
	}

	pObjManager->Delete_All();
}

void cEditor_Level :: Function_Settings( void )
{
	cLevel_Settings *settings = new cLevel_Settings();
	settings->Enter();
	delete settings;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEditor_Level *pLevel_Editor = NULL;
