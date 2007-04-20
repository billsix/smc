/***************************************************************************
           hud.cpp  -  human user interfaces in the game
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

#include "../gui/hud.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../core/camera.h"
#include "../video/font.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../core/obj_manager.h"
#include "../objects/bonusbox.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

cHudSprite :: cHudSprite( GL_Surface *new_image /* = NULL */, float x /* = 0 */, float y /* = 0 */, bool del_img /* = 0 */ )
: cSprite( new_image, x, y, del_img )
{
	player_range = 0;
	posz = 0.13f;

	Set_Ignore_Camera( 1 );
}

cHudSprite :: ~cHudSprite( void )
{

}

cHudSprite *cHudSprite :: Copy( void )
{
	cHudSprite *hud_sprite = new cHudSprite( start_image, posx, posy );

	hud_sprite->type = type;
	hud_sprite->sprite_array = sprite_array;
	hud_sprite->massivetype = massivetype;
	hud_sprite->Set_Ignore_Camera( no_camera );
	hud_sprite->Set_Shadow_Pos( shadow_pos );
	hud_sprite->Set_Shadow_Color( shadow_color );

	return hud_sprite;
}

/* *** *** *** *** *** *** *** cHudManager *** *** *** *** *** *** *** *** *** *** */

cHudManager :: cHudManager( void )
{
	loaded = 0;

	Load();
}

cHudManager :: ~cHudManager( void )
{
	Unload();
}

void cHudManager :: Load( void )
{
	if( !loaded && objects.size() )
	{
		Unload();
	}
	else if( loaded )
	{
		Update_Text();
		return;
	}

	// Menu Background ( Maryo head and the Goldpiece )
	Add( (cHudSprite *)new cMenuBackground() );
	// Point Display
	pointsdisplay = new cPlayerPoints();
	Add( (cHudSprite *)pointsdisplay );
	// Time Display
	timedisplay = new cTimeDisplay();
	Add( (cHudSprite *)timedisplay );
	// Live Display
	livedisplay = new cLiveDisplay();
	Add( (cHudSprite *)livedisplay );
	// Gold Display
	golddisplay = new cGoldDisplay();
	Add( (cHudSprite *)golddisplay );
	// Itembox
	Itembox = new cItemBox();
	Add( (cHudSprite *)Itembox );
	// Debug Display
	debugdisplay = new cDebugDisplay();
	Add( (cHudSprite *)debugdisplay );

	loaded = 1;
}

void cHudManager :: Unload( void )
{
	if( !objects.size() )
	{
		return;
	}
	
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();

	livedisplay = NULL;
	golddisplay = NULL;
	pointsdisplay = NULL;
	timedisplay = NULL;
	debugdisplay = NULL;
	Itembox = NULL;
	
	loaded = 0;
}

void cHudManager :: Add( cHudSprite *obj )
{
	if( !obj )
	{
		return;
	}

	objects.push_back( obj );
}

void cHudManager :: Update_Text( void )
{
	// note : update the lifedisplay before you update the timedisplay
	
	if( objects.size() > 0 )
	{
		cMenuBackground *item = (cMenuBackground *)objects[0];

		if( Game_Mode != MODE_OVERWORLD )
		{
			// goldpiece
			item->rect_goldpiece.y = item->rect_maryo_head.y + 6;
		}
		else
		{
			// goldpiece
			item->rect_goldpiece.y = 4;
		}
	}

	if( livedisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			livedisplay->Set_Pos( GAME_RES_W - GAME_RES_W / 10, 18.0f );
		}
		else
		{
			livedisplay->Set_Pos( GAME_RES_W - GAME_RES_W / 7.5f, 4 );
		}

		livedisplay->AddLives( 0 );
	}

	if( golddisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			golddisplay->Set_Pos( 280.0, 18.0 );
		}
		else
		{
			golddisplay->Set_Pos( 280.0, 1.0 );
		}

		golddisplay->AddGold( 0 );
	}

	if( pointsdisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pointsdisplay->Set_Pos( 50.0, 18.0 );
		}
		else
		{
			pointsdisplay->Set_Pos( 50.0, 4 );
		}

		pointsdisplay->Add_Points( 0 );
	}

	if( timedisplay )
	{
		timedisplay->Set_Pos( GAME_RES_W * 0.75, 18.0 );
		timedisplay->Update();
	}

	if( debugdisplay )
	{
		debugdisplay->Set_Pos( GAME_RES_W / 2 - 100, 80.0 );
		debugdisplay->Update();
	}

	if( Itembox ) 
	{
		Itembox->Set_Pos( GAME_RES_W / 2 - 20, 10 );
		Itembox->Update();
	}
}

void cHudManager :: Update( void )
{
	// update HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update();
	}
}

void cHudManager :: Draw( void )
{
	// draw HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

unsigned int cHudManager :: Get_Size( void )
{
	return (unsigned int)objects.size();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cHudManager *pHudManager = NULL;

/* *** *** *** *** *** *** PointsText *** *** *** *** *** *** *** *** *** *** *** */

PointsText :: PointsText( void )
: cHudSprite()
{
	points = 0;
	vely = 0;
}

PointsText :: ~PointsText( void )
{
	//
}

/* *** *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** */

cMenuBackground :: cMenuBackground( float x /* = 0 */, float y /* = 0 */ )
: cHudSprite( NULL, x, y )
{
	type = TYPE_MENUBG;
	sprite_array = ARRAY_HUD;

	maryo_head = pVideo->Get_Surface( "game/maryo_l.png" );
	goldpiece = pVideo->Get_Surface( "game/gold_m.png" );

	if(	!maryo_head || !goldpiece )
	{
		printf( "Error : MenuBackground images loading failed\n" );
		return;
	}

	// maryo head
	rect_maryo_head.x = GAME_RES_W - GAME_RES_W / 11 + 25;
	rect_maryo_head.y = 15;
	// goldpiece
	rect_goldpiece.x = 250;
}

cMenuBackground :: ~cMenuBackground( void )
{
	//
}

void cMenuBackground :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU )
	{
		return;
	}

	// maryo head
	if( Game_Mode != MODE_OVERWORLD )
	{
		maryo_head->Blit( rect_maryo_head.x, rect_maryo_head.y, posz );
	}

	// goldpiece
	goldpiece->Blit( rect_goldpiece.x, rect_goldpiece.y, posz );
}

/* *** *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** */

cStatusText :: cStatusText( float x /* = 0 */, float y /* = 0 */ )
: cHudSprite( NULL, x, y )
{
	type = TYPE_STATUSTEXT;
	sprite_array = ARRAY_HUD;

	Set_Shadow( black, 1.5f );
}

cStatusText :: ~cStatusText( void )
{
	//
}

void cStatusText :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( Game_Mode == MODE_MENU )
	{
		return;
	}

	cHudSprite::Draw();
}

/* *** *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints :: cPlayerPoints( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_POINTDISPLAY;
	
	Set_Points( pPlayer->points );

	pointsobjects.reserve( 50 );
}

cPlayerPoints :: ~cPlayerPoints( void )
{
	//
}

void cPlayerPoints :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU )
	{
		return;
	}
	
	cHudSprite::Draw( request );

	// draw small points
	for( PointsTextList::iterator itr = pointsobjects.begin(); itr != pointsobjects.end(); )
	{
		// get object pointer
		PointsText *obj = (*itr);
		
		// if finished
		if( !obj->image )
		{
			itr = pointsobjects.erase( itr );
			delete obj;
		}
		// active
		else
		{
			obj->vely -= obj->vely * 0.01f * pFramerate->speedfactor;
			obj->posy += obj->vely * pFramerate->speedfactor;

			if( obj->vely > -1.0 )
			{
				obj->Set_Image( NULL );
				continue;
			}

			float x = obj->posx - pCamera->x;
			float y = obj->posy - pCamera->y;

			// out in left
			if( x < 0 )
			{
				x = 5;
			}
			// out in right
			else if( x > GAME_RES_W )
			{
				x = (float)GAME_RES_W - obj->col_rect.w - 2; 
			}

			// out on bottom
			if( y < 0 )
			{
				y = 5;
			}
			// out on top
			else if( y > GAME_RES_H )
			{
				y = (float)GAME_RES_H - obj->col_rect.h - 20;
			}

			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			obj->image->Blit( x, y, posz, request );

			// shadow
			request->shadow_color = black;
			request->shadow_pos = 1;

			// color
			request->color = Color( (Uint8)( 255 - ( obj->points / 150 ) ), (Uint8)( 255 - ( obj->points / 150 ) ), (Uint8)( 255 - ( obj->points / 30 ) ) );

			// add request
			pRenderer->Add( request );

			++itr;
		}
	}
}

void cPlayerPoints :: Set_Points( unsigned int points )
{
	pPlayer->points = points;
	
	char text[70];
	sprintf( text, "Points %08d", (int)pPlayer->points );
	Set_Image( pFont->RenderText( pFont->font_normal, text, white ), 0, 1 );
}

void cPlayerPoints :: Add_Points( unsigned int points, float x /* = 0 */, float y /* = 0 */, string strtext /* = "" */, Color color /* = (Uint8)255 */, bool allow_multiplier /* = 0 */ )
{
	if( !pPlayer )
	{
		return;
	}
	
	if( allow_multiplier )
	{
		points = (unsigned int)( pPlayer->kill_multiplier * (float)points );
	}

	Set_Points( pPlayer->points + points );

	if( x == 0 || y == 0 || pointsobjects.size() > 50 )
	{
		return;
	}

	// if empty set the points as text
	if( strtext.empty() )
	{
		strtext = int_to_string( points );
	}

	PointsText *new_obj = new PointsText();
	new_obj->Set_Image( pFont->RenderText( pFont->font_small, strtext, color ), 1, 1 );

	new_obj->Set_Pos( x, y );
	new_obj->vely = -1.4f;
	new_obj->points = points;
	
	// check if it collides with an already active points text
	for( PointsTextList::iterator itr = pointsobjects.begin(); itr != pointsobjects.end(); ++itr )
	{
		// get object pointer
		PointsText *obj = (*itr);

		if( Col_Box( &new_obj->rect, &obj->col_rect ) )
		{
			new_obj->Move( obj->col_rect.w + 5, 0, 1 );
		}
	}

	pointsobjects.push_back( new_obj );
}

void cPlayerPoints :: Clear( void )
{
	for( PointsTextList::iterator itr = pointsobjects.begin(), itr_end = pointsobjects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	pointsobjects.clear();
}

/* *** *** *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** */

cTimeDisplay :: cTimeDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_GAMETIMEDISPLAY;

	counter = -1;
}

cTimeDisplay :: ~cTimeDisplay( void )
{
	//
}

void cTimeDisplay :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	counter += pFramerate->speedfactor;

	int time = (int)( counter / DESIRED_FPS );	// time = seconds since start
	int minutes = (int)( time / 60 );

	sprintf( text, "Time %02d:%02d", minutes, time - ( minutes * 60 ) );

	if( strcmp( text, text_old ) != 0 )
	{
		strcpy( text_old, text );
		Set_Image( pFont->RenderText( pFont->font_normal, text, white ), 0, 1 );
	}

	cHudSprite::Draw();
}

/* *** *** *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** */

cItemBox :: cItemBox( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_ITEMBOXDISPLAY;

	Set_Image( pVideo->Get_Surface( "game/itembox.png" ) );
	// disable shadow
	Set_Shadow_Pos( 0 );

	box_color = white;

	item_counter = 0;
	item_counter_mod = 0;
	item_id = TYPE_UNDEFINED;

	item = new cMovingSprite();
	item->Set_Ignore_Camera( 1 );
	item->player_range = 0;
	item->Set_Massivetype( MASS_MASSIVE );
	item->posz = 0.1299f;
}

cItemBox :: ~cItemBox( void )
{
	delete item;
}

void cItemBox :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	if( pPlayer->maryo_type == MARYO_DEAD )
	{
		item_counter = 90;
	}
	else if( item_counter )
	{
		item->posy += pFramerate->speedfactor * 4;

		if( item_counter_mod )
		{
			item_counter += pFramerate->speedfactor * 10;

			if( item_counter >= 90 )
			{
				item_counter_mod = 0;
				item_counter = 90;
			}
		}
		else
		{
			item_counter -= pFramerate->speedfactor * 10;

			if( item_counter <= 0 )
			{
				item_counter_mod = 1;
				item_counter = 1;
			}
		}

		if( item->posy > GAME_RES_H )
		{
			Reset();
		}

		cObjectCollisionType col_list = item->ColCheckAuto( 0, 0, 0, 0, COLLIDE_ONLY_CHECK );

		// if colliding with the Player
		if( col_list.find( CO_PLAYER ) )
		{
			SpriteType item_id_temp = item_id; // player can set the item back
			Reset();
			pPlayer->Get_Item( item_id_temp, 1 );
		}
	}

	if( item_id && item->image )
	{
		// with alpha
		if( item_counter )
		{
			item->Set_Color( 255, 255, 255, 100 + (Uint8)item_counter );
		}
		
		item->Draw();
	}

	Set_Color( box_color.red, box_color.green, box_color.blue );
	cHudSprite::Draw();
}

void cItemBox :: Set_Item( SpriteType item_type )
{
	pAudio->PlaySound( "itembox_set.ogg" );
	Reset();

	// reset startposition
	item->Set_Pos( 0, 0, 1 );
	// reset color
	item->Set_Color( white );

	if( item_type == TYPE_MUSHROOM_DEFAULT )
	{
		box_color = Color( (Uint8)250, 50, 50 );
		item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_red.png" ) );
	}
	else if( item_type == TYPE_FIREPLANT )
	{
		box_color = Color( (Uint8)250, 200, 150 );
		item->Set_Image( pVideo->Get_Surface( "game/items/fire_flower.png" ) );
	}
	else if( item_type == TYPE_MUSHROOM_BLUE )
	{
		box_color = Color( (Uint8)100, 100, 250 );
		item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_blue.png" ) );
	}


	if( item->image )
	{
		item->Set_Pos( posx - ( ( item->image->w - rect.w ) / 2 ), posy - ( ( item->image->h - rect.h ) / 2 ) );
	}

	item_id = item_type;
}

void cItemBox :: Request_Item( void )
{
	if( !item_id || item_counter ) 
	{
		return;
	}

	pAudio->PlaySound( "itembox_get.ogg" );

	item_counter = 255;
	// draw item with camera
	item->Set_Ignore_Camera( 0 );
	item->Set_Pos( item->posx + pCamera->x, item->posy + pCamera->y );
}

void cItemBox :: Push_back( void )
{
	item_counter = 0;
	item_counter_mod = 0;

	// draw item without camera
	item->Set_Ignore_Camera( 1 );
	item->Set_Pos( item->startposx, item->startposy );
	item->Set_Color( white );
}

void cItemBox :: Reset( void )
{
	item->Set_Ignore_Camera( 1 );
	item_id = TYPE_UNDEFINED;
	item_counter = 0;
	item_counter_mod = 0;
	box_color = white;
}

/* *** *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** */

cDebugDisplay :: cDebugDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_DEBUGDISPLAY;

	level_old = ".";
	text.clear();
	text_old.clear();

	// debug text window
	window_debugtext = WindowManager::getSingleton().loadWindowLayout( "debugtext.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( window_debugtext );
	// debug text
	text_debugtext = (Window *)WindowManager::getSingleton().getWindow( "text_debugmessage" );

	// debug box positions
	float tempx = GAME_RES_W - 200;
	float tempy = GAME_RES_H / 2 - 250;

	for( unsigned int i = 0; i < 20; i++ )
	{
		sprites.push_back( new cHudSprite( NULL, tempx, tempy ) );
		sprites[i]->Set_Shadow( black, 1 );
		sprites[i]->posz = posz;

		// not the framerate text
		if( i > 2 )
		{
			tempy += 19;
		}

		// active box
		if( i > 9 && i < 14 )
		{
			sprites[i]->Set_PosX( sprites[i]->startposx += 5, 1 );
		}
	}

	// fps display position
	sprites[0]->Set_Pos( 15, 5, 1 );
	// average
	sprites[1]->Set_Pos( 280, 5, 1 );
	// speedfactor
	sprites[2]->Set_Pos( 500, 5, 1 );

	// Debug type text
	sprites[4]->Set_Image( pFont->RenderText( pFont->font_small, "Level information", lightblue ), 0, 1 );
	sprites[16]->Set_Image( pFont->RenderText( pFont->font_small, "Player information", lightblue ), 0, 1 );

	passcount = -1;
	masscount = -1;
	enemycount = -1;
	activecount = -1;

	counter = 0;
}

cDebugDisplay :: ~cDebugDisplay( void )
{
	pGuiSystem->getGUISheet()->removeChildWindow( window_debugtext );
	WindowManager::getSingleton().destroyWindow( window_debugtext );

	for( unsigned int i = 0; i < sprites.size(); i++ )
	{
		delete sprites[i];
	}

	sprites.clear();
}

void cDebugDisplay :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	// draw complete debug info
	GameDebugDraw();

	// no text to display
	if( text.empty() )
	{
		return;
	}

	// if time reached hide the text display
	if( counter <= 0 )
	{
		text.clear();
		text_old.clear();

		text_debugtext->setVisible( 0 );
		return;
	}

	// update counter
	counter -= pFramerate->speedfactor;

	// set new text
	if( text.compare( text_old ) != 0 )
	{
		text_old = text;

		// display the new text
		text_debugtext->setText( text );
		text_debugtext->setVisible( 1 );

		// update position
		Font *font = FontManager::getSingleton().getFont( "bluebold_medium" );
		text_debugtext->setXPosition( UDim( 0, GAME_RES_W / 2 - font->getTextExtent( text ) / 2 ) );
	}

}

void cDebugDisplay :: Set_Text( string ntext, float display_time /* = DESIRED_FPS * 2 */ )
{
	text = ntext;

	if( text.empty() )
	{
		text = text_old;
		text_old.empty();

		counter = 0;
		return;
	}

	counter = display_time;
}

void cDebugDisplay :: GameDebugDraw( void )
{
	if( !Game_debug || ( Game_Mode == MODE_LEVEL && pPlayer->maryo_type == MARYO_DEAD ) ) 
	{
		return;
	}

	string temp_text;

	/* ### Extend
	- Other Special ( Active ) Objects
	*/

	// black background
	Color color = blackalpha128;
	pVideo->Draw_Rect( GAME_RES_W - 205, GAME_RES_H / 12, 180, 325, posz - 0.00001f, &color );

	// ### Frames per Second
	temp_text = "FPS : best " + int_to_string( (int)pFramerate->fps_best ) + ", worst " + int_to_string( (int)pFramerate->fps_worst ) + ", current " + int_to_string( (int)pFramerate->fps );
	sprites[0]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	// average
	temp_text = "average " + int_to_string( (int)pFramerate->fps_average );
	sprites[1]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	// speedfactor
	temp_text = "Speedfactor " + float_to_string( pFramerate->speedfactor );
	sprites[2]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );

	// ### Camera position
	temp_text = "Camera : X " + int_to_string( (int)pCamera->x ) + ", Y " + int_to_string( (int)pCamera->y );
	sprites[3]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );

	// ### Level informations
	if( pLevel->levelfile.compare( level_old ) != 0 ) 
	{
		string lvl_text = "Level : " + Get_filename( pLevel->data_file, 0, 0 );
		level_old = pLevel->levelfile;
		
		sprites[5]->Set_Image( pFont->RenderText( pFont->font_very_small, lvl_text, white ), 0, 1 );
	}

	// ### Level Objects
	// # Passive
	if( passcount != (int)(pObjManager->Get_Size_array( ARRAY_PASSIVE ) + pObjManager->Get_Size_array( ARRAY_FRONT_PASSIVE )) ) 
	{
		passcount = pObjManager->Get_Size_array( ARRAY_PASSIVE ) + pObjManager->Get_Size_array( ARRAY_FRONT_PASSIVE );

		temp_text = "Passive Objects : " + int_to_string( passcount );
		sprites[6]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	}
	// # Massive
	if( masscount != (int)pObjManager->Get_Size_array( ARRAY_MASSIVE ) ) 
	{
		masscount = pObjManager->Get_Size_array( ARRAY_MASSIVE );

		temp_text = "Massive Objects : " + int_to_string( masscount );
		sprites[7]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	}
	// # Enemy
	if( enemycount != (int)pObjManager->Get_Size_array( ARRAY_ENEMY ) ) 
	{
		enemycount = pObjManager->Get_Size_array( ARRAY_ENEMY );

		temp_text ="Enemy Objects : " + int_to_string( enemycount );
		sprites[8]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	}
	// # Active
	if( activecount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		activecount = pObjManager->Get_Size_array( ARRAY_ACTIVE );

		temp_text = "Active Objects : " + int_to_string( activecount );
		sprites[9]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );

		// # Active : Halfmassive
		unsigned int halfmassive = 0;

		for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->type == TYPE_HALFMASSIVE )
			{
				halfmassive++;
			}
		}

		temp_text = "Halfmassive : " + int_to_string( halfmassive );
		sprites[10]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
		// # Active : Moving Platform
		unsigned int clouds = 0;

		for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->type == TYPE_MOVING_PLATFORM )
			{
				clouds++;
			}
		}

		temp_text = "Moving Platform : " + int_to_string( clouds );
		sprites[11]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
		// # Active : Goldbox
		unsigned int goldbox = 0;

		for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type == TYPE_GOLDPIECE )
				{
					goldbox++;
				}
			}
		}

		temp_text = "Goldbox : " + int_to_string( goldbox );
		sprites[12]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
		// # Active : Bonusbox
		unsigned int bonusbox = 0;

		for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type != TYPE_GOLDPIECE )
				{
					bonusbox++;
				}
			}
		}

		temp_text = "Bonusbox : " + int_to_string( bonusbox );
		sprites[13]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
		// # Active : Other
		unsigned int active_other = pObjManager->Get_Size_array( ARRAY_ACTIVE );

		for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->type == TYPE_HALFMASSIVE || obj->type == TYPE_BONUSBOX || obj->type == TYPE_MOVING_PLATFORM )
			{
				active_other--;
			}
		}

		temp_text = "Other : " + int_to_string( active_other );
		sprites[14]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	}

	// # Active Objects Rect

	// create request
	cRectRequest *request = new cRectRequest();
	GL_rect r( sprites[10]->posx - 8, sprites[10]->posy, 135, 75 );
	pVideo->Draw_Rect( &r, posz, &white, request );

	// not filled
	request->filled = 0;

	// add request
	pRenderer->Add( request );


	// ### Player informations
	// position
	temp_text = "Position X " + int_to_string( (int)pPlayer->posx ) + " ,Y " + int_to_string( (int)pPlayer->posy );
	sprites[16]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	// velocity
	temp_text = "Velocity X " + int_to_string( (int)pPlayer->velx ) + " ,Y " + int_to_string( (int)pPlayer->vely );
	sprites[17]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	// state
	temp_text = "Moving State " + int_to_string( (int)pPlayer->state );
	sprites[18]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );
	// onground
	int groundtype = 0;
	if( pPlayer->ground_object )
	{
		groundtype = (int)pPlayer->ground_object->massivetype;
	}
	temp_text = "Ground type " + int_to_string( groundtype );
	sprites[19]->Set_Image( pFont->RenderText( pFont->font_very_small, temp_text, white ), 0, 1 );

	for( unsigned int i = 0; i < sprites.size(); i++ )
	{
		sprites[i]->Draw();
	}
}

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cGoldDisplay :: cGoldDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_GOLDDISPLAY;

	SetGold( pPlayer->goldpieces );
}

cGoldDisplay :: ~cGoldDisplay( void )
{
	//
}

void cGoldDisplay :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cGoldDisplay :: SetGold( int gold )
{
	if( gold >= 100 )
	{
		gold -= 100;
		pAudio->PlaySound( "item/live_up.ogg" );	
		livedisplay->AddLives( 1 );

		pointsdisplay->Add_Points( 0, pPlayer->posx + pPlayer->image->w/3, pPlayer->posy + 5, "1UP", lightred );
	}
	
	pPlayer->goldpieces = gold;
	string text = int_to_string( pPlayer->goldpieces );

	Color color = Color( (Uint8)255, 255, 255 - ( gold * 2 ) );

	Set_Image( pFont->RenderText( pFont->font_normal, text, color ), 0, 1 );
}

void cGoldDisplay :: AddGold( int gold )
{
	SetGold( pPlayer->goldpieces + gold );
}


/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cLiveDisplay :: cLiveDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	sprite_array = ARRAY_HUD;
	type = TYPE_LIFEDISPLAY;

	SetLives( pPlayer->lives );

	Set_Image( NULL );
}

cLiveDisplay :: ~cLiveDisplay( void )
{
	//
}

void cLiveDisplay :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cLiveDisplay :: SetLives( int lives )
{
	pPlayer->lives = lives;
	
	string text;

	if( lives >= 0 )
	{
		// if not in Overworld
		if( Game_Mode != MODE_OVERWORLD )
		{
			text = int_to_string( pPlayer->lives ) + "x";
		}
		else
		{
			text = "Lives : " + int_to_string( pPlayer->lives );
		}
	
		Set_Image( pFont->RenderText( pFont->font_normal, text, green ), 0, 1 );
	}
}

void cLiveDisplay :: AddLives( int lives )
{
	SetLives( pPlayer->lives + lives );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints *pointsdisplay = NULL;
cDebugDisplay *debugdisplay = NULL;
cGoldDisplay *golddisplay = NULL;
cLiveDisplay *livedisplay = NULL;
cTimeDisplay *timedisplay = NULL;
cItemBox *Itembox = NULL;
