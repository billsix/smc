/***************************************************************************
 * hud.cpp  -  human user interfaces in the game
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../gui/hud.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../video/font.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../core/sprite_manager.h"
#include "../objects/bonusbox.h"
#include "../video/renderer.h"
#include "../core/i18n.h"

namespace SMC
{

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

cHudSprite :: cHudSprite( cGL_Surface *new_image /* = NULL */, float x /* = 0 */, float y /* = 0 */, bool del_img /* = 0 */ )
: cSprite( new_image, x, y, del_img )
{
	m_player_range = 0;
	m_pos_z = 0.13f;

	Set_Ignore_Camera( 1 );
}

cHudSprite :: ~cHudSprite( void )
{

}

cHudSprite *cHudSprite :: Copy( void )
{
	cHudSprite *hud_sprite = new cHudSprite( m_start_image, m_pos_x, m_pos_y );

	hud_sprite->m_type = m_type;
	hud_sprite->m_sprite_array = m_sprite_array;
	hud_sprite->m_massive_type = m_massive_type;
	hud_sprite->Set_Ignore_Camera( m_no_camera );
	hud_sprite->Set_Shadow_Pos( m_shadow_pos );
	hud_sprite->Set_Shadow_Color( m_shadow_color );

	return hud_sprite;
}

/* *** *** *** *** *** *** *** cHud_Manager *** *** *** *** *** *** *** *** *** *** */

cHud_Manager :: cHud_Manager( void )
: cObject_Manager<cHudSprite>()
{
	loaded = 0;

	Load();
}

cHud_Manager :: ~cHud_Manager( void )
{
	Unload();
}

void cHud_Manager :: Load( void )
{
	if( !loaded && !objects.empty() )
	{
		Unload();
	}
	else if( loaded )
	{
		Update_Text();
		return;
	}

	// Menu Background ( Maryo head and the Goldpiece )
	Add( static_cast<cHudSprite *>(new cMenuBackground()) );
	// Point Display
	pHud_Points = new cPlayerPoints();
	Add( static_cast<cHudSprite *>(pHud_Points) );
	// Time Display
	pHud_Time = new cTimeDisplay();
	Add( static_cast<cHudSprite *>(pHud_Time) );
	// Live Display
	pHud_Lives = new cLiveDisplay();
	Add( static_cast<cHudSprite *>(pHud_Lives) );
	// Gold Display
	pHud_Goldpieces = new cGoldDisplay();
	Add( static_cast<cHudSprite *>(pHud_Goldpieces) );
	// Itembox
	pHud_Itembox = new cItemBox();
	Add( static_cast<cHudSprite *>(pHud_Itembox) );
	// Debug Display
	pHud_Debug = new cDebugDisplay();
	Add( static_cast<cHudSprite *>(pHud_Debug) );

	loaded = 1;
}

void cHud_Manager :: Unload( void )
{
	if( objects.empty() )
	{
		return;
	}
	
	Delete_All();

	pHud_Lives = NULL;
	pHud_Goldpieces = NULL;
	pHud_Points = NULL;
	pHud_Time = NULL;
	pHud_Debug = NULL;
	pHud_Itembox = NULL;
	
	loaded = 0;
}

void cHud_Manager :: Update_Text( void )
{
	// note : update the life display before you update the timedisplay
	
	if( !objects.empty() )
	{
		cMenuBackground *item = static_cast<cMenuBackground *>(objects[0]);

		if( Game_Mode != MODE_OVERWORLD )
		{
			// goldpiece
			item->rect_goldpiece.m_y = item->rect_maryo_head.m_y + 6;
		}
		else
		{
			// goldpiece
			item->rect_goldpiece.m_y = 7;
		}
	}

	if( pHud_Lives )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Lives->Set_Pos( game_res_w - game_res_w * 0.1f, 18 );
		}
		else
		{
			pHud_Lives->Set_Pos( game_res_w - game_res_w / 7.5f, 4 );
		}

		pHud_Lives->Add_Lives( 0 );
	}

	if( pHud_Goldpieces )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Goldpieces->Set_Pos( 280, 18 );
		}
		else
		{
			pHud_Goldpieces->Set_Pos( 280, 4 );
		}

		pHud_Goldpieces->Add_Gold( 0 );
	}

	if( pHud_Points )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pHud_Points->Set_Pos( 50, 18 );
		}
		else
		{
			pHud_Points->Set_Pos( 50, 4 );
		}

		pHud_Points->Add_Points( 0 );
	}

	if( pHud_Time )
	{
		pHud_Time->Set_Pos( game_res_w * 0.70f, 18 );
		pHud_Time->Update();
	}

	if( pHud_Debug )
	{
		pHud_Debug->Set_Pos( game_res_w * 0.45f, 80 );
		pHud_Debug->Update();
	}

	if( pHud_Itembox ) 
	{
		pHud_Itembox->Set_Pos( game_res_w * 0.49f, 10 );
		pHud_Itembox->Update();
	}
}

void cHud_Manager :: Update( void )
{
	// update HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Update();
	}
}

void cHud_Manager :: Draw( void )
{
	// draw HUD objects
	for( HudSpriteList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cHud_Manager *pHud_Manager = NULL;

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
	m_type = TYPE_MENUBG;
	m_sprite_array = ARRAY_HUD;
	m_name = "HUD Menu Background";

	maryo_head = pVideo->Get_Surface( "game/maryo_l.png" );
	goldpiece = pVideo->Get_Surface( "game/gold_m.png" );

	if(	!maryo_head || !goldpiece )
	{
		printf( "Error : MenuBackground images loading failed\n" );
		return;
	}

	// maryo head
	rect_maryo_head.m_x = game_res_w * 0.933f;
	rect_maryo_head.m_y = 15;
	// goldpiece
	rect_goldpiece.m_x = 250;
	rect_goldpiece.m_y = 0;
}

cMenuBackground :: ~cMenuBackground( void )
{
	//
}

void cMenuBackground :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU )
	{
		return;
	}

	// maryo head
	if( Game_Mode != MODE_OVERWORLD )
	{
		maryo_head->Blit( rect_maryo_head.m_x, rect_maryo_head.m_y, m_pos_z );
	}

	// goldpiece
	goldpiece->Blit( rect_goldpiece.m_x, rect_goldpiece.m_y, m_pos_z );
}

/* *** *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** */

cStatusText :: cStatusText( float x /* = 0 */, float y /* = 0 */ )
: cHudSprite( NULL, x, y )
{
	m_type = TYPE_STATUSTEXT;
	m_sprite_array = ARRAY_HUD;

	Set_Shadow( black, 1.5f );
}

cStatusText :: ~cStatusText( void )
{
	//
}

void cStatusText :: Draw( cSurface_Request *request /* = NULL */ )
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
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_POINTDISPLAY;
	m_name = "HUD Player points";
	
	Set_Points( pPlayer->points );

	pointsobjects.reserve( 50 );
}

cPlayerPoints :: ~cPlayerPoints( void )
{
	Clear();
}

void cPlayerPoints :: Draw( cSurface_Request *request /* = NULL */ )
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
		if( !obj->m_image )
		{
			itr = pointsobjects.erase( itr );
			delete obj;
		}
		// active
		else
		{
			obj->vely -= obj->vely * 0.01f * pFramerate->speedfactor;
			obj->m_pos_y += obj->vely * pFramerate->speedfactor;

			if( obj->vely > -1.0 )
			{
				obj->Set_Image( NULL );
				continue;
			}

			float x = obj->m_pos_x - pActive_Camera->x;
			float y = obj->m_pos_y - pActive_Camera->y;

			// out in left
			if( x < 0 )
			{
				x = 5;
			}
			// out in right
			else if( x > game_res_w )
			{
				x = static_cast<float>(game_res_w) - obj->m_col_rect.m_w - 2; 
			}

			// out on bottom
			if( y < 0 )
			{
				y = 5;
			}
			// out on top
			else if( y > game_res_h )
			{
				y = static_cast<float>(game_res_h) - obj->m_col_rect.m_h - 20;
			}

			// create request
			cSurface_Request *request = new cSurface_Request();
			obj->m_image->Blit( x, y, m_pos_z, request );

			// shadow
			request->shadow_color = black;
			request->shadow_pos = 1;

			// color
			request->color = Color( static_cast<Uint8>( 255 - ( obj->points / 150 ) ), static_cast<Uint8>( 255 - ( obj->points / 150 ) ), static_cast<Uint8>( 255 - ( obj->points / 30 ) ) );

			// add request
			pRenderer->Add( request );

			++itr;
		}
	}
}

void cPlayerPoints :: Set_Points( long points )
{
	pPlayer->points = points;

	char text[70];
	sprintf( text, _("Points %08d"), static_cast<int>(pPlayer->points) );
	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, white ), 0, 1 );
}

void cPlayerPoints :: Add_Points( unsigned int points, float x /* = 0 */, float y /* = 0 */, std::string strtext /* = "" */, const Color &color /* = static_cast<Uint8>(255) */, bool allow_multiplier /* = 0 */ )
{
	if( allow_multiplier )
	{
		points = static_cast<unsigned int>( pPlayer->kill_multiplier * static_cast<float>(points) );
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
	new_obj->Set_Image( pFont->Render_Text( pFont->m_font_small, strtext, color ), 1, 1 );

	new_obj->Set_Pos( x, y );
	new_obj->vely = -1.4f;
	new_obj->points = points;
	
	// check if it collides with an already active points text
	for( PointsTextList::iterator itr = pointsobjects.begin(); itr != pointsobjects.end(); ++itr )
	{
		// get object pointer
		PointsText *obj = (*itr);

		if( new_obj->m_rect.Intersects( obj->m_col_rect ) )
		{
			new_obj->Move( obj->m_col_rect.m_w + 5, 0, 1 );
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
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_GAMETIMEDISPLAY;
	m_name = "HUD Time";

	Reset();
}

cTimeDisplay :: ~cTimeDisplay( void )
{
	//
}

void cTimeDisplay :: Update( void )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	counter += pFramerate->speedfactor;

	// get seconds since start
	unsigned int seconds = static_cast<unsigned int>( counter / speedfactor_fps );

	// no change
	if( seconds == old_seconds )
	{
		return;
	}

	old_seconds = seconds;

	unsigned int minutes = seconds / 60;

	// Set new time
	sprintf( text, _("Time %02d:%02d"), minutes, seconds - ( minutes * 60 ) );
	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, white ), 0, 1 );
}

void cTimeDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	cHudSprite::Draw();
}

void cTimeDisplay :: Reset( void )
{
	sprintf( text, "Time" );
	old_seconds = 1000;
	counter = 0;
}

/* *** *** *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** */

cItemBox :: cItemBox( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_ITEMBOXDISPLAY;
	m_name = "HUD Itembox";

	Set_Image( pVideo->Get_Surface( "game/itembox.png" ) );
	// disable shadow
	Set_Shadow_Pos( 0 );

	box_color = white;

	item_counter = 0;
	item_counter_mod = 0;
	item_id = TYPE_UNDEFINED;

	item = new cMovingSprite();
	item->Set_Ignore_Camera( 1 );
	item->m_player_range = 0;
	item->Set_Massive_Type( MASS_MASSIVE );
	item->m_pos_z = 0.1299f;
}

cItemBox :: ~cItemBox( void )
{
	delete item;
}

void cItemBox :: Update( void )
{
	if( item_counter )
	{
		item->Move( 0, 4 );

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

		if( item->m_pos_y > game_res_h )
		{
			Reset();
		}

		cObjectCollisionType *col_list = item->Collision_Check( &item->m_col_rect, COLLIDE_ONLY_BLOCKING );

		// if colliding with the player
		if( col_list->Is_Included( TYPE_PLAYER ) )
		{
			// player can send an item back
			SpriteType item_id_temp = item_id;
			Reset();
			pPlayer->Get_Item( item_id_temp, 1 );
		}

		delete col_list;
	}
}

void cItemBox :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU )
	{
		return;
	}

	if( item_id && item->m_image )
	{
		// with alpha
		if( item_counter )
		{
			item->Set_Color( 255, 255, 255, 100 + static_cast<Uint8>(item_counter) );
		}
		
		item->Draw();
	}

	Set_Color( box_color.red, box_color.green, box_color.blue );
	cHudSprite::Draw();
}

void cItemBox :: Set_Item( SpriteType item_type, bool sound /* = 1 */ )
{
	// play sound
	if( sound )
	{
		pAudio->Play_Sound( "itembox_set.ogg" );
	}
	// reset data
	Reset();

	// reset startposition
	item->Set_Pos( 0, 0, 1 );
	// reset color
	item->Set_Color( white );

	if( item_type == TYPE_MUSHROOM_DEFAULT )
	{
		box_color = Color( static_cast<Uint8>(250), 50, 50 );
		item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_red.png" ) );
	}
	else if( item_type == TYPE_FIREPLANT )
	{
		box_color = Color( static_cast<Uint8>(250), 200, 150 );
		item->Set_Image( pVideo->Get_Surface( "game/items/fireplant.png" ) );
	}
	else if( item_type == TYPE_MUSHROOM_BLUE )
	{
		box_color = Color( static_cast<Uint8>(100), 100, 250 );
		item->Set_Image( pVideo->Get_Surface( "game/items/mushroom_blue.png" ) );
	}


	if( item->m_image )
	{
		item->Set_Pos( m_pos_x - ( ( item->m_image->m_w - m_rect.m_w ) / 2 ), m_pos_y - ( ( item->m_image->m_h - m_rect.m_h ) / 2 ) );
	}

	item_id = item_type;
}

void cItemBox :: Request_Item( void )
{
	if( !item_id || item_counter ) 
	{
		return;
	}

	pAudio->Play_Sound( "itembox_get.ogg" );

	item_counter = 255;
	// draw item with camera
	item->Set_Ignore_Camera( 0 );
	item->Set_Pos( item->m_pos_x + pActive_Camera->x, item->m_pos_y + pActive_Camera->y );
}

void cItemBox :: Push_back( void )
{
	item_counter = 0;
	item_counter_mod = 0;

	// draw item without camera
	item->Set_Ignore_Camera( 1 );
	item->Set_Pos( item->m_start_pos_x, item->m_start_pos_y );
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
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_DEBUGDISPLAY;
	m_name = "HUD Debug";

	text.clear();
	text_old.clear();

	// debug box text data
	Game_Mode_last = MODE_NOTHING;
	level_old = ".";
	obj_counter = -1;
	pass_counter = -1;
	mass_counter = -1;
	enemy_counter = -1;
	active_counter = -1;

	// debug text window
	window_debugtext = CEGUI::WindowManager::getSingleton().loadWindowLayout( "debugtext.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( window_debugtext );
	// debug text
	text_debugtext = static_cast<CEGUI::Window *>(CEGUI::WindowManager::getSingleton().getWindow( "text_debugmessage" ));
	// hide
	text_debugtext->setVisible( 0 );

	// debug box positions
	float tempx = static_cast<float>(game_res_w) - 200;
	float tempy = static_cast<float>(game_res_h) / 2 - 250;

	for( unsigned int i = 0; i < 23; i++ )
	{
		sprites.push_back( new cHudSprite( NULL, tempx, tempy ) );
		sprites[i]->Set_Shadow( black, 1 );
		sprites[i]->m_pos_z = m_pos_z;

		// not the framerate text
		if( i > 2 )
		{
			tempy += 19;
		}

		// active box
		if( i > 10 && i < 16 )
		{
			sprites[i]->Set_Pos_X( sprites[i]->m_start_pos_x += 5, 1 );
		}
	}

	// fps display position
	sprites[0]->Set_Pos( 15, 5, 1 );
	// average
	sprites[1]->Set_Pos( 290, 5, 1 );
	// speedfactor
	sprites[2]->Set_Pos( 480, 5, 1 );

	// Debug type text
	sprites[4]->Set_Image( pFont->Render_Text( pFont->m_font_small, _("Level"), lightblue ), 0, 1 );
	sprites[16]->Set_Image( pFont->Render_Text( pFont->m_font_small, _("Player"), lightblue ), 0, 1 );

	counter = 0;
}

cDebugDisplay :: ~cDebugDisplay( void )
{
	pGuiSystem->getGUISheet()->removeChildWindow( window_debugtext );
	CEGUI::WindowManager::getSingleton().destroyWindow( window_debugtext );

	for( HudSpriteList::iterator itr = sprites.begin(), itr_end = sprites.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	sprites.clear();
}

void cDebugDisplay :: Update( void )
{
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
		CEGUI::String gui_text = reinterpret_cast<const CEGUI::utf8*>(text.c_str());

		// display the new text
		text_debugtext->setText( gui_text );
		text_debugtext->setVisible( 1 );

		// update position
		CEGUI::Font *font = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" );
		float text_width = font->getTextExtent( gui_text ) * global_downscalex;

		if( text_width > 800 )
		{
			text_width = 800;
		}

		text_debugtext->setWidth( CEGUI::UDim( 0, ( text_width + 15 ) * global_upscalex ) );
		text_debugtext->setXPosition( CEGUI::UDim( 0, ( ( game_res_w * 0.5f ) - text_width * 0.5f ) * global_upscalex ) );
	}
}

void cDebugDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	// debug mod info
	Draw_Debug_Mode();
	Draw_Performance_Debug_Mode();
}

void cDebugDisplay :: Set_Text( const std::string &ntext, float display_time /* = speedfactor_fps * 2 */ )
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

void cDebugDisplay :: Draw_fps( void )
{
	// ### Frames per Second
	sprites[0]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("FPS : best ") + int_to_string( static_cast<int>(pFramerate->fps_best) ) + _(", worst ") + int_to_string( static_cast<int>(pFramerate->fps_worst) ) + _(", current ") + int_to_string( static_cast<int>(pFramerate->fps) ), white ), 0, 1 );
	// average
	sprites[1]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("average ") + int_to_string( static_cast<int>(pFramerate->fps_average) ), white ), 0, 1 );
	// speedfactor
	sprites[2]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("Speedfactor ") + float_to_string( pFramerate->speedfactor, 4 ), white ), 0, 1 );
}

void cDebugDisplay :: Draw_Debug_Mode( void )
{
	if( !game_debug || ( Game_Mode == MODE_LEVEL && pPlayer->maryo_type == MARYO_DEAD ) ) 
	{
		return;
	}

	/* ### Extend
	- Ground Object name, position and velocity
	*/

	// black background
	Color color = blackalpha128;
	pVideo->Draw_Rect( static_cast<float>(game_res_w) - 205, static_cast<float>(game_res_h) * 0.08f, 190, 390, m_pos_z - 0.00001f, &color );

	// Active objects rect

	// create request
	cRect_Request *request = new cRect_Request();
	pVideo->Draw_Rect( sprites[11]->m_pos_x - 4, sprites[11]->m_pos_y - 4, 135, 95, m_pos_z, &white, request );

	// not filled
	request->filled = 0;

	// add request
	pRenderer->Add( request );

	// fps
	Draw_fps();

	std::string temp_text;

	// Camera position
	temp_text = _("Camera : X ") + int_to_string( static_cast<int>(pActive_Camera->x) ) + ", Y " + int_to_string( static_cast<int>(pActive_Camera->y) );
	sprites[3]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

	// Level information
	if( pActive_Level->m_level_filename.compare( level_old ) != 0 ) 
	{
		std::string lvl_text = _("Name : ") + Get_Filename( pActive_Level->m_level_filename, 0, 0 );
		level_old = pActive_Level->m_level_filename;

		sprites[5]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, lvl_text, white ), 0, 1 );
	}

	// Level objects
	if( obj_counter != static_cast<int>(pActive_Sprite_Manager->size()) )
	{
		obj_counter = pActive_Sprite_Manager->size();

		temp_text = _("Objects : ") + int_to_string( obj_counter );
		sprites[6]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Passive
	if( pass_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_PASSIVE )) )
	{
		pass_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_PASSIVE );

		temp_text = _("Passive : ") + int_to_string( pass_counter );
		sprites[7]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Massive
	if( mass_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_MASSIVE )) )
	{
		mass_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_MASSIVE );

		temp_text = _("Massive : ") + int_to_string( mass_counter );
		sprites[8]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Enemy
	if( enemy_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_ENEMY )) ) 
	{
		enemy_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_ENEMY );

		temp_text = _("Enemy : ") + int_to_string( enemy_counter );
		sprites[9]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}
	// Active
	if( active_counter != static_cast<int>(pActive_Sprite_Manager->Get_Size_Array( ARRAY_ACTIVE )) )
	{
		active_counter = pActive_Sprite_Manager->Get_Size_Array( ARRAY_ACTIVE );

		temp_text = _("Active : ") + int_to_string( active_counter );
		sprites[10]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Halfmassive
		unsigned int halfmassive = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_HALFMASSIVE )
			{
				halfmassive++;
			}
		}

		temp_text = _("Halfmassive : ") + int_to_string( halfmassive );
		sprites[11]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Moving Platform
		unsigned int moving_platform = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_MOVING_PLATFORM )
			{
				moving_platform++;
			}
		}

		temp_text = _("Moving Platform : ") + int_to_string( moving_platform );
		sprites[12]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Goldbox
		unsigned int goldbox = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type == TYPE_GOLDPIECE )
				{
					goldbox++;
				}
			}
		}

		temp_text = _("Goldbox : ") + int_to_string( goldbox );
		sprites[13]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Bonusbox
		unsigned int bonusbox_count = 0;

		for( cSprite_List::iterator itr = pActive_Sprite_Manager->objects.begin(), itr_end = pActive_Sprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *obj = (*itr);

			if( obj->m_type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(obj);

				if( bonusbox->box_type != TYPE_GOLDPIECE )
				{
					bonusbox_count++;
				}
			}
		}

		temp_text = _("Bonusbox : ") + int_to_string( bonusbox_count );
		sprites[14]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );

		// Other
		unsigned int active_other = active_counter - halfmassive - moving_platform - goldbox - bonusbox_count;

		temp_text = _("Other : ") + int_to_string( active_other );
		sprites[15]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	}

	// Player information
	// position x
	temp_text = "X1 " + float_to_string( pActive_Player->m_pos_x, 4 ) + "  X2 " + float_to_string( pPlayer->m_col_rect.m_x + pPlayer->m_col_rect.m_w, 4 );
	sprites[17]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// position y
	temp_text = "Y1 " + float_to_string( pActive_Player->m_pos_y, 4 ) + "  Y2 " + float_to_string( pPlayer->m_col_rect.m_y + pPlayer->m_col_rect.m_h, 4 );
	sprites[18]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// velocity
	temp_text = _("Velocity X ") + float_to_string( pPlayer->m_velx, 2 ) + " ,Y " + float_to_string( pPlayer->m_vely, 2 );
	sprites[19]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// moving state
	temp_text = _("Moving State ") + int_to_string( static_cast<int>(pPlayer->m_state) );
	sprites[20]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// ground type
	std::string ground_type;
	if( pPlayer->m_ground_object )
	{
		ground_type = int_to_string( pPlayer->m_ground_object->m_massive_type ) + " (" + Get_Massive_Type_Name( pPlayer->m_ground_object->m_massive_type ) + ")";
	}
	temp_text = _("Ground ") + ground_type;
	sprites[21]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, temp_text, white ), 0, 1 );
	// game mode
	if( Game_Mode != Game_Mode_last )
	{
		sprites[22]->Set_Image( pFont->Render_Text( pFont->m_font_very_small, _("Game Mode : ") + int_to_string( Game_Mode ), white ), 0, 1 );
	}

	// draw text
	for( HudSpriteList::iterator itr = sprites.begin(), itr_end = sprites.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cDebugDisplay :: Draw_Performance_Debug_Mode( void )
{
	if( !game_debug_performance ) 
	{
		return;
	}


	std::string temp_text;
	float ypos = game_res_h * 0.08f;

	// black background
	Color color = blackalpha128;
	pVideo->Draw_Rect( 15, ypos, 190, 390, m_pos_z - 0.00001f, &color );

	// don't draw it twice
	if( !game_debug )
	{
		// fps
		Draw_fps();
		sprites[0]->Draw();
		sprites[1]->Draw();
		sprites[2]->Draw();
	}

	vector<std::string> text_strings;
	// draw
	text_strings.push_back( _("Draw") );
	// overworld
	if( Game_Mode == MODE_OVERWORLD )
	{
		text_strings.push_back( _("World : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_OVERWORLD]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// menu
	else if( Game_Mode == MODE_MENU )
	{
		text_strings.push_back( _("Menu : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_MENU]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level settings
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		text_strings.push_back( _("Level Settings : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_SETTINGS]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level (default)
	else
	{
		text_strings.push_back( _("Level Layer 1 : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_LAYER1]->ms ) );
		text_strings.push_back( _("Level Player : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_PLAYER]->ms ) );
		text_strings.push_back( _("Level Layer 2 : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_LAYER2]->ms ) );
		text_strings.push_back( _("Level Hud : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_HUD]->ms ) );
		text_strings.push_back( _("Level Editor : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_LEVEL_EDITOR]->ms ) );
	}
	text_strings.push_back( _("Mouse : ") + int_to_string( pFramerate->perf_timer[PERF_DRAW_MOUSE]->ms ) );
	// update
	text_strings.push_back( _("Update") );
	// overworld
	if( Game_Mode == MODE_OVERWORLD )
	{
		text_strings.push_back( _("World : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_OVERWORLD]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// menu
	else if( Game_Mode == MODE_MENU )
	{
		text_strings.push_back( _("Menu : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_MENU]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level settings
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		text_strings.push_back( _("Level Settings : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_LEVEL_SETTINGS]->ms ) );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
		text_strings.push_back( "- " );
	}
	// level (default)
	else
	{
		text_strings.push_back( _("process input : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_PROCESS_INPUT]->ms ) );
		text_strings.push_back( _("level : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_LEVEL]->ms ) );
		text_strings.push_back( _("level editor : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_LEVEL_EDITOR]->ms ) );
		text_strings.push_back( _("hud : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_HUD]->ms ) );
		text_strings.push_back( _("player : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_PLAYER]->ms ) );
		text_strings.push_back( _("player collisions : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_PLAYER_COLLISIONS]->ms ) );
		text_strings.push_back( _("level late : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_LATE_LEVEL]->ms ) );
		text_strings.push_back( _("level collisions : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_LEVEL_COLLISIONS]->ms ) );
		text_strings.push_back( _("camera : ") + int_to_string( pFramerate->perf_timer[PERF_UPDATE_CAMERA]->ms ) );
	}

	// render
	text_strings.push_back( _("Render") );
	text_strings.push_back( _("Game : ") + int_to_string( pFramerate->perf_timer[PERF_RENDER_GAME]->ms ) );
	text_strings.push_back( _("Gui : ") + int_to_string( pFramerate->perf_timer[PERF_RENDER_GUI]->ms ) );
	text_strings.push_back( _("Buffer : ") + int_to_string( pFramerate->perf_timer[PERF_RENDER_BUFFER]->ms ) );

	unsigned int pos = 0;

	for( vector<std::string>::iterator itr = text_strings.begin(), itr_end = text_strings.end(); itr != itr_end; ++itr )
	{
		// sections
		float xpos = 20;
		ypos += 12;

		// move non header a bit to the right right
		if( pos != 0 && pos != 7 && pos != 17 )
		{
			xpos += 10;
		}
		// if new group starts move a bit more down
		if( pos == 7 || pos == 17 )
		{
			ypos += 10;
		}

		std::string current_text = (*itr);

		cGL_Surface *surface_temp = pFont->Render_Text( pFont->m_font_small, current_text, white );

		// create request
		cSurface_Request *request = new cSurface_Request();
		surface_temp->Blit( xpos, ypos, m_pos_z, request );
		request->delete_texture = 1;

		// shadow
		request->shadow_pos = 1;
		request->shadow_color = black;

		// add request
		pRenderer->Add( request );

		surface_temp->m_auto_del_img = 0;
		delete surface_temp;
		
		pos++;
	}
}

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cGoldDisplay :: cGoldDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_GOLDDISPLAY;
	m_name = "HUD Goldpieces";

	Set_Gold( pPlayer->goldpieces );
}

cGoldDisplay :: ~cGoldDisplay( void )
{
	//
}

void cGoldDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cGoldDisplay :: Set_Gold( int gold )
{
	if( gold >= 100 )
	{
		gold -= 100;
		pAudio->Play_Sound( "item/live_up_2.ogg", RID_1UP_MUSHROOM );	
		pHud_Lives->Add_Lives( 1 );

		pHud_Points->Add_Points( 0, pPlayer->m_pos_x + pPlayer->m_image->m_w/3, pPlayer->m_pos_y + 5, "1UP", lightred );
	}
	
	pPlayer->goldpieces = gold;
	std::string text = int_to_string( pPlayer->goldpieces );

	Color color = Color( static_cast<Uint8>(255), 255, 255 - ( gold * 2 ) );

	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, color ), 0, 1 );
}

void cGoldDisplay :: Add_Gold( int gold )
{
	Set_Gold( pPlayer->goldpieces + gold );
}


/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cLiveDisplay :: cLiveDisplay( float x /* = 0 */, float y /* = 0 */ )
: cStatusText( x, y )
{
	m_sprite_array = ARRAY_HUD;
	m_type = TYPE_LIFEDISPLAY;
	m_name = "HUD Lives";

	Set_Lives( pPlayer->lives );

	Set_Image( NULL );
}

cLiveDisplay :: ~cLiveDisplay( void )
{
	//
}

void cLiveDisplay :: Draw( cSurface_Request *request /* = NULL */ )
{
	if( editor_enabled || Game_Mode == MODE_MENU ) 
	{
		return;
	}

	cHudSprite::Draw();
}

void cLiveDisplay :: Set_Lives( int lives )
{
	pPlayer->lives = lives;
	
	if( lives < 0 )
	{
		return;
	}

	std::string text;

	// if not in Overworld
	if( Game_Mode != MODE_OVERWORLD )
	{
		text = int_to_string( pPlayer->lives ) + "x";
	}
	else
	{
		text = _("Lives : ") + int_to_string( pPlayer->lives );
	}

	Set_Image( pFont->Render_Text( pFont->m_font_normal, text, green ), 0, 1 );

	// set position
	int w, h;

	if( TTF_SizeText( pFont->m_font_normal, text.c_str(), &w, &h ) == 0 )
	{
		Set_Pos_X( (game_res_w * 0.94f) - w );
	}
}

void cLiveDisplay :: Add_Lives( int lives )
{
	Set_Lives( pPlayer->lives + lives );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints *pHud_Points = NULL;
cDebugDisplay *pHud_Debug = NULL;
cGoldDisplay *pHud_Goldpieces = NULL;
cLiveDisplay *pHud_Lives = NULL;
cTimeDisplay *pHud_Time = NULL;
cItemBox *pHud_Itembox = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
