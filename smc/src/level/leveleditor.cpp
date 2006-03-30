/***************************************************************************
     leveleditor.cpp  -  class for the Leveleditor
                             -------------------
    copyright            : (C) 2004 - 2006 by FluXy
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

/* *** *** *** *** *** *** *** *** Main Menu Object *** *** *** *** *** *** *** *** *** */

cMenu_Main_Object :: cMenu_Main_Object( void )
{
	name = "";

	CountId = 0;
	Item_Menu_Id = MENU_SPACER;
	state = 0;
	pfunction = NULL;

	def_img = NULL;
	hover_img = NULL;

	Col_rect.clear();
	color = (Uint8)255;

	cur_posy = 0;
	spacer_height = 1;
}

cMenu_Main_Object :: ~cMenu_Main_Object( void )
{
	if( def_img )
	{
		delete def_img;
	}

	if( hover_img )
	{
		delete hover_img;
	}
}
/* *** *** *** *** *** *** *** *** Item Menu Object *** *** *** *** *** *** *** *** *** */

cMenu_Item_Object :: cMenu_Item_Object( void )
{
	visible = 0;

	preview_img = NULL;
	name_img = NULL;
	size_img = NULL;

	name = "";
	path = "";

	Sprite_type = NULL;

	CountId = 0;
	Col_rect.clear();
	scale = 1;
}

cMenu_Item_Object :: ~cMenu_Item_Object( void )
{
	if( name_img )
	{
		delete name_img;
	}

	if( size_img )
	{
		delete size_img;
	}

	if( Sprite_type )
	{
		delete Sprite_type;
	}
}

/* *** *** *** *** *** *** *** Leveleditor *** *** *** *** *** *** *** *** *** *** */

cLevelEditor :: cLevelEditor( void )
{
	wMain.x = 0;
	wMain.y = 0;
	wMain.w = 150;
	wMain.h = 480;
	
	wItem.x = 0;
	wItem.y = GAME_RES_H - 120;
	wItem.w = GAME_RES_W;
	wItem.h = GAME_RES_H - wItem.y;
	
	Item_pos = 0;
	
	wMain_open = wMain.w;
	wItem_open = GAME_RES_H - 5;
	
	wMain_timer = 2 * DESIRED_FPS;
	wItem_timer = 0;

	Item_scoll = 0;

	Item_scroller_left = new cObjectScroller();
	Item_scroller_left->SetPos( wItem.x + 5, wItem.y + 30 );
	Item_scroller_left->image_default = GetImage( "game/leveleditor/arrow_left.png" );
	Item_scroller_left->image_hover = GetImage( "game/leveleditor/arrow_left_hover.png" );
	Item_scroller_left->SetImage( Item_scroller_left->image_default );

	Item_scroller_right = new cObjectScroller();
	Item_scroller_right->SetPos( wItem.x + wItem.w - 20, wItem.y + 30 );
	Item_scroller_right->image_default = GetImage( "game/leveleditor/arrow_right.png" );
	Item_scroller_right->image_hover = GetImage( "game/leveleditor/arrow_right_hover.png" );
	Item_scroller_right->SetImage( Item_scroller_right->image_default );

	main_cur_posy = 0;
}

cLevelEditor :: ~cLevelEditor( void )
{
	Unload_Item_Menu();
	Unload_Main_Menu();

	delete Item_scroller_left;
	delete Item_scroller_right;
}

void cLevelEditor :: Load_Default_Menu( void )
{
	Unload_Main_Menu();

	Add_Main_Object( "Ground Basic", MENU_GROUND_GREEN_1, NULL, Get_Color( 50, 220, 90 ) );
	Add_Main_Object( "Ground Sand", MENU_GROUND_SAND_1, NULL, Get_Color( 150, 120, 90 ) );
	Add_Main_Object( "Plastic", MENU_PLASTIC_1, NULL, Get_Color( 140, 150, 250 ) );
	Add_Main_Object( "Jungle Basic", MENU_JUNGLE_1, NULL, Get_Color( 0, 150, 50 ) );
	Add_Main_Object( "Green Hedge", MENU_HEDGES_GREEN, NULL, Get_Color( 120, 180, 10 ) );
	Add_Main_Object( "Ghost", MENU_GHOST_1, NULL, Get_Color( 190, 160, 100 ) );
	Add_Main_Object( "Box", MENU_BOX, NULL, Get_Color( 125, 30, 180 ) );
	Add_Main_Object( "Pipe", MENU_PIPES, NULL, Get_Color( 20, 80, 240 ) );
	Add_Main_Object( "Hill", MENU_HILLS, NULL, Get_Color( 130, 130, 220 ) );
	Add_Main_Object( "Sign", MENU_SIGNS, NULL, Get_Color( 100, 100, 180 ) );
	Add_Main_Object( "Block", MENU_BLOCKS, NULL, Get_Color( 145, 95, 12 ) );
	Add_Main_Object( "Cloud", MENU_CLOUDS, NULL, Get_Color( 170, 130, 220 ) );
	Add_Main_Object( "Extra", MENU_EXTRA, NULL, Get_Color( 20, 200, 120 ) );
	Add_Main_Object( "5", MENU_SPACER, NULL );
	Add_Main_Object( "Enemy", MENU_ENEMY, NULL, Get_Color( 250, 80, 40 ) );
	Add_Main_Object( "Special", MENU_SPECIAL, NULL, Get_Color( 220, 200, 10 ) );
	Add_Main_Object( "8", MENU_SPACER, NULL );
	Add_Main_Object( "Level settings", MENU_FUNCTION, &LE_Settings, Get_Color( 150, 150, 150 ) );
	Add_Main_Object( "New Level", MENU_FUNCTION, &LE_New, Get_Color( 50, 50, 230 ) );
	Add_Main_Object( "Load Level", MENU_FUNCTION, &LE_Load, Get_Color( 200, 200, 200 ) );
	Add_Main_Object( "Save", MENU_FUNCTION, &LE_Save, Get_Color( 0, 220, 0 ) );
	Add_Main_Object( "Save as", MENU_FUNCTION, &LE_Save_as, Get_Color( 0, 220, 0 ) );
	Add_Main_Object( "Clear Level", MENU_FUNCTION, &LE_Clear, Get_Color( 180, 40, 40 ) );
	Add_Main_Object( "9", MENU_SPACER, NULL );
	Add_Main_Object( "Exit Leveleditor", MENU_FUNCTION, &LE_Exit, Get_Color( 230, 180, 90 ) );
	Add_Main_Object( "Enter Overworld", MENU_FUNCTION, &LE_Enter_Overworld, Get_Color( 245, 230, 190 ) );
}

void cLevelEditor :: Unload_Main_Menu( void )
{
	for( unsigned int i = 0; i < wMenu_Objects.size(); i++ )
	{
		delete wMenu_Objects[i];
	}
	
	wMenu_Objects.clear();

	main_cur_posy = 0;
}

void cLevelEditor :: Add_Main_Object( string name, editor_main_type Item_Menu_link, void ( *nfunction )( void ), Color def_color /* = (Uint8)0 */ )
{
	if( Item_Menu_link < 1 || Item_Menu_link > 20 && Item_Menu_link != MENU_FUNCTION && Item_Menu_link != MENU_SPACER )
	{
		printf( "Warning : Unknown Menu Link used : %d\n", Item_Menu_link );
		return;
	}

	cMenu_Main_Object *wMenu = new cMenu_Main_Object();

	wMenu->name = name;
	wMenu->Item_Menu_Id = Item_Menu_link;

	wMenu->color = def_color;

	if( Item_Menu_link != MENU_SPACER ) // default menu
	{
		Color hover_color;
		
		if( wMenu->color.red + 40 <= 255 ) 
		{
			hover_color.red = wMenu->color.red + 40;
		}
		else
		{
			hover_color.red = 255;
		}
		
		if( wMenu->color.green + 40 <= 255 ) 
		{
			hover_color.green = wMenu->color.green + 40;
		}
		else
		{
			hover_color.green = 255;
		}
		
		if( wMenu->color.blue + 40 <= 255 ) 
		{
			hover_color.blue = wMenu->color.blue + 40;
		}
		else
		{
			hover_color.blue = 255;
		}
		
		// text images
		wMenu->def_img = RenderText( font_small, name, def_color );
		wMenu->hover_img = RenderText( font_small, name, hover_color );

		wMenu->cur_posy = main_cur_posy;
		main_cur_posy += (int)wMenu->def_img->h + 3;
	}
	else // spacer
	{
		int yspace = string_to_int( name );

		if( yspace < 0 || yspace > 20 ) 
		{
			return;
		}
		
		wMenu->spacer_height = yspace;

		wMenu->cur_posy = main_cur_posy;
		main_cur_posy += wMenu->spacer_height;
	}

	// function
	wMenu->pfunction = nfunction;
	
	wMenu_Objects.push_back( wMenu );

	Set_Main_Objects_Pos();
}

cMenu_Main_Object *cLevelEditor :: Get_Main_Object( unsigned int nId )
{
	if( wMenu_Objects.size() <= nId ) 
	{
		return NULL;
	}

	return wMenu_Objects[nId];
}

void cLevelEditor :: Set_Main_Objects_Pos( void )
{
	for( unsigned int i = 0; i < wMenu_Objects.size(); i++ )
	{
		if( !wMenu_Objects[i] )
		{
			continue;
		}

		if( wMenu_Objects[i]->Item_Menu_Id != MENU_SPACER ) 
		{
			// The Collision Rect
			wMenu_Objects[i]->Col_rect.w = wMenu_Objects[i]->def_img->w;
			wMenu_Objects[i]->Col_rect.h = wMenu_Objects[i]->def_img->h;
		}
		else
		{
			wMenu_Objects[i]->Col_rect.w = 0;
			wMenu_Objects[i]->Col_rect.h = 0;
		}

		if( wMain.w - 20 > 0 ) 
		{
			wMenu_Objects[i]->Col_rect.x = wMain.w - 130;
		}
		else
		{
			wMenu_Objects[i]->Col_rect.x = 0;
		}
		
		wMenu_Objects[i]->Col_rect.y = wMain.y + 8 + wMenu_Objects[i]->cur_posy;
	}
}

void cLevelEditor :: Set_Main_Active( unsigned int object_id )
{
	for( unsigned int i = 0; i < wMenu_Objects.size(); i++ )
	{
		if( !wMenu_Objects[i] )
		{
			continue;
		}

		if( wMenu_Objects[i]->Item_Menu_Id != MENU_SPACER ) 
		{
			if( i == object_id ) 
			{
				wMenu_Objects[i]->state = 2;
			}
			else
			{
				wMenu_Objects[i]->state = 0;
			}
		}
	}
}

void cLevelEditor :: Load_Item_Menu( unsigned int menu_id )
{
	if( menu_id == MENU_FUNCTION || menu_id == MENU_SPACER ) 
	{
		return;
	}

	Unload_Item_Menu();
	
	cSprite *Sprite_type = NULL;

	if( menu_id == MENU_BOX ) 
	{
		Sprite_type = new cSpinBox( 0, 0 );
		Add_Item_Object( Sprite_type, "SpinBox" );

		Sprite_type = new cGoldBox( 0, 0, COL_YELLOW );
		Add_Item_Object( Sprite_type, "GoldBox" );

		Sprite_type = new cGoldBox( 0, 0, COL_RED );
		Add_Item_Object( Sprite_type, "Red GoldBox" );

		Sprite_type = new cBonusBox( 0, 0, TYPE_BONUSBOX_MUSHROOM_FIRE );
		Add_Item_Object( Sprite_type, "Box M-F" );

		Sprite_type = new cBonusBox( 0, 0,TYPE_BONUSBOX_LIVE );
		Add_Item_Object( Sprite_type, "Box 1-UP" );

		Sprite_type = new cBonusBox( 0, 0,TYPE_BONUSBOX_JSTAR );
		Add_Item_Object( Sprite_type, "Box Star" );

	}
	else if( menu_id == MENU_GROUND_GREEN_1 ) 
	{
		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Down" );
		
		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left Down" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right Down" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/hill_left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Hill Left" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/hill_right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Hill Right" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/hill_left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Hill Left up" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/ground/hill_right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Hill Right up" );
		
		Sprite_type = new cSprite( GetImage( "ground/green_1/kplant.png" ) );
		Sprite_type->massivetype = MASS_CLIMBABLE;
		Add_Item_Object( Sprite_type, "Climbing Plant" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/kplant_head.png" ) );
		Add_Item_Object( Sprite_type, "Plant Head" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/plant_l.png" ) );
		Add_Item_Object( Sprite_type, "Plant left" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/plant_m.png" ) );
		Add_Item_Object( Sprite_type, "Plant middle" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/plant_r.png" ) );
		Add_Item_Object( Sprite_type, "Plant right" );
	}
	else if( menu_id == MENU_GROUND_SAND_1 ) 
	{
		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Down" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left Down" );

		Sprite_type = new cSprite( GetImage( "ground/sand_1/ground/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right Down" );
	}
	else if( menu_id == MENU_PLASTIC_1 ) 
	{
		// Blue
		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/down.png" ) );
		Add_Item_Object( Sprite_type, "Down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/right.png" ) );
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/left.png" ) );
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_blue/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right down" );

		// Green
		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/down.png" ) );
		Add_Item_Object( Sprite_type, "Down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/right.png" ) );
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/left.png" ) );
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_green/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right down" );

		// Grey
		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/down.png" ) );
		Add_Item_Object( Sprite_type, "Down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/right.png" ) );
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/left.png" ) );
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_grey/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right down" );

		// Red
		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/down.png" ) );
		Add_Item_Object( Sprite_type, "Down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/right.png" ) );
		Add_Item_Object( Sprite_type, "Right" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/left.png" ) );
		Add_Item_Object( Sprite_type, "Left" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/middle.png" ) );
		Add_Item_Object( Sprite_type, "Middle" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/left_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Left up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/right_up.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Right up" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/left_down.png" ) );
		Add_Item_Object( Sprite_type, "Left down" );

		Sprite_type = new cSprite( GetImage( "ground/plastic_1/screw_block_red/right_down.png" ) );
		Add_Item_Object( Sprite_type, "Right down" );
	}
	else if( menu_id == MENU_JUNGLE_1 ) 
	{
		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/hill_up_1.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Yoshi Hill up" );

		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/rope_1_leftright.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Rope" );

		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/extra_1_red.png" ) );
		Add_Item_Object( Sprite_type, "Plant Red" );

		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/extra_1_yellow.png" ) );
		Add_Item_Object( Sprite_type, "Plant Yellow" );

		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/extra_1_blue.png" ) );
		Add_Item_Object( Sprite_type, "Plant Blue" );

		Sprite_type = new cSprite( GetImage( "ground/yoshi_1/extra_1_green.png" ) );
		Add_Item_Object( Sprite_type, "Plant Green" );

		Sprite_type = new cSprite( GetImage( "ground/jungle_1/berry_1.png" ) );
		Add_Item_Object( Sprite_type, "Berry" );

		Sprite_type = new cSprite( GetImage( "ground/jungle_1/kplant.png" ) );
		Sprite_type->massivetype = MASS_CLIMBABLE;
		Add_Item_Object( Sprite_type, "Climbing Plant" );

		Sprite_type = new cSprite( GetImage( "ground/jungle_1/beanstalk.png" ) );
		Add_Item_Object( Sprite_type, "Beanstalk" );
	}
	else if( menu_id == MENU_GHOST_1 )
	{
		Sprite_type = new cSprite( GetImage( "ground/ghost_1/way_1_left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Way left" );

		Sprite_type = new cSprite( GetImage( "ground/ghost_1/way_1_middle.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Way middle" );

		Sprite_type = new cSprite( GetImage( "ground/ghost_1/way_1_right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Way right" );
		
		Sprite_type = new cSprite( GetImage( "ground/ghost_1/lamp_1.png" ) );
		Add_Item_Object( Sprite_type, "Lamp" );

		Sprite_type = new cSprite( GetImage( "ground/ghost_1/window_1.png" ) );
		Add_Item_Object( Sprite_type, "Window" );
	}
	else if( menu_id == MENU_HEDGES_GREEN ) 
	{
		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/medium_1.png" ) );
		Add_Item_Object( Sprite_type, "Hedge Med" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/medium_2.png" ) );
		Add_Item_Object( Sprite_type, "Hedge Med" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/small_1.png" ) );
		Add_Item_Object( Sprite_type, "Hedge Sm" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/small_2.png" ) );
		Add_Item_Object( Sprite_type, "Hedge Sm" );

		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/big_1.png" ) );
		Add_Item_Object( Sprite_type, "Hedge Big" );
		
		Sprite_type = new cSprite( GetImage( "ground/green_1/hedges/wild_medium.png" ) );
		Add_Item_Object( Sprite_type, "Wild Hedge" );
	}
	else if( menu_id == MENU_PIPES ) 
	{
		// Green
		Sprite_type = new cSprite( GetImage( "pipes/green/up.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Up" );

		Sprite_type = new cSprite( GetImage( "pipes/green/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Down" );

		Sprite_type = new cSprite( GetImage( "pipes/green/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Left" );

		Sprite_type = new cSprite( GetImage( "pipes/green/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Right" );

		Sprite_type = new cSprite( GetImage( "pipes/green/ver.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Ver" );

		Sprite_type = new cSprite( GetImage( "pipes/green/hor.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Green Hor" );

		// Blue
		Sprite_type = new cSprite( GetImage( "pipes/blue/up.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Up" );

		Sprite_type = new cSprite( GetImage( "pipes/blue/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Down" );

		Sprite_type = new cSprite( GetImage( "pipes/blue/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Left" );

		Sprite_type = new cSprite( GetImage( "pipes/blue/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Right" );

		Sprite_type = new cSprite( GetImage( "pipes/blue/ver.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Ver" );

		Sprite_type = new cSprite( GetImage( "pipes/blue/hor.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Blue Hor" );

		// Yellow
		Sprite_type = new cSprite( GetImage( "pipes/yellow/up.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Up" );

		Sprite_type = new cSprite( GetImage( "pipes/yellow/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Down" );

		Sprite_type = new cSprite( GetImage( "pipes/yellow/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Left" );

		Sprite_type = new cSprite( GetImage( "pipes/yellow/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Right" );

		Sprite_type = new cSprite( GetImage( "pipes/yellow/ver.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Ver" );

		Sprite_type = new cSprite( GetImage( "pipes/yellow/hor.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Yellow Hor" );

		// Grey
		Sprite_type = new cSprite( GetImage( "pipes/grey/up.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Up" );

		Sprite_type = new cSprite( GetImage( "pipes/grey/down.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Down" );

		Sprite_type = new cSprite( GetImage( "pipes/grey/left.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Left" );

		Sprite_type = new cSprite( GetImage( "pipes/grey/right.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Right" );

		Sprite_type = new cSprite( GetImage( "pipes/grey/ver.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Ver" );

		Sprite_type = new cSprite( GetImage( "pipes/grey/hor.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Grey Hor" );
	}
	else if( menu_id == MENU_HILLS ) 
	{
		Sprite_type = new cSprite( GetImage( "hills/blue_1/1_big.png" ) );
		Add_Item_Object( Sprite_type, "Blue Big" );

		Sprite_type = new cSprite( GetImage( "hills/blue_1/1_medium.png" ) );
		Add_Item_Object( Sprite_type, "Blue Med" );

		Sprite_type = new cSprite( GetImage( "hills/light_blue_1/head.png" ) );
		Add_Item_Object( Sprite_type, "Blue Head" );

		Sprite_type = new cSprite( GetImage( "hills/light_blue_1/middle.png" ) );
		Add_Item_Object( Sprite_type, "Blue" );

		Sprite_type = new cSprite( GetImage( "hills/green_1/head.png" ) );
		Add_Item_Object( Sprite_type, "Green Head" );

		Sprite_type = new cSprite( GetImage( "hills/green_1/middle.png" ) );
		Add_Item_Object( Sprite_type, "Green" );
	}
	else if( menu_id == MENU_BLOCKS ) 
	{
		Sprite_type = new cSprite( GetImage( "game/box/brown1_1.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "default" );

		Sprite_type = new cSprite( GetImage( "blocks/stone/1.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Stone" );

		Sprite_type = new cSprite( GetImage( "blocks/wood/1.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Wood" );

		Sprite_type = new cSprite( GetImage( "game/box/white1_1.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "white" );

		Sprite_type = new cSprite( GetImage( "blocks/ice/1.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Ice" );

		Sprite_type = new cSprite( GetImage( "blocks/sand/1.png" ) );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Sand" );
	}
	else if( menu_id == MENU_SIGNS ) 
	{
		Sprite_type = new cSprite( GetImage( "signs/default_1/1_ending.png" ) );
		Add_Item_Object( Sprite_type, "Ending" );

		Sprite_type = new cSprite( GetImage( "signs/default_1/1_ending_big.png" ) );
		Add_Item_Object( Sprite_type, "Ending Big" );

		Sprite_type = new cSprite( GetImage( "signs/yoshi/post.png" ) );
		Add_Item_Object( Sprite_type, "Yoshi Post" );
	}
	else if( menu_id == MENU_CLOUDS ) 
	{
		Sprite_type = new cSprite( GetImage( "clouds/default_1/1_middle.png" ) );
		Add_Item_Object( Sprite_type, "Cloud" );

		Sprite_type = new cSprite( GetImage( "clouds/default_1/2_big.png" ) );
		Add_Item_Object( Sprite_type, "Cloud Big" );

		Sprite_type = new cSprite( GetImage( "clouds/default_1/2_small.png" ) );
		Add_Item_Object( Sprite_type, "Cloud Sm" );

		Sprite_type = new cSprite( GetImage( "clouds/cloudy_1/middle.png" ) );
		Add_Item_Object( Sprite_type, "Cloud" );
	}
	else if( menu_id == MENU_EXTRA ) 
	{
		Sprite_type = new cSprite( GetImage( "slider/grey_1/slider_left.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Slider left" );

		Sprite_type = new cSprite( GetImage( "slider/grey_1/slider_middle.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Slider mid" );

		Sprite_type = new cSprite( GetImage( "slider/grey_1/slider_right.png" ) );
		Sprite_type->massivetype = MASS_HALFMASSIVE;
		Add_Item_Object( Sprite_type, "Slider right" );

		Sprite_type = new cSprite( GetImage( "game/level/door_yellow_1.png" ) );
		Add_Item_Object( Sprite_type, "Door" );
	}
	else if( menu_id == MENU_ENEMY ) 
	{
		Sprite_type = new cGoomba( 0, 0, COL_BROWN );
		Add_Item_Object( Sprite_type, "Goomba Brown" );

		Sprite_type = new cGoomba( 0, 0, COL_BROWN );
		Sprite_type->Set_Direction( DIR_LEFT, 1 );
		Sprite_type->Update_Rotation_velx();
		Add_Item_Object( Sprite_type, "Goomba Brown" );

		Sprite_type = new cGoomba( 0, 0, COL_RED );
		Add_Item_Object( Sprite_type, "Goomba Blue" );

		Sprite_type = new cGoomba( 0, 0, COL_RED );
		Sprite_type->Set_Direction( DIR_LEFT, 1 );
		Sprite_type->Update_Rotation_velx();
		Add_Item_Object( Sprite_type, "Goomba Blue" );

		Sprite_type = new cTurtle( 0, 0, 1 ); // Red
		Add_Item_Object( Sprite_type, "Turtle Red" );
		
		Sprite_type = new cjPiranha( 0, 0, DIR_UP );
		Add_Item_Object( Sprite_type, "jPiranha" );

		Sprite_type = new cjPiranha( 0, 0, DIR_DOWN );
		Add_Item_Object( Sprite_type, "jPiranha" );

		Sprite_type = new cjPiranha( 0, 0, DIR_LEFT );
		Add_Item_Object( Sprite_type, "jPiranha" );

		Sprite_type = new cjPiranha( 0, 0, DIR_RIGHT );
		Add_Item_Object( Sprite_type, "jPiranha" );
		
		Sprite_type = new cbanzai_bill( 0, 0, DIR_LEFT ); // left
		Add_Item_Object( Sprite_type, "Banzai Bill" );

		Sprite_type = new cbanzai_bill( 0, 0, DIR_RIGHT ); // right
		Add_Item_Object( Sprite_type, "Banzai Bill" );

		Sprite_type = new cRex( 0, 0 );
		Add_Item_Object( Sprite_type, "Rex" );
	}
	else if( menu_id == MENU_SPECIAL ) 
	{
		Sprite_type = new cLevelExit( 0, 0 );
		Add_Item_Object( Sprite_type, "Level Beam", GetImage( "game/leveleditor/special.png" ) );

		Sprite_type = new cLevelExit( 0, 0, LVLCHANGE_WARP );
		Sprite_type->massivetype = MASS_MASSIVE;
		Add_Item_Object( Sprite_type, "Level Warp", GetImage( "game/leveleditor/special.png" ) );

		Sprite_type = new cCloud( 0, 0 );
		Add_Item_Object( Sprite_type, "Moving Cloud" );

		Sprite_type = new cMoving_Platform( 0, 0 );
		Add_Item_Object( Sprite_type, "Moving Plat." );

		Sprite_type = new cGoldPiece( 0, 0, COL_YELLOW );
		Add_Item_Object( Sprite_type, "Gold", GetImage( "animation/goldpiece/yellow/3.png" ) );

		Sprite_type = new cGoldPiece( 0, 0, COL_RED );
		Add_Item_Object( Sprite_type, "Red Gold", GetImage( "animation/goldpiece/red/3.png" ) );

		Sprite_type = new cMoon( 0, 0 );
		Add_Item_Object( Sprite_type, "Moon" );

		Sprite_type = new cEnemyStopper( 0, 0 );
		Add_Item_Object( Sprite_type, "Enemystopper", GetImage( "game/leveleditor/special.png" ) );
	}
	else
	{
		printf( "Unknown default Menu : %d\n", menu_id );
	}
	
	Set_Item_Objects_Pos();
	
	wItem_timer = DESIRED_FPS * 5;
}

void cLevelEditor :: Unload_Item_Menu( void )
{
	for( unsigned int i = 0; i < wItem_Objects.size(); i++ )
	{
		delete wItem_Objects[i];
	}
	
	wItem_Objects.clear();
	
	Item_scoll = 0;
}

void cLevelEditor :: Draw( void )
{
	if( !Leveleditor_Mode )
	{
		return;
	}

	Color color = (Uint8)0;

	// Level bottom line
	if( cameraposy > 0 && cameraposy < GAME_RES_H )
	{
		int start_x = 0;

		if( cameraposx < 0 )
		{
			start_x = -(int)cameraposx;
		}

		Color color = Get_Color( 0, 0, 100, 192 );
		DrawLine( start_x, GAME_RES_H - (int)cameraposy, GAME_RES_W, GAME_RES_H - (int)cameraposy, &color );
	}

	// Level left line
	if( cameraposx < 0 && cameraposx > -GAME_RES_W ) 
	{
		int start_y = GAME_RES_H;

		if( cameraposy < GAME_RES_H )
		{
			start_y = GAME_RES_H - (int)cameraposy;
		}

		Color color = Get_Color( 0, 100, 0, 192 );
		DrawLine( 0 - (int)cameraposx, start_y, 0 - (int)cameraposx, 0, &color );
	}


	// The Main Window Animation
	if( wMain_timer <= 0 )
	{
		if( (int)wMain_open > 10 )
		{
			wMain_open -= Framerate.speedfactor * 8;

			if( wMain_open < 10 )
			{
				wMain_open = 10;
			}

			wMain.w = (int)wMain_open;

			Set_Main_Objects_Pos();
		}
	}
	else
	{
		wMain_timer -= Framerate.speedfactor;

		if( (int)wMain_open < 150 )
		{
			wMain_open += Framerate.speedfactor * 8;

			if( wMain_open > 150 )
			{
				wMain_open = 150;
			}

			wMain.w = (int)wMain_open;

			Set_Main_Objects_Pos();
		}
	}

	// The Item Window Animation
	if( wItem_timer <= 0 )
	{
		if( (int)wItem_open > 10 )
		{
			wItem_open += Framerate.speedfactor * 8;

			if( wItem_open > GAME_RES_H - 10 )
			{
				wItem_open = GAME_RES_H - 10;
			}

			wItem.y = (int)wItem_open;

			Set_Item_Objects_Pos();
		}
	}
	else
	{
		wItem_timer -= Framerate.speedfactor;

		if( (int)wItem_open > GAME_RES_H - wItem.h )
		{
			wItem_open -= Framerate.speedfactor * 8;

			if( wItem_open < (double)( GAME_RES_H - wItem.h ) )
			{
				wItem_open = (double)( GAME_RES_H - wItem.h );
			}

			wItem.y = (int)wItem_open;

			Set_Item_Objects_Pos();
		}
	}

	// Main Window Box
	color = Get_Color( 0, 0, 0, 128 );
	DrawRect( wMain.x, wMain.y, wMain.w, wMain.h, &color );
	color = Get_Color( 0, 0, 130, 128 );
	DrawRect( wMain.x, wMain.y, 10, wMain.h, &color );

	if( wItem_Objects.size() )
	{
		// Item Window Box
		color = Get_Color( 0, 0, 0, 128 );
		DrawRect( wItem.x, wItem.y, wItem.w, wItem.h, &color );
		color = Get_Color( 130, 0, 0, 128 );
		DrawRect( wItem.x, wItem.y, wItem.w, 10, &color );
	}

	unsigned int i;

	// Item Menu
	for( i = 0; i < wItem_Objects.size(); i++ )
	{
		if( !wItem_Objects[i] )
		{
			continue;
		}

		if( wItem_Objects[i]->Col_rect.x > wItem.x + wItem.w - 27 - wItem_Objects[i]->Col_rect.w || wItem_Objects[i]->Col_rect.x < wItem.x + 20 ) // limits
		{
			wItem_Objects[i]->visible = 0;
			continue;
		}
		else
		{
			wItem_Objects[i]->visible = 1;
		}

		color = Get_Color( 0, 0, 0, 64 );
		DrawRect( wItem_Objects[i]->Col_rect.x, wItem_Objects[i]->Col_rect.y, wItem_Objects[i]->Col_rect.w, wItem_Objects[i]->Col_rect.h, &color );

		// preview image
		double posx = wItem_Objects[i]->Col_rect.x + 5;
		double posy = wItem_Objects[i]->Col_rect.y + 5;

		wItem_Objects[i]->preview_img->Blit( posx, posy, NULL, wItem_Objects[i]->Sprite_type->rotx, wItem_Objects[i]->Sprite_type->roty, wItem_Objects[i]->Sprite_type->rotz, wItem_Objects[i]->scale );

		// image name
		posy = wItem_Objects[i]->Col_rect.y - 15;
		wItem_Objects[i]->name_img->Blit( posx, posy, &black );

		// image dimensions
		posy = wItem_Objects[i]->Col_rect.y + wItem_Objects[i]->Col_rect.h + 4;
		wItem_Objects[i]->size_img->Blit( posx, posy, &black );
	}

	// Item Scroller
	if( wItem.y == GAME_RES_H - wItem.h )
	{
		Item_scroller_left->col_rect.x = Item_scroller_left->posx;
		Item_scroller_left->col_rect.y = Item_scroller_left->posy;
		Item_scroller_right->col_rect.x = Item_scroller_right->posx;
		Item_scroller_right->col_rect.y = Item_scroller_right->posy;

		if( Item_scroller_left->active )
		{
			Item_scroller_left->image_hover->Blit( Item_scroller_left->col_rect.x, Item_scroller_left->col_rect.y );
			Item_scoll += Framerate.speedfactor * 8;
			Item_scroller_left->active = 0;
			pLeveleditor->Set_Item_Objects_Pos();
		}
		else
		{
			Item_scroller_left->image_default->Blit( Item_scroller_left->col_rect.x, Item_scroller_left->col_rect.y );
		}

		if( Item_scroller_right->active )
		{
			Item_scroller_right->image_hover->Blit( Item_scroller_right->col_rect.x, Item_scroller_right->col_rect.y );
			Item_scoll -= Framerate.speedfactor * 8;
			Item_scroller_right->active = 0;
			pLeveleditor->Set_Item_Objects_Pos();
		}
		else
		{
			Item_scroller_right->image_default->Blit( Item_scroller_right->col_rect.x, Item_scroller_right->col_rect.y );
		}
	}

	// Main Menu
	for( i = 0; i < wMenu_Objects.size(); i++ )
	{
		if( !wMenu_Objects[i] || wMenu_Objects[i]->Col_rect.x == 0 )
		{
			continue;
		}

		if( wMenu_Objects[i]->Item_Menu_Id != MENU_SPACER )
		{
			if( wMenu_Objects[i]->state == 1 )	// Hovered
			{
				wMenu_Objects[i]->hover_img->Blit( wMenu_Objects[i]->Col_rect.x, wMenu_Objects[i]->Col_rect.y, &black );
			}
			else // default
			{
				wMenu_Objects[i]->def_img->Blit( wMenu_Objects[i]->Col_rect.x, wMenu_Objects[i]->Col_rect.y, &black );
			}

			if( wMenu_Objects[i]->state == 2 && wMain_open == 150 )	// Active
			{
				DrawRect( wMain.x + 11, wMenu_Objects[i]->Col_rect.y, 5, wMenu_Objects[i]->Col_rect.h - 1, &orange );
			}
		}
		else
		{
			color = Get_Color( 100, 100, 100, 192 );
			DrawRect( wMain.x + 15, wMain.y + 5 + wMenu_Objects[i]->cur_posy + wMenu_Objects[i]->spacer_height/2, wMain.w - 25, 1, &color );
		}
	}

	pMouseCursor->Update_Position();
	pMouseCursor->MouseObject_Update();
	pMouseCursor->Update();
}

void cLevelEditor :: Add_Item_Object( cSprite *nSprite_type, string nName /* = "" */, GL_Surface *image /* = NULL */ )
{
	if( !nSprite_type || ( !nSprite_type->image && !image ) )
	{
		return;
	}

	// if no image is given use the Sprite image
	if( !image )
	{
		image = nSprite_type->image;
	}

	string full_path = image->filename;

	if( !full_path.length() ) 
	{
		printf( "Invalid Menu Item path : %s\n", full_path.c_str() );
		return;
	}

	cMenu_Item_Object *mItem = new cMenu_Item_Object();

	double img_w = image->w;
	double img_h = image->h;

	mItem->preview_img = image;
	mItem->scale = Get_scale( image, 50, 50 );

	mItem->path = full_path;

	if( nName.length() > 1 )
	{
		mItem->name = nName;
	}
	else
	{
		full_path.erase( 0, full_path.rfind( "/" ) + 1 ); // Remove the path
		full_path.erase( full_path.length() - 4, full_path.length() ); // Remove the image extension

		mItem->name = full_path;
	}

	// description text
	mItem->name_img = RenderText( font_small, mItem->name.c_str(), white );
	// image dimension text
	char size_text[10];
	sprintf( size_text, "%dx%d", (int)img_w, (int)img_h );
	mItem->size_img = RenderText( font_small, size_text, white );
	// type pointer
	mItem->Sprite_type = nSprite_type;

	wItem_Objects.push_back( mItem );

	Set_Item_Objects_Pos();
}

cMenu_Item_Object *cLevelEditor :: Get_Item_Object( unsigned int nId )
{
	if( wItem_Objects.size() <= nId ) 
	{
		return NULL;
	}

	return wItem_Objects[nId];
}

void cLevelEditor :: Set_Item_Objects_Pos( void )
{
	for( unsigned int i = 0; i < wItem_Objects.size(); i++ )
	{
		if( !wItem_Objects[i] )
		{
			continue;
		}

		// Collision Rect
		wItem_Objects[i]->Col_rect.w = 60;
		wItem_Objects[i]->Col_rect.h = 60;
		wItem_Objects[i]->Col_rect.x = wItem.x + Item_scoll + 30 + ( ( wItem_Objects[i]->Col_rect.w + 20 ) * ( i ) );
		wItem_Objects[i]->Col_rect.y = wItem.y + 30;
	}
}

void cLevelEditor :: Switch_ObjectState( void )
{
	cObjectCollision *col = pMouseCursor->Get_firstCollision();

	if( !col )
	{
		return;
	}

	// from Passive to Massive
	if( col->type == CO_PASSIVE )
	{
		pMouseCursor->MouseObject->massivetype = MASS_MASSIVE;
		pMouseCursor->MouseObject->array = ARRAY_MASSIVE;
	}
	// From Massive to Halfmassive
	else if( col->type == CO_MASSIVE && pMouseCursor->MouseObject->massivetype == MASS_MASSIVE )
	{
		pMouseCursor->MouseObject->massivetype = MASS_HALFMASSIVE;
		pMouseCursor->MouseObject->type = TYPE_HALFMASSIVE;
		pMouseCursor->MouseObject->array = ARRAY_ACTIVE;
	}
	// From Halfmassive to Climbable
	else if( pMouseCursor->MouseObject->type == TYPE_HALFMASSIVE && pMouseCursor->MouseObject->massivetype == MASS_HALFMASSIVE )
	{
		pMouseCursor->MouseObject->massivetype = MASS_CLIMBABLE;
	}
	// From Climbable to Passive
	else if( pMouseCursor->MouseObject->massivetype == MASS_CLIMBABLE )
	{
		pMouseCursor->MouseObject->massivetype = MASS_PASSIVE;
		pMouseCursor->MouseObject->type = TYPE_SPRITE;
		pMouseCursor->MouseObject->array = ARRAY_PASSIVE;
	}

	pMouseCursor->MouseObject = NULL;
}

void cLevelEditor :: Change_DrawPosition( bool push_back )
{
	cObjectCollision *col = pMouseCursor->Get_firstCollision();

	if( !col )
	{
		return;
	}

	if( col->type == CO_MASSIVE || col->type == CO_PASSIVE || col->type == CO_ACTIVE )
	{
		cSprite *temp = NULL;

		if( !push_back )
		{
			temp = pObjManager->items.back();

			if( pObjManager->items[col->number]->ID == temp->ID )
			{
				return;
			}

			pObjManager->items.back() = pObjManager->items[col->number];
			pObjManager->items.erase( pObjManager->items.begin() + col->number );
			pObjManager->items.insert( pObjManager->items.end() - 1, temp );
		}
		else
		{
			temp = pObjManager->items.front();

			if( pObjManager->items[col->number]->ID == temp->ID )
			{
				return;
			}

			pObjManager->items.front() = pObjManager->items[col->number];
			pObjManager->items.erase( pObjManager->items.begin() + col->number );
			pObjManager->items.insert( pObjManager->items.begin() + 1, temp );
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void LE_Settings( void )
{
	cLevel_Settings *settings = new cLevel_Settings();
	settings->Enter();
	delete settings;
}

void LE_New( void )
{
	Color color = Get_Color( 0, 0, 0, 64 );
	DrawRect( NULL, &color );

	string levelname = EditMessageBox( "Create a new Level", "Levelname", GAME_RES_W/2 - 100, GAME_RES_H/2 - 10, 1 );

	if( levelname.length() < 2 )
	{
		return;
	}

	if( levelname.find( ".txt" ) == string::npos ) 
	{
		levelname.insert( levelname.length(), ".txt" );
	}

	if( levelname.find( pPreferences->level_dir ) == string::npos ) 
	{
		levelname.insert( 0, "/" );
		levelname.insert( 0, pPreferences->level_dir );
	}

	if( pLevel->New( levelname ) )
	{
		debugdisplay->text = "Created " + levelname;
		debugdisplay->counter = DESIRED_FPS * 2;
	}
	else
	{
		debugdisplay->text = "Level already exist " + levelname;
		debugdisplay->counter = DESIRED_FPS * 2;
	}
}

void LE_Load( void )
{
	KeyDown( SDLK_l );
}

void LE_Save( void )
{
	pLevel->Save();
}

void LE_Save_as( void )
{
	Color color = Get_Color( 0, 0, 0, 64 );
	DrawRect( NULL, &color );

	string levelname = EditMessageBox( "Save Level as", "Levelname", GAME_RES_W/2 - 100, GAME_RES_H/2 - 10, 1 );

	if( levelname.length() < 2 )
	{
		return;
	}

	pLevel->Set_Levelfile( levelname, 0 );
	pLevel->Save();
}


void LE_Clear( void )
{
	pObjManager->Delete_All();
}

void LE_Exit( void )
{
	KeyDown( SDLK_F8 );
}

void LE_Enter_Overworld( void )
{
	pOverWorld->Enter();
}
