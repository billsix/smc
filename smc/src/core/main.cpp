/***************************************************************************
           main.cpp  -  main routines and initialization
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

// for linux directory creation
#ifdef __unix__
	#include <sys/stat.h>
#endif

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
	if( argc == 2 && !strlen( argv[1] ) && pLevel->Load( argv[1] ) )
	{
		Change_Game_Mode( MODE_LEVEL );
	}
	else
	{
		pMenuCore->Enter();
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

#ifdef __unix__
	// get linux user data dir ( home directory )
	user_data_dir = (string)getenv( "HOME" ) + "/.smc/";
	// create user directories
	mkdir( user_data_dir.c_str(), S_IRWXU );
	chdir( user_data_dir.c_str() );
	mkdir( SAVE_DIR, S_IRWXU );
	mkdir( SCREENSHOT_DIR, S_IRWXU );
#endif

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

	pObjManager = new cObjectManager();
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
			if( pMouseCursor->Handle_Event( ev ) )
			{
				return 1; 
			}
			
			if( pJoystick->Handle_Event( ev ) )
			{
				return 1;
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

	// input

	// todo : move Update_Position() back to SDL_MOUSEMOTION with disabled mover_mode ? ( also to camera move() )
	pMouseCursor->Update_Position();
	pMouseCursor->Update_Doubleclick();
	pMouseCursor->Update();

	while( SDL_PollEvent( &input_event ) )
	{
		// handle
		Handle_Input_Global( &input_event );
	}

	// if a level isn't loaded
	if( !pLevel->is_Loaded() )
	{
		// custom level
		if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
		{
			pMenuCore->Enter( MENU_START );
		}
		// world level
		else
		{
			pActive_Overworld->Enter();
		}
	}

	// delay enter overworld
	if( pActive_Overworld->delayed_enter )
	{
		pActive_Overworld->Enter();
	}

	// audio
	pAudio->Resume_Music();
	pAudio->Update();

	if( Game_Mode == MODE_LEVEL )
	{
		// Camera
		pPlayer->Update_Camera();
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
			pObjManager->Handle_Collision_items();
		}
		// object events
		pObjManager->Update();
	}
	// todo
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// editor
		pWorld_Editor->Update();
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
		pActive_Overworld->Draw();
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenuCore->Draw();
		// mouse
		pMouseCursor->Draw();
	}
}
