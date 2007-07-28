/***************************************************************************
				menu.cpp  -  nice menu visible at the beginning of the game :)
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
#include "../overworld/overworld.h"

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
	for( MenuList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	items.clear();

	// nothing is active
	active = -1;
	// disable quit logo
	use_quit_image = 0;

	// Default Gradient Colors
	color_start = Color( 0.7f, 0.93f, 1 );
	color_end = Color( 0.26f, 0.8f, 1 );
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
	for( MenuList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
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

/* *** *** *** *** *** *** *** cMenuCore *** *** *** *** *** *** *** *** *** *** */

cMenuCore :: cMenuCore( void )
{
	next_menu = MENU_NOTHING;
	options_menu_id = MENU_GAME;

	pMenu_Data = NULL;
	
	handler = new cMenuHandler();

	// particle animation
	pMenu_AnimManager = new cAnimationManager();

	// left side
	cParticleAnimation *anim = new cParticleAnimation( -100, 0, 0, GAME_RES_H * 0.5f );
	anim->Set_Emitter_Time_to_Live( -1 );
	anim->Set_Emitter_Iteration_Interval( 6 );
	anim->Set_DirectionRange( 350, 20 );
	anim->Set_Image( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ) );
	anim->Set_Time_to_Live( 400 );
	anim->Set_Fading_Alpha( 0 );
	anim->Set_Fading_Size( 1 );
	anim->Set_Scale( 0.4f, 0.15f );
	anim->Set_Color( Color( (Uint8)220, 240, 255 ), Color( (Uint8)25, 5, 0 ) );
	anim->Set_Speed( 0.1f, 0.01f );
	anim->Set_ZPos( 0.105f );

	pMenu_AnimManager->Add( anim );

	// right side
	anim = new cParticleAnimation( GAME_RES_W + 100, 0, 0, GAME_RES_H * 0.5f );
	anim->Set_Emitter_Time_to_Live( -1 );
	anim->Set_Emitter_Iteration_Interval( 6 );
	anim->Set_DirectionRange( 170, 20 );
	anim->Set_Image( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ) );
	anim->Set_Time_to_Live( 400 );
	anim->Set_Fading_Alpha( 0 );
	anim->Set_Fading_Size( 1 );
	anim->Set_Scale( 0.4f, 0.15f );
	anim->Set_Color( Color( (Uint8)220, 240, 255 ), Color( (Uint8)25, 5, 0 ) );
	anim->Set_Speed( 0.1f, 0.01f );
	anim->Set_ZPos( 0.105f );

	pMenu_AnimManager->Add( anim );
}

cMenuCore :: ~cMenuCore( void )
{
	delete handler;
	delete pMenu_AnimManager;
}

bool cMenuCore :: Handle_Event( SDL_Event *ev )
{
	switch( ev->type )
	{
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
			return 1;
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
		// random level name
		string lvl_name;

		if( !WindowManager::getSingleton().isWindowPresent( "listbox_levels" ) )
		{
			// Create virtual start menu
			cMenu_Start *menu_start = new cMenu_Start();

			menu_start->Init();
			// Get Levels Listbox
			Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
			// select random level
			listbox_levels->setItemSelectState( rand() % listbox_levels->getItemCount(), 1 );
			// get level name
			lvl_name = listbox_levels->getFirstSelectedItem()->getText().c_str();
			// destroy virtual menu
			delete menu_start;
		}
		else
		{
			// Get Levels Listbox
			Listbox *listbox_levels = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_levels" ));
			// select random level
			listbox_levels->setItemSelectState( rand() % listbox_levels->getItemCount(), 1 );
			// get level name
			lvl_name = listbox_levels->getFirstSelectedItem()->getText().c_str();
		}

		Game_Action = GA_ENTER_LEVEL;
		Game_Action_Data = lvl_name;
	}
	// exit
	else if( key == SDLK_ESCAPE )
	{
		pMenu_Data->Exit();
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

void cMenuCore :: Load( MenuID menu /* = MENU_MAIN */ )
{
	Unload();

	// unload level if possible
	if( pLevel->delayed_unload )
	{
		pLevel->Unload();
	}

	// Set ID
	menu_id = menu;

	// check if not entering from another menu
	if( next_menu == MENU_NOTHING )
	{
		// clear animations
		static_cast<cParticleAnimation *>(pMenu_AnimManager->objects[0])->Clear();

		// pre render
		pFramerate->speedfactor = 2;

		for( unsigned int i = 0; i < DESIRED_FPS * 100; i++ )
		{
			pMenu_AnimManager->Update();
		}

		pFramerate->Update();
	}
	
	next_menu = MENU_NOTHING;

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
}

void cMenuCore :: Unload( void )
{
	menu_id = MENU_NOTHING;

	if( pMenu_Data )
	{
		delete pMenu_Data;
		pMenu_Data = NULL;
	}
}

void cMenuCore :: Update( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	pMenu_Data->Update();
}

void cMenuCore :: Draw( void ) 
{
	if( !pMenu_Data )
	{
		return;
	}

	pMenu_Data->Draw();

	// limit frames for no full CPU usage
	CorrectFrameTime( 90 );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenuCore *pMenuCore = NULL;
