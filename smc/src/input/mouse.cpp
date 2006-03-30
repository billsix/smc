/***************************************************************************
           mouse.cpp  -  mousecursor class
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

#include "../globals.h"
#include "../main.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMouseCursor :: cMouseCursor( void ) : cSprite( GetImage( "game/mouse.png" ) )
{
	type = TYPE_MOUSECURSOR;

	clickcounter = 0;

	HoveredObject.x = 0;
	HoveredObject.y = 0;
	HoveredObject.w = 0;
	HoveredObject.h = 0;

	MouseObject = NULL;
	CopyObject = NULL;
	ActiveObject = NULL;

	mouse_h = 0;
	mouse_w = 0;

	fastCopyMode = 0;
	mover_mode = 0;
	click_temp = NULL;

	left = 0;
	right = 0;
	middle = 0;

	Mover_mouse = GetImage( "game/axis_m.png" );

	SDL_GetMouseState( &x, &y );
	SDL_ShowCursor( SDL_DISABLE );
}

cMouseCursor :: ~cMouseCursor( void )
{
	//
}

void cMouseCursor :: Reset( void )
{
	CopyObject = NULL;
	MouseObject = NULL;
	ActiveObject = NULL;
	mover_mode = 0;
	fastCopyMode = 0;

	ClearCollisions();
	pDialogManager->DeleteAll();
}

void cMouseCursor :: Handle_Events( void )
{
	switch( event.type )
	{
		case SDL_MOUSEMOTION:
		{
			pGuiSystem->injectMousePosition( (float)event.motion.x, (float)event.motion.y );
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			Gui_handle_Mouse_up( event.button.button );

			if( event.button.button == SDL_BUTTON_LEFT )
			{
				left = 0;
			}
			else if( event.button.button == SDL_BUTTON_MIDDLE )
			{
				middle = 0;
			}
			else if( event.button.button == SDL_BUTTON_RIGHT )
			{
				right = 0;
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			Gui_handle_Mouse_down( event.button.button );

			if( event.button.button == SDL_BUTTON_LEFT )
			{
				left = 1;
			}
			else if( event.button.button == SDL_BUTTON_MIDDLE )
			{
				middle = 1;
			}
			else if( event.button.button == SDL_BUTTON_RIGHT )
			{
				right = 1;
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

bool cMouseCursor :: CollsionCheck( GL_rect *rect )
{
	if( !rect )
	{
		return 0;
	}

	if( x > rect->x + rect->w )
	{
		return 0;
	}
	if( x < rect->x )
	{
		return 0;
	}
	if( y > rect->y + rect->h )
	{
		return 0;
	}
	if( y < rect->y )
	{
		return 0;
	}

	return 1;
}

bool cMouseCursor :: Editor_CollsionCheck( double x, double y )
{
	ClearCollisions();

	if( mover_mode )
	{
		return 0;
	}

	GL_rect rect1;
	GL_rect rect2;
	
	rect1.x = x;
	rect1.y = y;
	rect1.w = 1;
	rect1.h = 1;
	
	HoveredObject.x = 0;
	HoveredObject.y = 0;
	HoveredObject.w = 0;
	HoveredObject.h = 0;

	// Leveleditor Objects
	if( Leveleditor_Mode > 0 && !MouseObject )
	{
		// Item Menu
		if( pLeveleditor->wItem.y == GAME_RES_H - pLeveleditor->wItem.h ) 
		{
			for( unsigned int i = 0; i < pLeveleditor->wItem_Objects.size(); i++ )
			{
				cMenu_Item_Object *item = pLeveleditor->Get_Item_Object( i );

				if( !item || !item->visible ) 
				{
					continue;
				}

				rect2 = item->Col_rect;
				rect2.x += cameraposx;
				rect2.y += cameraposy;
				
				if( Col_Box( &rect1, &rect2 ) )
				{
					Add_Collision( this, NULL, i, CO_EDITOR_ITEM );

					HoveredObject = item->Col_rect;
					pLeveleditor->wItem_timer = DESIRED_FPS * 3;

					return 1;
				}
			}
		}
		
		// Main menu
		if( pLeveleditor->wMain_open == 150 ) 
		{
			bool found = 0;

			for( unsigned int i = 0; i < pLeveleditor->wMenu_Objects.size() ; i++ )
			{
				cMenu_Main_Object *item = pLeveleditor->Get_Main_Object( i );

				if( !item ) 
				{
					continue;
				}

				rect2 = item->Col_rect;
				rect2.x = pLeveleditor->wMain.x + 15;
				rect2.w = pLeveleditor->wMain.w - 22;
				rect2.x += cameraposx;
				rect2.y += cameraposy;
				
				if( Col_Box( &rect1, &rect2 ) )
				{
					Add_Collision( this, NULL, i, CO_EDITOR_MENU );

					rect2.x -= cameraposx;
					rect2.y -= cameraposy;
					HoveredObject = rect2;

					if( item->state == 0 )
					{
						item->state = 1;
					}
					
					pLeveleditor->wMain_timer = DESIRED_FPS * 3;

					found = 1;
				}
				else // reset hovered
				{
					if( item->state == 1 )
					{
						item->state = 0;
					}
				}
			}

			if( found )
			{
				return 1;
			}
		}
	}
	// The Leveleditor Main Menu
	rect2 = pLeveleditor->wMain;
	rect2.x += cameraposx;
	rect2.y += cameraposy;

	if( Leveleditor_Mode && Col_Box( &rect1, &rect2 ) ) 
	{
		pLeveleditor->wMain_timer = ( DESIRED_FPS * 3 );
		return 0;
	}
	
	// Leveleditor item menu
	if( pLeveleditor->wItem_Objects.size() && Leveleditor_Mode ) 
	{
		if( pLeveleditor->wItem.y == GAME_RES_H - pLeveleditor->wItem.h ) 
		{
			rect2 = pLeveleditor->Item_scroller_left->col_rect;
			rect2.x += cameraposx;
			rect2.y += cameraposy;

			// left Item Object Scroller
			if( Col_Box( &rect1, &rect2 ) ) 
			{
				pLeveleditor->wItem_timer = DESIRED_FPS * 3;
				Add_Collision( this, NULL, 0, CO_EDITOR_SCROLL );
				Get_lastCollision()->direction = DIR_LEFT;

				HoveredObject = pLeveleditor->Item_scroller_left->col_rect;

				return 1;	
			}

			rect2 = pLeveleditor->Item_scroller_right->col_rect;
			rect2.x += cameraposx;
			rect2.y += cameraposy;

			// right Item Object Scroller
			if( Col_Box( &rect1, &rect2 ) ) 
			{
				pLeveleditor->wItem_timer = DESIRED_FPS * 3;
				Add_Collision( this, NULL, 0, CO_EDITOR_SCROLL );
				Get_lastCollision()->direction = DIR_RIGHT;

				HoveredObject = pLeveleditor->Item_scroller_right->col_rect;
				
				return 1;	
			}
		}

		rect2 = pLeveleditor->wItem;
		rect2.x += cameraposx;
		rect2.y += cameraposy;

		if( Col_Box( &rect1, &rect2 ) ) 
		{
			pLeveleditor->wItem_timer = DESIRED_FPS * 3;
			return 0;
		}
	}

	// Dialog Boxes
	if( ( !MouseObject || MouseObject != ActiveObject ) && pDialogManager->Get_Collision( (int)x, (int)y ) >= 0 )
	{
		Add_Collision( this, NULL, pDialogManager->Get_Collision( (int)x, (int)y ), CO_DIALOG );
		return 1;
	}

	// Active Objects
	// only Enemystopper and Levelexit
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ACTIVE || ( pObjManager->items[i]->type != TYPE_ENEMYSTOPPER && pObjManager->items[i]->type!= TYPE_LEVELEXIT ) )
		{
			continue;
		}

		
		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_ACTIVE );
			return 1;
		}
	}
	
	// Enemy Objects
	// only jPiranha
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ENEMY || pObjManager->items[i]->type != TYPE_JPIRANHA )
		{
			continue;
		}

		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_ENEMY );
			return 1;
		}
	}
	
	// Massive Objects
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || !pObjManager->items[i]->massivetype || pObjManager->items[i]->array != ARRAY_MASSIVE )
		{
			continue;
		}

		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_MASSIVE );
			return 1;
		}
	}

	// Active Objects
	// All non Enemystopper
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ACTIVE || pObjManager->items[i]->type == TYPE_ENEMYSTOPPER || pObjManager->items[i]->type == TYPE_LEVELEXIT 
			|| pObjManager->items[i]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[i]->type == TYPE_MUSHROOM_LIVE_1 || pObjManager->items[i]->type == TYPE_FIREPLANT )
		{
			continue;
		}

		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_ACTIVE );
			return 1;
		}
	}

	// Enemy Objects
	// only non jPiranha
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ENEMY || pObjManager->items[i]->type == TYPE_JPIRANHA )
		{
			continue;
		}

		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_ENEMY );
			return 1;
		}
	}

	// player
	if( pPlayer )
	{
		rect2 = pPlayer->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pPlayer, 0, CO_PLAYER );
			return 1;
		}
	}

	// passive
	for( int i = pObjManager->Get_Size() - 1; i >= 0; i-- )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_PASSIVE )
		{
			continue;
		}

		rect2 = pObjManager->items[i]->start_rect;

		if( Col_Box( &rect1, &rect2 ) )
		{
			Add_Collision( this, pObjManager->items[i], i, CO_PASSIVE );
			return 1;
		}
	}
	
	
	ClearCollisions();
	return 0;
}

void cMouseCursor :: Update( void )
{
	if( Game_Mode == MODE_LEVEL )
	{
		if( !mover_mode ) 
		{
			cObjectCollision *col = Get_firstCollision();

			if( !left && ( MouseObject || ( col && ( col->type == CO_EDITOR_ITEM || col->type == CO_EDITOR_MENU || col->type == CO_EDITOR_SCROLL ) ) ) )
			{
				Draw_HoveredObject();
			}
			
			if( !( left && MouseObject && pPreferences->Lvleditor_automousehide ) )
			{
				start_image->Blit( x, y );
			}
		}
		else
		{
			Mover_mouse->Blit( x, y );
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		start_image->Blit( x, y );
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		start_image->Blit( x, y );
	}
}

void cMouseCursor :: Update_Position( void )
{
	SDL_GetMouseState( &x, &y );

	if( Game_Mode == MODE_LEVEL )
	{
		if( !mover_mode )
		{
			SetPos( x + cameraposx, y + cameraposy );
		}
	}
	else
	{
		SetPos( x, y );
	}
	
	col_rect.x = posx;
	col_rect.y = posy;
}

void cMouseCursor :: Update_Doubleclick( void )
{
	if( clickcounter )
	{
		clickcounter -= Framerate.speedfactor;

		if( clickcounter < 0 )
		{
			clickcounter = 0;
		}
	}
}

void cMouseCursor :: MouseObject_Update( void )
{
	if( !Leveleditor_Mode || !MouseObject || ( mover_mode && Game_Mode == MODE_LEVEL ) ) 
	{
		return;
	}

	if( !left )
	{
		HoveredObject.x = (Sint16)( MouseObject->startposx - cameraposx );
		HoveredObject.y = (Sint16)( MouseObject->startposy - cameraposy );
		
		if( MouseObject->start_image )
		{
			HoveredObject.w = (Uint16)MouseObject->start_image->w;
			HoveredObject.h = (Uint16)MouseObject->start_image->h;
		}
		else
		{
			HoveredObject.w = MouseObject->col_rect.w;
			HoveredObject.h = MouseObject->col_rect.h;
		}
	}
	else
	{
		MouseObject->SetPos( (int)posx - mouse_w, (int)posy - mouse_h, 1 );

		if( ActiveObject && ActiveObject == MouseObject )
		{
			ActiveObject->Editor_pos_update();
		}

	}
}

void cMouseCursor :: Click( void )
{
	Editor_CollsionCheck( posx, posy );
	cObjectCollision *col = Get_firstCollision();
	cSprite *col_sprite = NULL;

	if( col && ( col->type == CO_PASSIVE || col->type == CO_MASSIVE || col->type == CO_ACTIVE || col->type == CO_ENEMY || col->type == CO_PLAYER ) )
	{
		col_sprite = pObjManager->Get_Pointer( col->number );
	}

	if( !col_sprite )
	{
		clickcounter = 0;
		return;
	}

	if( clickcounter > 0 ) 
	{
		if( click_temp == col_sprite ) // if last lciked object was the same
		{
			Double_Click();
		}
		else
		{
			clickcounter = 0;
		}
	}
	else
	{
		clickcounter = DESIRED_FPS * 0.4; // double click counter
		click_temp = col_sprite; // save last clicked object
	}
}

void cMouseCursor :: Double_Click( bool activate )
{
	if( ActiveObject )
	{
		ActiveObject->Editor_Deactivate();
		ActiveObject = NULL;
	}

	if( activate ) // add new
	{
		Editor_CollsionCheck( posx, posy );
		cObjectCollision *col = Get_firstCollision();

		if( col && col->number >= 0 )
		{
			ActiveObject = pObjManager->Get_Pointer( col->number );

			if( ActiveObject )
			{
				ActiveObject->Editor_Activate();
			}
		}
	}

	clickcounter = 0;
}

void cMouseCursor :: Set_MouseObject( cSprite *sprite )
{
	MouseObject = sprite;
}

void cMouseCursor :: Copy( cSprite *nCopyObject, double x, double y )
{
	cSprite *new_Sprite = Copy_Object( nCopyObject, x, y );

	if( !new_Sprite )
	{
		printf( "MouseCursor CopyObject invald source Object\n" );
		return;
	}

	if( new_Sprite->array == ARRAY_MASSIVE || new_Sprite->array == ARRAY_PASSIVE || new_Sprite->array == ARRAY_ACTIVE || new_Sprite->array == ARRAY_ENEMY ) 
	{
		pObjManager->Add( new_Sprite );
	}
	else
	{
		printf( "MouseCursor CopyObject unknown Array : %d\n", new_Sprite->array );
		delete new_Sprite;
	}
}

void cMouseCursor :: Delete( void )
{
	if( !MouseObject )
	{
		return;
	}

	cObjectCollision *col = Get_firstCollision();

	if( col && ( col->type == CO_PASSIVE || col->type == CO_MASSIVE || col->type == CO_ACTIVE || col->type == CO_ENEMY ) )
	{
		if( ActiveObject == pObjManager->Get_Pointer( col->number ) )
		{
			Double_Click( 0 );
		}

		pObjManager->Delete( col->number );
	}
}

void cMouseCursor :: Draw_HoveredObject( void )
{
	if( HoveredObject.w == 0 || HoveredObject.h == 0 )
	{
		return;
	}

	Color dColor = (Uint8)0;

	if( fastCopyMode )
	{
		dColor = green;
	}
	else
	{
		dColor = white;
	}

	GL_rect r2 = HoveredObject;
	
	r2.h = 1;
	DrawRect( &r2, &dColor ); // Up

	r2.y += HoveredObject.h;
	DrawRect( &r2, &dColor ); // Down

	r2.y += 1;
	DrawRect( &r2, &grey );	// Down Shadow

	r2.y = HoveredObject.y;
	r2.h = HoveredObject.h;
	r2.w = 1;
	DrawRect( &r2, &dColor );	// Left

	r2.x = HoveredObject.x + HoveredObject.w;
	r2.h += 1;
	DrawRect( &r2, &dColor ); // Right

	r2.x += 1;
	DrawRect( &r2, &grey );	// Right Shadow
}

void cMouseCursor :: Mover_Update( Sint16 move_x, Sint16 move_y )
{
	if( !mover_mode )
	{
		return;
	}
	
	cameraposx += move_x;
	cameraposy += move_y;

	SDL_WarpMouse( x, y );

	SDL_Event inEvent;
	SDL_PeepEvents( &inEvent, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK );

	while( SDL_PollEvent( &inEvent ) )
	{
		switch( inEvent.type )
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				if( inEvent.button.button == 2 )
				{
					mover_mode = 0;
				}
				break;
			}
			case SDL_KEYDOWN:
			{
				mover_mode = 0;
				KeyDown( event.key.keysym.sym );
				break;
			}
			case SDL_QUIT:
			{
				done = 1;
				break;
			}
		}
	}
}

void cMouseCursor :: Editor_Update( void )
{
	bool is_col = Editor_CollsionCheck( posx, posy );
	cObjectCollision *col = Get_firstCollision();

	if( is_col && col )
	{
		string type;
		type.reserve( 60 );

		if( col->type == CO_MASSIVE )
		{
			if( !left || !MouseObject )
			{
				Set_MouseObject( pObjManager->items[col->number] );
			}

			if( MouseObject->massivetype == MASS_HALFMASSIVE )
			{
				type = "Halfmassive";
			}
			else
			{
				type = "Massive";
			}
		}
		else if( col->type == CO_ACTIVE )
		{
			if( !left || !MouseObject )
			{
				Set_MouseObject( pObjManager->items[col->number] );
			}

			if( MouseObject->type == TYPE_ENEMYSTOPPER )
			{
				type = "Enemystopper";
			}
			else if( MouseObject->type == TYPE_GOLDPIECE )
			{
				cGoldPiece *active = (cGoldPiece*)MouseObject;

				if( active->color == COL_RED )
				{
					type = "Red Goldpiece";
				}
				else
				{
					type = "Goldpiece";
				}
			}
			else if( MouseObject->type == TYPE_MOON )
			{
				type = "Moon (3-UP)";
			}
			else if( MouseObject->type == TYPE_CLOUD )
			{
				type = "Moving Cloud";
			}
			else if( MouseObject->type == TYPE_MOVING_PLATFORM )
			{
				type = "Moving Platform";
			}
			else if( MouseObject->type == TYPE_HALFMASSIVE )
			{
				if( MouseObject->massivetype != MASS_CLIMBABLE )
				{
					type = "Halfmassive";
				}
				else
				{
					type = "Climbable";
				}
			}
			else if( MouseObject->type == TYPE_GOLDBOX )
			{
				cGoldBox *active = (cGoldBox*)MouseObject;

				if( active->color == COL_RED )
				{
					type = "Red Goldbox";
				}
				else
				{
					type = "Goldbox";
				}
			}
			else if( MouseObject->type == TYPE_BONUSBOX_MUSHROOM_FIRE )
			{
				type = "Bonus box Mushroom - Fire";
			}
			else if( MouseObject->type == TYPE_BONUSBOX_LIVE )
			{
				type = "Bonus box Level-UP";
			}
			else if( MouseObject->type == TYPE_BONUSBOX_JSTAR )
			{
				type = "Bonus box Star";
			}
			else if( MouseObject->type == TYPE_SPINBOX )
			{
				type = "Spin box";
			}
			else if( MouseObject->type == TYPE_LEVELEXIT )
			{
				cLevelExit *active = (cLevelExit*)MouseObject;

				if( active->levelchange == LVLCHANGE_WARP )
				{
					type = "Levelexit (Warp)";
				}
				else
				{
					type = "Levelexit (Beam)";
				}
			}
			else
			{
				type = "Active";
			}
		}
		else if( col->type == CO_ENEMY )
		{
			if( !left || !MouseObject )
			{
				Set_MouseObject( pObjManager->items[col->number] );
			}

			if( MouseObject->type == TYPE_TURTLE )
			{
				type = "Red Turtle";
			}
			else if( MouseObject->type == TYPE_GOOMBA )
			{
				cGoomba *enemy = (cGoomba*)MouseObject;

				if( enemy->color == 1 ) // Red
				{
					type = "Blue Goomba";
				}
				else
				{
					type = "Brown Goomba";
				}
			}
			else if( MouseObject->type == TYPE_JPIRANHA )
			{
				type = "Jumping Piranha";
			}
			else if( MouseObject->type == TYPE_BANZAI_BILL )
			{
				type = "Banzai Bill";
			}
			else if( MouseObject->type == TYPE_REX )
			{
				type = "Rex";
			}
			else
			{
				type = "Enemy";
			}
		}
		else if( col->type == CO_PLAYER )
		{
			if( !left || !MouseObject )
			{
				Set_MouseObject( (cSprite *)pPlayer );
			}
			
			type = "Maryo";
		}
		else if( col->type == CO_PASSIVE )
		{
			if( !left || !MouseObject )
			{
				Set_MouseObject( pObjManager->items[col->number] );
			}
			
			type = "Passive";
		}
		else if( col->type == CO_EDITOR_ITEM )
		{
			cMenu_Item_Object *item = pLeveleditor->Get_Item_Object( col->number );

			if( item && !left )
			{
				type = item->name;
			}
			else
			{
				type = "";
			}

			if( !MouseObject ) 
			{
				if( left ) 
				{
					cSprite *new_sprite = Copy_Object( item->Sprite_type, posx, posy );

					if( !new_sprite ) 
					{
						printf( "Error : Sprite copy failed\n" );
						return;
					}

					pObjManager->Add( new_sprite );

					new_sprite->posx = posx;
					new_sprite->posy = posy;

					mouse_h = (int)( new_sprite->col_rect.h / 2 );
					mouse_w = (int)( new_sprite->col_rect.w / 2 );

					Set_MouseObject( new_sprite );
				}
			}
			else if( !left ) 
			{
				MouseObject = NULL;
			}
		}
		else if( col->type == CO_EDITOR_MENU )
		{
			cMenu_Main_Object *item = pLeveleditor->Get_Main_Object( col->number );

			if( item && !left ) 
			{
				type = item->name;
			}
			else
			{
				type = "";
			}

			if( !MouseObject ) 
			{
				if( left ) 
				{
					if( item->Item_Menu_Id != MENU_FUNCTION ) 
					{
						// load the menu
						pLeveleditor->Load_Item_Menu( item->Item_Menu_Id );
						pLeveleditor->Set_Main_Active( col->number );
					}
					else
					{
						// call the function
						item->pfunction();
						left = 0;
						
						// exit because col pointers could have changed
						return;
					}
				}
			}
			else
			{
				MouseObject = NULL;
			}
		}
		else if( col->type == CO_EDITOR_SCROLL )
		{
			type = "Object Scroller";

			if( !MouseObject && left ) 
			{
				if( col->direction == DIR_LEFT )
				{
					pLeveleditor->Item_scroller_left->active = 1;
				}
				else if( col->direction == DIR_RIGHT )
				{
					pLeveleditor->Item_scroller_right->active = 1;
				}
			}
			else if( !left ) 
			{
				MouseObject = NULL;
			}

		}
		else if( col->type == CO_DIALOG )
		{
			if( col->number >= 0 && pDialogManager->Get_Size() >= (unsigned int)col->number )
			{
				MouseObject = NULL;
				type = pDialogManager->Objects[col->number]->identifier;

				if( left ) 
				{
					pDialogManager->Objects[col->number]->Get_Focus();
					left = 0;

					if( ActiveObject )
					{
						ActiveObject->Editor_val_update();
					}
				}
			}
		}

		if( col->type > CO_NOTHING && col->type < CO_EDITOR_ITEM && MouseObject )
		{
			if( !left )
			{
				if( col->type == CO_PLAYER || col->type == CO_ENEMY )
				{
					mouse_w = (int)posx - (int)MouseObject->startposx;
					mouse_h = (int)posy - (int)MouseObject->startposy;
				}
				else
				{
					mouse_w = (int)posx - (int)MouseObject->posx;
					mouse_h = (int)posy - (int)MouseObject->posy;
				}
			}
		
			string info = "X : " + int_to_string( (int)MouseObject->posx ) + "  Y : " + int_to_string( (int)MouseObject->posy );
			GL_Surface *position_info = RenderText( font_small, info, white );
			position_info->Blit( x + 20, y + 35, &black );
			delete position_info;
		}
		
		if( debugdisplay->counter <= 0 )
		{
			debugdisplay->text = type;
			debugdisplay->counter = 1;
		}
	}
	else
	{
		if( !left )
		{
			MouseObject = NULL;

			if( !col )
			{
				mouse_w = 0;
				mouse_h = 0;
			}
		}
		else
		{
			if( !MouseObject )
			{
				Double_Click( 0 );
			}
		}
	}
}
