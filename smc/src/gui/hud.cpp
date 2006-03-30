/***************************************************************************
           hud.cpp  -  the User interfaces in the game
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

/* *** *** *** *** *** *** Paoints text*** *** *** *** *** *** *** *** *** *** *** */

PointsText :: PointsText( void ) : cSprite()
{
	points = 0;
	vely = 0;
}

PointsText :: ~PointsText( void )
{
	//
}

/* *** *** *** *** *** *** *** Hud Manager *** *** *** *** *** *** *** *** *** *** */

cHudManager :: cHudManager( void )
{
	Objects.clear();
	loaded = 0;

	Load();
}

cHudManager :: ~cHudManager( void )
{
	Unload();
}

void cHudManager :: Load( void )
{
	if( !loaded && Get_Size() )
	{
		Unload();
	}
	else if( loaded )
	{
		UpdateText();
		return;
	}

	// Menu Background ( Mario head and the Goldpiece )
	AddObject( (cSprite*)new cMenuBackground() );
	// Point Display
	pointsdisplay = new cPlayerPoints();
	AddObject( (cSprite*)pointsdisplay );
	// Time Display
	timedisplay = new cTimeDisplay();
	AddObject( (cSprite*)timedisplay );
	// Debug Display
	debugdisplay = new cDebugDisplay();
	AddObject( (cSprite*)debugdisplay );
	// Live Display
	livedisplay = new cLiveDisplay();
	AddObject( (cSprite*)livedisplay );
	// Gold Display
	golddisplay = new cGoldDisplay();
	AddObject( (cSprite*)golddisplay );
	// Itembox
	Itembox = new cItemBox();
	AddObject( (cSprite*)Itembox );

	loaded = 1;
}

void cHudManager :: Unload( void )
{
	if( !Get_Size() )
	{
		return;
	}
	
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( Objects[i] )
		{
			delete Objects[i];
			Objects[i] = NULL;
		}
	}

	Objects.clear();

	livedisplay = NULL;
	golddisplay = NULL;
	pointsdisplay = NULL;
	timedisplay = NULL;
	debugdisplay = NULL;
	Itembox = NULL;
	
	loaded = 0;
}

void cHudManager :: AddObject( cSprite *obj )
{
	Objects.push_back( obj );
}

void cHudManager :: UpdateText( void )
{
	// note : update the lifedisplay before you update the timedisplay !
	
	if( Objects.size() > 0 )
	{
		cMenuBackground *item = (cMenuBackground *)Objects[0];

		if( Game_Mode != MODE_OVERWORLD )
		{
			item->rect_goldpiece.y = item->rect_mario_head.y + 6;
		}
		else
		{
			// Goldpiece
			item->rect_goldpiece.y = 4;
		}
	}

	if( livedisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			livedisplay->SetPos( GAME_RES_W - GAME_RES_W / 10, 18.0 );
		}
		else
		{
			livedisplay->SetPos( GAME_RES_W - GAME_RES_W / 7.5, 4 );
		}

		livedisplay->AddLives( 0 );
	}

	if( golddisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			golddisplay->SetPos( 280.0, 18.0 );
		}
		else
		{
			golddisplay->SetPos( 280.0, 1.0 );
		}

		golddisplay->AddGold( 0 );
	}

	if( pointsdisplay )
	{
		if( Game_Mode != MODE_OVERWORLD )
		{
			pointsdisplay->SetPos( 50.0, 18.0 );
		}
		else
		{
			pointsdisplay->SetPos( 50.0, 4 );
		}

		pointsdisplay->AddPoints( 0 );
	}

	if( timedisplay )
	{
		timedisplay->SetPos( GAME_RES_W * 0.75, 18.0 );
		timedisplay->Update();
	}

	if( debugdisplay )
	{
		debugdisplay->SetPos( GAME_RES_W / 2 - 100, 80.0 );
		debugdisplay->Update();
	}

	if( Itembox ) 
	{
		Itembox->SetPos( GAME_RES_W / 2 - 20, 10 );
		Itembox->Update();
	}
}

void cHudManager :: Update( void )
{
	// HUD Objects
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}

		Objects[i]->Update();
	}
}

unsigned int cHudManager :: Get_Size( void )
{
	return (unsigned int)Objects.size();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMenuBackground :: cMenuBackground( double x /* = 0 */, double y /* = 0 */ ) : cActiveSprite( x, y )
{
	type = TYPE_MENUBG;
	array = ARRAY_HUD;

	mario_head = GetImage( "game/maryo_l.png" );
	goldpiece = GetImage( "game/gold_m.png" );

	massivetype = MASS_PASSIVE;

	if(	!mario_head || !goldpiece )
	{
		printf( "Error : MenuBackground images loading failed\n" );
		return;
	}

	// Maryo head
	rect_mario_head.x = GAME_RES_W - GAME_RES_W / 11 + 25;
	rect_mario_head.y = 15;
	// Goldpiece
	rect_goldpiece.x = 250;
}

cMenuBackground :: ~cMenuBackground( void )
{
	//
}

void cMenuBackground :: Draw( void )
{
	if( Game_Mode != MODE_OVERWORLD )
	{
		mario_head->Blit( rect_mario_head.x, rect_mario_head.y );
	}
	goldpiece->Blit( rect_goldpiece.x, rect_goldpiece.y );
}

void cMenuBackground :: Update( void )
{
	if( Leveleditor_Mode )
	{
		return;
	}

	Draw();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cStatusText :: cStatusText( double x /* = 0 */, double y /* = 0 */ ) : cSprite( NULL, x, y, 0, &black, 1 )
{
	type = TYPE_STATUSTEXT;
	array = ARRAY_HUD;
}

cStatusText :: ~cStatusText( void )
{
	//
}

void cStatusText :: Update( void )
{
	Draw();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints :: cPlayerPoints( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_POINTDISPLAY;
	
	SetPoints( pPlayer->points );
	massivetype = MASS_PASSIVE;

	PointTexts.reserve( 50 );
}

cPlayerPoints :: ~cPlayerPoints( void )
{
	//
}

void cPlayerPoints :: Update( void )
{
	if( Leveleditor_Mode )
	{
		return;
	}
	
	Draw();

	for( unsigned int i = 0; i < PointTexts.size(); i++ )
	{
		if( PointTexts[i]->image )
		{
			PointTexts[i]->vely -= ( PointTexts[i]->vely * 0.01 ) * Framerate.speedfactor;
			PointTexts[i]->posy += PointTexts[i]->vely * Framerate.speedfactor;

			double x = PointTexts[i]->posx - cameraposx;
			double y = PointTexts[i]->posy - cameraposy;
			double distance_x = PointTexts[i]->posx - ( GAME_RES_W + cameraposx );
			double distance_y = ( PointTexts[i]->posy - cameraposy ) - GAME_RES_H;
			double distance_final = 0;

			if( distance_x > 0 )
			{
				x = GAME_RES_W - (Sint16)PointTexts[i]->col_rect.w - 2;

				distance_final += distance_x;
			}

			if( distance_y > 0 )
			{
				y = GAME_RES_H - 20;

				distance_final += distance_y;
			}
			glColor3ub( (Uint8)( 255 - ( distance_final/ 12 ) - ( PointTexts[i]->points / 55 ) ), (Uint8)( 255 - ( PointTexts[i]->points/ 45 ) ), (Uint8)( 255 - ( distance_final / 15 ) - ( PointTexts[i]->points / 18 ) ) );
			PointTexts[i]->image->Blit( x, y, & black );
			glColor3ub( 255, 255, 255 );

			if( PointTexts[i]->vely > -1.0 )
			{
				PointTexts[i]->SetImage( NULL );
			}
		}
		else
		{
			delete PointTexts[i];
			PointTexts.erase( PointTexts.begin() + i );
		}
	}
}

void cPlayerPoints :: SetPoints( unsigned int points )
{
	pPlayer->points = points;
	
	char text[70];
	sprintf( text, "Points %08d", (int)pPlayer->points );
	SetImage( RenderText( font_normal, text, white ), 0, 1 );
}

void cPlayerPoints :: AddPoints( unsigned int points, double x /* = 0 */, double y /* = 0 */, string strtext /* = "" */, Color color /* = 255 */ )
{
	if( !pPlayer )
	{
		return;
	}
	
	SetPoints( pPlayer->points + points );

	if( x != 0 && y != 0 && PointTexts.size() < 50 )
	{
		PointsText *new_PointText = new PointsText();

		string text;

		if( strtext.length() )
		{
			text = strtext;
		}
		else
		{
			text= int_to_string( points );
		}

		new_PointText->SetImage( RenderText( font_small, text, color ), 1, 1 );

		new_PointText->SetPos( x, y );
		new_PointText->vely = -1.4;
		new_PointText->points = points;
		
		PointTexts.push_back( new_PointText );
	}
}

void cPlayerPoints :: Clear( void )
{

}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cTimeDisplay :: cTimeDisplay( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_GAMETIMEDISPLAY;

	massivetype = MASS_PASSIVE;
	counter = -1;
}

cTimeDisplay :: ~cTimeDisplay( void )
{
	//
}

void cTimeDisplay :: Update( void )
{
	if( Leveleditor_Mode || Game_Mode == MODE_OVERWORLD)
	{
		return;
	}

	counter += Framerate.speedfactor;

	int time = (int)( counter / DESIRED_FPS );	// time = seconds since start
	int minutes = (int)( time / 60 );

	sprintf( text, "Time %02d:%02d", minutes, time - ( minutes * 60 ) );

	if( strcmp( text, text_old ) != 0 )
	{
		strcpy( text_old, text );
		SetImage( RenderText( font_normal, text, white ), 0, 1 );
	}

	Draw();
}

cItemBox :: cItemBox( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_ITEMBOXDISPLAY;

	SetImage( GetImage( "game/itembox.png" ) );
	SetShadow( NULL );
	massivetype = MASS_PASSIVE;

	box_color = white;

	item_counter = 0; // The alpha
	item_counter_mod = 0;
	item_id = 0;

	item = new cSprite();
	item->massivetype = MASS_MASSIVE;
	item->no_camera = 1;
}

cItemBox :: ~cItemBox( void )
{
	if( item )
	{
		delete item;
	}
}

void cItemBox :: Update( void )
{
	if( Leveleditor_Mode || !pPlayer || Game_Mode == MODE_OVERWORLD )
	{
		return;
	}

	if( pPlayer->maryo_type == MARYO_DEAD )
	{
		item_counter = 50;
	}
	else if( item_counter )
	{
		item->posy += Framerate.speedfactor * 4;

		if( item_counter_mod )
		{
			item_counter += Framerate.speedfactor * 10;

			if( item_counter >= 50 )
			{
				item_counter_mod = 0;
				item_counter = 50;
			}
		}
		else
		{
			item_counter -= Framerate.speedfactor * 10;

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

		if( item->ColCheckAuto( 0, 0, 0, 0, 1 ) == 4 ) // if colliding with the Player
		{
			unsigned int item_id_temp = item_id; // player can set the item back
			Reset();
			pPlayer->Get_Item( item_id_temp, 1 );
		}
	}

	if( item_id && item->image && !Leveleditor_Mode )
	{
		if( !item_counter ) // Draw it without Camera
		{
			item->Draw();
		}
		else
		{
			glColor4ub( 255, 255, 255, 100 + (Uint8)item_counter );
			item->no_camera = 0;
			item->Draw();
			item->no_camera = 1;
			glColor4f( 1, 1, 1, 1 );
		}
	}

	glColor3ub( box_color.red, box_color.green, box_color.blue );
	Draw();
	glColor3f( 1, 1, 1 );
}

void cItemBox :: Set_Item( unsigned int item_type )
{
	pAudio->PlaySound( SOUNDS_DIR "/itembox_set.ogg" );
	Reset();

	// reset startposition
	item->SetPos( 0, 0, 1 );

	if( item_type == TYPE_MUSHROOM_DEFAULT )
	{
		box_color = Get_Color( 250, 50, 50 );
		item->SetImage( GetImage( "game/items/mushroom_red.png" ) );
		item->SetPos( posx + 11, posy + 11 );
	}
	else if( item_type == TYPE_FIREPLANT )
	{
		box_color = Get_Color( 250, 200, 150 );
		item->SetImage( GetImage( "game/items/flower_red_1.png" ) );
		item->SetPos( posx + 12, posy + 17 );
	}

	item_id = item_type;
}

void cItemBox :: Request_Item( void )
{
	if( !item_id || item_counter ) 
	{
		return;
	}

	pAudio->PlaySound( SOUNDS_DIR "/itembox_get.ogg" );

	item_counter = 255;
	item->SetPos( item->posx + cameraposx, item->posy + cameraposy );
}

void cItemBox :: Push_back( void )
{
	item_counter = 0;
	item_counter_mod = 0;

	item->posx = item->startposx;
	item->posy = item->startposy;
}

void cItemBox :: Reset( void )
{
	item_id = 0;
	item_counter = 0;
	item_counter_mod = 0;
	box_color = white;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cDebugDisplay :: cDebugDisplay( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_DEBUGDISPLAY;

	level_old = "." ;
	text.clear();
	text_old.clear();

	double tempx = GAME_RES_W - 200;
	double tempy = GAME_RES_H / 2 - 250;

	for( unsigned int i = 0; i < 20; i++ )
	{
		sprites.push_back( new cSprite( NULL, tempx, tempy, 0, &black, 1 ) );

		// not the framerate text
		if( i > 2 )
		{
			tempy += 19;
		}

		// active box
		if( i > 9 && i < 14 )
		{
			sprites[i]->posx += 5;
			sprites[i]->startposx += 5;
		}
	}

	// fps display position
	sprites[0]->SetPos( 15, 5, 1 );
	sprites[1]->SetPos( 230, 5, 1 );
	sprites[2]->SetPos( 450, 5, 1 );

	// Debug type text
	sprites[4]->SetImage( RenderText( font_small, "Level information", lightblue ), 0, 1 );
	sprites[16]->SetImage( RenderText( font_small, "Player information", lightblue ), 0, 1 );

	passcount = -1;
	masscount = -1;
	enemcount = -1;
	activcount = -1;

	counter = 0;
	massivetype = MASS_PASSIVE;
}

cDebugDisplay :: ~cDebugDisplay( void )
{
	for( unsigned int i = 0; i < sprites.size(); i++ )
	{
		delete sprites[i];
	}

	sprites.clear();
}

void cDebugDisplay :: Update( void )
{
	if( counter <= 0 )
	{
		if( text.compare( text_old ) != 0 )
		{
			text = "empty";
			text_old = text;

			SetImage( NULL );
		}

		return;
	}
	else
	{
		counter -= Framerate.speedfactor;
	}
	
	if( text.compare( text_old ) != 0 )
	{
		text_old = text;
		SetImage( RenderText( font_normal, text, white ), 0, 1 );
	}
	
	SetPos( GAME_RES_W / 2 - col_rect.w / 2 - 10, posy, 1 );

	Draw();
}

void cDebugDisplay :: GameDebugDraw( void )
{
	if( !Game_debug || pPlayer->maryo_type == MARYO_DEAD ) 
	{
		return;
	}

	string temp_text;

	/* ### Extend
	- Other Special ( Active ) Objects
	*/

	// black background
	Color color = Get_Color( 0, 0, 0, 128 );
	DrawRect( GAME_RES_W - 205, GAME_RES_H / 12, 180, 325, &color );

	// ### Frames per Second
	temp_text = "FPS : best " + int_to_string( (int)Framerate.fps_best ) + ", worst " + int_to_string( (int)Framerate.fps_worst ) + ", current " + int_to_string( (int)Framerate.fps );
	sprites[0]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	// average
	temp_text = "average " + int_to_string( (int)Framerate.fps_average );
	sprites[1]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	// speedfactor
	temp_text = "Speedfactor " + double_to_string( Framerate.speedfactor );
	sprites[2]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );

	// ### Cameraposition
	temp_text = "Camera : X " + int_to_string( (int)cameraposx ) + ", Y " + int_to_string( (int)cameraposy );
	sprites[3]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );

	// ### Level informations
	if( pLevel->levelfile.compare( level_old ) != 0 ) 
	{
		string lvl_text = "Level : " + pLevel->Get_filename( 0, 0 );
		level_old = pLevel->levelfile;
		
		sprites[5]->SetImage( RenderText( font_small, lvl_text, white ), 0, 1 );
	}

	// ### Level Objects
	// # Passive
	if( passcount != (int)pObjManager->Get_Size_array( ARRAY_PASSIVE ) ) 
	{
		passcount = pObjManager->Get_Size_array( ARRAY_PASSIVE );

		temp_text = "Passive Objects : " + int_to_string( pObjManager->Get_Size_array( ARRAY_PASSIVE ) );
		sprites[6]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Massive
	if( masscount != (int)pObjManager->Get_Size_array( ARRAY_MASSIVE ) ) 
	{
		masscount = pObjManager->Get_Size_array( ARRAY_MASSIVE );

		temp_text = "Massive Objects : " + int_to_string( pObjManager->Get_Size_array( ARRAY_MASSIVE ) );
		sprites[7]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Enemy
	if( enemcount != (int)pObjManager->Get_Size_array( ARRAY_ENEMY ) ) 
	{
		enemcount = pObjManager->Get_Size_array( ARRAY_ENEMY );

		temp_text ="Enemy Objects : " + int_to_string( pObjManager->Get_Size_array( ARRAY_ENEMY ) );
		sprites[8]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Active
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		temp_text = "Active Objects : " + int_to_string( pObjManager->Get_Size_array( ARRAY_ACTIVE ) );
		sprites[9]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}

	// # Active Objects Rect
	GL_rect r( sprites[10]->posx - 8, sprites[10]->posy, 135, 1 );

	DrawRect( r.x, r.y + 74, r.w, r.h, &white );	// Up
	DrawRect( r.x, r.y + 75, r.w, r.h, &white );	// Down
	DrawRect( r.x, r.y, r.w, r.h, &grey );			// Down Shadow
	r.w = 1;
	r.h = 75;
	DrawRect( r.x, r.y, r.w, r.h, &white );			// Left
	DrawRect( r.x + 135, r.y, r.w, r.h, &white );	// Right
	DrawRect( r.x + 136, r.y, r.w, r.h + 1, &grey );// Right Shadow
	
	// # Active : HalfMassive
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		unsigned int halfmassive = 0;

		for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_HALFMASSIVE )
			{
				halfmassive++;
			}
		}

		temp_text = "HalfMassive : " + int_to_string( halfmassive );
		sprites[10]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Active : Cloud
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		unsigned int clouds = 0;

		for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_CLOUD )
			{
				clouds++;
			}
		}

		temp_text = "Cloud : " + int_to_string( clouds );
		sprites[11]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Active : Goldbox
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		unsigned int goldbox = 0;

		for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_GOLDBOX )
			{
				goldbox++;
			}
		}

		temp_text = "Goldbox : " + int_to_string( goldbox );
		sprites[12]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Active : Bonusbox
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) ) 
	{
		unsigned int bonusbox = 0;

		for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_BONUSBOX_LIVE || pObjManager->Get_Pointer( i )->type == TYPE_BONUSBOX_MUSHROOM_FIRE )
			{
				bonusbox++;
			}
		}

		temp_text = "Bonusbox : " + int_to_string( bonusbox );
		sprites[13]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}
	// # Active : Other
	if( activcount != (int)pObjManager->Get_Size_array( ARRAY_ACTIVE ) )
	{
		activcount = pObjManager->Get_Size_array( ARRAY_ACTIVE );

		unsigned int active_other = pObjManager->Get_Size_array( ARRAY_ACTIVE );

		for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_HALFMASSIVE || pObjManager->Get_Pointer( i )->type == TYPE_BONUSBOX_LIVE || 
				pObjManager->Get_Pointer( i )->type == TYPE_BONUSBOX_MUSHROOM_FIRE || pObjManager->Get_Pointer( i )->type == TYPE_GOLDBOX || 
				pObjManager->Get_Pointer( i )->type == TYPE_CLOUD )
			{
				active_other--;
			}
		}

		temp_text = "Other : " + int_to_string( active_other );
		sprites[14]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	}

	// ### Player informations
	// position
	temp_text = "Position X " + int_to_string( (int)pPlayer->posx ) + " ,Y " + int_to_string( (int)pPlayer->posy );
	sprites[16]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	// velocity
	temp_text = "Velocity X " + int_to_string( (int)pPlayer->velx ) + " ,Y " + int_to_string( (int)pPlayer->vely );
	sprites[17]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	// state
	temp_text = "Moving State " + int_to_string( (int)pPlayer->state );
	sprites[18]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );
	// onground
	int groundtype = 0;
	if( pPlayer->groundObject )
	{
		groundtype = (int)pPlayer->groundObject->massivetype;
	}
	temp_text = "Ground type " + int_to_string( groundtype );
	sprites[19]->SetImage( RenderText( font_small, temp_text, white ), 0, 1 );

	for( unsigned int i = 0; i < sprites.size(); i++ )
	{
		sprites[i]->Draw();
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cGoldDisplay :: cGoldDisplay( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_GOLDDISPLAY;

	massivetype = MASS_PASSIVE;

	SetGold( pPlayer->goldpieces );
}

cGoldDisplay :: ~cGoldDisplay( void )
{
	//
}

void cGoldDisplay :: Update( void )
{
	if( Leveleditor_Mode ) 
	{
		return;
	}

	Draw();
}

void cGoldDisplay :: SetGold( int gold )
{
	if( gold >= 100 )
	{
		gold -= 100;
		pAudio->PlaySound( SOUNDS_DIR "/live_up.ogg" );	
		livedisplay->AddLives( 1 );

		pointsdisplay->AddPoints( 0, pPlayer->posx + pPlayer->image->w/3, pPlayer->posy + 5, "1UP", lightred );
	}
	
	pPlayer->goldpieces = gold;
	string text = int_to_string( pPlayer->goldpieces );

	Color color = Get_Color( 255, 255, 255 - ( gold * 2 ) );

	SetImage( RenderText( font_normal, text, color ), 0, 1 );
}

void cGoldDisplay :: AddGold( int gold )
{
	SetGold( pPlayer->goldpieces + gold );
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLiveDisplay :: cLiveDisplay( double x /* = 0 */, double y /* = 0 */ ) : cStatusText( x, y )
{
	array = ARRAY_HUD;
	type = TYPE_LIFEDISPLAY;

	massivetype = MASS_PASSIVE;
	SetLives( pPlayer->lives );

	SetImage( NULL );
}

cLiveDisplay :: ~cLiveDisplay( void )
{
	//
}

void cLiveDisplay :: Update( void )
{
	if( Leveleditor_Mode ) 
	{
		return;
	}

	Draw();
}

void cLiveDisplay :: SetLives( int lives )
{
	pPlayer->lives = lives;
	
	string text;

	if( lives >= 0 ) 
	{
		if( Game_Mode != MODE_OVERWORLD ) // if not in OverWorld
		{
			text = int_to_string( pPlayer->lives ) + "x";
		}
		else
		{
			text = "Lives : " + int_to_string( pPlayer->lives );
		}
	}

	SetImage( RenderText( font_normal, text, green ), 0, 1 );
}

void cLiveDisplay :: AddLives( int lives )
{
	SetLives( pPlayer->lives + lives );
}
