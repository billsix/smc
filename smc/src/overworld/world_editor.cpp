/***************************************************************************
			world_editor.cpp  -  class for the World Editor
                             -------------------
    copyright            : (C) 2007 by Florian Richter
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
#include "../overworld/world_editor.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../input/keyboard.h"

/* *** *** *** *** *** *** *** cEditor_World *** *** *** *** *** *** *** *** *** *** */

cEditor_World :: cEditor_World( void )
: cEditor()
{
	menu_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_menu.xml";
	items_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_items.xml";

	editor_item_tag = "world";
	camera_speed = 20;
}

cEditor_World :: ~cEditor_World( void )
{
	//
}

void cEditor_World :: Init( void )
{
	// already loaded
	if( editor_window )
	{
		return;
	}

	// nothing

	cEditor::Init();
}

void cEditor_World :: Enable( void )
{
	// already enabled
	if( enabled )
	{
		return;
	}
	
	editor_world_enabled = 1;
	pActive_Overworld->pLayer->draw = 1;

	if( Game_Mode == MODE_OVERWORLD )
	{
		editor_enabled = 1;
	}
	
	cEditor::Enable();
}

void cEditor_World :: Disable( void )
{
	// already disabled
	if( !enabled )
	{
		return;
	}

	editor_world_enabled = 0;
	pActive_Overworld->pLayer->draw = 0;
	pOverworld_manager->cameramode = 0;

	if( Game_Mode == MODE_OVERWORLD )
	{
		editor_enabled = 0;
	}

	cEditor::Disable();
}

bool cEditor_World :: Key_Down( SDLKey key )
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
		pActive_Overworld->Save();
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

void cEditor_World :: Add_Object( cSprite *object )
{
	// add to level object manager
	pActive_Overworld->Add_Map_Object( object );
}

void cEditor_World :: Activate_Menu( cEditor_Menu_Object *entry )
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
		/*else if( entry->tags.compare( "save_as" ) == 0 )
		{
			Function_Save_as();
		}*/
		else if( entry->tags.compare( "reload" ) == 0 )
		{
			Function_Reload();
		}
		else if( entry->tags.compare( "clear" ) == 0 )
		{
			Function_Clear();
		}
		/*else if( entry->tags.compare( "settings" ) == 0 )
		{
			Function_Settings();
		}*/
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

void cEditor_World :: Function_New( void )
{
	string world_name = Box_Text_Input( "Create a new World", "Name" );

	// invalid
	if( world_name.empty() )
	{
		return;
	}

	if( pOverworld_manager->New( world_name ) )
	{
		debugdisplay->Set_Text( "Created " + world_name );
	}
	else
	{
		debugdisplay->Set_Text( "World " + world_name + " already exists" );
	}
}

void cEditor_World :: Function_Load( void )
{
	pKeyboard->Key_Down( SDLK_w );
}

void cEditor_World :: Function_Save( void )
{
	pActive_Overworld->Save();
}

void cEditor_World :: Function_Reload( void )
{
	pActive_Overworld->Save();
	pActive_Overworld->Load();
}

void cEditor_World :: Function_Clear( void )
{
	pActive_Overworld->sprite_manager->Delete_All();
	pActive_Overworld->waypoints.clear();
	pOverworld_Player->Reset();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEditor_World *pWorld_Editor = NULL;
