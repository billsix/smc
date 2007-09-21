/***************************************************************************
           main.cpp  -  main routines and initialization
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
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
#include "../core/game_core.h"
#include "../core/main.h"
#include "../level/level.h"
#include "../gui/menu.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../video/video.h"
#include "../video/font.h"
#include "../user/preferences.h"
#include "../video/img_manager.h"
#include "../audio/sound_manager.h"
#include "../core/obj_manager.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../input/joystick.h"
#include "../overworld/worlds.h"
#include "../overworld/overworld.h"
#include "../player/player.h"
#include "../input/mouse.h"
#include "../user/savegame.h"
#include "../input/keyboard.h"
#include "../video/renderer.h"
#include "../video/img_settings.h"
#include "../objects/levelexit.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main( int argc, char **argv )
{
	if( argc >= 2 )
	{
		if( strcmp( argv[1], "--help" ) == 0 || strcmp( argv[1], "-h" ) == 0 )
		{
			printf( "%s V.%s\n\n", CAPTION, VERSION );
			printf( "Usage: %s [OPTIONS] [LEVELFILE]\n", argv[0] );
			printf( "Where LEVELFILE is the name of the level to play or OPTIONS is one of the following.\n" );
			printf( "-h, --help\tDisplay this message\n" );
			printf( "-v, --version\tShow the version of this binary\n" );
			return 0;
		}
		else if( strcmp( argv[1], "--version" ) == 0 || strcmp( argv[1], "-v" ) == 0 )
		{
			printf( "%s %s\n", CAPTION, VERSION );
			return 0;
		}
		else
		{
			printf( "Unknown argument %s\n", argv[1] );
		}
	}

	// initialize everything
	InitGame();

	// command line level loading
	if( argc == 2 && strlen( argv[1] ) )
	{
		// set start menu active
		pMenuCore->Load( MENU_START );
		// enter level
		Game_Action = GA_ENTER_LEVEL;
		Game_Action_Data = argv[1];
	}

	// Game Loop
	while( !done )
	{
		// update
		Update_Game();
		// draw
		Draw_Game();
		// render
		pVideo->Render();

		// update speedfactor
		pFramerate->Update();
	}

	// exit
	ExitGame();
	// no errors
	return 0;
}

void InitGame( void )
{
	srand( (unsigned int)time( NULL ) );

	// Init Stage 1
	pCamera = new cCamera();
	pVideo = new cVideo();
	pFont = new cFont();
	pFramerate = new cFramerate();
	pRenderer = new cRenderQueue( 200 );
	pRenderer_GUI = new cRenderQueue( 5 );
	pPreferences = new cPreferences();
	pImageManager = new cImageManager();
	pSoundManager = new cSoundManager();
	pSettingsParser = new cImage_settings();

	/* We need to initialise SDL and set the videomode before initializing
	   CEGUI, because CEGUI creates and uses an OpenGL-renderer and OpenGL
	   calls may only be made with a valid OpenGL-context, which we get by
	   setting the videomode. This means that we cannot use pPreferences
	   in Init_SDL and Init_Video to determine the videomode settings,
	   because pPreferences needs the XMLParser from CEGUI. Thus we use
	   a set of default video-settings in Init_Video and later apply the
	   user preferences to the video-settings in pPreferences->Apply().
	*/
	pVideo->Init_SDL();
	pVideo->Init_Video();
	pVideo->Init_CEGUI();

	// Init Stage 2
	pPreferences->Load();

	pSprite_Manager = new cSprite_Manager();
	pAudio = new cAudio();

	pPreferences->Apply();
	pVideo->Init_CEGUI_data();
	pVideo->Init_BasicColors();
	
	Draw_Loading_Screen();

	// Init Stage 3
	pFont->Init();
	pPlayer = new cPlayer();
	pLevel_Editor = new cEditor_Level();
	pWorld_Editor = new cEditor_World();
	pMouseCursor = new cMouseCursor();
	pKeyboard = new cKeyboard();
	pJoystick = new cJoystick();
	pOverworld_Player = new cOverworld_Player();
	pOverworld_manager = new cOverworld_manager();
	
	pLevel = new cLevel();
	pHudManager = new cHudManager();
	pAnimationManager = new cAnimationManager();
	pMenuCore = new cMenuCore();

	pSavegame = new cSavegame();

	pMenuCore->Load();

	// only precache in release builds
#ifndef _DEBUG
	// load data into the memory
	Preload_images();
	Preload_sounds();
#endif
}

void ExitGame( void )
{
	if( pPreferences )
	{
		pPreferences->Save();
	}

	if( pAudio )
	{
		delete pAudio;
		pAudio = NULL;
	}

	if( pLevel )
	{
		delete pLevel;
		pLevel = NULL;
	}

	if( pPlayer )
	{
		delete pPlayer;
		pPlayer = NULL;
	}

	if( pHudManager )
	{
		delete pHudManager;
		pHudManager = NULL;
	}

	if( pImageManager )
	{
		delete pImageManager;
		pImageManager = NULL;
	}

	if( pSoundManager )
	{
		delete pSoundManager;
		pSoundManager = NULL;
	}

	if( pAnimationManager )
	{
		delete pAnimationManager;
		pAnimationManager = NULL;
	}

	if( pLevel_Editor )
	{
		delete pLevel_Editor;
		pLevel_Editor = NULL;
	}

	if( pPreferences )
	{
		delete pPreferences;
		pPreferences = NULL;
	}

	if( pSavegame )
	{
		delete pSavegame;
		pSavegame = NULL;
	}

	if( pMouseCursor )
	{
		delete pMouseCursor;
		pMouseCursor = NULL;
	}

	if( pJoystick )
	{
		delete pJoystick;
		pJoystick = NULL;
	}

	if( pKeyboard )
	{
		delete pKeyboard;
		pKeyboard = NULL;
	}

	if( pOverworld_manager )
	{
		delete pOverworld_manager;
		pOverworld_manager = NULL;
	}

	if( pOverworld_Player )
	{
		delete pOverworld_Player;
		pOverworld_Player = NULL;
	}

	if( pMenuCore )
	{
		delete pMenuCore;
		pMenuCore = NULL;
	}

	if( pRenderer )
	{
		delete pRenderer;
		pRenderer = NULL;
	}

	if( pRenderer_GUI )
	{
		delete pRenderer_GUI;
		pRenderer_GUI = NULL;
	}

	if( pGuiSystem )
	{
		delete pGuiSystem;
		pGuiSystem = NULL;
	}

	if( pGuiRenderer )
	{
		delete pGuiRenderer;
		pGuiRenderer = NULL;
	}

	if( pVideo )
	{
		delete pVideo;
		pVideo = NULL;
	}

	if( pCamera )
	{
		delete pCamera;
		pCamera = NULL;
	}

	if( pSettingsParser )
	{
		delete pSettingsParser;
		pSettingsParser = NULL;
	}

	if( strlen( SDL_GetError() ) > 0 )
	{
		printf( "Last known Error : %s\n", SDL_GetError() );
	}

	SDL_Quit();
}

bool Handle_Input_Global( SDL_Event *ev )
{
	switch( ev->type )
	{
		case SDL_QUIT:
		{
			done = 1;
			ClearInputEvents();

			// handle on all handlers
			return 0;
			break;
		}
		case SDL_KEYDOWN:
		{
			if( pKeyboard->Key_Down( ev->key.keysym.sym ) )
			{
				return 1;
			}
			break;
		}
		case SDL_KEYUP:
		{
			if( pKeyboard->Key_Up( ev->key.keysym.sym ) )
			{
				return 1;
			}
			break;
		}
		default: // other events
		{
			// mouse
			if( pMouseCursor->Handle_Event( ev ) )
			{
				return 1; 
			}
			// joystick
			if( pJoystick->Handle_Event( ev ) )
			{
				return 1;
			}

			// send events
			if( Game_Mode == MODE_LEVEL )
			{
				// editor events
				if( pLevel_Editor->enabled )
				{
					if( pLevel_Editor->Handle_Event( ev ) )
					{
						return 1;
					}
				}
			}
			else if( Game_Mode == MODE_OVERWORLD )
			{
				// editor events
				if( pWorld_Editor->enabled )
				{
					if( pWorld_Editor->Handle_Event( ev ) )
					{
						return 1;
					}
				}
			}
			else if( Game_Mode == MODE_MENU )
			{
				if( pMenuCore->Handle_Event( ev ) )
				{
					return 1;
				}
			}
			break;
		}
	}

	return 0;
}

void Update_Game( void )
{
	// don't update if exiting
	if( done )
	{
		return;
	}

	// ## game events
	// if in level mode
	if( Game_Mode == MODE_LEVEL )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			GameAction Current_Game_Action = Game_Action;
			string Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			Game_Action = GA_NONE;
			Game_Action_Data.clear();
			Game_Action_ptr = NULL;

			// handle player downgrade
			if( Current_Game_Action == GA_DOWNGRADE_PLAYER )
			{
				pPlayer->DownGrade( 0 );
			}
			// activate level exit
			else if( Current_Game_Action == GA_ACTIVATE_LEVEL_EXIT )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(Current_Game_Action_ptr);
				level_exit->Activate();
			}
			// Enter Level
			else if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// fade out
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
				{
					Draw_Effect_out( EFFECT_OUT_BLACK, 3 );
				}
				else
				{
					Draw_Effect_out();
				}
				// load new level
				pLevel->Load( Current_Game_Action_Data );
				// reset player
				pPlayer->Reset();
				pCamera->Center();

				// play new music
				if( pLevel->valid_music )
				{
					pAudio->PlayMusic( pLevel->musicfile, -1, 0, 1000 );
				}

				// fade in
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
				{
					Draw_Effect_in( EFFECT_IN_BLACK, 3 );
				}
				else
				{
					Draw_Effect_in();
				}
			}
			// Enter World
			else if( Current_Game_Action == GA_ENTER_WORLD )
			{
				// Random fade effect
				Draw_Effect_out();
				// delay unload level
				pLevel->Unload( 1 );
				// enter world
				pActive_Overworld->Enter();
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				Draw_Effect_out( EFFECT_OUT_BLACK, 3 );

				// custom level
				if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
				{
					pMenuCore->Load( MENU_START );

					// Get Tab Control
					TabControl *tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
					// Select Level Tab
					tabcontrol->setSelectedTab( "tab_level" );

					// Get Levels Listbox
					Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
					// Get Item
					ListboxItem *list_item = listbox_levels->findItemWithText( Get_filename( pLevel->data_file, 0, 0 ).c_str(), NULL );
					// select level
					if( list_item )
					{
						listbox_levels->setItemSelectState( list_item, 1 );
						listbox_levels->ensureItemIsVisible( list_item );
					}

					Game_Mode_Type = MODE_TYPE_DEFAULT;
				}
				// default menu
				else
				{
					pMenuCore->Load();
				}

				Draw_Effect_in( EFFECT_IN_BLACK, 3 );
			}
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			GameAction Current_Game_Action = Game_Action;
			string Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			Game_Action = GA_NONE;
			Game_Action_Data.clear();
			Game_Action_ptr = NULL;

			// Enter Level
			if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// fade out
				Draw_Effect_out( EFFECT_OUT_FIXED_COLORBOX );
				// change game mode
				Change_Game_Mode( MODE_LEVEL );
				if( !Current_Game_Action_Data.empty() )
				{
					// load new level
					pLevel->Load( Current_Game_Action_Data );
					// play new music
					if( pLevel->valid_music )
					{
						pAudio->PlayMusic( pLevel->musicfile, -1, 0, 1000 );
					}
				}
				// reset player
				pPlayer->Reset();
				// fade in
				Draw_Effect_in();
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				pMenuCore->Load();
			}
			// Enter Credits Menu
			else if( Current_Game_Action == GA_ENTER_MENU_CREDITS )
			{
				pMenuCore->Load( MENU_CREDITS );
			}
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// if Game Action set
		while( Game_Action != GA_NONE )
		{
			GameAction Current_Game_Action = Game_Action;
			string Current_Game_Action_Data = Game_Action_Data;
			void *Current_Game_Action_ptr = Game_Action_ptr;
			Game_Action = GA_NONE;
			Game_Action_Data.clear();
			Game_Action_ptr = NULL;

			// Enter World
			if( Current_Game_Action == GA_ENTER_WORLD )
			{
				pMenuCore->Unload();
				// fade music out
				pAudio->FadeOutMusic( 1000 );
				// enter world
				pActive_Overworld->Enter();
			}
			// Enter Level
			if( Current_Game_Action == GA_ENTER_LEVEL )
			{
				// if level not loaded or entering custom level
				if( !pLevel->is_Loaded() || !Current_Game_Action_Data.empty() )
				{
					// fade music out
					pAudio->FadeOutMusic( 1000 );
				}
				// fade out
				Draw_Effect_out( EFFECT_OUT_BLACK, 3 );
				// load custom level
				if( !Current_Game_Action_Data.empty() )
				{
					// load new level
					pLevel->Load( Current_Game_Action_Data );
					// reset player
					pPlayer->Reset();
				}
				// change mode
				Change_Game_Mode( MODE_LEVEL );
				// if custom level
				if( pMenuCore->menu_id == MENU_START )
				{
					Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
				}
				// unload menu
				pMenuCore->Unload();
				// fade in
				Draw_Effect_in( EFFECT_IN_BLACK, 3 );
			}
			// Enter Menu
			else if( Current_Game_Action == GA_ENTER_MENU )
			{
				if( pMenuCore->next_menu != MENU_NOTHING )
				{
					pMenuCore->Load( pMenuCore->next_menu );
				}
			}
		}
	}

	// ## input
	// todo : move Update_Position() back to SDL_MOUSEMOTION with disabled mover_mode ? ( also to camera move() )
	pMouseCursor->Update_Position();
	pMouseCursor->Update_Doubleclick();
	pMouseCursor->Update();

	while( SDL_PollEvent( &input_event ) )
	{
		// handle
		Handle_Input_Global( &input_event );
	}


	// ## audio
	pAudio->Resume_Music();
	pAudio->Update();


	// ## update
	if( Game_Mode == MODE_LEVEL )
	{
		// input
		pLevel->Process_Input();
		pLevel_Editor->Process_Input();
		// update
		pLevel->Update();
		// editor
		pLevel_Editor->Update();
		// hud
		pHudManager->Update();
		// player
		pPlayer->Update();
		// collisions
		if( !editor_enabled )
		{
			pPlayer->Collide_Move();
			pPlayer->Handle_Collisions();
			pSprite_Manager->Handle_Collision_items();
		}
		// Camera ( update after new player position was set )
		pPlayer->Update_Camera();
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		pActive_Overworld->Update();
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Update();
	}

	// gui
	Gui_handle_Time();
}

void Draw_Game( void )
{
	// don't draw if exiting
	if( done )
	{
		return;
	}

	if( Game_Mode == MODE_LEVEL )
	{
		// draw level layer 1
		pLevel->Draw_Layer1();
		// player draw
		pPlayer->Draw();
		// draw level layer 2
		pLevel->Draw_Layer2();
		// hud
		pHudManager->Draw();
		// level editor
		pLevel_Editor->Draw();
		// mouse
		pMouseCursor->Draw();
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// world
		pActive_Overworld->Draw();
		// Mouse
		pMouseCursor->Draw();
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Draw();
		// mouse
		pMouseCursor->Draw();
	}
}
