/***************************************************************************
				menu.cpp  -  nice menu visible at the beginning of the game :)
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
#include "../gui/menu.h"
#include "../gui/menu_data.h"
#include "../core/main.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../audio/audio.h"
#include "../player/player.h"
#include "../core/camera.h"
#include "../video/gl_surface.h"
#include "../level/level.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenu_Item :: cMenu_Item( void )
: cHudSprite()
{
	active = 0;
	isquit = 0;

	image_default = new cHudSprite();
	image_menu = new cHudSprite();
}

cMenu_Item :: ~cMenu_Item( void )
{
	if( image_default )
	{
		delete image_default;
	}

	if( image_menu )
	{
		delete image_menu;
	}
}

void cMenu_Item :: Set_Active( bool nactive /* = 0 */ )
{
	active = nactive;
	rotz = 0;
	Set_Scale( 1 );

	if( !nactive )
	{
		Set_Color_Combine( 0, 0, 0, 0 );
	}
}

void cMenu_Item :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( active )
	{
		// rotation is used for the scale state
		if( !rotz )
		{
			Add_Scale( ( 1.2f / image->w ) * pFramerate->speedfactor );
		}
		else
		{
			Add_Scale( -( 1.2f / image->w ) * pFramerate->speedfactor );
		}

		if( image->w * scalex > image->w + 10 )
		{
			rotz = 0.0001f;
		}
		else if( scalex < 1 )
		{
			rotz = 0;
		}

		// set correct position
		posx -= ( image->w / 2 ) * ( scalex - 1 );
	}

	cHudSprite::Draw( request );

	if( active )
	{
		float strength = image->w * ( scalex - 1 );

		// boost color to yellow
		Set_Color_Combine( strength / 40, strength / 40, 0, GL_ADD );

		posx = startposx;

		image_menu->Draw();
	}
}

cMenuHandler :: cMenuHandler( void )
{
	Reset();

	image_ground = pVideo->Get_Surface( "menu/ground.png" );
	image_ground_quit = pVideo->Get_Surface( "menu/ground_quit.png" );

	// Logo Image
	logo = new cHudSprite( pVideo->Get_Surface( "game/logo/lmario1.png" ), 50, 20 );
	logo->posz = 0.16f;
	// Ground Image
	ground = new cHudSprite( image_ground, 0, 400 );
	ground->posz = 0.10f;
	// Moon Image
	moon = new cHudSprite( pVideo->Get_Surface( "game/items/moon_1.png" ), 702, 39 );
	moon->posz = 0.16f;
	// Cloud Image
	cloud = new cHudSprite( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ), 250, 15 );
	cloud->posz = 0.15f;
}

cMenuHandler :: ~cMenuHandler( void )
{
	Reset();

	delete logo;
	delete ground;
	delete moon;
	delete cloud;
}

void cMenuHandler :: Add_MenuItem( cMenu_Item *item, float shadow_pos /* = 0 */, Color shadow_color /* = (Uint8)0 */ )
{
	if( !item )
	{
		printf( "Menu item is NULL ( current Menu size : %d )\n", Get_Size() );
		return;
	}

	item->Set_Shadow_Pos( shadow_pos );
	item->Set_Shadow_Color( shadow_color );
	item->Set_Image( item->image_default->image );
	items.push_back( item );

	if( active == -1 && Get_Size() == 1 )
	{
		Set_Active( 0 );
	}
}

void cMenuHandler :: Reset( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		delete items[i];
	}

	items.clear();

	// nothing is active
	active = -1;
	// disable quit logo
	use_quit_image = 0;

	// Default Gradient Colors
	color_start = Color( 0.7f, 0.5f, 0 );
	color_end = Color( 0.7f, 0.8f, 0.4f );
}

void cMenuHandler :: Set_Active( int num )
{
	// if not already active and exists
	if( num == (int)active || num >= (int)items.size() || ( num >= 0 && !items[num] ) )
	{
		return;
	}

	if( num >= 0 && (unsigned int)num < items.size() )
	{
		if( active >= 0 && (unsigned int)active < items.size() ) // set last active item un-active
		{
			items[active]->Set_Active( 0 );
		}

		if( !items[num]->isquit )
		{
			ground->Set_Image( image_ground );
		}
		if( items[num]->isquit && use_quit_image )
		{
			ground->Set_Image( image_ground_quit );
		}
	}
	else if( num == -1 )
	{
		items[active]->Set_Active( 0 );
	}

	active = num;

	if( active >= 0 )
	{
		items[active]->Set_Active( 1 );
	}
}

void cMenuHandler :: Use_Quit_Image( bool enabled /* = 1 */ )
{
	use_quit_image = enabled;
}

void cMenuHandler :: Update_Mouse( void )
{
	int found = -1;

	// check
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( pMouseCursor->Collsion_Check( &items[i]->col_rect ) )
		{
			found = i;
			break;
		}
	}

	// ignore mouse init
	if( found < 0 && input_event.motion.x == pMouseCursor->x )
	{
		return;
	}

	Set_Active( found );
}

void cMenuHandler :: Draw( void )
{
	// Gradient
	pVideo->Draw_Gradient( NULL, 0.08f, &color_start, &color_end, DIR_VERTICAL );

	// Logo Image
	logo->Draw();
	// Ground Image
	ground->Draw();
	// Moon Image
	moon->Draw();
	// Cloud Image
	cloud->Draw();

	// menu items
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		items[i]->Draw();
	}
}

cMenu_Item *cMenuHandler :: Get_Active_Item( void )
{
	if( active < 0 || (unsigned int)active > items.size() )
	{
		return NULL;
	}

	return items[active];
}

unsigned int cMenuHandler :: Get_Size( void )
{
	return (unsigned int)items.size();
}

cMenuCore :: cMenuCore( void )
{
	leave = 0;
	next_menu = MENU_NOTHING;
	options_menu_id = MENU_GAME;
	particle_counter = 0;

	pMenu_Data = NULL;
	
	handler = new cMenuHandler();

	pMenu_AnimManager = new cAnimationManager();
}

cMenuCore :: ~cMenuCore( void )
{
	delete handler;
	delete pMenu_AnimManager;
}

void cMenuCore :: Update_Generic( void )
{
	pMouseCursor->Update_Position();

	while( SDL_PollEvent( &input_event ) )
	{
		// was handled
		if( Handle_Input_Global( &input_event ) )
		{
			continue;
		}

		switch( input_event.type )
		{
			case SDL_QUIT:
			{
				leave = 2;
				break;
			}
			case SDL_MOUSEMOTION:
			{
				handler->Update_Mouse();
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				cMenu_Item *item = handler->Get_Active_Item();

				if( item && pMouseCursor->Collsion_Check( &item->col_rect ) )
				{
					pMenu_Data->action = 1;
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}

	Gui_handle_Time();
	pAudio->Update();
}

bool cMenuCore :: Key_Down( SDLKey key )
{
	// Down
	if( key == SDLK_DOWN )
	{
		if( handler->Get_Size() <= (unsigned int)( handler->active + 1 ) )
		{
			handler->Set_Active( 0 );
		}
		else
		{
			handler->Set_Active( handler->active + 1 );
		}
	}
	// Up
	else if( key == SDLK_UP )
	{
		if( handler->active <= 0 )
		{
			handler->Set_Active( handler->Get_Size() - 1 );
		}
		else
		{
			handler->Set_Active( handler->active - 1 );
		}
	}
	// Activate Button
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER )
	{
		if( pMenu_Data )
		{
			pMenu_Data->action = 1;
		}
	}
	// Fast Debug Level entering
	else if( key == SDLK_x && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pLevel->Load( "test_3" );
		
		pPlayer->Reset();

		handler->Set_Active( 0 );
		leave = 1;
		next_menu = MENU_NOTHING;
	}
	// exit
	else if( key == SDLK_ESCAPE )
	{
		pMenu_Data->action = 1;
		handler->active = -1;
		leave = 2;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cMenuCore :: Key_Up( SDLKey key )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

cMenu_Item *cMenuCore :: Auto_Menu( string imagename, string imagefilename_menu, float ypos /* = 0 */, bool is_quit /* = 0 */ )
{
	cMenu_Item *temp_item = new cMenu_Item();

	// The Menu Image
	if( imagefilename_menu.length() > 0 )
	{
		temp_item->image_menu->Set_Image( pVideo->Get_Surface( PIXMAPS_DIR "/menu/" "items/" + imagefilename_menu ), 1 );
	}

	// The Active Image
	if( imagename.length() > 0 )
	{
		temp_item->image_default->Set_Image( pVideo->Get_Surface( PIXMAPS_DIR "/menu/" + imagename ), 1 );
	}

	// Position and initialization
	temp_item->Set_Pos( ( GAME_RES_W / 2 ) - ( temp_item->image_default->col_rect.w / 2 ), ypos );
	temp_item->isquit = is_quit;

	return temp_item;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void cMenuCore :: Enter( MenuID menu_id /* = MENU_MAIN */ )
{
	if( done )
	{
		return;
	}

	// check if not entering from another menu
	if( next_menu == MENU_NOTHING )
	{
		// clear animations
		pMenu_AnimManager->Delete_All();

		// pre render
		pFramerate->speedfactor = 1;

		for( unsigned int i = 0; i < DESIRED_FPS * 20; i++ )
		{
			Update_Animation();
			pMenu_AnimManager->Update();
		}

		pFramerate->Update();
	}
	
	next_menu = MENU_NOTHING;
	leave = 0;

	// clear mouse active object
	pMouseCursor->Double_Click( 0 );

	handler->Reset();

	// default white background
	glClearColor( 1, 1, 1, 1 );

	// ## Create menu class
	// Main
	if( menu_id == MENU_MAIN )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Main());
	}
	// Start
	else if( menu_id == MENU_START )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Start());
	}
	// Options
	else if( menu_id == MENU_OPTIONS )
	{
		// Game
		if( options_menu_id == MENU_GAME )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Game());
		}
		// Video
		else if( options_menu_id == MENU_VIDEO )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Video());
		}
		// Audio
		else if( options_menu_id == MENU_AUDIO )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Audio());
		}
		// Controls
		else if( options_menu_id == MENU_CONTROLS )
		{
			pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Options_Controls());
		}
	}
	// Load
	else if( menu_id == MENU_LOAD )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Savegames( 0 ));
	}
	// Save
	else if( menu_id == MENU_SAVE )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Savegames( 1 ));
	}
	// Credits
	else if( menu_id == MENU_CREDITS )
	{
		pMenu_Data = static_cast<cMenu_Base *>(new cMenu_Credits());
	}

	pMenu_Data->Init();

	while( !leave )
	{
		// update
		Update();
		// draw
		Draw();

		// limit frames for no full CPU usage in the menu
		CorrectFrameTime( 90 );

		// render
		pVideo->Render();

		// update speedfactor
		pFramerate->Update();
	}

	// ESC quits the game if no progress was made only in debug builds
#ifdef _DEBUG
	if( menu_id == MENU_MAIN )
	{
		// if ESC got pressed
		if( leave == 2 && !pPlayer->points && !editor_enabled )	
		{
			done = 1;
		}
	}
#endif

	// back to level
	if( pLevel->is_Loaded() )
	{
		Change_Game_Mode( MODE_LEVEL );
		Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
	}
	// back to the overworld
	else
	{
		Change_Game_Mode( MODE_OVERWORLD );
	}

	delete pMenu_Data;
	pMenu_Data = NULL;

	if( next_menu != MENU_NOTHING )
	{
		Enter( next_menu );
	}
}

void cMenuCore :: Update( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	pMenu_Data->Update();

	Update_Animation();
}

void cMenuCore :: Update_Animation( void )
{
	particle_counter += pFramerate->speedfactor * 0.15f;

	while( particle_counter > 1 )
	{
		// particles ahoy !
		cParticleAnimation *anim = new cParticleAnimation( (float)( rand() % GAME_RES_W ), -20 );
		anim->Set_DirectionRange( 65, 50 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Time_to_Live( 70 );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Scale( 0.2f, 0.35f );
		anim->Set_Color( Color( (Uint8)( 220 + ( rand() % 25 ) ), 200 + ( rand() % 5 ), 5 + ( rand() % 5 ) ) );
		anim->Set_Speed( 0.1f, 0.45f );
		anim->Set_ZPos( 0.105f );
		anim->Set_RotationZ( (float)(rand() % 360), 1 );
		pMenuCore->pMenu_AnimManager->Add( anim );

		particle_counter--;
	}
}

void cMenuCore :: Draw( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	pMenu_Data->Draw();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenuCore *pMenuCore = NULL;
