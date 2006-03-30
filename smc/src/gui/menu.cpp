/***************************************************************************
         main.cpp  -  the nice menu at the beginning of the game
                             -------------------
    copyright            :	(C) 2003-2004 by Artur Hallmann
							(C) 2003-2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../globals.h"
#include "../main.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenu_Item :: cMenu_Item( void ) : cSprite()
{
	active = 0;
	isquit = 0;

	image_default = new cSprite();
	image_menu = new cSprite();
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
	scale = 1;
}

void cMenu_Item :: Draw( void )
{
	if( active )
	{
		// rotation is used for the scale state
		if( !rotz )
		{
			scale += ( 1.2 / image->w ) * Framerate.speedfactor;
		}
		else
		{
			scale -= ( 1.2 / image->w ) * Framerate.speedfactor;
		}

		if( image->w * scale > image->w + 10 )
		{
			rotz = 0.0001;
		}
		else if( scale < 1 )
		{
			rotz = 0;
		}

		// set correct position
		posx -= ( image->w / 2 ) * ( scale - 1 );
	}

	cSprite::Draw();

	if( active )
	{
		glBlendFunc( GL_SRC_COLOR, GL_DST_ALPHA );
		double strength = image->w * ( scale - 1 );
		Color color = Get_Color( 195, 150 - (int)( strength * 4 ), 70 - (int)( strength * 3 ) );
		DrawRect( posx, posy, col_rect.w * scale, col_rect.h * scale, &color );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		posx = startposx;

		image_menu->Draw();
	}
}

cMenuHandler :: cMenuHandler( GL_Surface *img_logo, GL_Surface *img_logo_quit, unsigned int posx, unsigned int posy /* = 10  */ )
{
	active = -1; // nothing is active

	image_logo = img_logo;
	image_logo_quit = img_logo_quit;

	logo = new cSprite( img_logo, (double)posx, (double)posy );
}

cMenuHandler :: ~cMenuHandler( void )
{
	Reset();
}

void cMenuHandler :: Add_MenuItem( cMenu_Item *item, Color *shadow /* = NULL */ )
{
	if( !item )
	{
		printf( "Menu item is NULL ( current Menu size : %d )\n", Get_Size() );
		return;
	}

	item->SetShadow( shadow );
	item->SetImage( item->image_default->image );
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
		if( items[i] )
		{
			delete items[i];
		}
	}

	items.clear();

	active = -1; // nothing is active
}

void cMenuHandler :: Set_Active( int num )
{
	if( num == (int)active || num >= (int)Get_Size() || ( num >= 0 && !items[num] ) ) // if not already active and exists
	{
		return;
	}

	if( num >= 0 && (unsigned int)num < Get_Size() )
	{
		if( active >= 0 && (unsigned int)active < Get_Size() ) // set last active item un-active
		{
			items[active]->Set_Active( 0 );
		}

		if( !items[num]->isquit )
		{
			logo->SetImage( image_logo );
		}
		if( items[num]->isquit && image_logo_quit )
		{
			logo->SetImage( image_logo_quit );
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

		// particles ahoy !
		cAnimation_3 *anim = new cAnimation_3( ( GAME_RES_W / 90 ) + ( rand() % 35 ), ( GAME_RES_H / 1.08 ) + ( rand() % 25 ), 1 );
		anim->SetDirectionRange( 265, 95 );
		anim->SetImage( GetImage( "animation/particles/star.png" ) );
		anim->SetLifetimeMod( 5 );
		anim->SetFading( FADE_SIZE );
		anim->SetScale( 0.6 );
		anim->SetColor( Color( (Uint8)255, 255, 150 + ( rand() % 105 ) ) );
		anim->SetSpeed( 3, 5 );
		pMenu->pMenu_AnimManager->Add( anim );
	}
}

void cMenuHandler :: Update_Mouse( void )
{
	int found = -1;

	// check
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( items[i] && pMouseCursor->CollsionCheck( &items[i]->col_rect ) )
		{
			found = i;
			break;
		}
	}

	if( found < 0 && event.motion.x == pMouseCursor->x ) // ignore mouse init
	{
		return;
	}

	Set_Active( found );
}

void cMenuHandler :: Draw( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( items[i] )
		{
			items[i]->Draw();
		}
	}
	
	logo->Draw();
}

cMenu_Item *cMenuHandler :: Get_Active_Item( void )
{
	if( active < 0 || (unsigned int)active > Get_Size() )
	{
		return NULL;
	}

	return items[active];
}

unsigned int cMenuHandler :: Get_Size( void )
{
	return (unsigned int)items.size();
}

cMainMenu :: cMainMenu( void )
{
	leave = 0;
	action = 0;
	next_menu = MENU_NOTHING;
	options_menu_id =  MENU_VIDEO;

	// Control Items
	// Keyboard
	gamepad = new cSprite();
	controls_up_keyname = new cSprite( NULL, 0, -100 );
	controls_down_keyname = new cSprite( NULL, 0, -100 );
	controls_left_keyname = new cSprite( NULL, 0, -100 );
	controls_right_keyname = new cSprite( NULL, 0, -100 );
	controls_shoot_keyname = new cSprite( NULL, 0, -100 );

	// Video Items
	fullscreen = new cSprite();

	// Audio Items
	music = new cSprite();
	sounds = new cSprite();
	audio_music = new cSprite();
	audio_sounds = new cSprite();

	// Savegames
	for( unsigned int i = 0; i < 9; i++ )
	{
		savegame_temp.push_back( new cSprite() );
	}

	handler = new cMenuHandler( GetImage( "menu/logo.png" ), NULL, (unsigned int)( GAME_RES_W / 5 ) );

	pMenu_AnimManager = new cAnimationManager();

	LoadImages();
}

cMainMenu :: ~cMainMenu( void )
{
	FreeImages();

	delete gamepad;
	delete controls_up_keyname;
	delete controls_down_keyname;
	delete controls_left_keyname;
	delete controls_right_keyname;
	delete controls_shoot_keyname;
	delete fullscreen;
	delete music;
	delete sounds;
	delete audio_music;
	delete audio_sounds;

	// Savegames
	for( unsigned int i = 0; i < savegame_temp.size(); i++ )
	{
		delete savegame_temp[i];
	}

	savegame_temp.clear();

	delete handler;
	delete pMenu_AnimManager;
}

void cMainMenu :: LoadImages( void )
{
	// Control Items
	// Keyboard
	gamepad->SetImage( GetImage( "menu/items/gamepad.png" ), 1 );
	gamepad->SetPos( GAME_RES_W / 1.9 + gamepad->col_rect.w + 50, GAME_RES_H / 1.24 );
	
	controls_up_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_up ), black ), 1, 1 );
	controls_down_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_down ), black ), 1, 1 );
	controls_left_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_left ), black ), 1, 1 );
	controls_right_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_right ), black ), 1, 1 );
	controls_shoot_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_shoot ), black ), 1, 1 );
	
	// Video
	fullscreen->SetImage( GetImage( "menu/items/fullscreen.png" ), 1 );
	fullscreen->SetPos( GAME_RES_W / 1.8 + fullscreen->col_rect.w + 50, GAME_RES_H / 1.42 );
	
	// Audio
	music->SetImage( GetImage( "menu/items/music.png" ), 1 );
	music->SetPos( GAME_RES_W / 1.8 + music->col_rect.w + 50, GAME_RES_H / 1.85 );
	sounds->SetImage( GetImage( "menu/items/sounds.png" ), 1 );
	sounds->SetPos( GAME_RES_W / 1.8 + sounds->col_rect.w + 50, GAME_RES_H / 1.50 );
}

void cMainMenu :: FreeImages( void )
{
	//
}

void cMainMenu :: UpdateGeneric( void )
{
	keys = SDL_GetKeyState( NULL );
	pMouseCursor->Update_Position();

	while( SDL_PollEvent( &event ) )
	{
		HandleInput();

		switch( event.type )
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

				if( item && pMouseCursor->CollsionCheck( &item->col_rect ) )
				{
					action = 1;
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

void cMainMenu :: KeyDown_menu( SDLKey key )
{
	if( key == SDLK_DOWN ) // Down
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
	else if( key == SDLK_UP ) // Up
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
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER ) // Button
	{
		action = 1;
	}
}

cMenu_Item *cMainMenu :: Auto_Menu( string imagename, string imagefilename_menu, double ypos /* = 0 */, bool is_quit /* = 0 */ )
{
	string filetemp, full_filename;

	cMenu_Item *temp_item = new cMenu_Item();

	// Menu pixmaps directory
	filetemp.insert( 0, "/menu/" );
	filetemp.insert( 0, PIXMAPS_DIR );

	// The Menu Image
	if( imagefilename_menu.length() > 0 )
	{
		full_filename = filetemp;
		full_filename.insert( full_filename.length(), "items/" );
		full_filename.insert( full_filename.length(), imagefilename_menu );
		temp_item->image_menu->SetImage( GetImage( full_filename ), 1 );
	}

	// The Active Image
	if( imagename.length() > 0 )
	{
		full_filename = filetemp;
		full_filename.insert( full_filename.length(), imagename );
		temp_item->image_default->SetImage( GetImage( full_filename ), 1 );
	}

	// Position and initialization
	temp_item->SetPos( ( GAME_RES_W / 2 ) - ( temp_item->image_default->col_rect.w / 2 ), ypos );
	temp_item->isquit = is_quit;

	return temp_item;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Enter_Menu( MenuID menu_id /* = MENU_MAIN */ )
{
	if( done )
	{
		return;
	}

	cameraposx = 0;
	cameraposy = 0;
	Game_Mode = MODE_MENU;
	next_menu = MENU_NOTHING;
	leave = 0;

	// play menu music if no music is playing
	if( !pAudio->isMusicPlaying() )
	{
		pAudio->FadeOutMusic( 1500 );
		pAudio->PlayMusic( MUSIC_DIR "/game/menu.ogg", -1, 0, 1500 );
	}

	handler->Reset();
	SpriteList drawlist;

	if( menu_id == MENU_MAIN ) // with exit image
	{
		handler->image_logo_quit = GetImage( "menu/logo_quit.png" );
	}
	else
	{
		handler->image_logo_quit = NULL;
	}

	cMenu_Item *temp_item = NULL;
	double item_ypos = ( GAME_RES_H / 3 ) + 80;

	// menu gui layout filename
	string layout_file = "data/gui/layout/default.layout";

	if( menu_id == MENU_MAIN )
	{
		layout_file = "data/gui/layout/menu_main.layout";

		// Start
		temp_item = Auto_Menu( "start.png", "start.png", item_ypos );
		temp_item->image_menu->SetPos( temp_item->posx + ( temp_item->image_default->col_rect.w + 16 ), temp_item->posy );
		handler->Add_MenuItem( temp_item );
		// Options
		item_ypos += 60;
		temp_item = Auto_Menu( "options.png", "options.png", item_ypos );
		temp_item->image_menu->SetPos( temp_item->posx - temp_item->image_menu->col_rect.w - 16, temp_item->posy );
		handler->Add_MenuItem( temp_item );
		// Load
		item_ypos += 60;
		temp_item = Auto_Menu( "load.png", "load.png", item_ypos );
		temp_item->image_menu->SetPos( temp_item->posx + ( temp_item->image_default->col_rect.w + 16 ), temp_item->posy );
		handler->Add_MenuItem( temp_item );
		// Save
		item_ypos += 60;
		temp_item = Auto_Menu( "save.png", "save.png", item_ypos );
		temp_item->image_menu->SetPos( temp_item->posx - temp_item->image_menu->col_rect.w - 16, temp_item->posy );
		handler->Add_MenuItem( temp_item );
		// Quit
		item_ypos += 60;
		temp_item = Auto_Menu( "quit.png", "", item_ypos, 1 );
		temp_item->image_menu->SetPos( temp_item->posx + temp_item->col_rect.w + 16, temp_item->posy );
		handler->Add_MenuItem( temp_item );

		drawlist.push_back( new cSprite( GetImage( "menu/logo_sdl.png" ), 10, (double)( GAME_RES_H / 1.1 ) ) );
	}
	else if( menu_id == MENU_OPTIONS )
	{
		GL_Surface *back1 = RenderText( font_normal, "Back", orange );

		item_ypos = ( GAME_RES_H / 3 ) + 150;

		temp_item = new cMenu_Item(); // Video

		temp_item = Auto_Menu( "video.png", "", (unsigned int)item_ypos );
		temp_item->posx = (double)( GAME_RES_W / 20 );
		temp_item->startposx = temp_item->posx;
		handler->Add_MenuItem( temp_item );
		item_ypos += 70;
		temp_item = new cMenu_Item(); // Audio
		temp_item = Auto_Menu( "audio.png", "", (unsigned int)item_ypos );
		temp_item->posx = (double)( GAME_RES_W / 20 );
		temp_item->startposx = temp_item->posx;
		handler->Add_MenuItem( temp_item );
		item_ypos += 70;
		temp_item = new cMenu_Item(); // Controls
		temp_item = Auto_Menu( "controls.png", "", (unsigned int)item_ypos );
		temp_item->posx = (double)( GAME_RES_W / 20 );
		temp_item->startposx = temp_item->posx;
		handler->Add_MenuItem( temp_item );

		temp_item = new cMenu_Item(); // back
		temp_item->image_default->SetImage( back1 );
		temp_item->SetPos( GAME_RES_W / 20 + 25, handler->logo->col_rect.h + 60 );
		temp_item->isquit = 1;
		handler->Add_MenuItem( temp_item, &grey );

		drawlist.push_back( new cSprite( GetImage( "menu/options.png" ), GAME_RES_W / 20, GAME_RES_H / 2.25, 0 ) );
		drawlist.push_back( new cSprite( back1, -100, 0, 1 ) );

		if( options_menu_id == MENU_VIDEO )
		{
			layout_file = "data/gui/layout/menu_video.layout";
			item_ypos = ( GAME_RES_H / 3 ) + 130;

			GL_Surface *video_resolution = RenderText( font_normal, "Resolution", orange );
			GL_Surface *video_bpp = RenderText( font_normal, "Bpp", orange );
			GL_Surface *video_fullscreen = RenderText( font_normal, "Fullscreen", orange );
			GL_Surface *video_apply = RenderText( font_normal, "Apply", orange );

			temp_item = new cMenu_Item(); // Resolution
			temp_item->image_default->SetImage( video_resolution );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			temp_item = new cMenu_Item(); // Bpp
			item_ypos += 50;
			temp_item->image_default->SetImage( video_bpp );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			temp_item = new cMenu_Item(); // Fullscreen
			item_ypos += 50;
			temp_item->image_default->SetImage( video_fullscreen );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			temp_item = new cMenu_Item(); // Apply
			item_ypos += 50;
			temp_item->image_default->SetImage( video_apply );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );

			// Video Info
			vid_w = pPreferences->Screen_W;
			vid_h = pPreferences->Screen_H;
			vid_bpp = pPreferences->Screen_Bpp;
			vid_fullscreen = pPreferences->Fullscreen;

			drawlist.push_back( new cSprite( GetImage( "menu/video.png" ), GAME_RES_W / 2.3, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( GetImage( "menu/items/video.png" ), GAME_RES_W / 2.6, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( video_resolution, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( video_bpp, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( video_fullscreen, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( video_apply, -100, 0, 1 ) );

			handler->Set_Active( 0 );
		}
		else if( options_menu_id == MENU_AUDIO )
		{
			GL_Surface *audio_music = RenderText( font_normal, "Music", orange );
			GL_Surface *audio_sounds = RenderText( font_normal, "Sounds", orange );

			layout_file = "data/gui/layout/menu_audio.layout";
			item_ypos = ( GAME_RES_H / 3 ) + 130;

			temp_item = new cMenu_Item(); // Music
			temp_item->image_default->SetImage( audio_music );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			temp_item = new cMenu_Item(); // Sounds
			item_ypos += 75;
			temp_item->image_default->SetImage( audio_sounds );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );

			drawlist.push_back( new cSprite( GetImage( "menu/audio.png" ), GAME_RES_W / 2.3, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( GetImage( "menu/items/audio.png" ), GAME_RES_W / 2.6, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( audio_music, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( audio_sounds, -100, 0, 1 ) );

			handler->Set_Active( 1 );
		}
		else if( options_menu_id == MENU_CONTROLS )
		{
			GL_Surface *controls_up = RenderText( font_normal, "Jump", orange );
			GL_Surface *controls_down = RenderText( font_normal, "Duck", orange );
			GL_Surface *controls_left = RenderText( font_normal, "Left", orange );
			GL_Surface *controls_right = RenderText( font_normal, "Right", orange );
			GL_Surface *controls_shoot = RenderText( font_normal, "Shoot", orange );
			GL_Surface *controls_usejoystick = RenderText( font_normal, "Joystick", orange );

			layout_file = "data/gui/layout/menu_controls.layout";
			item_ypos = ( GAME_RES_H / 3 ) + 130;

			temp_item = new cMenu_Item(); // Up
			temp_item->image_default->SetImage( controls_up );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			controls_up_keyname->SetPos( GAME_RES_W / 2, item_ypos );
			temp_item = new cMenu_Item(); // Down
			item_ypos += 30;
			temp_item->image_default->SetImage( controls_down );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			controls_down_keyname->SetPos( GAME_RES_W / 2, item_ypos );
			temp_item = new cMenu_Item(); // Left
			item_ypos += 30;
			temp_item->image_default->SetImage( controls_left );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			controls_left_keyname->SetPos( GAME_RES_W / 2, item_ypos );
			temp_item = new cMenu_Item(); // Right
			item_ypos += 30;
			temp_item->image_default->SetImage( controls_right );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			controls_right_keyname->SetPos( GAME_RES_W / 2, item_ypos );
			temp_item = new cMenu_Item(); // Shoot
			item_ypos += 30;
			temp_item->image_default->SetImage( controls_shoot );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			controls_shoot_keyname->SetPos( GAME_RES_W / 2, item_ypos );
			temp_item = new cMenu_Item(); // use joystick
			item_ypos += 50;
			temp_item->image_default->SetImage( controls_usejoystick );
			temp_item->SetPos( GAME_RES_W / 3, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );

			drawlist.push_back( new cSprite( GetImage( "menu/controls.png" ), GAME_RES_W / 2.3, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( GetImage( "menu/items/controls.png" ), GAME_RES_W / 2.75, GAME_RES_H / 2.25 ) );
			drawlist.push_back( new cSprite( controls_up, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( controls_down, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( controls_left, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( controls_right, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( controls_shoot, -100, 0, 1 ) );
			drawlist.push_back( new cSprite( controls_usejoystick, -100, 0, 1 ) );

			handler->Set_Active( 2 );
		}
	}
	else if( menu_id == MENU_LOAD )
	{
		GL_Surface *back1 = RenderText( font_normal, "Back", orange );

		item_ypos = ( GAME_RES_H / 3 ) + 90;

		GetSavedGames();

		// savegame descriptions
		for( unsigned int i = 0; i < savegame_temp.size(); i++ )
		{
			temp_item = new cMenu_Item();
			temp_item->image_default->SetImage( savegame_temp[i]->image );
			temp_item->SetPos( GAME_RES_W / 5, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			
			item_ypos += temp_item->image_default->col_rect.h;
		}

		temp_item = new cMenu_Item(); // back
		temp_item->image_default->SetImage( back1 );
		temp_item->SetPos( GAME_RES_W / 18, handler->logo->col_rect.h + 50 );
		temp_item->isquit = 1;
		handler->Add_MenuItem( temp_item, &grey );

		
		drawlist.push_back( new cSprite( GetImage( "menu/load.png" ), GAME_RES_W / 20, GAME_RES_H / 2.4 ) );
		drawlist.push_back( new cSprite( back1, -100, 0, 1 ) );
	}
	else if( menu_id == MENU_SAVE )
	{
		GL_Surface *back1 = RenderText( font_normal, "Back", orange );

		item_ypos = ( GAME_RES_H / 3 ) + 90;

		GetSavedGames();

		// savegame descriptions
		for( unsigned int i = 0; i < savegame_temp.size(); i++ )
		{
			temp_item = new cMenu_Item();
			temp_item->image_default->SetImage( savegame_temp[i]->image );
			temp_item->SetPos( GAME_RES_W / 5, item_ypos );
			handler->Add_MenuItem( temp_item, &grey );
			
			item_ypos += temp_item->image_default->col_rect.h;
		}

		temp_item = new cMenu_Item(); // back
		temp_item->image_default->SetImage( back1 );
		temp_item->SetPos( GAME_RES_W / 18, handler->logo->col_rect.h + 50 );
		temp_item->isquit = 1;
		handler->Add_MenuItem( temp_item, &grey );

		drawlist.push_back( new cSprite( GetImage( "menu/save.png" ), GAME_RES_W / 20, GAME_RES_H / 2.4 ) );
		drawlist.push_back( new cSprite( back1, -100, 0, 1 ) );
	}

	glClearColor( 1, 1, 1, 1 ); // white

	Window *guiwindow = WindowManager::getSingleton().loadWindowLayout( layout_file );
	pGuiSystem->setGUISheet( guiwindow );

	if( menu_id == MENU_MAIN )
	{
		StaticText *text_version = (StaticText *)WindowManager::getSingleton().getWindow( "text_version" );
		text_version->setText( "Version " VERSION );
	}
	if( menu_id == MENU_OPTIONS )
	{
		if( options_menu_id == MENU_VIDEO )
		{
			// ### Resolution
			Combobox *combo_resolution = (Combobox *)WindowManager::getSingleton().getWindow( "combo_resolution" );

			ListboxTextItem *item = new ListboxTextItem( "800x600", 1 );
			item->setTextColours( colour( 1, 1, 0 ) );
			combo_resolution->addItem( item );
			item = new ListboxTextItem( (utf8*)"1024x768", 2 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_resolution->addItem( item );
			item = new ListboxTextItem( (utf8*)"1280x1024", 3 );
			item->setTextColours( colour( 0.1f, 0.5f, 0.9f ) );
			combo_resolution->addItem( item );
			item = new ListboxTextItem( (utf8*)"1600x1200", 4 );
			item->setTextColours( colour( 0.5f, 0.8f, 1 ) );
			combo_resolution->addItem( item );
			item = new ListboxTextItem( (utf8*)"2048x1536", 5 );
			item->setTextColours( colour( 0.8f, 0.2f, 0.9f ) );
			combo_resolution->addItem( item );

			string temp = int_to_string( pPreferences->Screen_W ) + "x" + int_to_string( pPreferences->Screen_H );
			combo_resolution->setText( temp );

			combo_resolution->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Video_res_sel, this ) );

			// ### BPP
			Combobox *combo_bpp = (Combobox *)WindowManager::getSingleton().getWindow( "combo_bpp" );

			item = new ListboxTextItem( (utf8*)"16", 0 );
			item->setTextColours( colour( 1, 0.6f, 0.3f ) );
			combo_bpp->addItem( item );
			item = new ListboxTextItem( (utf8*)"32", 1 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_bpp->addItem( item );

			combo_bpp->setText( int_to_string( pPreferences->Screen_Bpp ).c_str() );

			combo_bpp->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Video_bpp_sel, this ) );

			// ### Fullscreen
			Combobox *combo_fullscreen = (Combobox *)WindowManager::getSingleton().getWindow( "combo_fullscreen" );

			item = new ListboxTextItem( "On", 0 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_fullscreen->addItem( item );
			item = new ListboxTextItem( "Off", 1 );
			item->setTextColours( colour( 0, 0, 1 ) );
			combo_fullscreen->addItem( item );

			if( pPreferences->Fullscreen )
			{
				combo_fullscreen->setText( "On" );
			}
			else
			{
				combo_fullscreen->setText( "Off" );
			}

			combo_fullscreen->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Video_fullscreen_sel, this ) );
		}
		else if( options_menu_id == MENU_AUDIO )
		{
			// ### Music
			Combobox *combo_music = (Combobox *)WindowManager::getSingleton().getWindow( "combo_music" );

			ListboxTextItem *item = new ListboxTextItem( "On", 0 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_music->addItem( item );
			item = new ListboxTextItem( (utf8*)"Off", 1 );
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

			combo_music->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Audio_music_sel, this ) );

			// music volume slider
			Slider *slider_music = (Slider *)WindowManager::getSingleton().getWindow( "slider_music_volume" );
			slider_music->setCurrentValue( (float)pAudio->music_volume );
			slider_music->subscribeEvent( Slider::EventValueChanged, Event::Subscriber( &cMainMenu::Menu_Audio_music_vol_changed, this ) );
			

			// ### Sounds
			Combobox *combo_sounds = (Combobox *)WindowManager::getSingleton().getWindow( "combo_sounds" );

			item = new ListboxTextItem( (utf8*)"On", 0 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_sounds->addItem( item );
			item = new ListboxTextItem( (utf8*)"Off", 1 );
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

			combo_sounds->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Audio_sound_sel, this ) );

			// sound volume slider
			Slider *slider_sound = (Slider *)WindowManager::getSingleton().getWindow( "slider_sound_volume" );
			slider_sound->setCurrentValue( (float)pAudio->sound_volume );
			slider_sound->subscribeEvent( Slider::EventValueChanged, Event::Subscriber( &cMainMenu::Menu_Audio_sound_vol_changed, this ) );
		}
		else if( options_menu_id == MENU_CONTROLS )
		{
			// ### Joystick
			Combobox *combo_joystick = (Combobox *)WindowManager::getSingleton().getWindow( (utf8*)"combo_joystick" );

			ListboxTextItem *item = new ListboxTextItem( (utf8*)"On", 0 );
			item->setTextColours( colour( 0, 1, 0 ) );
			combo_joystick->addItem( item );
			item = new ListboxTextItem( (utf8*)"Off", 1 );
			item->setTextColours( colour( 0, 0, 1 ) );
			combo_joystick->addItem( item );

			if( pPreferences->Joy_enabled )
			{
				combo_joystick->setText( "On" );
			}
			else
			{
				combo_joystick->setText( "Off" );
			}

			combo_joystick->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cMainMenu::Menu_Controls_joystick_sel, this ) );
		}
	}

	while( !leave )
	{
		UpdateGeneric();
		ResetScreen();
		// animation
		pMenu_AnimManager->Update();
		pMenu_AnimManager->Draw();
		// gui
		handler->Draw();

		// menu items
		for( unsigned int i = 0; i < drawlist.size(); i++ )
		{
			drawlist[i]->Draw();
		}

		// menus
		if( menu_id == MENU_MAIN )
		{
			Menu_Action();
		}
		else if( menu_id == MENU_OPTIONS )
		{
			if( options_menu_id == MENU_VIDEO )
			{
				if( vid_fullscreen )
				{
					fullscreen->Draw();
				}
				Menu_Video_Action();
			}
			else if( options_menu_id == MENU_AUDIO )
			{
				if( pAudio->music_enabled ) // Music
				{
					music->Draw();
				}
				if( pAudio->sound_enabled ) // Sounds
				{
					sounds->Draw();
				}
				Menu_Audio_Action();
			}
			else if( options_menu_id == MENU_CONTROLS )
			{
				controls_up_keyname->Draw();
				controls_down_keyname->Draw();
				controls_left_keyname->Draw();
				controls_right_keyname->Draw();
				controls_shoot_keyname->Draw();
				if( pPreferences->Joy_enabled )
				{
					gamepad->Draw();
				}
				Menu_Controls_Action();
			}
			
			Menu_Options_Action();
		}
		else if( menu_id == MENU_LOAD )
		{
			Menu_Load_Action();
		}
		else if( menu_id == MENU_SAVE )
		{
			Menu_Save_Action();
		}

		pGuiSystem->renderGUI();
		SDL_GL_SwapBuffers();

		Framerate.Update();
	}

	WindowManager::getSingleton().destroyWindow( guiwindow );

	for( unsigned int i = 0; i < drawlist.size(); i++ )
	{
		delete drawlist[i];
	}
	
	drawlist.clear();

	if( menu_id == MENU_MAIN )
	{
		// if ESC got pressed
		if( leave == 2 && !pPlayer->points && !Leveleditor_Mode )	
		{
			done = 1;
		}
	}

	if( pLevel->is_Loaded() )
	{
		pPlayer->Update_Camera( 1 );
		Game_Mode = MODE_LEVEL;
	}

	if( next_menu != MENU_NOTHING )
	{
		Enter_Menu( next_menu );
	}
}


void cMainMenu :: Menu_Action( void )
{
	if( !action )
	{
		return;
	}

	action = 0;

	if( handler->active == 0 ) // Start
	{
		leave = 1;
	}
	else if( handler->active == 1 ) // Options
	{
		next_menu = MENU_OPTIONS;
		leave = 1;
	}
	else if( handler->active == 2 )	// Load
	{
		next_menu = MENU_LOAD;
		leave = 1;
	}
	else if( handler->active == 3 ) // Save
	{
		next_menu = MENU_SAVE;
		leave = 1;
	}
	else if( handler->active == 4 ) // Quit
	{
		leave = 1;
		done = 1;
	}
}

/* *** *** *** *** *** *** *** *** Menu Options *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Options_Action( void )
{
	if( !action )
	{
		return;
	}

	if( handler->active > 3 ) // only menu actions
	{
		return;
	}

	action = 0;


	if( handler->active == 0 ) // Video
	{
		next_menu = MENU_OPTIONS;
		options_menu_id	= MENU_VIDEO;
		leave = 1;
	}
	else if( handler->active == 1 ) // Audio
	{
		next_menu = MENU_OPTIONS;
		options_menu_id	= MENU_AUDIO;
		leave = 1;
	}
	else if( handler->active == 2 ) // Controls
	{
		next_menu = MENU_OPTIONS;
		options_menu_id = MENU_CONTROLS;
		leave = 1;
	}

	else if( handler->active == 3 || leave == 2 ) // back
	{
		next_menu = MENU_MAIN;
		pPreferences->Save();
		leave = 1;
	}
}

/* *** *** *** *** *** *** *** *** Menu Load *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Load_Action( void )
{
	if( !action )
	{
		return;
	}

	action = 0;

	if( handler->active == 9 || leave == 2 ) // back
	{
		next_menu = MENU_MAIN;
		leave = 1;
	}
	else if( pSavegame->is_valid( handler->active + 1 ) )
	{
		pAudio->PauseMusic();
		pAudio->PlaySound( SOUNDS_DIR "/savegame_load.ogg" );

		pLevel->Load_Savegame( handler->active + 1 );
		leave = 1;
	}
}

/* *** *** *** *** *** *** *** *** Menu Save *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Save_Action( void )
{
	if( !action )
	{
		return;
	}

	action = 0;

	if( handler->active == 9 || leave == 2 ) // back
	{
		next_menu = MENU_MAIN;
		leave = 1;
	}
	else if( pOverWorld->Maryo->current_waypoint >= 0 || pLevel->is_Loaded() ) // savegame
	{
		pAudio->PlaySound( SOUNDS_DIR "/beep_1.ogg" );
		string tmp_Descripion = Set_SaveDescription( handler->active + 1 );
		
		Framerate.Reset();

		if( tmp_Descripion.compare( "Not enough Points" ) == 0 ) 
		{
			next_menu = MENU_MAIN;
			leave = 1;
			return;
		}
		else if( tmp_Descripion.length() > 0 )
		{
			pAudio->PlaySound( SOUNDS_DIR "/savegame_save.ogg" );
			pLevel->Save_Savegame( handler->active + 1, tmp_Descripion );

#ifndef _DEBUG
			pointsdisplay->SetPoints( pPlayer->points - 3000 );
#endif
			leave = 1;
		}
	}
}

string cMainMenu :: Set_SaveDescription( unsigned int save_slot )
{
	if( save_slot <= 0 || save_slot >= 10 )
	{
		return "Wrong Savefile";
	}
#ifndef _DEBUG // saving costs no Points in debug builds
	if( pPlayer ) 
	{
		if( pLevel->is_Loaded() && pPlayer->points < 3000 )
		{
			GL_rect temp_rect;
			temp_rect.x = GAME_RES_W / 2.5;
			temp_rect.y = GAME_RES_H / 2;
			cSprite *temp_sprite = new cSprite( RenderText( font_normal, "Not enough Points [ 3000 needed ]", orange ), temp_rect.x, temp_rect.y, 1 );
			temp_rect.x -= 40;
			temp_rect.y -= 10;
			temp_rect.w = temp_sprite->rect.w + 80;
			temp_rect.h = temp_sprite->rect.h + 20;
			DrawRect( &temp_rect, &grey );
			temp_rect.x += 5;
			temp_rect.y += 5;
			temp_rect.w -= 10;
			temp_rect.h -= 10;
			DrawRect( &temp_rect, &black );
			temp_sprite->Draw();
			delete temp_sprite;
			temp_sprite = NULL;
			
			SDL_GL_SwapBuffers();

			bool sub_done = 0;

			ClearEvents();

			while( !sub_done )
			{
				while( SDL_PollEvent( &event ) )
				{
					if( event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_JOYBUTTONDOWN )
					{
						sub_done = 1;
						break;
					}
				}

				SDL_GL_SwapBuffers();
			}

			return "Not enough Points";
		}
	}
	else
	{
		return NULL;
	}
#endif
	string Save_Description;

	bool auto_controls = 0;

	if( pSavegame->is_valid( save_slot ) ) // If Savegame exists use old Description
	{
		Save_Description.clear();
		Save_Description = pSavegame->Get_Description( save_slot, 1 ); // Only the Description
	}
	else
	{
		Save_Description = " No Description "; // Standard Description
		auto_controls = 1;
	}

	GL_rect temp_rect;
	temp_rect.x = GAME_RES_W / 3;
	temp_rect.y = GAME_RES_H / 1.9;
	temp_rect.w = 450;
	temp_rect.h = 80;

	temp_rect.x -= 50;
	temp_rect.y -= 40;
	DrawRect( &temp_rect, &lightblue );
	temp_rect.x += 50;
	temp_rect.y += 40;

	return EditMessageBox( Save_Description, "Enter Description", temp_rect.x, temp_rect.y, auto_controls );
}

/* *** *** *** *** *** *** *** *** Menu Video *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Video_Action( void )
{
	if( !action )
	{
		return;
	}

	if( handler->active < 4 ) // only video actions
	{
		return;
	}

	action = 0;

	if( handler->active == 4 ) // resolution
	{
		if( vid_w <= 800 && vid_h <= 600 )
		{
			vid_w = 1024;
			vid_h = 768;
		}
		else if( vid_w == 1024 && vid_h == 768 )
		{
			vid_w = 1280;
			vid_h = 1024;
		}
		else if( vid_w == 1280 && vid_h == 1024 )
		{
			vid_w = 1600;
			vid_h = 1200;
		}
		else if( vid_w == 1600 && vid_h == 1200 )
		{
			vid_w = 2048;
			vid_h = 1536;
		}
		else if( vid_w >= 2048 && vid_h >= 1536 )
		{
			vid_w = 800;
			vid_h = 600;
		}

		string temp = int_to_string( vid_h );
		temp.insert( 0, "x" );
		temp.insert( 0, int_to_string( vid_w ) );

		WindowManager::getSingleton().getWindow( (utf8*)"combo_resolution" )->setText( temp );
	}
	else if( handler->active == 5 ) // bpp
	{
		if( vid_bpp == 16 )
		{
			vid_bpp = 32;
		}
		else if( vid_bpp == 32 )
		{
			vid_bpp = 16;
		}

		WindowManager::getSingleton().getWindow( (utf8*)"combo_bpp" )->setText( int_to_string( vid_bpp ).c_str() );	
	}
	else if( handler->active == 6 ) // fullscreen
	{
		vid_fullscreen = !vid_fullscreen;

		if( vid_fullscreen )
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_fullscreen" )->setText( "On" );	
		}
		else
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_fullscreen" )->setText( "Off" );
		}
	}
	else if( handler->active == 7 ) // Apply
	{
		if( pPreferences->Fullscreen != vid_fullscreen )
		{
			Toggle_Fullscreen();
		}

		pPreferences->Screen_W = vid_w;
		pPreferences->Screen_H = vid_h;
		pPreferences->Screen_Bpp = vid_bpp;
		pPreferences->Fullscreen = vid_fullscreen;
	}
}

bool cMainMenu :: Menu_Video_res_sel( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	string temp = item->getText().c_str();

	unsigned int w = string_to_int( temp.substr( 0, temp.find( "x" ) ) );
	unsigned int h = string_to_int( temp.substr( temp.find( "x" ) + 1, temp.length() ) );

	if( !TestVideo( w, h, vid_bpp ) )
	{
		return 0;
	}

	vid_w = w;
	vid_h = h;

	return 1;
}

bool cMainMenu :: Menu_Video_bpp_sel( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	unsigned int bpp = string_to_int( item->getText().c_str() );

	if( !TestVideo( vid_w, vid_h, bpp ) )
	{
		return 0;
	}

	vid_bpp = bpp;

	return 1;
}

bool cMainMenu :: Menu_Video_fullscreen_sel( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool bfullscreen = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		bfullscreen = 1;
	}

	vid_fullscreen = bfullscreen;

	return 1;
}

/* *** *** *** *** *** *** *** *** Menu Audio *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Audio_Action( void )
{
	if( !action )
	{
		return;
	}

	if( handler->active < 4 ) // only audio actions
	{
		return;
	}

	action = 0;

	if( handler->active == 4 ) // Music
	{
		pAudio->ToggleMusic();

		if( pAudio->music_enabled )
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_music" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_music" )->setText( "Off" );
		}
	} 
	else if( handler->active == 5 ) // Sounds
	{
		pAudio->ToggleSounds();

		if( pAudio->sound_enabled )
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_sounds" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_sounds" )->setText( "Off" );
		}
	}
}

bool cMainMenu :: Menu_Audio_music_sel( const EventArgs &event )
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

	pAudio->music_enabled = bmusic;

	return 1;
}

bool cMainMenu :: Menu_Audio_music_vol_changed( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Uint8 val = (Uint8)static_cast<Slider *>( windowEventArgs.window )->getCurrentValue();

	pAudio->SetMusicVolume( val );

	return 1;
}

bool cMainMenu :: Menu_Audio_sound_sel( const EventArgs &event )
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

	pAudio->sound_enabled = bsound;

	return 1;
}

bool cMainMenu :: Menu_Audio_sound_vol_changed( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	Uint8 val = (Uint8)static_cast<Slider *>( windowEventArgs.window )->getCurrentValue();

	pAudio->SetSoundVolume( val );

	return 1;
}

/* *** *** *** *** *** *** *** *** Menu Controls *** *** *** *** *** *** *** *** *** *** */

void cMainMenu :: Menu_Controls_Action( void )
{
	if( !action )
	{
		return;
	}

	if( handler->active < 4 ) // only controls actions
	{
		return;
	}

	action = 0;

	if( handler->active == 9 ) // Joystick
	{
		pPreferences->Joy_enabled = !pPreferences->Joy_enabled;

		if( pPreferences->Joy_enabled )
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_joystick" )->setText( "On" );
		}
		else
		{
			WindowManager::getSingleton().getWindow( (utf8*)"combo_joystick" )->setText( "Off" );
		}
	}

	if( handler->active < 4 || handler->active > 8 )
	{
		return;
	}

	// keyboard key change
	GL_rect temp_rect;
	temp_rect.x = GAME_RES_W / 2.4;
	temp_rect.y = GAME_RES_H / 2;
	cSprite *temp_sprite = new cSprite( RenderText( font_normal, "Press a key", orange ), (double)temp_rect.x, (double)temp_rect.y, 1 );
	temp_rect.x -= 40;
	temp_rect.y -= 10;
	temp_rect.w = temp_sprite->col_rect.w + 80;
	temp_rect.h = temp_sprite->col_rect.h + 20;
	DrawRect( &temp_rect, &grey );
	temp_rect.x += 5;
	temp_rect.y += 5;
	temp_rect.w -= 10;
	temp_rect.h -= 10;
	DrawRect( &temp_rect, &black );
	temp_sprite->Draw();
	delete temp_sprite;
	
	SDL_GL_SwapBuffers();

	bool sub_done = 0;

	while( !sub_done )
	{
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_KEYDOWN )
			{
				if( event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_BACKSPACE )
				{
					sub_done = 1;
					break;
				}

				switch( handler->active )
				{
				case 4:
				{
					pPreferences->Key_up = event.key.keysym.sym; 
					controls_up_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_up ), black ), 1, 1 );
					break;
				}
				case 5:
				{
					pPreferences->Key_down = event.key.keysym.sym; 
					controls_down_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_down ), black ), 1, 1 );
					break;
				}
				case 6:
				{
					pPreferences->Key_left = event.key.keysym.sym; 
					controls_left_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_left ), black ), 1, 1 );
					break;
				}
				case 7:
				{
					pPreferences->Key_right = event.key.keysym.sym; 
					controls_right_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_right ), black ), 1, 1 );
					break;
				}
				case 8:
				{
					pPreferences->Key_shoot = event.key.keysym.sym; 
					controls_shoot_keyname->SetImage( RenderText( font_normal, SDL_GetKeyName( pPreferences->Key_shoot ), black ), 1, 1 );
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
	}
}

bool cMainMenu :: Menu_Controls_joystick_sel( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox*>( windowEventArgs.window )->getSelectedItem();

	bool joystick = 0;

	if( item->getText().compare( "On" ) == 0 )
	{
		joystick = 1;
	}

	pPreferences->Joy_enabled = joystick;

	return 1;
}

// ############################ Menu's End ####################################### //

void cMainMenu :: GetSavedGames( void )
{
	for( unsigned int i = 0; i < savegame_temp.size(); i++ )
	{
		savegame_temp[i]->SetImage( RenderText( font_normal, pSavegame->Get_Description( i + 1 ), greenyellow ), 1, 1 );
	}
}
