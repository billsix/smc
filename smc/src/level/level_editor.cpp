/***************************************************************************
 * level_editor.cpp  -  Level Editor class
 *
 * Copyright (C) 2006 - 2008 Florian Richter
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
#include "../core/sprite_manager.h"
#include "../core/camera.h"
#include "../user/preferences.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../audio/audio.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** *** cEditor_Level *** *** *** *** *** *** *** *** *** *** */

cEditor_Level :: cEditor_Level( void )
: cEditor()
{
	menu_filename = DATA_DIR "/" GAME_EDITOR_DIR "/level_menu.xml";
	items_filename = DATA_DIR "/" GAME_EDITOR_DIR "/level_items.xml";

	editor_item_tag = "level";

	pSettings = new cLevel_Settings();
}

cEditor_Level :: ~cEditor_Level( void )
{
	delete pSettings;
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

void cEditor_Level :: Disable( bool native_mode /* = 0 */ )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	debugdisplay->Set_Text( _("Level Editor disabled") );

	editor_level_enabled = 0;

	if( Game_Mode == MODE_LEVEL )
	{
		native_mode = 1;
		editor_enabled = 0;
	}
	
	cEditor::Disable( native_mode );
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
		pActive_Level->Save();
	}
	// focus last levelexit
	else if( key == SDLK_END )
	{
		float new_cameraposx = 0;
		float new_cameraposy = 0;

		for( SpriteList::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			if( obj->sprite_array != ARRAY_ACTIVE )
			{
				continue;
			}

			if( obj->type == TYPE_LEVEL_EXIT && new_cameraposx < obj->posx )
			{
				new_cameraposx = obj->posx;
				new_cameraposy = obj->posy;
			}
		}

		if( new_cameraposx != 0 || new_cameraposy != 0 )
		{
			pActive_Camera->Set_Pos( new_cameraposx - ( game_res_w / 2 ), new_cameraposy - ( game_res_h / 2 ) );
		}
	}
	// modify selected objects state
	else if( key == SDLK_m )
	{
		if( pMouseCursor->selected_objects.size() )
		{
			// change state of the base object
			if( pLevel_Editor->Switch_Object_State( pMouseCursor->selected_objects[0]->obj ) )
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
					obj->Set_Massive_Type( pMouseCursor->selected_objects[0]->obj->massivetype );
				}
			}
		}
	}
	// modify mouse object state
	else if( key == SDLK_m && pMouseCursor->mouse_object->obj )
	{
		Switch_Object_State( pMouseCursor->mouse_object->obj );
		pMouseCursor->Clear_Mouse_Object();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
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
		else if( entry->tags.compare( "delete" ) == 0 )
		{
			Function_Delete();
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

bool cEditor_Level :: Switch_Object_State( cSprite *obj )
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
			obj->Set_Massive_Type( MASS_MASSIVE );
		}
		else if( obj->massivetype == MASS_MASSIVE )
		{
			obj->Set_Massive_Type( MASS_HALFMASSIVE );
		}
		else if( obj->massivetype == MASS_HALFMASSIVE )
		{
			obj->Set_Massive_Type( MASS_CLIMBABLE );
		}
		else if( obj->massivetype == MASS_CLIMBABLE )
		{
			obj->Set_Massive_Type( MASS_PASSIVE );
		}
	}
	// invalid object type
	else
	{
		return 0;
	}

	return 1;
}

bool cEditor_Level :: Function_New( void )
{
	string level_name = Box_Text_Input( _("Create a new Level"), _("Name") );

	// aborted/invalid
	if( level_name.empty() )
	{
		return 0;
	}

	if( pActive_Level->New( level_name ) )
	{
		debugdisplay->Set_Text( _("Created ") + level_name );
		return 1;
	}
	else
	{
		debugdisplay->Set_Text( _("Level ") + level_name + _(" already exists") );
	}

	return 0;
}

void cEditor_Level :: Function_Load( void )
{
	string level_name = _("Name");

	// valid level
	while( level_name.length() )
	{
		level_name = Box_Text_Input( level_name, _("Load a Level"), level_name.compare( _("Name") ) == 0 ? 1 : 0 );

		// break if empty
		if( level_name.empty() )
		{
			break;
		}

		// if available
		if( pActive_Level->Get_Path( level_name ) )
		{
			Game_Action = GA_ENTER_LEVEL;
			Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
			Game_Action_Data.add( "level", level_name.c_str() );

			debugdisplay->Set_Text( _("Loaded ") + Get_Filename( level_name, 0, 0 ) );

			break;
		}
		// not found
		else
		{
			pAudio->Play_Sound( "error.ogg" );
		}
	}
}

void cEditor_Level :: Function_Save( bool with_dialog /* = 0 */ )
{
	// not loaded
	if( !pActive_Level->Is_Loaded() )
	{
		return;
	}

	// if denied
	if( with_dialog && !Box_Question( _("Save ") + Get_Filename( pActive_Level->levelfile, 0, 0 ) + " ?" ) )
	{
		return;
	}

	pActive_Level->Save();
}

void cEditor_Level :: Function_Save_as( void )
{
	string levelname = Box_Text_Input( _("Save Level as"), _("New name"), 1 );

	// aborted/invalid
	if( levelname.empty() )
	{
		return;
	}

	pActive_Level->Set_Levelfile( levelname, 0 );
	pActive_Level->Save();
}

void cEditor_Level :: Function_Delete( void )
{
	string filename = pActive_Level->levelfile;
	if( !pActive_Level->Get_Path( filename, 1 ) )
	{
		return;
	}

	// if denied
	if( !Box_Question( _("Delete and Unload ") + Get_Filename( filename, 0, 0 ) + " ?" ) )
	{
		return;
	}

	pActive_Level->Delete();
	pLevel_Editor->Disable();

	Game_Action = GA_ENTER_MENU;
}

void cEditor_Level :: Function_Reload( void )
{
	// if denied
	if( !Box_Question( _("Reload Level ?") ) )
	{
		return;
	}

	pActive_Level->Save();
	pActive_Level->Load( Get_Filename( pActive_Level->data_file, 0 ) );
}

void cEditor_Level :: Function_Clear( void )
{
	// if denied
	if( !Box_Question( _("Clear Level ?") ) )
	{
		return;
	}

	pActive_Sprite_Manager->Delete_All();
}

void cEditor_Level :: Function_Settings( void )
{
	Game_Action = GA_ENTER_LEVEL_SETTINGS;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEditor_Level *pLevel_Editor = NULL;
