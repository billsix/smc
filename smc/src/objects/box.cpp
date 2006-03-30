/***************************************************************************
           box.cpp  -  classes for some boxes used in the game
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

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

cBaseBox :: cBaseBox( double x, double y, SpriteType boxtype ) : cActiveSprite( x, y )
{
	type = boxtype;
	move_col_dir = DIR_NOTHING;
	move_back = 0;
	useable_count = 1; // default = 1 time usable
	start_useable_count = 1;

	if( type == TYPE_BONUSBOX_MUSHROOM_FIRE )
	{
		item_image = GetImage( "game/items/mushroom_red.png" );
	}
	else if( type == TYPE_BONUSBOX_LIVE )
	{
		item_image = GetImage( "game/items/mushroom_green.png" );
	}
	else if( type == TYPE_BONUSBOX_JSTAR )
	{
		item_image = GetImage( "game/items/star.png" );
	}
	else if( type == TYPE_GOLDBOX )
	{
		item_image = GetImage( "animation/goldpiece/yellow/4.png" );
	}
	else if( type == TYPE_SPINBOX )
	{
		item_image = GetImage( "world/arrows/up_small.png" );
	}
	else
	{
		item_image = NULL;
	}
}

cBaseBox :: ~cBaseBox( void )
{
	//
}

void cBaseBox :: Set_UseableCount( int count, bool new_startcount /* = 0 */ )
{
	if( count < -1 )
	{
		count = -1;
	}
	else if( count == 0 )
	{
		count = 1;
	}

	useable_count = count;

	if( new_startcount )
	{
		start_useable_count = useable_count;
	}
}

void cBaseBox :: Activate_collision( ObjectDirection cdirection )
{
	// if already active ignore event
	if( move_col_dir != DIR_NOTHING )
	{
		return;
	}

	move_col_dir = cdirection;

	if( useable_count > 0 )
	{
		useable_count--;
	}
}

void cBaseBox :: Update_collision( void )
{
	if( move_col_dir == DIR_NOTHING )
	{
		return;
	}

	double xcheck = 0, ycheck = 0;

	if( move_col_dir == DIR_UP )
	{
		ycheck = -2.5;
	}
	else if( move_col_dir == DIR_RIGHT )
	{
		xcheck = 2.5;
	}
	else if( move_col_dir == DIR_LEFT )
	{
		xcheck = -2.5;
	}
	else
	{
		printf( "Warning : wrong box collision direction %d\n", move_col_dir );
		move_col_dir = DIR_NOTHING;
		return;
	}

	if( xcheck != 0 || ycheck != 0 ) // move into the given direction
	{
		// move into the given direction
		if( !move_back )
		{
			Move( xcheck, ycheck, 0, 1, 0 );

			// retain collision pos
			col_pos.x -= xcheck * Framerate.speedfactor;
			col_pos.y -= ycheck * Framerate.speedfactor;

			// check if reached final position
			if( ( xcheck > 0 && posx - ( xcheck * 4 ) > startposx ) || // right
				( xcheck < 0 && posx - ( xcheck * 4 ) < startposx ) || // left
				( ycheck > 0 && posy - ( ycheck * 4 ) > startposy ) || // down
				( ycheck < 0 && posy - ( ycheck * 4 ) < startposy ) )  // up
			{
				move_back = 1;
			}
		}
		// move back to the original position
		else if( posy != startposy || posy != startposx )
		{
			Move( -xcheck, -ycheck, 0, 1, 0 );

			// retain collision pos
			col_pos.x += xcheck * Framerate.speedfactor;
			col_pos.y += ycheck * Framerate.speedfactor;

			// check if reached original position
			if(	( xcheck > 0 && posx < startposx ) || // right
				( xcheck < 0 && posx > startposx ) || // left
				( ycheck > 0 && posy < startposy ) || // down
				( ycheck < 0 && posy > startposy ) ) // up
			{
				// reset position
				posy = startposy;
				posx = startposx;

				// reset rect
				col_pos = image->col_pos;
				move_col_dir = DIR_NOTHING;
				move_back = 0;
			}
		}
	}
}

void cBaseBox :: Update( void )
{
	Update_collision();
}

void cBaseBox :: Draw( void )
{
	if( !visible && !Leveleditor_Mode )
	{
		return;
	}

	if( !isVisible_onScreen() ) 
	{
		return;
	}

	cActiveSprite::Draw();

	if( Leveleditor_Mode && item_image )
	{
		// correct item position if needed
		if( type == TYPE_BONUSBOX_LIVE )
		{
			item_image->Blit( startposx - 4 - cameraposx, startposy - 8 - cameraposy );
		}
		else if( type == TYPE_GOLDBOX )
		{
			item_image->Blit( startposx + 4 - cameraposx, startposy - cameraposy );
		}
		else
		{
			item_image->Blit( startposx - cameraposx, startposy - cameraposy );
		}
	}
}

void cBaseBox :: PlayerCollision( ObjectDirection cdirection )
{
	if( cdirection == DIR_UP ) // if player jumps from below
	{
		if( useable_count != 0 )
		{
			Activate_collision( cdirection );

			if( ColCheck( posx, posy - col_rect.h, col_rect.w * 0.9, 7 ) != CO_NOTHING )
			{
				cObjectCollision *col = Get_lastCollision();

				 // Enemy collision
				if( col->type == CO_ENEMY && pObjManager->items[col->number]->type != TYPE_JPIRANHA )
				{
					pAudio->PlaySound( SOUNDS_DIR "/death_box.ogg" );
					pObjManager->items[col->number]->DownGrade();
				}
				// Mushroom collision
				else if( col->type == CO_ACTIVE && ( pObjManager->items[col->number]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[col->number]->type == TYPE_MUSHROOM_LIVE_1 ) ) 
				{
					pObjManager->items[col->number]->BoxCollision( DIR_DOWN, &col_rect );
				}

				Delete_lastCollision();
			}
		}
		else
		{
			if( isVisible_onScreen() )
			{
				pAudio->PlaySound( SOUNDS_DIR "/tock.ogg" );
			}
		}
	}
}

void cBaseBox :: EnemyCollision( ObjectDirection cdirection )
{
	if( cdirection == DIR_RIGHT || cdirection == DIR_LEFT )
	{
		if( useable_count != 0 )
		{
			Activate_collision( cdirection );

			if( cdirection == DIR_RIGHT ) 
			{
				if( ColCheck( posx + 7, posy, col_rect.w * 0.8 ) != CO_NOTHING )
				{
					cObjectCollision *col = Get_lastCollision();

					if( col->type == CO_ENEMY ) // Enemy
					{
						pAudio->PlaySound( SOUNDS_DIR "/death_box.ogg" );
						pObjManager->items[col->number]->DownGrade();
					}

					Delete_lastCollision();
				}
			}
			else if( cdirection == DIR_LEFT )
			{
				if( ColCheck( posx - 7, posy, col_rect.w * 0.8 ) != CO_NOTHING )
				{
					cObjectCollision *col = Get_lastCollision();

					if( col->type == CO_ENEMY ) // Enemy
					{
						pAudio->PlaySound( SOUNDS_DIR "/death_box.ogg" );
						pObjManager->items[col->number]->DownGrade();
					}

					Delete_lastCollision();
				}
			}
		}
		else
		{
			if( isVisible_onScreen() )
			{
				pAudio->PlaySound( SOUNDS_DIR "/tock.ogg" );
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** cGoldBox *** *** *** *** *** *** *** *** *** */

cGoldBox :: cGoldBox( double x, double y, DefaultColor goldcolor /* = COL_YELLOW */ ) : cBaseBox( x, y, TYPE_GOLDBOX )
{
	player_range = 10000;

	images.push_back( GetImage( "game/box/yellow1_1.png" ) );
	images.push_back( GetImage( "game/box/brown1_1.png" ) );
	SetImage( 0 );

	color = goldcolor;
}

cGoldBox :: ~cGoldBox( void )
{
	for( unsigned int i = 0; i < goldpieces.size(); i++ )
	{
		delete goldpieces[i];
	}

	goldpieces.clear();
}

void cGoldBox :: Activate( void )
{
	pAudio->PlaySound( SOUNDS_DIR "/goldpiece_1.ogg" );
	goldpieces.push_back( new cBouncingGoldPiece( posx, posy, color ) );
}

void cGoldBox :: Update( void )
{
	if( !isVisible_onScreen() )
	{
		return;
	}

	for( unsigned int i = 0; i < goldpieces.size(); i++ )
	{
		if( goldpieces[i]->visible )
		{
			if( goldpieces[i]->vely < 8 ) // add velocity downwards
			{
				goldpieces[i]->AddVel( 0, 1.62 );
			}
			else // finished
			{
				goldpieces[i]->Activate();
				goldpieces[i]->SetPos( goldpieces[i]->posx, posy );
			}

			goldpieces[i]->Update();
		}
	}

	for( unsigned int i = 0; i < goldpieces.size(); i++ )
	{
		if( !goldpieces[i]->visible )
		{
			delete goldpieces[i];
			goldpieces.erase( goldpieces.begin() + i );
		}
	}

	cBaseBox::Update();
	Draw();
}

void cGoldBox :: PlayerCollision( ObjectDirection cdirection )
{
	cBaseBox::PlayerCollision( cdirection );

	if( cdirection == DIR_UP ) // if player jumps from below
	{
		if( move_col_dir != DIR_NOTHING )
		{
			Activate();

			if( !useable_count )
			{
				SetImage( 1 );
			}
		}
	}
}

void cGoldBox :: EnemyCollision( ObjectDirection cdirection )
{
	cBaseBox::EnemyCollision( cdirection );

	if( cdirection == DIR_RIGHT || cdirection == DIR_LEFT )
	{
		if( move_col_dir != DIR_NOTHING )
		{
			Activate();

			if( !useable_count )
			{
				SetImage( 1 );
			}
		}
	}
}

void cGoldBox :: Editor_Activate( void )
{
	pDialogManager->Add( 0, 0, "gold_color", int_to_string( color ), TYPE_DIALOG_ONLY_NUMBERS, 4, 30 );
	pDialogManager->Add( 0, 0, "useable_count", int_to_string( start_useable_count ), TYPE_DIALOG_ONLY_NUMBERS, 100, 30 );

	// set position
	Editor_pos_update();
}

void cGoldBox :: Editor_Deactivate( void )
{
	// remove controls
	pDialogManager->Delete( "gold_color" );
	pDialogManager->Delete( "useable_count" );
}

void cGoldBox :: Editor_pos_update( void )
{
	// gold color
	cDialog *dialog = pDialogManager->Get( "gold_color" );

	if( dialog )
	{
		dialog->SetPos( posx + col_rect.w + 5, posy + 5 );
	}

	// useable count
	dialog = pDialogManager->Get( "useable_count" );

	if( dialog )
	{
		dialog->SetPos( posx + col_rect.w + 5, posy + 40 + 5 );
	}
}

void cGoldBox :: Editor_val_update( void )
{
	// gold color
	cDialog *dialog = pDialogManager->Get( "gold_color" );

	if( dialog )
	{
		// update
		if( dialog->gotChanged() )
		{
			color = (DefaultColor)dialog->Get_Value_int();

			if( color != COL_YELLOW && color != COL_RED )
			{
				color = COL_YELLOW;
			}

			dialog->Set_Text( int_to_string( color ) );
		}
	}

	// useable count
	dialog = pDialogManager->Get( "useable_count" );

	if( dialog )
	{
		// update
		if( dialog->gotChanged() )
		{
			Set_UseableCount( dialog->Get_Value_int(), 1 );

			dialog->Set_Text( int_to_string( start_useable_count ) );
		}
	}
}

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

cBonusBox :: cBonusBox( double x, double y, SpriteType stype ) : cBaseBox( x, y, stype )
{
	counter = 0;
	powerup = NULL;
	player_range = 10000;

	if( type == TYPE_BONUSBOX_MUSHROOM_FIRE || type == TYPE_BONUSBOX_LIVE || type == TYPE_BONUSBOX_JSTAR )
	{
		images.push_back( GetImage( "game/box/yellow3_1.png" ) );
		images.push_back( GetImage( "game/box/yellow3_anim_1.png" ) );
		images.push_back( GetImage( "game/box/yellow3_anim_2.png" ) );
		images.push_back( GetImage( "game/box/yellow3_anim_3.png" ) );
		images.push_back( GetImage( "game/box/brown1_1.png" ) );
	}
	else
	{
		printf( "Unknown BonusBox type : %d\n", type );
		return;
	}

	SetImage( 0 );
}

cBonusBox :: ~cBonusBox( void )
{
	if( powerup )
	{
		delete powerup;
	}
}

void cBonusBox :: Activate( void )
{
	pAudio->PlaySound( SOUNDS_DIR "/sprout_1.ogg" );

	if( type == TYPE_BONUSBOX_MUSHROOM_FIRE )
	{
		if( pPlayer->maryo_type == MARYO_SMALL || ( pPlayer->maryo_type == MARYO_FIRE && !Itembox->item_id ) )
		{
			powerup = (cPowerUp *)new cMushroom( startposx, startposy, TYPE_MUSHROOM_DEFAULT );
		}
		else
		{
			powerup = (cPowerUp *)new cFirePlant( startposx, startposy );
		}
	}
	else if( type == TYPE_BONUSBOX_LIVE )
	{
		powerup = (cPowerUp *)new cMushroom( startposx - 4, startposy - 8, TYPE_MUSHROOM_LIVE_1 );
	}
	else if( type == TYPE_BONUSBOX_JSTAR )
	{
		powerup = (cPowerUp *)new cjStar( startposx, startposy );
	}
}

void cBonusBox :: PlayerCollision( ObjectDirection cdirection )
{
	cBaseBox::PlayerCollision( cdirection );

	if( cdirection == DIR_UP ) // if player jumps from below
	{
		if( move_col_dir != DIR_NOTHING && !powerup )
		{
			Activate();

			if( !useable_count )
			{
				SetImage( 4 );
			}
		}
	}
}

void cBonusBox :: EnemyCollision( ObjectDirection cdirection )
{
	cBaseBox::EnemyCollision( cdirection );

	if( cdirection == DIR_RIGHT || cdirection == DIR_LEFT )
	{
		if( move_col_dir != DIR_NOTHING && !powerup )
		{
			Activate();

			if( !useable_count )
			{
				SetImage( 4 );
			}
		}
	}
}

void cBonusBox :: Update( void )
{
	if( !isVisible_onScreen() )
	{
		return;
	}

	if( powerup )
	{
		if( powerup->posy < posy - powerup->col_rect.h - powerup->col_pos.y )
		{
			powerup->counter = 0;
			powerup->spawned = 1;

			// set the object on top
			powerup->Set_onTop( this, 0 );
			// add the powerup to the level objects
			pObjManager->Add( powerup );
			powerup = NULL;
		}
		else // animation
		{
			powerup->counter += Framerate.speedfactor;
			powerup->Move( 0, -3.1, 0, 1, 0 );

 			powerup->Draw();
		}
	}

	if( useable_count > 0 )
	{
		counter += Framerate.speedfactor;

		if( counter < 4 )
		{
			SetImage( 0 );
		}
		else if( counter < 8 )
		{
			SetImage( 1 );
		}
		else if( counter < 12 )
		{
			SetImage( 2 );
		}
		else if( counter < 16 )
		{
			SetImage( 3 );
		}
		else
		{
			counter = 0;
		}
	}

	cBaseBox::Update();
	Draw();
}

/* *** *** *** *** *** *** *** *** ccSpinBox *** *** *** *** *** *** *** *** *** */

cSpinBox :: cSpinBox( double x, double y ) : cBaseBox( x, y, TYPE_SPINBOX )
{
	player_range = 10000;

	counter = 0;
	spin_counter = 0;
	spin = 0;
	useable_count = -1; // infinite times activate-able
	
	images.push_back( GetImage( "game/box/yellow1_1.png" ) );
	images.push_back( GetImage( "game/box/yellow1_spin_1.png" ) );
	images.push_back( GetImage( "game/box/yellow1_spin_2.png" ) );
	images.push_back( GetImage( "game/box/yellow1_spin_3.png" ) );

	SetImage( 0 );
}

cSpinBox :: ~cSpinBox( void )
{
	//
}

void cSpinBox :: PlayerCollision( ObjectDirection cdirection )
{
	cBaseBox::PlayerCollision( cdirection );

	if( cdirection == DIR_UP )	// if player jumps from below
	{
		if( move_col_dir != DIR_NOTHING && !spin )
		{
			spin = 1;
			massivetype = MASS_PASSIVE;
		}
	}
}

void cSpinBox :: EnemyCollision( ObjectDirection cdirection )
{
	cBaseBox::EnemyCollision( cdirection );

	if( cdirection == DIR_RIGHT || cdirection == DIR_LEFT )
	{
		if( move_col_dir != DIR_NOTHING && !spin )
		{
			spin = 1;
			massivetype = MASS_PASSIVE;
		}
	}
}

void cSpinBox :: Update( void )
{
	if( !isVisible_onScreen() )
	{
		return;
	}

	if( spin )
	{
		counter += 10 * Framerate.speedfactor;
		spin_counter += Framerate.speedfactor;

		if( counter < DESIRED_FPS )
		{
			//
		}
		else if( counter < DESIRED_FPS * 2 )
		{
			SetImage( 1 );
		}
		else if( counter < DESIRED_FPS * 3 )
		{
			SetImage( 2 );
		}
		else if( counter < DESIRED_FPS * 4 )
		{
			SetImage( 3 );
		}
		else
		{
			SetImage( 0 );
			counter = 0;

			if( spin_counter > DESIRED_FPS * 5 )
			{
				spin_counter = 0;
				massivetype = MASS_MASSIVE; // needed for collision check
				ObjectCollisionType collision = ColCheck( posx, posy, 0, 0, 1 );

				if( collision == CO_PLAYER || collision == CO_ENEMY || collision == CO_ACTIVE )
				{
					massivetype = MASS_PASSIVE;
				}
				else
				{
					spin = 0;
				}
			}
		}
	}
	
	cBaseBox::Update();
	Draw();
}
