/***************************************************************************
				menu_data.cpp  -  menu data/handling classes
                             -------------------
    copyright            :	(C) 2004 - 2007 by Florian Richter
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
#include "../gui/menu_data.h"
#include "../audio/audio.h"
#include "../core/camera.h"
#include "../gui/hud.h"
#include "../core/game_core.h"
#include "../video/font.h"
#include "../overworld/overworld.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../input/mouse.h"
#include "../core/framerate.h"
#include "../user/savegame.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../input/keyboard.h"
#include "../level/level_editor.h"
// boost filesystem
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

/* *** *** *** *** *** *** *** *** cMenu_Base *** *** *** *** *** *** *** *** *** */

cMenu_Base :: cMenu_Base( void )
{
	guiwindow = NULL;
	action = 0;
	menu_posy = 140;
	text_color = Color( static_cast<Uint8>(255), 251, 98 );
	text_color_value = Color( static_cast<Uint8>(255), 190, 30 );
}

cMenu_Base :: ~cMenu_Base( void )
{
	if( guiwindow )
	{
		pGuiSystem->getGUISheet()->removeChildWindow( guiwindow );
		WindowManager::getSingleton().destroyWindow( guiwindow );
	}

	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}
	
	drawlist.clear();
}

void cMenu_Base :: Init( void )
{
	// continue playing level/world music but not if finished level 
	if( !pAudio->isMusicPlaying() || ( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM && !pLevel->is_Loaded() ) )
	{
		pAudio->PlayMusic( "game/menu.ogg", -1, 0, 1500 );
	}

	// init
	Change_Game_Mode( MODE_MENU );

	layout_file = "";
}

void cMenu_Base :: Init_GUI( void )
{
	if( layout_file.empty() )
	{
		return;
	}

	guiwindow = WindowManager::getSingleton().loadWindowLayout( layout_file.c_str() );
	pGuiSystem->getGUISheet()->addChildWindow( guiwindow );
}

void cMenu_Base :: Update( void )
{
	// animation
	pMenuCore->pMenu_AnimManager->Update();
	// hud
	pHudManager->Update();
}

void cMenu_Base :: Draw( void )
{
	pVideo->Clear_Screen();
	// animation
	pMenuCore->pMenu_AnimManager->Draw();
	// gui
	pMenuCore->handler->Draw();

	// menu items
	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cMenu_Base :: Draw_End( void )
{
	// hud
	pHudManager->Draw();
}

void cMenu_Base :: Exit( void )
{
	// virtual
}

/* *** *** *** *** *** *** *** *** cMenu_Main *** *** *** *** *** *** *** *** *** */

cMenu_Main :: cMenu_Main( void )
: cMenu_Base()
{

}

cMenu_Main :: ~cMenu_Main( void )
{

}

void cMenu_Main :: Init( void )
{
	cMenu_Base::Init();

	// with exit image
	pMenuCore->handler->Use_Quit_Image();

	cMenu_Item *temp_item = NULL;

	layout_file = "menu_main.layout";

	GL_Surface *credits = pFont->RenderText( pFont->font_normal, "Credits", yellow );

	// Start
	temp_item = pMenuCore->Auto_Menu( "start.png", "start.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->posx + ( temp_item->image_default->col_rect.w + 16 ), temp_item->posy );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Options
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "options.png", "options.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->posx - temp_item->image_menu->col_rect.w - 16, temp_item->posy );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Load
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "load.png", "load.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->posx + ( temp_item->image_default->col_rect.w + 16 ), temp_item->posy );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Save
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "save.png", "save.png", menu_posy );
	temp_item->image_menu->Set_Pos( temp_item->posx - temp_item->image_menu->col_rect.w - 16, temp_item->posy );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Quit
	menu_posy += 60;
	temp_item = pMenuCore->Auto_Menu( "quit.png", "", menu_posy, 1 );
	temp_item->image_menu->Set_Pos( temp_item->posx + temp_item->col_rect.w + 16, temp_item->posy );
	pMenuCore->handler->Add_MenuItem( temp_item );

	// Credits
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( credits );
	temp_item->Set_Pos( GAME_RES_W * 0.45f, GAME_RES_H - 30 );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/logo_sdl.png" ), 10, static_cast<float>(GAME_RES_H) * 0.935f ) );
	drawlist.push_back( new cHudSprite( credits, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Main :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();

	Window *text_version = WindowManager::getSingleton().getWindow( "text_version" );
	text_version->setProperty( "Text", "Version " VERSION );
}

void cMenu_Main :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// Start
	if( pMenuCore->handler->active == 0 )
	{
		pMenuCore->next_menu = MENU_START;
		Game_Action = GA_ENTER_MENU;
	}
	// Options
	else if( pMenuCore->handler->active == 1 )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		Game_Action = GA_ENTER_MENU;
	}
	// Load
	else if( pMenuCore->handler->active == 2 )
	{
		pMenuCore->next_menu = MENU_LOAD;
		Game_Action = GA_ENTER_MENU;
	}
	// Save
	else if( pMenuCore->handler->active == 3 )
	{
		pMenuCore->next_menu = MENU_SAVE;
		Game_Action = GA_ENTER_MENU;
	}
	// Quit
	else if( pMenuCore->handler->active == 4 )
	{
		done = 1;
	}
	// Credits
	else if( pMenuCore->handler->active == 5 )
	{
		pMenuCore->next_menu = MENU_CREDITS;
		Game_Action = GA_ENTER_MENU;
	}
}

void cMenu_Main :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Main :: Exit( void )
{
	if( pLevel->is_Loaded() )
	{
		Game_Action = GA_ENTER_LEVEL;
	}
	else if( pActive_Overworld )
	{
		Game_Action = GA_ENTER_WORLD;
	}
}

/* *** *** *** *** *** *** *** *** cMenu_Start *** *** *** *** *** *** *** *** *** */

cMenu_Start :: cMenu_Start( void )
: cMenu_Base()
{

}

cMenu_Start :: ~cMenu_Start( void )
{

}

void cMenu_Start :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;

	GL_Surface *back1 = pFont->RenderText( pFont->font_normal, "Back", text_color );

	layout_file = "menu_start.layout";
 
	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( GAME_RES_W / 18, 450 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/start.png" ), GAME_RES_W * 0.02f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/overworld.png" ), GAME_RES_W / 20, 210 ) );
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Start :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();

	// Tab Control
	TabControl *tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
	tabcontrol->activate();

	// events
	tabcontrol->subscribeEvent( Window::EventKeyDown, Event::Subscriber( &cMenu_Start::window_keydown, this ) );

	// Worlds
	Listbox *listbox_worlds = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_worlds" ));
	
	// overworld names
	for( vector<cOverworld *>::iterator itr = pOverworld_manager->objects.begin(), itr_end = pOverworld_manager->objects.end(); itr != itr_end; ++itr )
	{
		cOverworld_description *world = (*itr)->description;

// show all worlds in debug builds
#ifndef _DEBUG
		if( !world->visible )
		{
			continue;
		}
#endif
		
		ListboxTextItem *item = new ListboxTextItem( world->name, 0 );
		// game world
		if( !world->user )
		{
			item->setTextColours( colour( 1, 0.8f, 0.6f ) );
		}
		// user world
		else
		{
			item->setTextColours( colour( 0.8f, 1, 0.6f ) );
		}

		item->setSelectionColours( colour( 0.33f, 0.33f, 0.33f ) );
		item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
		listbox_worlds->addItem( item );
	}

	// Level Listbox
	Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
	listbox_levels->setSortingEnabled( 1 );

	// get game level
	Get_Levels( DATA_DIR "/" GAME_LEVEL_DIR, colour( 1, 0.8f, 0.6f ) );
	// get user level
	Get_Levels( user_data_dir + USER_LEVEL_DIR, colour( 0.8f, 1, 0.6f ) );

	// World Listbox events
	listbox_worlds->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cMenu_Start::world_Select, this ) );
	listbox_worlds->subscribeEvent( Listbox::EventMouseDoubleClick, Event::Subscriber( &cMenu_Start::world_Select_final_list, this ) );
	// Level Listbox events
	listbox_levels->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cMenu_Start::level_Select, this ) );
	listbox_levels->subscribeEvent( Listbox::EventMouseDoubleClick, Event::Subscriber( &cMenu_Start::level_Select_final_list, this ) );
	// Level Buttons
	PushButton *button_new = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_level_new" ));
	button_new->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cMenu_Start::Button_level_new_clicked, this ) );
	PushButton *button_edit = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_level_edit" ));
	button_edit->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cMenu_Start::Button_level_edit_clicked, this ) );
	PushButton *button_delete = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_level_delete" ));
	button_delete->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cMenu_Start::Button_level_delete_clicked, this ) );

	// select first Item
	listbox_worlds->setItemSelectState( (size_t)0, 1 );

	// Button Enter World
	PushButton *button_world_enter = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_world_enter" ));
	// Button Enter Level
	PushButton *button_level_enter = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_level_enter" ));

	// events
	button_world_enter->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cMenu_Start::Button_world_Select_final, this ) );
	button_level_enter->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cMenu_Start::Button_level_Select_final, this ) );
}

void cMenu_Start :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// back
	if( pMenuCore->handler->active == 0 )
	{
		Exit();
	}
}

void cMenu_Start :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Start :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}

void cMenu_Start :: Get_Levels( string dir, colour color )
{
	// Level Listbox
	Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));

	// get all level names
	fs::path full_path( dir, fs::native );

	for( fs::directory_iterator dir_itr( full_path ), end_iter; dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			// only files
			if( !fs::is_directory( *dir_itr ) )
			{
				string lvl_name = dir_itr->leaf();

				// erase file type only if smclvl
				if( lvl_name.rfind( ".smclvl" ) != string::npos )
				{
					lvl_name.erase( lvl_name.rfind( ".smclvl" ) );
				}

				// check if item with the same name already exists
				ListboxItem *old_item = listbox_levels->findItemWithText( lvl_name, NULL );
				// remove old item
				if( old_item )
				{
					listbox_levels->removeItem( old_item );
				}

				ListboxTextItem *item = new ListboxTextItem( lvl_name, 0 );

				// smclvl
				if( dir_itr->leaf().rfind( ".smclvl" ) != string::npos )
				{
					item->setTextColours( color );
				}
				// grey out txt/unknown levels
				else
				{
					item->setTextColours( colour( 0.6f, 0.6f, 0.6f ) );
				}

				item->setSelectionColours( colour( 0.33f, 0.33f, 0.33f ) );
				item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
				listbox_levels->addItem( item );
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->leaf().c_str(), ex.what() );
		}
	}
}

void cMenu_Start :: Load_World( string name )
{
	// if not successful
	if( !pOverworld_manager->Set_Active( name ) )
	{
		debugdisplay->Set_Text( "Could not set Overworld active " + name, DESIRED_FPS );
	}
	// if successfully set active
	else
	{
		// unload possible loaded level
		pLevel->Unload();
		// enter world
		Game_Action = GA_ENTER_WORLD;
	}
}

bool cMenu_Start :: Load_Level( string level_name )
{
	// if not available
	if( !pLevel->Get_path( level_name ) )
	{
		pAudio->PlaySound( "error.ogg" );
		debugdisplay->Set_Text( "Couldn't load level " + level_name, DESIRED_FPS );
		return 0;
	}

	// enter level
	Game_Action = GA_ENTER_LEVEL;
	Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
	Game_Action_Data.add( "level", level_name.c_str() );

	return 1;
}

bool cMenu_Start :: window_keydown( const EventArgs &e )
{
	const KeyEventArgs &ke = static_cast<const KeyEventArgs &>(e);

	// Return
	if( ke.scancode == Key::Return || ke.scancode == Key::NumpadEnter )
	{
		// Get Tab Control
		TabControl *tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));

		// World
		if( tabcontrol->getSelectedTabIndex() == 0 )
		{
			ListboxItem *item = (static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_worlds" )))->getFirstSelectedItem();

			// load world
			if( item )
			{
				Load_World( item->getText().c_str() );
			}
		}
		// Level
		else
		{
			ListboxItem *item = (static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" )))->getFirstSelectedItem();

			// load level
			if( item )
			{
				Load_Level( item->getText().c_str() );
			}
		}

		return 1;
	}
	// Shift Tab
	else if( ( pKeyboard->keys[SDLK_LSHIFT] || pKeyboard->keys[SDLK_RSHIFT] ) && ke.scancode == Key::Tab )
	{
		// Get Tab Control
		TabControl *tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));

		// if last tab
		if( tabcontrol->getSelectedTabIndex() + 1 == tabcontrol->getTabCount() )
		{
			tabcontrol->setSelectedTabAtIndex( 0 );
		}
		// select next tab
		else
		{
			tabcontrol->setSelectedTabAtIndex( tabcontrol->getSelectedTabIndex() + 1 );
		}
	}
	// Tab
	else if( ke.scancode == Key::ArrowDown || ke.scancode == Key::ArrowUp )
	{
		// Get Tab Control
		TabControl *tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));

		// Items Listbox
		Listbox *listbox;

		// World
		if( tabcontrol->getSelectedTabIndex() == 0 )
		{
			listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_worlds" ));
		}
		// Level
		else
		{
			listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
		}

		
		ListboxItem *new_selected = NULL;

		// if none selected
		if( listbox->getSelectedCount() == 0 )
		{
			new_selected = listbox->getListboxItemFromIndex( 0 );
		}
		else
		{
			unsigned int selected = listbox->getItemIndex( listbox->getFirstSelectedItem() );

			// last item selected
			if( ke.scancode == Key::ArrowDown && selected == listbox->getItemCount() - 1 )
			{
				new_selected = listbox->getListboxItemFromIndex( 0 );
			}
			// first item selected
			else if( ke.scancode == Key::ArrowUp && selected == 0 )
			{
				new_selected = listbox->getListboxItemFromIndex( listbox->getItemCount() - 1 );
			}
			// select next item
			else
			{
				if( ke.scancode == Key::ArrowDown )
				{
					new_selected = listbox->getListboxItemFromIndex( selected + 1 );
				}
				// up
				else
				{
					new_selected = listbox->getListboxItemFromIndex( selected - 1 );
				}
			}
		}
		
		listbox->setItemSelectState( new_selected, 1 );
		listbox->ensureItemIsVisible( new_selected );
	}

	return 0;
}

bool cMenu_Start :: world_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// World Comment
	Editbox *editbox_world_comment = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_world_comment" ));

	// set world comment
	if( item )
	{
		editbox_world_comment->setText( pOverworld_manager->Get_from_Name( item->getText().c_str() )->description->comment );
	}
	// clear
	else
	{
		editbox_world_comment->setText( "" );
	}

	return 1;
}

bool cMenu_Start :: world_Select_final_list( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// load world
	if( item )
	{
		Load_World( item->getText().c_str() );
	}

	return 1;
}

bool cMenu_Start :: Button_world_Select_final( const EventArgs &event )
{
	// Get Selected World
	Listbox *listbox_worlds = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_worlds" ));
	ListboxItem *item = listbox_worlds->getFirstSelectedItem();

	// load world
	if( item )
	{
		Load_World( item->getText().c_str() );
	}

	return 1;
}


bool cMenu_Start :: level_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// set level something
	if( item )
	{
		// todo : needs level manager
	}
	// clear
	else
	{
		// todo : needs level manager
	}

	return 1;
}

bool cMenu_Start :: level_Select_final_list( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Listbox *>( windowEventArgs.window )->getFirstSelectedItem();

	// load level
	if( item )
	{
		Load_Level( item->getText().c_str() );
	}

	return 1;
}

bool cMenu_Start :: Button_level_Select_final( const EventArgs &event )
{
	// Get Selected Level
	Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
	ListboxItem *item = listbox_levels->getFirstSelectedItem();

	// load level
	if( item )
	{
		Load_Level( item->getText().c_str() );
	}

	return 1;
}

bool cMenu_Start :: Button_level_new_clicked( const EventArgs &event )
{
	// Create level
	if( !pLevel_Editor->Function_New() )
	{
		// aborted/failed
		return 0;
	}

	// Enter level
	Game_Action = GA_ENTER_LEVEL;
	Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM_EDITOR;

	return 1;
}

bool cMenu_Start :: Button_level_edit_clicked( const EventArgs &event )
{
	// Get Selected Level
	Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
	ListboxItem *item = listbox_levels->getFirstSelectedItem();

	// load level
	if( item && Load_Level( item->getText().c_str() ) )
	{
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM_EDITOR;
	}

	return 1;
}

bool cMenu_Start :: Button_level_delete_clicked( const EventArgs &event )
{
	// Get Selected Level
	Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
	ListboxItem *item = listbox_levels->getFirstSelectedItem();

	// load level
	if( item )
	{
		string filename = item->getText().c_str();

		// if denied
		if( !Box_Question( "Delete " + filename + " ?" ) )
		{
			return 1;
		}

		// only user directory
		if( pLevel->Get_path( filename, 1 ) )
		{
			Delete_file( filename );
			listbox_levels->removeItem( item );
		}
	}

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options *** *** *** *** *** *** *** *** *** */

cMenu_Options :: cMenu_Options( void )
: cMenu_Base()
{

}

cMenu_Options :: ~cMenu_Options( void )
{

}

void cMenu_Options :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;
	menu_posy = 210;

	GL_Surface *back1 = pFont->RenderText( pFont->font_normal, "Back", text_color );

	// Game
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "game.png", "", menu_posy );
	temp_item->Set_Scale( 0.7f );
	temp_item->Set_PosX( GAME_RES_W / 20, 1 );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Video
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "video.png", "", menu_posy );
	temp_item->Set_PosX( GAME_RES_W / 20, 1 );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Audio
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "audio.png", "", menu_posy );
	temp_item->Set_PosX( GAME_RES_W / 20, 1 );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// Controls
	menu_posy += 60;
	temp_item = new cMenu_Item();
	temp_item = pMenuCore->Auto_Menu( "controls.png", "", menu_posy );
	temp_item->Set_PosX( GAME_RES_W / 20, 1 );
	pMenuCore->handler->Add_MenuItem( temp_item );
	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( GAME_RES_W / 20 + 25, 450 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/options.png" ), GAME_RES_W / 20, 140, 0 ) );
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );
}

void cMenu_Options :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Options :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	// only menu actions
	if( pMenuCore->handler->active > 4 )
	{
		return;
	}

	action = 0;

	// Game
	if( pMenuCore->handler->active == MENU_GAME )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_GAME;
		Game_Action = GA_ENTER_MENU;
	}
	 // Video
	else if( pMenuCore->handler->active == MENU_VIDEO )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_VIDEO;
		Game_Action = GA_ENTER_MENU;
	}
	// Audio
	else if( pMenuCore->handler->active == MENU_AUDIO )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id	= MENU_AUDIO;
		Game_Action = GA_ENTER_MENU;
	}
	// Controls
	else if( pMenuCore->handler->active == MENU_CONTROLS )
	{
		pMenuCore->next_menu = MENU_OPTIONS;
		pMenuCore->options_menu_id = MENU_CONTROLS;
		Game_Action = GA_ENTER_MENU;
	}
	// back
	else if( pMenuCore->handler->active == 4 )
	{
		Exit();
	}
}

void cMenu_Options :: Draw( void )
{
	cMenu_Base::Draw();
}

void cMenu_Options :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	pPreferences->Save();
	Game_Action = GA_ENTER_MENU;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Game *** *** *** *** *** *** *** *** *** */

cMenu_Options_Game :: cMenu_Options_Game( void )
: cMenu_Options()
{

}

cMenu_Options_Game :: ~cMenu_Options_Game( void )
{

}

void cMenu_Options_Game :: Init( void )
{
	cMenu_Options::Init();

	cMenu_Item *temp_item = NULL;

	layout_file = "menu_game.layout";
	menu_posy = 220;

	GL_Surface *controls_always_run = pFont->RenderText( pFont->font_normal, "Always Run", text_color );
	GL_Surface *game_camera_hor_speed = pFont->RenderText( pFont->font_normal, "Camera Hor Speed", text_color );
	GL_Surface *game_camera_ver_speed = pFont->RenderText( pFont->font_normal, "Camera Ver Speed", text_color );

	// always run
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( controls_always_run );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	drawlist.push_back( new cHudSprite( controls_always_run, -200, 0, 1 ) );

	// Camera Horizontal Speed
	menu_posy += 40;
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( game_camera_hor_speed );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	drawlist.push_back( new cHudSprite( game_camera_hor_speed, -200, 0, 1 ) );
	// Camera Vertical Speed
	menu_posy += 30;
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( game_camera_ver_speed );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	drawlist.push_back( new cHudSprite( game_camera_ver_speed, -200, 0, 1 ) );

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/game.png" ), GAME_RES_W * 0.47f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/game.png" ), GAME_RES_W / 2.6f, 140 ) );

	pMenuCore->handler->Set_Active( MENU_GAME );

	Init_GUI();
}

void cMenu_Options_Game :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// get the CEGUI window manager
	WindowManager &windowmanager = WindowManager::getSingleton();

	// always run
	Combobox *combo_always_run = static_cast<Combobox *>(windowmanager.getWindow( "combo_always_run" ));

	ListboxTextItem *item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_always_run->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_always_run->addItem( item );

	if( pPreferences->always_run )
	{
		combo_always_run->setText( "On" );
	}
	else
	{
		combo_always_run->setText( "Off" );
	}

	combo_always_run->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Game::always_run_Select, this ) );

	// Camera Horizontal Speed
	Spinner *spinner_camera_hor_speed = static_cast<Spinner *>(windowmanager.getWindow( "spinner_camera_hor_speed" ));
	spinner_camera_hor_speed->setCurrentValue( pCamera->hor_offset_speed );

	spinner_camera_hor_speed->subscribeEvent( Spinner::EventValueChanged, Event::Subscriber( &cMenu_Options_Game::camera_hor_Select, this ) );

	// Camera Vertical Speed
	Spinner *spinner_camera_ver_speed = static_cast<Spinner *>(windowmanager.getWindow( "spinner_camera_ver_speed" ));
	spinner_camera_ver_speed->setCurrentValue( pCamera->ver_offset_speed );

	spinner_camera_ver_speed->subscribeEvent( Spinner::EventValueChanged, Event::Subscriber( &cMenu_Options_Game::camera_ver_Select, this ) );
}

void cMenu_Options_Game :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only video actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// always run
	if( pMenuCore->handler->active == 5 )
	{
		// todo : handle joy init failed
		pPreferences->always_run = !pPreferences->always_run;

		if( pPreferences->always_run )
		{
			WindowManager::getSingleton().getWindow( "combo_always_run" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_always_run" )->setText( "Off" );
		}
	}
	// Camera Horizontal
	else if( pMenuCore->handler->active == 6 )
	{
		// nothing
	}
	// Camera Vertical
	else if( pMenuCore->handler->active == 7 )
	{
		// nothing
	}
}

void cMenu_Options_Game :: Draw( void )
{
	cMenu_Options::Draw();
	Draw_End();
}

bool cMenu_Options_Game :: always_run_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool always_run = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		always_run = 1;
	}

	pPreferences->always_run = always_run;

	return 1;
}

bool cMenu_Options_Game :: camera_hor_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Spinner *spinner_camera_hor = static_cast<Spinner*>( windowEventArgs.window );
	
	pCamera->hor_offset_speed = spinner_camera_hor->getCurrentValue();

	return 1;
}

bool cMenu_Options_Game :: camera_ver_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Spinner *spinner_camera_ver = static_cast<Spinner*>( windowEventArgs.window );
	
	pCamera->ver_offset_speed = spinner_camera_ver->getCurrentValue();

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Video *** *** *** *** *** *** *** *** *** */

cMenu_Options_Video :: cMenu_Options_Video( void )
: cMenu_Options()
{
	fullscreen = new cHudSprite();
}

cMenu_Options_Video :: ~cMenu_Options_Video( void )
{
	delete fullscreen;
}

void cMenu_Options_Video :: Init( void )
{
	cMenu_Options::Init();

	// Fullscreen
	fullscreen->Set_Image( pVideo->Get_Surface( "menu/items/fullscreen.png" ), 1 );

	cMenu_Item *temp_item = NULL;

	layout_file = "menu_video.layout";
	menu_posy = 220;

	GL_Surface *video_resolution = pFont->RenderText( pFont->font_normal, "Resolution", text_color );
	GL_Surface *video_bpp = pFont->RenderText( pFont->font_normal, "Bpp", text_color );
	GL_Surface *video_fullscreen = pFont->RenderText( pFont->font_normal, "Fullscreen", text_color );
	GL_Surface *video_vsync = pFont->RenderText( pFont->font_normal, "VSync", text_color );
	GL_Surface *video_apply = pFont->RenderText( pFont->font_normal, "Apply", text_color );

	// Resolution
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( video_resolution );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( video_resolution, -200, 0, 1 ) );
	// Bpp
	temp_item = new cMenu_Item();
	menu_posy += 50;
	temp_item->image_default->Set_Image( video_bpp );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( video_bpp, -200, 0, 1 ) );
	// Fullscreen
	temp_item = new cMenu_Item();
	menu_posy += 50;
	temp_item->image_default->Set_Image( video_fullscreen );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	fullscreen->Set_Pos( GAME_RES_W / 1.8f + fullscreen->col_rect.w + 50, menu_posy - 5 );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( video_fullscreen, -200, 0, 1 ) );
	// VSync
	temp_item = new cMenu_Item();
	menu_posy += 50;
	temp_item->image_default->Set_Image( video_vsync );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( video_vsync, -200, 0, 1 ) );
	// Apply
	temp_item = new cMenu_Item();
	menu_posy += 50;
	temp_item->image_default->Set_Image( video_apply );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( video_apply, -200, 0, 1 ) );

	// Video Info
	vid_w = pPreferences->video_screen_w;
	vid_h = pPreferences->video_screen_h;
	vid_bpp = pPreferences->video_screen_bpp;
	vid_fullscreen = pPreferences->video_fullscreen;
	vid_vsync = pPreferences->video_vsync;

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/video.png" ), GAME_RES_W * 0.47f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/video.png" ), GAME_RES_W / 2.6f, 140 ) );

	pMenuCore->handler->Set_Active( MENU_VIDEO );

	Init_GUI();
}

void cMenu_Options_Video :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Resolution
	Combobox *combo_resolution = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_resolution" ));

	ListboxTextItem *item = new ListboxTextItem( "640x480", 1 );
	item->setTextColours( colour( 1, 0, 0 ) );
	combo_resolution->addItem( item );
	item = new ListboxTextItem( "800x600", 2 );
	item->setTextColours( colour( 1, 1, 0 ) );
	combo_resolution->addItem( item );
	item = new ListboxTextItem( "1024x768", 2 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_resolution->addItem( item );
	item = new ListboxTextItem( "1280x1024", 3 );
	item->setTextColours( colour( 0.1f, 0.5f, 0.9f ) );
	combo_resolution->addItem( item );
	item = new ListboxTextItem( "1600x1200", 4 );
	item->setTextColours( colour( 0.5f, 0.8f, 1 ) );
	combo_resolution->addItem( item );
	item = new ListboxTextItem( "2048x1536", 5 );
	item->setTextColours( colour( 0.8f, 0.2f, 0.9f ) );
	combo_resolution->addItem( item );

	string temp = int_to_string( pPreferences->video_screen_w ) + "x" + int_to_string( pPreferences->video_screen_h );
	combo_resolution->setText( temp.c_str() );

	combo_resolution->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Video::res_Select, this ) );

	// Bpp
	Combobox *combo_bpp = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_bpp" ));

	item = new ListboxTextItem( "16", 0 );
	item->setTextColours( colour( 1, 0.6f, 0.3f ) );
	combo_bpp->addItem( item );
	item = new ListboxTextItem( "32", 1 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_bpp->addItem( item );

	combo_bpp->setText( int_to_string( pPreferences->video_screen_bpp ).c_str() );

	combo_bpp->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Video::bpp_Select, this ) );

	// Fullscreen
	Combobox *combo_fullscreen = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_fullscreen" ));

	item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_fullscreen->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_fullscreen->addItem( item );

	if( pPreferences->video_fullscreen )
	{
		combo_fullscreen->setText( "On" );
	}
	else
	{
		combo_fullscreen->setText( "Off" );
	}

	combo_fullscreen->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Video::fullscreen_Select, this ) );

	// VSync
	Combobox *combo_vsync = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_vsync" ));

	item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_vsync->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_vsync->addItem( item );

	if( pPreferences->video_vsync )
	{
		combo_vsync->setText( "On" );
	}
	else
	{
		combo_vsync->setText( "Off" );
	}

	combo_vsync->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Video::vsync_Select, this ) );
}

void cMenu_Options_Video :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only video actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// Resolution
	if( pMenuCore->handler->active == 5 )
	{
		// Resolution
		Combobox *combo_resolution = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_resolution" ));
		unsigned int selected = combo_resolution->getItemIndex( combo_resolution->findItemWithText( combo_resolution->getText(), NULL ) );

		ListboxItem *new_selected = NULL;

		// last item selected
		if( selected == combo_resolution->getItemCount() - 1 )
		{
			new_selected = combo_resolution->getListboxItemFromIndex( 0 );
		}
		// select next item
		else
		{
			new_selected = combo_resolution->getListboxItemFromIndex( selected + 1 );
		}
		
		combo_resolution->setText( new_selected->getText() );
		combo_resolution->setItemSelectState( new_selected, 1 );
		res_Select( WindowEventArgs( combo_resolution ) );
	}
	// BPP
	else if( pMenuCore->handler->active == 6 )
	{
		if( vid_bpp == 16 )
		{
			vid_bpp = 32;
		}
		else if( vid_bpp == 32 )
		{
			vid_bpp = 16;
		}

		WindowManager::getSingleton().getWindow( "combo_bpp" )->setText( int_to_string( vid_bpp ).c_str() );	
	}
	// Fullscreen
	else if( pMenuCore->handler->active == 7 )
	{
		vid_fullscreen = !vid_fullscreen;

		if( vid_fullscreen )
		{
			WindowManager::getSingleton().getWindow( "combo_fullscreen" )->setText( "On" );	
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_fullscreen" )->setText( "Off" );
		}
	}
	// VSync
	else if( pMenuCore->handler->active == 8 )
	{
		vid_vsync = !vid_vsync;

		if( vid_vsync )
		{
			WindowManager::getSingleton().getWindow( "combo_vsync" )->setText( "On" );	
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_vsync" )->setText( "Off" );
		}
	}
	// Apply
	else if( pMenuCore->handler->active == 9 )
	{
		// draw reinitialization text
		Draw_StaticText( "Reinitialization", &green, NULL, 0 );

		pGuiSystem->renderGUI();
		pRenderer->Render();
		SDL_GL_SwapBuffers();

		// if no resolution, vsync or bpp changed
		if( pPreferences->video_screen_w == vid_w && pPreferences->video_screen_h == vid_h && pPreferences->video_screen_bpp == vid_bpp && pPreferences->video_vsync == vid_vsync )
		{
			// if only fullscreen changed
			if( pPreferences->video_fullscreen != vid_fullscreen )
			{
				pVideo->Toggle_Fullscreen();
			}
		}
		else
		{
			// new settings
			pPreferences->video_screen_w = vid_w;
			pPreferences->video_screen_h = vid_h;
			pPreferences->video_screen_bpp = vid_bpp;
			pPreferences->video_fullscreen = vid_fullscreen;
			pPreferences->video_vsync = vid_vsync;

			// apply new settings
			pPreferences->Apply();
		}

		// clear
		pMenuCore->next_menu = MENU_OPTIONS;
		Game_Action = GA_ENTER_MENU;
	}
}

void cMenu_Options_Video :: Draw( void )
{
	cMenu_Options::Draw();

	if( vid_fullscreen )
	{
		fullscreen->Draw();
	}

	Draw_End();
}

bool cMenu_Options_Video :: res_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	string temp = item->getText().c_str();

	unsigned int w = string_to_int( temp.substr( 0, temp.find( "x" ) ) );
	unsigned int h = string_to_int( temp.substr( temp.find( "x" ) + 1, temp.length() ) );

	if( !pVideo->Test_Video( w, h, vid_bpp ) )
	{
		return 0;
	}

	vid_w = w;
	vid_h = h;

	return 1;
}

bool cMenu_Options_Video :: bpp_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	unsigned int bpp = string_to_int( item->getText().c_str() );

	if( !pVideo->Test_Video( vid_w, vid_h, bpp ) )
	{
		return 0;
	}

	vid_bpp = bpp;

	return 1;
}

bool cMenu_Options_Video :: fullscreen_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bfullscreen = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		bfullscreen = 1;
	}

	vid_fullscreen = bfullscreen;

	return 1;
}

bool cMenu_Options_Video :: vsync_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bvsync = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		bvsync = 1;
	}

	vid_vsync = bvsync;

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Audio *** *** *** *** *** *** *** *** *** */

cMenu_Options_Audio :: cMenu_Options_Audio( void )
: cMenu_Options()
{
	music = new cHudSprite();
	sounds = new cHudSprite();
}

cMenu_Options_Audio :: ~cMenu_Options_Audio( void )
{
	delete music;
	delete sounds;
}

void cMenu_Options_Audio :: Init( void )
{
	cMenu_Options::Init();

	// Audio
	music->Set_Image( pVideo->Get_Surface( "menu/items/music.png" ), 1 );
	sounds->Set_Image( pVideo->Get_Surface( "menu/items/sounds.png" ), 1 );

	cMenu_Item *temp_item = NULL;

	GL_Surface *audio_hz = pFont->RenderText( pFont->font_normal, "Hz", text_color );
	GL_Surface *audio_music = pFont->RenderText( pFont->font_normal, "Music", text_color );
	GL_Surface *audio_sounds = pFont->RenderText( pFont->font_normal, "Sounds", text_color );

	layout_file = "menu_audio.layout";
	menu_posy = 220;


	// Audio Hz
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( audio_hz );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( audio_hz, -200, 0, 1 ) );

	// Music
	temp_item = new cMenu_Item();
	menu_posy += 85;
	temp_item->image_default->Set_Image( audio_music );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	music->Set_Pos( GAME_RES_W / 1.8f + music->col_rect.w + 50, menu_posy - 5 );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( audio_music, -200, 0, 1 ) );
	// Sounds
	temp_item = new cMenu_Item();
	menu_posy += 55;
	temp_item->image_default->Set_Image( audio_sounds );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	sounds->Set_Pos( GAME_RES_W / 1.8f + sounds->col_rect.w + 50, menu_posy - 5 );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5, grey );
	drawlist.push_back( new cHudSprite( audio_sounds, -200, 0, 1 ) );

	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/audio.png" ), GAME_RES_W * 0.47f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/audio.png" ), GAME_RES_W / 2.6f, 140 ) );
	
	pMenuCore->handler->Set_Active( MENU_AUDIO );

	Init_GUI();
}

void cMenu_Options_Audio :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Audio Hz
	Combobox *combo_audio_hz = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_audio_hz" ));

	ListboxTextItem *item = new ListboxTextItem( "44800", 0 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_audio_hz->addItem( item );
	item = new ListboxTextItem( "44100", 1 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_audio_hz->addItem( item );
	item = new ListboxTextItem( "22050", 1 );
	item->setTextColours( colour( 1, 0, 0 ) );
	combo_audio_hz->addItem( item );

	// Set current value
	combo_audio_hz->setText( int_to_string( pPreferences->audio_hz ) );

	combo_audio_hz->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Audio::Audio_Hz_Select, this ) );


	// Music
	Combobox *combo_music = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_music" ));

	item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_music->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_music->addItem( item );

	if( pAudio->music_enabled )
	{
		combo_music->setText( "On" );
	}
	else
	{
		combo_music->setText( "Off" );
	}

	combo_music->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Audio::music_Select, this ) );

	// music volume slider
	Slider *slider_music = (Slider *)WindowManager::getSingleton().getWindow( "slider_music_volume" );
	slider_music->setCurrentValue( static_cast<float>(pAudio->music_volume) );
	slider_music->subscribeEvent( Slider::EventValueChanged, Event::Subscriber( &cMenu_Options_Audio::music_vol_changed, this ) );
	

	// Sounds
	Combobox *combo_sounds = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_sounds" ));

	item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_sounds->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 1, 0.6f, 0.3f ) );
	combo_sounds->addItem( item );

	if( pAudio->sound_enabled )
	{
		combo_sounds->setText( "On" );
	}
	else
	{
		combo_sounds->setText( "Off" );
	}

	combo_sounds->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Audio::sound_Select, this ) );

	// sound volume slider
	Slider *slider_sound = (Slider *)WindowManager::getSingleton().getWindow( "slider_sound_volume" );
	slider_sound->setCurrentValue( static_cast<float>(pAudio->sound_volume) );
	slider_sound->subscribeEvent( Slider::EventValueChanged, Event::Subscriber( &cMenu_Options_Audio::sound_vol_changed, this ) );
}

void cMenu_Options_Audio :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// only audio actions
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// Music
	if( pMenuCore->handler->active == 5 )
	{
		pAudio->ToggleMusic();

		if( pAudio->music_enabled )
		{
			WindowManager::getSingleton().getWindow( "combo_music" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_music" )->setText( "Off" );
		}
	}
	// Sounds
	else if( pMenuCore->handler->active == 6 )
	{
		pAudio->ToggleSounds();

		if( pAudio->sound_enabled )
		{
			WindowManager::getSingleton().getWindow( "combo_sounds" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_sounds" )->setText( "Off" );
		}
	}
}

void cMenu_Options_Audio :: Draw( void )
{
	cMenu_Options::Draw();

	// Music image
	if( pAudio->music_enabled )
	{
		music->Draw();
	}
	// Sounds image
	if( pAudio->sound_enabled )
	{
		sounds->Draw();
	}

	Draw_End();
}

bool cMenu_Options_Audio :: Audio_Hz_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	pPreferences->audio_hz = string_to_int( item->getText().c_str() );
	// todo : add Audio function to reload sounds and music when needed
	//pAudio->Init( pPreferences->Sounds, pPreferences->Music );
	Draw_StaticText( "You need to restart for the change to take effect.", &white, &lightgreyalpha64 );

	return 1;
}

bool cMenu_Options_Audio :: music_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bmusic = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		bmusic = 1;
	}

	if( pAudio->music_enabled != bmusic )
	{
		pAudio->ToggleMusic();
	}

	return 1;
}

bool cMenu_Options_Audio :: music_vol_changed( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Uint8 val = static_cast<Uint8>(static_cast<Slider *>( windowEventArgs.window )->getCurrentValue());

	pAudio->SetMusicVolume( val );

	return 1;
}

bool cMenu_Options_Audio :: sound_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	bool bsound = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		bsound= 1;
	}

	if( pAudio->sound_enabled != bsound )
	{
		pAudio->ToggleSounds();
	}

	return 1;
}

bool cMenu_Options_Audio :: sound_vol_changed( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Uint8 val = static_cast<Uint8>(static_cast<Slider *>( windowEventArgs.window )->getCurrentValue());

	pAudio->SetSoundVolume( val );

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Options_Controls *** *** *** *** *** *** *** *** *** */

cMenu_Options_Controls :: cMenu_Options_Controls( void )
: cMenu_Options()
{
	// Keyboard
	gamepad = new cHudSprite();
	keyboard_up_keyname = new cHudSprite( NULL, 0, -200, 0 );
	keyboard_up_keyname->Set_Shadow( grey, 1.5f );
	keyboard_down_keyname = keyboard_up_keyname->Copy();
	keyboard_left_keyname = keyboard_up_keyname->Copy();
	keyboard_right_keyname = keyboard_up_keyname->Copy();
	keyboard_jump_keyname = keyboard_up_keyname->Copy();
	keyboard_shoot_keyname = keyboard_up_keyname->Copy();
	keyboard_action_keyname = keyboard_up_keyname->Copy();
	// Joypad
	joypad_jump_keyname = keyboard_up_keyname->Copy();
	joypad_item_keyname = keyboard_up_keyname->Copy();
	joypad_shoot_keyname = keyboard_up_keyname->Copy();
	joypad_action_keyname = keyboard_up_keyname->Copy();
	joypad_exit_keyname = keyboard_up_keyname->Copy();
}

cMenu_Options_Controls :: ~cMenu_Options_Controls( void )
{
	delete gamepad;
	// Keyboard
	delete keyboard_up_keyname;
	delete keyboard_down_keyname;
	delete keyboard_left_keyname;
	delete keyboard_right_keyname;
	delete keyboard_jump_keyname;
	delete keyboard_shoot_keyname;
	delete keyboard_action_keyname;
	// Joypad
	delete joypad_jump_keyname;
	delete joypad_item_keyname;
	delete joypad_shoot_keyname;
	delete joypad_action_keyname;
	delete joypad_exit_keyname;
}

void cMenu_Options_Controls :: Init( void )
{
	cMenu_Options::Init();

	// gamepad
	gamepad->Set_Image( pVideo->Get_Surface( "menu/items/gamepad.png" ), 1 );
	// Keyboard
	keyboard_up_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_up ), text_color_value ), 1, 1 );
	keyboard_down_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_down ), text_color_value ), 1, 1 );
	keyboard_left_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_left ), text_color_value ), 1, 1 );
	keyboard_right_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_right ), text_color_value ), 1, 1 );
	keyboard_jump_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_jump ), text_color_value ), 1, 1 );
	keyboard_shoot_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_shoot ), text_color_value ), 1, 1 );
	keyboard_action_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_action ), text_color_value ), 1, 1 );
	// Joystick/Gamepad
	joypad_jump_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_jump ), text_color_value ), 1, 1 );
	joypad_item_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_item ), text_color_value ), 1, 1 );
	joypad_shoot_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_shoot ), text_color_value ), 1, 1 );
	joypad_action_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_action ), text_color_value ), 1, 1 );
	joypad_exit_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_exit ), text_color_value ), 1, 1 );

	cMenu_Item *temp_item = NULL;

	GL_Surface *keyboard_up = pFont->RenderText( pFont->font_normal, "Up", text_color );
	GL_Surface *keyboard_down = pFont->RenderText( pFont->font_normal, "Down", text_color );
	GL_Surface *keyboard_left = pFont->RenderText( pFont->font_normal, "Left", text_color );
	GL_Surface *keyboard_right = pFont->RenderText( pFont->font_normal, "Right", text_color );
	GL_Surface *keyboard_jump = pFont->RenderText( pFont->font_normal, "Jump", text_color );
	GL_Surface *keyboard_shoot = pFont->RenderText( pFont->font_normal, "Shoot", text_color );
	GL_Surface *keyboard_action = pFont->RenderText( pFont->font_normal, "Action/Run", text_color );

	GL_Surface *joypad_jump = pFont->RenderText( pFont->font_normal, "Jump", text_color );
	GL_Surface *joypad_item = pFont->RenderText( pFont->font_normal, "Item", text_color );
	GL_Surface *joypad_shoot = pFont->RenderText( pFont->font_normal, "Shoot", text_color );
	GL_Surface *joypad_action = pFont->RenderText( pFont->font_normal, "Action/Run", text_color );
	GL_Surface *joypad_exit = pFont->RenderText( pFont->font_normal, "Exit", text_color );

	GL_Surface *controls_joy_enabled = pFont->RenderText( pFont->font_normal, "Joystick", text_color );
	GL_Surface *controls_joy_analog_jump = pFont->RenderText( pFont->font_normal, "Analog Jump", text_color );

	layout_file = "menu_controls.layout";
	menu_posy = 220;

	// ## Keyboard
	// Up
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( keyboard_up );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_up_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_up, -200, 0, 1 ) );
	// Down
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_down );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_down_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_down, -200, 0, 1 ) );
	// Left
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_left );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_left_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_left, -200, 0, 1 ) );
	// Right
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_right );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_right_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_right, -200, 0, 1 ) );
	// Jump
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_jump );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_jump_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_jump, -200, 0, 1 ) );
	// Shoot
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_shoot );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_shoot_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_shoot, -200, 0, 1 ) );
	// Action
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( keyboard_action );
	temp_item->Set_Pos( GAME_RES_W / 3, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	keyboard_action_keyname->Set_Pos( GAME_RES_W / 2, menu_posy );
	drawlist.push_back( new cHudSprite( keyboard_action, -200, 0, 1 ) );

	// ## right side
	menu_posy = 220;

	// ## Joypad
	// Jump
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( joypad_jump );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	joypad_jump_keyname->Set_Pos( GAME_RES_W / 1.25f, menu_posy );
	drawlist.push_back( new cHudSprite( joypad_jump, -200, 0, 1 ) );
	// Item
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( joypad_item );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	joypad_item_keyname->Set_Pos( GAME_RES_W / 1.25f, menu_posy );
	drawlist.push_back( new cHudSprite( joypad_item, -200, 0, 1 ) );
	// Shoot
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( joypad_shoot );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	joypad_shoot_keyname->Set_Pos( GAME_RES_W / 1.25f, menu_posy );
	drawlist.push_back( new cHudSprite( joypad_shoot, -200, 0, 1 ) );
	// Action
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( joypad_action );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	joypad_action_keyname->Set_Pos( GAME_RES_W / 1.25f, menu_posy );
	drawlist.push_back( new cHudSprite( joypad_action, -200, 0, 1 ) );
	// Exit
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( joypad_exit );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	joypad_exit_keyname->Set_Pos( GAME_RES_W / 1.25f, menu_posy );
	drawlist.push_back( new cHudSprite( joypad_exit, -200, 0, 1 ) );
	
	// use joystick
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( controls_joy_enabled );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	gamepad->Set_Pos( GAME_RES_W * 0.74f, menu_posy - gamepad->col_rect.h * 0.2f );
	drawlist.push_back( new cHudSprite( controls_joy_enabled, -200, 0, 1 ) );
	// allow analog jump
	temp_item = new cMenu_Item();
	menu_posy += 30;
	temp_item->image_default->Set_Image( controls_joy_analog_jump );
	temp_item->Set_Pos( GAME_RES_W / 1.6f, menu_posy );
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
	drawlist.push_back( new cHudSprite( controls_joy_analog_jump, -200, 0, 1 ) );


	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/controls.png" ), GAME_RES_W * 0.47f, 140 ) );
	drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/controls.png" ), GAME_RES_W / 2.6f, 140 ) );

	pMenuCore->handler->Set_Active( MENU_CONTROLS );

	Init_GUI();
}

void cMenu_Options_Controls :: Init_GUI( void )
{
	cMenu_Options::Init_GUI();

	// Joystick enabled
	Combobox *combo_joy = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_joy" ));

	// Add None
	ListboxTextItem *item = new ListboxTextItem( "None", 0 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_joy->addItem( item );

	// Add all Joy names
	vector<string> joy_names = pJoystick->Get_Names();

	for( unsigned int i = 0; i < joy_names.size(); i++ )
	{
		item = new ListboxTextItem( joy_names[i], 1 );
		item->setTextColours( colour( 0.3f, 1, 0.3f ) );
		combo_joy->addItem( item );
	}

	// Selected Item
	ListboxTextItem *selected_item = NULL;

	// Set current Joy name
	if( pPreferences->joy_enabled )
	{
		selected_item = static_cast<ListboxTextItem *>( combo_joy->findItemWithText( pJoystick->Get_Name(), NULL ) );
	}
	// disabled
	else
	{
		selected_item = static_cast<ListboxTextItem *>( combo_joy->getListboxItemFromIndex( 0 ) );
	}
	// set Item
	combo_joy->setText( selected_item->getText() );

	combo_joy->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Controls::joy_name_Select, this ) );

	// Joystick analog jump
	Combobox *combo_joy_analog_jump = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" ));

	item = new ListboxTextItem( "On", 0 );
	item->setTextColours( colour( 0, 1, 0 ) );
	combo_joy_analog_jump->addItem( item );
	item = new ListboxTextItem( "Off", 1 );
	item->setTextColours( colour( 0, 0, 1 ) );
	combo_joy_analog_jump->addItem( item );

	if( pPreferences->joy_analog_jump )
	{
		combo_joy_analog_jump->setText( "On" );
	}
	else
	{
		combo_joy_analog_jump->setText( "Off" );
	}

	combo_joy_analog_jump->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMenu_Options_Controls::joy_analog_jump_Select, this ) );
}

void cMenu_Options_Controls :: Update( void )
{
	cMenu_Options::Update();

	if( !action )
	{
		return;
	}

	cMenu_Options::Update();

	// Controls
	if( pMenuCore->handler->active <= 4 )
	{
		return;
	}

	action = 0;

	// Joystick name
	if( pMenuCore->handler->active == 16 )
	{
		// Get Joy Combo
		Combobox *combo_joy = static_cast<Combobox *>( WindowManager::getSingleton().getWindow( "combo_joy" ) );
		
		// selected item id
		int selected_item = 0;
		// current list item
		ListboxTextItem *list_item = static_cast<ListboxTextItem *>( combo_joy->findItemWithText( combo_joy->getText(), NULL ) );

		// if selected
		if( list_item )
		{
			selected_item = combo_joy->getItemIndex( list_item );
		}

		// select first
		if( selected_item >= SDL_NumJoysticks() )
		{
			selected_item = 0;
		}
		// select next item
		else
		{
			selected_item++;
		}

		// Disable Joy
		if( selected_item == 0 )
		{
			Joy_Disable();
		}
		// Select Joy
		else
		{
			Joy_Default( selected_item - 1 );
		}

		// check if initialization succeeded
		if( selected_item )
		{
			// initialised
			if( pPreferences->joy_enabled )
			{
				Draw_StaticText( "Enabled : " + pJoystick->Get_Name(), &yellow );
			}
			// failed
			else
			{
				selected_item = 0;
			}
		}

		combo_joy->setText( combo_joy->getListboxItemFromIndex( selected_item )->getText() );
	}

	// Joystick analog jump
	if( pMenuCore->handler->active == 17 )
	{
		pPreferences->joy_analog_jump = !pPreferences->joy_analog_jump;

		if( pPreferences->joy_analog_jump )
		{
			WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( "combo_joy_analog_jump" )->setText( "Off" );
		}
	}

	// Keys / Joy Buttons
	if( pMenuCore->handler->active > 16 )
	{
		return;
	}

	Draw_StaticText( "Press a key. ( ESC to cancel )", &orange, NULL, 0 );

	bool sub_done = 0;

	while( !sub_done )
	{
		// no event
		if( !SDL_PollEvent( &input_event ) )
		{
			continue;
		}

		if( input_event.type != SDL_KEYDOWN && input_event.type != SDL_JOYBUTTONDOWN )
		{
			continue;
		}

		if( input_event.key.keysym.sym == SDLK_ESCAPE || input_event.key.keysym.sym == SDLK_BACKSPACE )
		{
			sub_done = 1;
			break;
		}

		switch( pMenuCore->handler->active )
		{
		// up
		case 5:
		{
			pPreferences->key_up = input_event.key.keysym.sym; 
			keyboard_up_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_up ), black ), 1, 1 );
			break;
		}
		// down
		case 6:
		{
			pPreferences->key_down = input_event.key.keysym.sym; 
			keyboard_down_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_down ), black ), 1, 1 );
			break;
		}
		// left
		case 7:
		{
			pPreferences->key_left = input_event.key.keysym.sym; 
			keyboard_left_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_left ), black ), 1, 1 );
			break;
		}
		// right
		case 8:
		{
			pPreferences->key_right = input_event.key.keysym.sym; 
			keyboard_right_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_right ), black ), 1, 1 );
			break;
		}
		// jump
		case 9:
		{
			pPreferences->key_jump = input_event.key.keysym.sym; 
			keyboard_jump_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_jump ), black ), 1, 1 );
			break;
		}
		// shoot
		case 10:
		{
			pPreferences->key_shoot = input_event.key.keysym.sym; 
			keyboard_shoot_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_shoot ), black ), 1, 1 );
			break;
		}
		// action
		case 11:
		{
			pPreferences->key_action = input_event.key.keysym.sym; 
			keyboard_action_keyname->Set_Image( pFont->RenderText( pFont->font_normal, SDL_GetKeyName( pPreferences->key_action ), black ), 1, 1 );
			break;
		}
		// joy jump
		case 12:
		{
			pPreferences->joy_button_jump = input_event.jbutton.button; 
			joypad_jump_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_jump ), black ), 1, 1 );
			break;
		}
		// joy item
		case 13:
		{
			pPreferences->joy_button_item = input_event.jbutton.button; 
			joypad_item_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_item ), black ), 1, 1 );
			break;
		}
		// joy shoot
		case 14:
		{
			pPreferences->joy_button_shoot = input_event.jbutton.button; 
			joypad_shoot_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_shoot ), black ), 1, 1 );
			break;
		}
		// joy action
		case 15:
		{
			pPreferences->joy_button_action = input_event.jbutton.button; 
			joypad_action_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_action ), black ), 1, 1 );
			break;
		}
		// joy exit
		case 16:
		{
			pPreferences->joy_button_exit = input_event.jbutton.button; 
			joypad_exit_keyname->Set_Image( pFont->RenderText( pFont->font_normal, int_to_string( pPreferences->joy_button_exit ), black ), 1, 1 );
			break;
		}
		default:
		{
			break;
		}
		}

		sub_done = 1;
	}
}

void cMenu_Options_Controls :: Draw( void )
{
	cMenu_Options::Draw();

	// Keyboard
	keyboard_up_keyname->Draw();
	keyboard_down_keyname->Draw();
	keyboard_left_keyname->Draw();
	keyboard_right_keyname->Draw();
	keyboard_jump_keyname->Draw();
	keyboard_shoot_keyname->Draw();
	keyboard_action_keyname->Draw();
	// Joypad
	joypad_jump_keyname->Draw();
	joypad_item_keyname->Draw();
	joypad_shoot_keyname->Draw();
	joypad_action_keyname->Draw();
	joypad_exit_keyname->Draw();

	if( pPreferences->joy_enabled )
	{
		gamepad->Draw();
	}

	Draw_End();
}

void cMenu_Options_Controls :: Joy_Default( unsigned int index )
{
	pPreferences->joy_enabled = 1;
	pPreferences->joy_name = SDL_JoystickName( index );

	// initialize and if no joystick available disable
	pJoystick->Init();
}

void cMenu_Options_Controls :: Joy_Disable( void )
{
	pPreferences->joy_enabled = 0;
	pPreferences->joy_name.clear();

	// close the joystick
	pJoystick->Stick_Close();
}

bool cMenu_Options_Controls :: joy_name_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Combobox *combo = static_cast<Combobox*>( windowEventArgs.window );
	ListboxItem *item = combo->getSelectedItem();

	if( item->getText().compare( "None" ) == 0 )
	{
		Joy_Disable();
	}
	else
	{
		Joy_Default( combo->getItemIndex( item ) - 1 );
	}

	return 1;
}

bool cMenu_Options_Controls :: joy_analog_jump_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool analog_jump = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		analog_jump = 1;
	}

	pPreferences->joy_analog_jump = analog_jump;

	return 1;
}

/* *** *** *** *** *** *** *** *** cMenu_Savegames *** *** *** *** *** *** *** *** *** */

cMenu_Savegames :: cMenu_Savegames( bool ntype_save )
: cMenu_Base()
{
	type_save = ntype_save;

	for( unsigned int i = 0; i < 9; i++ )
	{
		savegame_temp.push_back( new cHudSprite() );
	}
}

cMenu_Savegames :: ~cMenu_Savegames( void )
{
	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	savegame_temp.clear();
}

void cMenu_Savegames :: Init( void )
{
	cMenu_Base::Init();

	cMenu_Item *temp_item = NULL;

	GL_Surface *back1 = pFont->RenderText( pFont->font_normal, "Back", text_color );

	Update_SavedGames_Text();

	// savegame descriptions
	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		temp_item = new cMenu_Item();
		temp_item->image_default->Set_Image( (*itr)->image );
		temp_item->Set_Pos( GAME_RES_W / 5, menu_posy );
		pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );
		
		menu_posy += temp_item->image_default->col_rect.h;
	}

	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( GAME_RES_W / 18, 450 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_MenuItem( temp_item, 1.5f, grey );

	if( type_save )
	{
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/save.png" ), GAME_RES_W * 0.5f, GAME_RES_H * 0.1f ) );
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/save.png" ), GAME_RES_W * 0.07f, GAME_RES_H * 0.24f ) );
	}
	else
	{
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/load.png" ), GAME_RES_W * 0.5f, GAME_RES_H * 0.1f ) );
		drawlist.push_back( new cHudSprite( pVideo->Get_Surface( "menu/items/load.png" ), GAME_RES_W * 0.07f, GAME_RES_H * 0.24f ) );
	}
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Savegames :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Savegames :: Update( void )
{
	cMenu_Base::Update();

	if( !action )
	{
		return;
	}

	action = 0;

	// back
	if( pMenuCore->handler->active == 9 )
	{
		Exit();
		return;
	}

	if( !type_save )
	{
		Update_Load();
	}
	else
	{
		Update_Save();
	}
}

void cMenu_Savegames :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Savegames :: Exit( void )
{
	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}

void cMenu_Savegames :: Update_Load( void )
{
	// savegame
	if( pSavegame->is_valid( pMenuCore->handler->active + 1 ) )
	{
		pAudio->FadeOutMusic( 1000 );
		pAudio->PlaySound( "savegame_load.ogg" );

		int save_type = pSavegame->Load_Game( pMenuCore->handler->active + 1 );

		// Level save
		if( save_type == 1 )
		{
			Game_Action = GA_ENTER_LEVEL;
		}
		// World save
		else if( save_type == 2 )
		{
			Game_Action = GA_ENTER_WORLD;
		}
	}
}

void cMenu_Savegames :: Update_Save( void )
{
	// savegame
	if( pOverworld_Player->current_waypoint >= 0 || pLevel->is_Loaded() )
	{
		string descripion = Set_SaveDescription( pMenuCore->handler->active + 1 );
		
		pFramerate->Reset();

		if( descripion.compare( "Not enough Points" ) == 0 ) 
		{
			Game_Action = GA_ENTER_MENU;
			pMenuCore->next_menu = MENU_MAIN;
			return;
		}

		if( descripion.empty() )
		{
			return;
		}

		pAudio->PlaySound( "savegame_save.ogg" );

		// no costs in debug builds
#ifndef _DEBUG
		if( pLevel->is_Loaded() )
		{
			pointsdisplay->Set_Points( pPlayer->points - 3000 );
		}
#endif
		// save
		pSavegame->Save_Game( pMenuCore->handler->active + 1, descripion );

		Game_Action = GA_ENTER_MENU;
		pMenuCore->next_menu = MENU_MAIN;
	}
}
string cMenu_Savegames :: Set_SaveDescription( unsigned int save_slot )
{
	if( save_slot == 0 || save_slot > 9 )
	{
		return "";
	}
// save always in debug builds
#ifndef _DEBUG
	if( pLevel->is_Loaded() && pPlayer->points < 3000 )
	{
		ClearInputEvents();
		Draw_StaticText( "3000 Points needed for saving in a level.\nSaving on the Overworld is free." );

		return "Not enough Points";
	}
#endif
	string save_description;

	bool auto_erase_description = 0;

	// if Savegame exists use old description
	if( pSavegame->is_valid( save_slot ) )
	{
		save_description.clear();
		// get only the description
		save_description = pSavegame->Get_Description( save_slot, 1 );
	}
	else
	{
		// use default description
		save_description = "No Description";
		auto_erase_description = 1;
	}

	return Box_Text_Input( save_description, "Enter Description", auto_erase_description );
}

void cMenu_Savegames :: Update_SavedGames_Text( void )
{
	unsigned int save_slot = 0;

	for( HudSpriteList::iterator itr = savegame_temp.begin(), itr_end = savegame_temp.end(); itr != itr_end; ++itr )
	{
		save_slot++;
		(*itr)->Set_Image( pFont->RenderText( pFont->font_normal, pSavegame->Get_Description( save_slot ), text_color_value ), 1, 1 );
	}
}

/* *** *** *** *** *** *** *** *** cMenu_Credits *** *** *** *** *** *** *** *** *** */

cMenu_Credits :: cMenu_Credits( void )
: cMenu_Base()
{

}

cMenu_Credits :: ~cMenu_Credits( void )
{

}

void cMenu_Credits :: Init( void )
{
	pAudio->PlayMusic( "land/hyper_1.ogg", -1, 1, 1500 );

	cMenu_Base::Init();

	pAudio->FadeOutMusic( 1500 );

	cMenu_Item *temp_item = NULL;
	menu_posy = GAME_RES_H * 1.1f;

	// fade out
	Color color = black;

	for( float i = 0; i < 180; i += 4.5f * pFramerate->speedfactor )
	{
		color.alpha = static_cast<Uint8>(i);

		// create request
		cRectRequest *request = new cRectRequest();
		
		// Draw
		pVideo->Clear_Screen();
		pVideo->Draw_Rect( NULL, 0.09f, &color, request );
		pMenuCore->handler->Draw();
		pMenuCore->pMenu_AnimManager->Draw();
		
		request->render_count = 2;

		// add request
		pRenderer->Add( request );

		pVideo->Render();

		pFramerate->Update();
		// maximum fps
		CorrectFrameTime( 100 );
	}

	// black background
	glClearColor( 0, 0, 0, 1 );
	// set menu gradient colors
	pMenuCore->handler->color_start = Color( 0, 0, 0.01f );
	pMenuCore->handler->color_end = Color( 0, 0.1f, 0.2f );

	GL_Surface *back1 = pFont->RenderText( pFont->font_normal, "Back", text_color );

	// clear credits
	drawlist.clear();

	// add credits texts
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Florian Richter (FluXy)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( orange, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Dedicated Programmer, Organizer, and Leader", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Holger Fey (Nemo)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lila, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Torrent Packager, Quality Assurance", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Grant ... (youngheart80)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( green, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphics", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Tristan Heaven (nyhm)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Gentoo eBuild Maintainer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Mario Fink (maYO)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( blue, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphics, Website Images, Other Support", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Mark Richards (dteck)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( blue, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphics", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Ursula ... (Pipgirl)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.2f, 0.9f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphics", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	// Retired
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "-- Retired --", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightgrey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Tobias Maasland (Weirdnose)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.9f, 0.7f, 0.2f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Level and World Contributor, Assistant Developer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Markus Hiebl (Frostbringer)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( Color( 0.9f, 0.1f, 0.8f ), 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphic Designer and Level Contributor", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Robert ... (Consonance)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightred, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Sound and Music Artist", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Justin ... (LoXodonte)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightblue, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Music Artist", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Matt J... (mattwj)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( red, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   eDonkey Packager, Quality Assurance", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Bodhi Crandall-Rus (Boder)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( green, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Assistant Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "John Daly (Johnlein)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( greenyellow, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Gustavo Gutierrez (Enzakun)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( greenyellow, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Maryo Graphic Designer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Thomas Huth (Thothy)", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( greenyellow, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "   Linux Maintainer", white ), 0, -3, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	// Thanks
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "-- Thanks --", white ), 0, 20, 1 ) );
	drawlist.back()->Set_Shadow( lightblue, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, " Jason Cox (XOC), Ricardo Cruz, Devendra (Yuki),", white ), 0, 0, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );
	drawlist.push_back( new cHudSprite( pFont->RenderText( pFont->font_normal, "Hans de Goede (Hans), xPatrickx, rolosworld", white ), 0, 0, 1 ) );
	drawlist.back()->Set_Shadow( grey, 1 );

	// set credits position
	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		cHudSprite *obj = (*itr);
		obj->posx += GAME_RES_W / 5;
		obj->posy += menu_posy;
		// behind ground
		obj->posz = 0.09f;
		// set color combine
		obj->Set_Color_Combine( 0, 0, 0, GL_MODULATE );
		obj->color.alpha = 0;
		obj->shadow_color.alpha = 0;

		menu_posy = obj->posy + obj->col_rect.h;
	}

	// back
	temp_item = new cMenu_Item();
	temp_item->image_default->Set_Image( back1 );
	temp_item->Set_Pos( GAME_RES_W / 18, 250 );
	temp_item->isquit = 1;
	pMenuCore->handler->Add_MenuItem( temp_item, 2, grey );
	
	drawlist.push_back( new cHudSprite( back1, -200, 0, 1 ) );

	Init_GUI();
}

void cMenu_Credits :: Init_GUI( void )
{
	cMenu_Base::Init_GUI();
}

void cMenu_Credits :: Update( void )
{
	cMenu_Base::Update();

	for( HudSpriteList::iterator itr = drawlist.begin(), itr_end = drawlist.end(); itr != itr_end; ++itr )
	{
		cHudSprite *obj = (*itr);

		// long inactive reset
		if( obj->posy < -1000 )
		{
			obj->Set_PosY( GAME_RES_H * 1.1 );
		}
		// fading out
		else if( obj->posy < GAME_RES_H * 0.3f )
		{
			float new_value = obj->combine_col[0] - ( pFramerate->speedfactor * 0.01f );

			if( new_value < 0 )
			{
				new_value = 0;
			}

			obj->Set_Color_Combine( new_value, new_value, new_value, GL_MODULATE );
			obj->color.alpha = static_cast<Uint8>( new_value * 255 );
			obj->shadow_color.alpha = obj->color.alpha;
		}
		// fading in
		else if( obj->posy < GAME_RES_H * 0.9f )
		{
			float new_value = obj->combine_col[0] + ( pFramerate->speedfactor * 0.01f );

			if( new_value > 1 )
			{
				new_value = 1;

				// add particles
				if( obj->combine_col[0] < 1 )
				{
					cParticleAnimation *anim = new cParticleAnimation( static_cast<float>( rand() % GAME_RES_W ), static_cast<float>( rand() % 200 ) );
					anim->Set_Quota( rand() % 25 );
					anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
					anim->Set_DirectionRange( 0, 360 );
					anim->Set_Time_to_Live( 1.2f, 1 );
					anim->Set_Fading_Size( 1 );
					anim->Set_Scale( 0.2f, 0.1f );
					anim->Set_Color( Color( static_cast<Uint8>( 100 + ( rand() % 155 ) ), 100 + ( rand() % 155 ), 100 + ( rand() % 155 ) ) );
					anim->Set_Speed( 2, 1 );
					anim->Set_ConstRotationZ( 1, 8 );
					anim->Set_ZPos( 0.16f );
					pMenuCore->pMenu_AnimManager->Add( anim );
				}
			}

			obj->Set_Color_Combine( new_value, new_value, new_value, GL_MODULATE );
			obj->color.alpha = static_cast<Uint8>( new_value * 255 );
			obj->shadow_color.alpha = obj->color.alpha;
		}

		// default upwards scroll
		obj->Move( 0, -1.1f );
	}

	if( !action )
	{
		return;
	}

	action = 0;

	// back
	if( pMenuCore->handler->active == 0 )
	{
		Exit();
	}
}

void cMenu_Credits :: Draw( void )
{
	cMenu_Base::Draw();
	Draw_End();
}

void cMenu_Credits :: Exit( void )
{
	// stop credits music
	pAudio->FadeOutMusic();
	pAudio->PlayMusic( "game/menu.ogg", -1, 0, 1500 );

	pMenuCore->next_menu = MENU_MAIN;
	Game_Action = GA_ENTER_MENU;
}
