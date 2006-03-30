/***************************************************************************
           main.cpp  -  main routines and initialisation
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "globals.h"
#include "main.h"

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

	InitGame();

	if( argc == 2 && !strlen( argv[1] ) && pLevel->Load( argv[1] ) )
	{
		Game_Mode = MODE_LEVEL;
	}
	else
	{
		pMenu->Enter_Menu();
	}

	while( !done )
	{
		HandleInput_Level();
		ProcessInput_Level();
		UpdateGame();
		Framerate.Update();
	}

	return 0; // Successful
}

void InitGame( void )
{
	srand( (unsigned int)time( NULL ) );
	atexit( ExitGame );

	Init_SDL();
	Init_CEGUI(); // temp system for preferences loading

	pPreferences = new cPreferences();
	pPreferences->Load();
	
	// delete temp system
	delete pGuiSystem;
	delete pGuiRenderer;

	pImageManager = new cImageManager();
	pObjManager = new cObjectManager();

	pAudio = new cAudio();
	pPreferences->Apply();
	pAudio->Init();
	Init_Video();
	Init_CEGUI();
	Init_CEGUI_data();
	Init_OpenGL();
	
	Init_BasicColors();

	DrawLoadingScreen();
	
	pJoystick = new cJoystick();
	
	if( TTF_Init() == -1 )
	{
		printf( "Error : SDL_TTF initialisation failed\nReason : %s\n", SDL_GetError() );
		exit( 0 );
	}

	font_normal = TTF_OpenFont( FONT_DIR "/bluebold.ttf", 26 );
	font_small = TTF_OpenFont( FONT_DIR "/bluebold.ttf", 16 );

	if( !font_normal || !font_small )
	{
		printf( "Error : Font loading failed\n" );
		exit( 0 );
	}

	pOverWorld = new cOverWorld();
	pPlayer = new cPlayer( 120, 0 );
	pLevel = new cLevel();
	pHudManager = new cHudManager();
	pAnimationManager = new cAnimationManager();
	pDialogManager = new cDialogManager();
	pMenu = new cMainMenu();
	pMouseCursor = new cMouseCursor();
	pLeveleditor = new cLevelEditor();
	pSavegame = new cSavegame();

	Preload_images();
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
	
	if( pPlayer )
	{
		delete pPlayer;
		pPlayer = NULL;
	}
	if( pLevel )
	{
		delete pLevel;
		pLevel = NULL;
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

	if( pAnimationManager )
	{
		delete pAnimationManager;
		pAnimationManager = NULL;
	}

	if( pLeveleditor )
	{
		delete pLeveleditor;
		pLeveleditor = NULL;
	}
	
	if( pDialogManager )
	{
		delete pDialogManager;
		pDialogManager = NULL;
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

	if( strlen( SDL_GetError() ) > 0 )
	{
		printf( "Last known Error : %s\n", SDL_GetError() );
	}

	SDL_Quit();
}

void HandleInput( void )
{
	switch( event.type )
	{
		case SDL_QUIT:
		{
			done = 1;
			ClearEvents();
			break;
		}
		case SDL_KEYDOWN:
		{
			KeyDown( event.key.keysym.sym );
			break;
		}
		case SDL_KEYUP:
		{
			KeyUp( event.key.keysym.sym );
			break;
		}
		default: // other events
		{
			pMouseCursor->Handle_Events();
			pJoystick->Handle_Events();
			break;
		}
	}
}

void KeyUp( SDLKey key )
{
	keys = SDL_GetKeyState( NULL );
	
	// inject the scancode directly
	System::getSingleton().injectKeyUp( event.key.keysym.scancode );

	if( Game_Mode == MODE_LEVEL )
	{
		if( key == pPreferences->Key_right || key == pPreferences->Key_left )
		{
			pPlayer->Hold();
		}
		else if( key == pPreferences->Key_down )
		{
			if( pPlayer->ducked )
			{
				pPlayer->Stop_ducking();
			}
		}
		else if( key == pPreferences->Key_up )
		{
			pPlayer->UpKeytime = 0;
		}
	}
}

void KeyDown( SDLKey key )
{
	keys = SDL_GetKeyState( NULL );

	// inject the scancode
	System::getSingleton().injectKeyDown( event.key.keysym.scancode );
	
	// use for translated unicode value
	if( ( event.key.keysym.unicode & 0xFF80 ) == 0 )
	{
		System::getSingleton().injectChar( event.key.keysym.unicode & 0x7F );
	}

	if( key == SDLK_F4 && event.key.keysym.mod & KMOD_ALT )
	{
		pMenu->leave = 1;
		pOverWorld->entered = 1;
		done = 1;
		return;
	}
	if( key == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT )
	{
		Toggle_Fullscreen();
	}
	else if( key == SDLK_l && !( Game_Mode == MODE_OVERWORLD && pOverWorld->debugmode ) && Game_Mode != MODE_LEVEL_SETTINGS )
	{
		Color color = Get_Color( 0, 0, 0, 64 );
		DrawRect( NULL, &color );

		string levelname = EditMessageBox( "Load a new Level", "Levelname", GAME_RES_W / 2 - 200, GAME_RES_H / 2 - 10, 1 );

		if( pLevel->Load( levelname ) )
		{
			pPlayer->Reset();

			if( Game_Mode == MODE_OVERWORLD )
			{
				pOverWorld->entered = 1;
				Game_Mode = MODE_LEVEL;
			}
			else if( Game_Mode == MODE_MENU )
			{
				pMenu->handler->Set_Active( 0 );
				pMenu->action = 1;
			}

			debugdisplay->text = "Loaded " + levelname;
			debugdisplay->counter = DESIRED_FPS * 2;
		}
		else
		{
			debugdisplay->text = "Error Loading " + levelname;
			pAudio->PlaySound( SOUNDS_DIR "/error.ogg" );
			debugdisplay->counter = DESIRED_FPS * 2;
		}
	}
	else if( key == SDLK_F10 )
	{
		pAudio->ToggleSounds();

		if( !pAudio->sound_enabled )
		{
			debugdisplay->text = "Sound Disabled";
		}
		else
		{
			debugdisplay->text = "Sound Enabled";
		}

		debugdisplay->counter = DESIRED_FPS * 2;
	}
	else if( key == SDLK_F11 )
	{
		pAudio->ToggleMusic();

		if( !pAudio->music_enabled )
		{
			debugdisplay->text = "Music Disabled";
		}
		else
		{
			debugdisplay->text = "Music Enabled";
		}

		debugdisplay->counter = DESIRED_FPS * 2;
	}
	else if( key == SDLK_ESCAPE  )
	{
		if( Game_Mode == MODE_LEVEL )
		{
			pMenu->Enter_Menu();
			return;
		}
		else if( Game_Mode == MODE_MENU )
		{
			pMenu->action = 1;
			pMenu->handler->active = -1;
			pMenu->leave = 2;
		}
	}

	if( Game_Mode == MODE_LEVEL )
	{
		KeyDown_Level( key );
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		pOverWorld->KeyDown_OW( key );
	}
	else if( Game_Mode == MODE_MENU )
	{
		pMenu->KeyDown_menu( key );
	}
}

void KeyDown_Level( SDLKey key )
{
	if( key == SDLK_d && ( event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL ) )
	{
		if( Game_debug ) 
		{
			debugdisplay->counter = DESIRED_FPS * 2;
			debugdisplay->text = "Debugmode disabled";
		}
		else
		{
			debugdisplay->counter = DESIRED_FPS * 2;
			Framerate.fps_worst = 1000;
			Framerate.fps_best = 0;
			debugdisplay->text = "Debugmode enabled";
		}

		Game_debug = !Game_debug;
	}
	// debug keys for testing ( F2 - F4 )
	else if( key == SDLK_F2 )
	{
		DrawEffect( BLACK_FADEOUT );
	}
	else if( key == SDLK_F3 && Game_debug && !Leveleditor_Mode )
	{
		pPlayer->GotoNextLevel();
	}
	else if( key == SDLK_F4 )
	{
		//DrawEffect( BIG_MUSHROOM );
		for( unsigned int i = 0; i < 40; i++ )
		{
			cAnimation_3 *anim = new cAnimation_3( pPlayer->posx + (double)( pPlayer->col_rect.w / 2 ), pPlayer->posy - 100, 10 );
			//anim->SetFadingSpeed( 0.1 );
			anim->SetImage( GetImage( "animation/particles/star.png" ) );
			//anim->SetDirectionRange( 320, 100 );
			anim->SetLifetimeMod( 6 );
			anim->SetFading( FADE_ALPHA );
			//anim->SetDirectionRange( 0, 20 );
			anim->SetSpeed( 2, 8 );
			anim->Set_ConstRotationZ( 0.5, 15 );
			pAnimationManager->Add( anim );
		}
		//pPlayer->invincible = DESIRED_FPS * 2.2;
		//pPlayer->Set_onTop( pObjManager->Get_Pointer( 0 ) );
	}
	else if( key == SDLK_F5 )
	{
		Save_Screenshot();
	}
	else if( key == SDLK_F8 )
	{
		if( !Leveleditor_Mode )
		{
			if( !pLeveleditor->wMenu_Objects.size() )
			{
				pLeveleditor->Load_Default_Menu();
			}

			debugdisplay->text = "Editor enabled";
			pAnimationManager->DeleteAll();
			Leveleditor_Mode = 1;
		}
		else if( Leveleditor_Mode )
		{
			debugdisplay->text = "Editor disabled";
			pPlayer->Update_Camera( 1 );
			pMouseCursor->Reset();
			
			Leveleditor_Mode = 0;
		}

		debugdisplay->counter = DESIRED_FPS * 2;
		pAudio->PlaySound( SOUNDS_DIR "/editor.ogg" );
	}
	else if( !Leveleditor_Mode ) // only Game
	{
		if( key == pPreferences->Key_shoot )
		{
			pPlayer->Fireball_add();
		}
		else if( key == SDLK_RETURN )
		{
			Itembox->Request_Item();
		}
		else if( keys[SDLK_g] && keys[SDLK_o] && keys[SDLK_d] )
		{
			if( pPlayer->godmode )
			{
				debugdisplay->text = "Funky Godmode disabled";
			}
			else
			{
				debugdisplay->text = "Funky Godmode enabled";
			}
			
			pPlayer->godmode = !pPlayer->godmode;
			debugdisplay->counter = DESIRED_FPS * 2;
		}
		else if( key == pPreferences->Key_up )
		{
			if( pPlayer->godmode )
			{
				pPlayer->Start_jump();
			}
			else
			{
				pPlayer->Start_jump_keytime();
			}

		}
	}
	else if( Leveleditor_Mode ) // only Leveleditor
	{
		if ( key == SDLK_F1 )
		{
			// Todo : a leveleditor help screen
		}
		else if( key == SDLK_s && ( event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL ) )
		{
			pLevel->Save();
		}
		else if( key == SDLK_HOME )
		{
			cameraposx = 0;
			cameraposy = 0;
		}
		else if( key == SDLK_END )
		{
			double new_cameraposx = 0;
			double new_cameraposy = 0;
			
			for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
			{
				if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ACTIVE )
				{
					continue;
				}

				if( pObjManager->items[i]->type == TYPE_LEVELEXIT && new_cameraposx < pObjManager->items[i]->posx )
				{
					new_cameraposx = pObjManager->items[i]->posx;
					new_cameraposy = pObjManager->items[i]->posy;
				}
			}

			if( new_cameraposx != 0 || new_cameraposy != 0 ) 
			{
				cameraposx = new_cameraposx - ( GAME_RES_W/2 );
				cameraposy = new_cameraposy - ( GAME_RES_H/2 );
			}
			return;
		}
		else if( key == SDLK_n )
		{
			cameraposx += GAME_RES_W;
		}
		else if( key == SDLK_p )
		{
			cameraposx -= GAME_RES_W;
		}
		// Precise Pixel-Positioning
		else if( key == SDLK_KP2 && pMouseCursor->MouseObject )
		{
			if( pMouseCursor->fastCopyMode )
			{
				if( pMouseCursor->MouseObject->start_image ) 
				{
					pMouseCursor->Copy( pMouseCursor->MouseObject, pMouseCursor->MouseObject->posx, pMouseCursor->MouseObject->posy + pMouseCursor->MouseObject->start_image->h );
					cameraposy += pMouseCursor->MouseObject->start_image->h;
				}
			}
			else
			{
				cameraposy++;
			}
		}
		else if( key == SDLK_KP4 && pMouseCursor->MouseObject )
		{
			if( pMouseCursor->fastCopyMode )
			{
				if( pMouseCursor->MouseObject->start_image ) 
				{
					pMouseCursor->Copy( pMouseCursor->MouseObject, pMouseCursor->MouseObject->posx - pMouseCursor->MouseObject->start_image->w, pMouseCursor->MouseObject->posy );
					cameraposx -= pMouseCursor->MouseObject->start_image->w;	
				}
			}
			else
			{
				cameraposx--;
			}
		}
		else if( key == SDLK_KP6 && pMouseCursor->MouseObject )
		{
			if( pMouseCursor->fastCopyMode )
			{
				if( pMouseCursor->MouseObject->start_image ) 
				{
					pMouseCursor->Copy( pMouseCursor->MouseObject, pMouseCursor->MouseObject->posx + pMouseCursor->MouseObject->start_image->w, pMouseCursor->MouseObject->posy );
					cameraposx += pMouseCursor->MouseObject->start_image->w;
				}
			}
			else
			{
				cameraposx++;
			}
		}
		else if( key == SDLK_KP8 && pMouseCursor->MouseObject )
		{
			if( pMouseCursor->fastCopyMode )
			{
				if( pMouseCursor->MouseObject->start_image ) 
				{
					pMouseCursor->Copy( pMouseCursor->MouseObject, pMouseCursor->MouseObject->posx, pMouseCursor->MouseObject->posy - pMouseCursor->MouseObject->start_image->h );
					cameraposy -= pMouseCursor->MouseObject->start_image->h;
				}
			}
			else
			{
				cameraposy--;
			}
		}
		else if( key == SDLK_m && pMouseCursor->MouseObject )
		{
			// modify Passive/Massive/Halfmassive/Climbable
			pLeveleditor->Switch_ObjectState();
		}
		else if( key == SDLK_KP_PLUS && pMouseCursor->MouseObject )
		{
			// push into the front
			pLeveleditor->Change_DrawPosition( 0 );
		}
		else if( key == SDLK_KP_MINUS && pMouseCursor->MouseObject )
		{
			// push into the back
			pLeveleditor->Change_DrawPosition( 1 );
		}
		else if( key == SDLK_c && ( event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL ) ) // Copy a Sprite
		{
			if( pMouseCursor->MouseObject )
			{
				pMouseCursor->CopyObject = pMouseCursor->MouseObject;
			}
		}
		else if( key == SDLK_v && ( event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL ) ) // Paste a Sprite
		{
			if( pMouseCursor->CopyObject )
			{
				pMouseCursor->Copy( pMouseCursor->CopyObject, pMouseCursor->posx, pMouseCursor->posy );
				pMouseCursor->mouse_h = (int)( pMouseCursor->CopyObject->col_rect.h / 2 );
				pMouseCursor->mouse_w = (int)( pMouseCursor->CopyObject->col_rect.w / 2 );
			}
		}
	}
}

void HandleInput_Level( void )
{
	pMouseCursor->Update_Position();

	while( SDL_PollEvent( &event ) )
	{
		HandleInput();

		switch( event.type )
		{
			case SDL_MOUSEMOTION:
			{
				if( pMouseCursor->left )
				{
					pMouseCursor->MouseObject_Update();
				}
				else if( Leveleditor_Mode && pMouseCursor->mover_mode ) 
				{
					pMouseCursor->Mover_Update( event.motion.xrel, event.motion.yrel );
				}
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				if( event.button.button == 1 ) // left
				{
					if( Leveleditor_Mode ) 
					{
						pMouseCursor->Click();
					}
				}
				else if( event.button.button == 2 ) // middle
				{
					if( Leveleditor_Mode && pMouseCursor->MouseObject ) // Activates the Fastcopy mode
					{
						pMouseCursor->fastCopyMode = 1;
					}
					else if( Leveleditor_Mode )  // The Mover mode
					{
						if( !pMouseCursor->mover_mode )
						{
							pMouseCursor->mover_mode = 1;
						}
						else
						{
							pMouseCursor->mover_mode = 0;
						}
					}
				}
				else if( event.button.button == 3 ) // right
				{
					if( Leveleditor_Mode && !pMouseCursor->left ) 
					{
						pMouseCursor->Delete();
					}
				}

				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				if( event.button.button == 2 )
				{
					pMouseCursor->fastCopyMode = 0;
				}
				break;
			}
			default: // other events
			{
				break;
			}
		} // event.type switch end
	} // Pollevent end
}

void ProcessInput_Level( void )
{
	keys = SDL_GetKeyState( NULL );
	
	// Drag Delete
	if( ( keys[SDLK_RCTRL] || keys[SDLK_LCTRL] ) && pMouseCursor->right && pMouseCursor->Editor_CollsionCheck( pMouseCursor->posx, pMouseCursor->posy ) ) 
	{
		pMouseCursor->Delete();
	}

	// Camera Movement
	if( keys[pPreferences->Key_right] || pJoystick->right )
	{
		if( !Leveleditor_Mode )
		{
			pPlayer->direction = DIR_RIGHT;
		}
		else
		{
			cameraposx += CAMERASPEED * Framerate.speedfactor;
		}
	}
	else if( keys[pPreferences->Key_left] || pJoystick->left )
	{
		if( !Leveleditor_Mode )
		{
			pPlayer->direction = DIR_LEFT;
		}
		else
		{
			cameraposx -= CAMERASPEED * Framerate.speedfactor;
		}
	}

	if( keys[pPreferences->Key_up] || ( pJoystick->up || pJoystick->Button( pPreferences->Joy_jump ) ) )
	{
		if( Leveleditor_Mode )
		{
			cameraposy -= CAMERASPEED * Framerate.speedfactor;
		}
	}

	if( keys[pPreferences->Key_down] || pJoystick->down )
	{
		if( !Leveleditor_Mode )
		{
			if( pPlayer->groundObject )
			{
				if( pPlayer->groundObject->massivetype == MASS_MASSIVE ) 
				{
					pPlayer->Start_ducking();
				}
				else if( pPlayer->groundObject->massivetype == MASS_HALFMASSIVE )
				{
					pPlayer->Start_falling();
				}
			}
		}
		else
		{
			cameraposy += CAMERASPEED * Framerate.speedfactor;
		}
	}

	if( Leveleditor_Mode && ( ( keys[pPreferences->Key_down] || keys[pPreferences->Key_up] || keys[pPreferences->Key_left] || 
		keys[pPreferences->Key_right] ) || ( pJoystick->down || pJoystick->up || pJoystick->left || pJoystick->right ) ) )
	{
		if( pMouseCursor->left && pMouseCursor->MouseObject )
		{
			if( pMouseCursor->MouseObject->type != TYPE_PLAYER )
			{
				pMouseCursor->MouseObject->SetPos( (int)pMouseCursor->posx - pMouseCursor->mouse_w, (int)pMouseCursor->posy - pMouseCursor->mouse_h, 1 );
			}
			else
			{
				pMouseCursor->MouseObject->startposx = (int)pMouseCursor->posx - pMouseCursor->mouse_w;
				pMouseCursor->MouseObject->startposy = (int)pMouseCursor->posy - pMouseCursor->mouse_h;
			}
		}
	}
}

void Save_Screenshot( void )
{
	char filename[20];
	
	for( unsigned int i = 1; i < 1000; i++ )
	{
		sprintf( filename, "screenshots/%03d.bmp", i );

		if( !valid_file( filename ) )
		{
			// Todo : save it as png

			// create image data
			GLubyte *data = new GLubyte[pPreferences->Screen_W * pPreferences->Screen_H * 3];

			// read gl screen
			glReadPixels( 0, 0, pPreferences->Screen_W, pPreferences->Screen_H,
				GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)data );

			// create SDL surface
			SDL_Surface *temp = SDL_CreateRGBSurface( SDL_SWSURFACE, pPreferences->Screen_W, 
				pPreferences->Screen_H, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );

			SDL_LockSurface( temp );

			Uint32 *pdata = (Uint32 *)temp->pixels;

			// copy into SDL surface
			for( int y = temp->h - 1, y2 = 0; y >= 0 && y2 < pPreferences->Screen_H; --y, ++y2 )
			{
				for( int x = temp->w -1; x >= 0; --x )
				{
					pdata[y * temp->pitch/4 + x] =
						SDL_MapRGBA( temp->format,
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 0 ],
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 1 ],
							data[ y2 * pPreferences->Screen_W * 3 + x * 3 + 2 ],
							255 );
				}
			}

			SDL_UnlockSurface( temp );

			// save the surface
			SDL_SaveBMP( temp, filename );
			
			// delete screen data
			SDL_FreeSurface( temp );
			delete[] data;

			debugdisplay->text = "Screenshot " + int_to_string( i ) + " saved";
			debugdisplay->counter = DESIRED_FPS * 2.5;

			return;
		}
	}
}

void UpdateGame( void )
{
	// if a level isn't loaded -> enter overworld
	if( !pLevel->is_Loaded() )
	{
		pOverWorld->Enter();
	}

	pAudio->Resume_Music();
	pAudio->Update();

	pLevel->Draw();
	pPlayer->Update();
	pDialogManager->Draw();

	debugdisplay->GameDebugDraw();
	pMouseCursor->Update_Doubleclick();

	pLeveleditor->Draw();

	SDL_GL_SwapBuffers();

	pObjManager->Update();
}
