/***************************************************************************
           player.cpp  -  level maryo class
                             -------------------
    copyright            :	(C) 2003 - 2004 by Artur Hallmann
							(C) 2003 - 2006 by FluXy
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

cPlayer :: cPlayer( double x, double y ) : cObjectSprite( x, y )
{
	type = TYPE_PLAYER;

	massivetype = MASS_MASSIVE;
	state = FALL;
	maryo_type = MARYO_SMALL;

	images.reserve( 20 );

	ducked = 0;

	invincible = 0;
	invincible_star = 0;
	invincible_mod = 0;
	invincible_starcounter = 0;

	direction = DIR_RIGHT;
	walk_count = 0;
	update_downgrade = 0;

	godmode = 0;

	lives = 3;		// Starting with 3 lives 
	goldpieces = 0;
	points = 0;

	// jump data
	UpKeytime = 0;
	start_enemyjump = 0;
	jump_power = 0;
	jump_accel_up = 4.5;
	jump_vel_deaccel = 0.06;

	LoadImages( 1 ); // preload all images
}

cPlayer :: ~cPlayer( void )
{
	Fireball_clear();
}

void cPlayer :: Hold( void )
{
	if( state == JUMP || state == FALL || state == STAY || state == CLIMB )
	{
		return;
	}
	
	Set_MovingState( STAY );
}

void cPlayer :: Move_Player( double velocity, double vel_wrongway )
{
	if( direction == DIR_RIGHT && ColCheck( posx + 2, posy ) == CO_NOTHING ) // don't move if colliding
	{
		if( velx > 0 )
		{
			AddVel( velocity, 0 );

			if( velx > 13 )
			{
				velx = 13;
			}
		}
		else
		{
			if( groundObject && groundObject->massivetype == MASS_MASSIVE && velx < 0 ) // small smoke clouds under foots
			{
				cAnimation_3 *anim = new cAnimation_3( posx + 8, posy + col_rect.h - 5, 1 );
				anim->SetDirectionRange( 170, 30 );
				anim->SetSpeed( -velx / 5, -velx / 5 );
				pAnimationManager->Add( anim );
			}

			AddVel( vel_wrongway, 0 );	// slow down
		}
	}
	else if( direction == DIR_LEFT && ColCheck( posx - 2, posy ) == CO_NOTHING ) // don't move if colliding
	{
		if( velx < 0 )
		{
			AddVel( -velocity, 0 );

			if( velx < -13 )
			{
				velx = -13;
			}
		}
		else
		{
			if( groundObject && groundObject->massivetype == MASS_MASSIVE && velx > 0 ) // small smoke clouds under foots
			{
				cAnimation_3 *anim = new cAnimation_3( posx + col_rect.w - 8, posy + col_rect.h - 5, 1 );
				anim->SetDirectionRange( 330, 30 );
				anim->SetSpeed( velx / 5, velx / 5 );
				pAnimationManager->Add( anim );
			}

			AddVel( -vel_wrongway, 0 );	// slow down
		}
	}
}

void cPlayer :: Update_walking( void )
{
	if( ducked || state == CLIMB || state == JUMP || state == FALL || !groundObject )
	{
		return;
	}

	// only if left or right is pressed
	if( keys[pPreferences->Key_left] || keys[pPreferences->Key_right] || pJoystick->left || pJoystick->right )
	{
		Move_Player( 1, 1.8 );
	}
}

void cPlayer :: Update_staying( void )
{
	if( !groundObject || ducked || state == JUMP ) // only if player is onground
	{
		return;
	}

	// if left and right is not pressed
	if( !keys[pPreferences->Key_left] && !keys[pPreferences->Key_right] && !pJoystick->left && !pJoystick->right )
	{
		Auto_SlowDown( 1.1 );

		// if not moving anymore
		if( velx == 0 )
		{
			walk_count = 0;
		}
	}

	// if staying don't move vertical
	if( vely != 0 )
	{
		vely = 0;
	}
}

void cPlayer :: Update_flying( void )
{
	/* only if not onground
	 * if jumping update jump is already used
	*/
	if( groundObject || state == CLIMB || state == JUMP )
	{
		return;
	}

	if( ( keys[pPreferences->Key_left] || pJoystick->left ) && !ducked )
	{
		if( velx > -13 )
		{
			AddVel( -1.2, 0 );
		}
		
	}	
	else if( ( keys[pPreferences->Key_right] || pJoystick->right ) && !ducked )
	{
		if( velx < 13 )
		{
			AddVel( 1.2, 0 );
		}
	}
	else // slow down if nothing is pressed
	{
		Auto_SlowDown( 0.4 );
	}
}

void cPlayer :: Start_falling( void )
{
	posy += 1.9;
	Set_MovingState( FALL );
}

void cPlayer :: Update_falling( void )
{
	// only if not on ground
	if( groundObject || state == CLIMB )
	{
		return;
	}

	// Add Gravitation
	if( vely < 25.0 )
	{
		AddVel( 0, 2.8 );
	}

	if( state != JUMP && state != FALL )
	{
		Set_MovingState( FALL );
	}
}

void cPlayer :: Start_ducking( void )
{ 
	// only if massive ground
	if( ( state != STAY && state != WALK ) || state == CLIMB || !groundObject || groundObject->massivetype == MASS_HALFMASSIVE || ducked )
	{
		return;
	}

	rect.h = images[MARYO_IMG_DUCK]->h;
	rect.w = images[MARYO_IMG_DUCK]->w;
	col_rect.h = images[MARYO_IMG_DUCK]->col_h;
	col_rect.w = images[MARYO_IMG_DUCK]->col_w;

	Move( 0, images[MARYO_IMG_STAND]->h - images[MARYO_IMG_DUCK]->h, 1, 1 );

	ducked = 1;
	Set_MovingState( STAY );
}

void cPlayer :: Stop_ducking( void )
{
	if( !ducked )
	{
		return;
	}

	rect.h = images[MARYO_IMG_STAND]->h;
	rect.w = images[MARYO_IMG_STAND]->w;
	col_rect.h = images[MARYO_IMG_STAND]->col_h;
	col_rect.w = images[MARYO_IMG_STAND]->col_w;

	Move( 0, -( images[MARYO_IMG_STAND]->h - images[MARYO_IMG_DUCK]->h ), 1, 1 );

	ducked = 0;
	Set_MovingState( STAY );
}

void cPlayer :: Update_ducking( void )
{
	if( !ducked || !groundObject )
	{
		return;
	}

	Auto_SlowDown( 0.6 );
}

void cPlayer :: Start_climbing( void )
{
	if( ducked || state == CLIMB )
	{
		return;
	}

	Set_MovingState( CLIMB );
}

void cPlayer :: Update_climbing( void )
{
	if( state != CLIMB )
	{
		return;
	}

	velx = 0;
	vely = 0;

	if( keys[pPreferences->Key_left] || pJoystick->left )
	{
		velx = -3;
	}
	else if( keys[pPreferences->Key_right] || pJoystick->right )
	{
		velx = 3;
	}

	if( keys[pPreferences->Key_up] || pJoystick->up )
	{
		vely = -6;
	}
	else if( keys[pPreferences->Key_down] || pJoystick->down )
	{
		vely = 6;
	}

	// collision testing
	ColCheck( posx, posy, col_rect.w * 0.8, col_rect.h * 0.7 );
	cObjectCollision *col = Get_lastCollision();

	if( !col || !( col->type == CO_ACTIVE && pObjManager->items[col->number]->massivetype == MASS_CLIMBABLE ) ) // lost contact
	{
		Set_MovingState( FALL );
	}
	else if( vely < 0 ) // in contact testing
	{
		ClearCollisions();
		ColCheck( posx, posy + ( vely * Framerate.speedfactor ) - 15, col_rect.w * 0.8, 2 );
		col = Get_firstCollision();

		// do not climb further upwards if maryo will loose contact
		if( !col || !( col->type == CO_ACTIVE && pObjManager->items[col->number]->massivetype == MASS_CLIMBABLE ) )
		{
			if( vely < 0 ) // only stop upwards
			{
				vely = 0;
			}
		}
	}

	ClearCollisions();
}

void cPlayer :: Start_jump_keytime( void )
{
	if( !ducked && ( state == STAY || state == WALK || state == RUN || state == FALL || state == FLY || state == JUMP ) )
	{
		UpKeytime = DESIRED_FPS / 4;
	}
}

void cPlayer :: Start_jump( double power /* = 17.0 */, double accel /* = 4.0 */, double deaccel /* = 0.06 */ )
{
	jump_accel_up = accel;
	jump_vel_deaccel = deaccel;

	if( keys[pPreferences->Key_up] || pJoystick->up || pJoystick->Button( pPreferences->Joy_jump ) )
	{
		jump_power = 10;
	}
	else
	{
		jump_power = 2;
	}

	if( !start_enemyjump )
	{
		if( maryo_type == MARYO_SMALL )
		{
			pAudio->PlaySound( SOUNDS_DIR "/jump_small.ogg", RID_MARYO_JUMP );
		}
		else // big and fire
		{
			// random jump sound
			string jump_file = SOUNDS_DIR "/jump_big_" + int_to_string( (int)rand() % (3) ) + ".ogg";
			pAudio->PlaySound( jump_file, RID_MARYO_JUMP );
		}
	}

	UpKeytime = 0;
	start_enemyjump = 0;

	Set_MovingState( JUMP );

	vely = -power;

	// jump higher when running
	if( velx < 0 )
	{
		vely += velx / 9.5;
	}
	else if( velx > 0 )
	{
		vely -= velx / 9.5;
	}

	// slow down if running
	velx = velx * 0.9;
}

void cPlayer :: Update_jump( void )
{
	// jumping keytime
	if( UpKeytime )
	{
		UpKeytime -= Framerate.speedfactor;

		if( UpKeytime < 0 )
		{
			UpKeytime = 0;
		}
	}

	// only if jumping
	if( state != JUMP )
	{
		return;
	}

	// jumping physics
	if( keys[pPreferences->Key_up] || pJoystick->up || pJoystick->Button( pPreferences->Joy_jump ) )
	{
		AddVel( 0, -( jump_accel_up + ( vely * jump_vel_deaccel ) ) );
		jump_power -= Framerate.speedfactor;
	}
	else
	{
		AddVel( 0, 0.5 );
		jump_power -= 6 * Framerate.speedfactor;
	}
	
	// left right physics
	if( ( keys[pPreferences->Key_left] || pJoystick->left ) && !ducked )
	{
		if( velx > -13 )
		{
			AddVel( -1.7 - ( velx / 100 ), 0 );
		}
		
	}	
	else if( ( keys[pPreferences->Key_right] || pJoystick->right ) && !ducked && state )
	{
		if( velx < 13 )
		{
			AddVel( 1.7 + ( velx / 100 ), 0 );
		}
	}
	else // slow down if nothing is pressed
	{
		Auto_SlowDown( 0.2 );
	}

	// if no more jump power set to falling
	if( jump_power <= 0 )
	{
		Set_MovingState( FALL );
	}
}

void cPlayer :: Set_MovingState( Moving_state new_state )
{
	if( new_state == state )
	{
		return;
	}

	if( new_state == STAY )
	{
		vely = 0;
	}
	else if( new_state == FALL )
	{
		Reset_onGround();
		walk_count = 0;
		jump_power = 0;
	}
	else if( new_state == FLY )
	{
		Reset_onGround();
	}
	else if( new_state == JUMP )
	{
		Reset_onGround();
	}
	else if( new_state == CLIMB )
	{
		Reset_onGround();
		velx = 0;
		vely = 0;
	}

	state = new_state;
}

bool cPlayer :: ChangeSize( double x, double y, bool only_check /* = 0 */ )
{
	if( !only_check )
	{
		startposx += x;
		startposy += y;
	}

	int valid = 0;

	if( ColCheckAuto( x, 0 ) == CO_NOTHING )
	{
		if( !only_check )
		{
			Move( x, 0, 1, 1 );
		}

		valid++;
	}
	else if( ColCheckAuto( -x, 0 ) == CO_NOTHING )
	{
		if( !only_check )
		{
			Move( -x, 0, 1, 1 );
		}

		valid++;
	}


	if( ColCheckAuto( 0, y ) == CO_NOTHING )
	{
		if( !only_check )
		{
			Move( 0, y, 1, 1 );
		}

		valid++;
	}
	else if( ColCheckAuto( 0, -y ) == CO_NOTHING )
	{
		if( !only_check )
		{
			Move( 0, -y, 1, 1 );
		}

		valid++;
	}

	// if both directions valid
	if( valid == 2 )
	{
		return 1;
	}

	return 0;
}

void cPlayer :: ResetSave( void )
{
	pOverWorld->Maryo->current_waypoint = -2;
	pOverWorld->current_world = 0;
	
	pOverWorld->Maryo->Set_Waypoint( 0 );
	pOverWorld->Reset_Waypoints();

	pLevel->Unload();
	Reset();

	lives = 3;
	goldpieces = 0;
	points = 0;

	timedisplay->counter = 0;

	maryo_type = MARYO_SMALL;
	LoadImages();

	Itembox->Reset();
	pOverWorld->Load( "default" );
	pMenu->Enter_Menu();
}

void cPlayer :: Reset( void )
{
	SetPos( startposx, startposy );
	direction = DIR_RIGHT;
	ducked = 0;
	Set_MovingState( FALL );
	update_downgrade = 0;
	jump_power = 0;
	start_enemyjump = 0;
	jump_accel_up = 0;
	jump_vel_deaccel = 0;
	UpKeytime = 0;
	velx = 0;
	vely = 0;
	ClearCollisions();
	walk_count = 0;
	cameraposx = 0;
	cameraposy = 0;
	Reset_onGround();
	invincible = 0;
	invincible_mod = 0;
	invincible_star = 0;
	Itembox->Push_back();
}

void cPlayer :: GotoNextLevel( void )
{
	pLevel->Draw( LVL_DRAW );
	DrawEffect();

	pLevel->Unload();
	pOverWorld->GotoNextLevel();
	Reset();
	pOverWorld->Enter();
}

void cPlayer :: GotoSubLevel( string sublevel )
{
	pLevel->Load( sublevel );
	Reset();
}

void cPlayer :: Update( void )
{
	if( Leveleditor_Mode ) // only draw
	{
		cSprite::Draw();
		return;
	}

	// handle downgrade
	if( update_downgrade )
	{
		DownGrade( 1 );
	}

	// handle jumping start
	if( UpKeytime && groundObject )
	{
		Start_jump();
	}
	else if( start_enemyjump )
	{
		Start_jump( 18.0, 4.1 );
	}
	
	//**  Collision handling

	ClearCollisions();

	// Check if Maryo got stuck
	if( !ducked )
	{
		Update_antistuck();
	}

	if( groundObject && state != JUMP )
	{
		Set_MovingState( STAY );
	}
	if( state == STAY && velx != 0 )
	{
		Set_MovingState( WALK );
	}

	// update states
	Update_jump();
	Update_climbing();
	Update_falling();
	Update_walking();
	Update_ducking();
	Update_staying();
	Update_flying();

	// collision and movement handling
	CollideMove();

	if( godmode && posy > 550 )
	{
		vely = -35;
	}

	//** Drawing
	if( state == CLIMB )
	{
		if( vely != 0 || velx != 0 )
		{
			walk_count += Framerate.speedfactor * 0.7;
		}
	}
	/*else if( state == FALL || state == JUMP )
	{
		walk_count += Framerate.speedfactor / 3;
	}*/
	else if( state == WALK )
	{
		if( maryo_type == MARYO_SMALL ) // 2 frames animation
		{
			walk_count += Framerate.speedfactor * 0.15;
		}
		else if( maryo_type == MARYO_BIG ) // 3 frames animation
		{
			walk_count += Framerate.speedfactor * 0.35;
		}
		else if( maryo_type == MARYO_FIRE ) // 3 frames animation
		{
			walk_count += Framerate.speedfactor * 0.35;
		}
	}

	if( state == WALK || state == CLIMB )
	{
		if( velx > 0 )
		{
			walk_count += ( velx * 0.05 ) * Framerate.speedfactor;
		}
		else if( velx < 0 )
		{
			walk_count += ( -velx * 0.05 ) * Framerate.speedfactor;
		}
	}

	if( state == WALK )
	{
		if( maryo_type == MARYO_SMALL )
		{
			if( walk_count > 4 )
			{
				walk_count = 0;
			}
		}
		else if( maryo_type == MARYO_BIG )
		{
			if( walk_count > 6 )
			{
				walk_count = 0;
			}
		}
		else if( maryo_type == MARYO_FIRE )
		{
			if( walk_count > 6 )
			{
				walk_count = 0;
			}
		}
	}
	/*else if( state == FALL || state == JUMP )
	{
		if( walk_count > 6 )
		{
			walk_count = 6;
		}
	}*/
	else
	{
		if( walk_count > 8 )
		{
			walk_count = 0;
		}
	}

	Draw();

	//* Camera
	Update_Camera();

	//**  Fireballs
	for( unsigned int i = 0; i < fireballs.size(); i++ )
	{
		fireballs[i]->Update();
		fireballs[i]->Draw();

		if( fireballs[i]->destroy )
		{
			pAnimationManager->Add( new cAnimation_2( fireballs[i]->posx + fireballs[i]->col_rect.w / 2, fireballs[i]->posy + fireballs[i]->col_rect.h / 2 ) );

			delete fireballs[i];
			fireballs.erase( fireballs.begin() + i );

			continue;
		}
	}

	//** Special

	// if player is dead because of falling down
	if( posy - cameraposy > ( GAME_RES_H - col_rect.h ) )
	{
		invincible = 0;
		DownGrade();
	}

	if( invincible > 0 )
	{
		invincible -= Framerate.speedfactor;

		if( invincible < 0 )
		{
			invincible = 0;
		}
	}
}

void cPlayer :: HandleCollision( cObjectCollision *collision )
{
	if( collision->direction != DIR_NOTHING )
	{
		if( !( collision->type == CO_ACTIVE && ( pObjManager->items[collision->number]->massivetype == MASS_HALFMASSIVE || pObjManager->items[collision->number]->massivetype == MASS_CLIMBABLE ) ) )
		{
			if( collision->direction == DIR_UP )
			{
				vely = 0;

				if( state != CLIMB )
				{
					Set_MovingState( FALL );
					UpKeytime = 0;
					jump_power = 0;

					if( collision->type == CO_MASSIVE )
					{
						pAudio->PlaySound( SOUNDS_DIR "/tock.ogg", RID_MARYO_TOCK );
					}
				}
			}
			else if( collision->direction == DIR_DOWN && collision->type != CO_ENEMY )
			{
				vely = 0;
			}
			else if( collision->direction == DIR_LEFT && collision->type != CO_ENEMY )
			{
				velx = 0;
				vely -= ( vely / 35 );
			}
			else if( collision->direction == DIR_RIGHT && collision->type != CO_ENEMY )
			{
				velx = 0;
				vely -= ( vely / 35 );
			}

			if( collision->type == CO_ACTIVE || collision->type == CO_ENEMY )
			{
				pObjManager->items[collision->number]->PlayerCollision( collision->direction );
			}
		}

		if( collision->direction == DIR_DOWN && pObjManager->items[collision->number]->massivetype != MASS_CLIMBABLE )
		{
			// stop if falling
			if( vely > 0 )
			{
				vely = 0;
			}
		}
	}
}

void cPlayer :: Update_Camera( bool init /* = 0 */ )
{
	if( init )
	{
		cameraposy = posy - ( GAME_RES_H / 2 );
	}

	// x camera
	if( posx - cameraposx > ( (double)GAME_RES_W / 6 ) * 3 ) //  scrolling right
	{
		cameraposx = posx - ( (double)GAME_RES_W / 6 ) * 3;
	}
	else if( cameraposx && posx - cameraposx < ( (double)GAME_RES_W / 6 ) * 2 ) //  scrolling left
	{
		cameraposx = posx - ( (double)GAME_RES_W / 6 ) * 2;
	}

	// y camera
	if( posy - cameraposy < ( (double)GAME_RES_H / 6 ) ) // scrolling up
	{
		cameraposy += ( ( posy - cameraposy - ( (double)GAME_RES_H / 6 ) * 3 ) / 27 ) * Framerate.speedfactor;
	}
	else if( cameraposy && !( posy - cameraposy < 0 ) && posy - cameraposy > ( (double)GAME_RES_H / 6 ) * 2.8 ) // scrolling right
	{
		cameraposy += ( ( posy - cameraposy - ( (double)GAME_RES_H / 6 ) * 3 ) / 4.8 ) * Framerate.speedfactor;
	}

	// # limits
	if( cameraposx < 0 ) // left
	{
		cameraposx = 0;
	}
	if( cameraposy > 0 ) // down
	{
		cameraposy = 0;
	}
	// todo : level limits
}

void cPlayer :: Draw( void )
{
	if( state != CLIMB )
	{
		Draw_Player( Get_Image() + direction );
	}
	else
	{
		Draw_Player( Get_Image() );
	}
}

void cPlayer :: Draw_Player( unsigned int imgnr )
{
	SetImage( imgnr );

	if( invincible <= 0 ) // default
	{
		cSprite::Draw();
	}
	else // player is invincible
	{
		if( invincible_mod < 0 )
		{
			invincible_mod = invincible * 3;

			if( invincible_mod > 180 ) 
			{
				invincible_mod = 180;
			}
		}

		if( invincible_star > 0 ) // star invincible
		{
			glColor4ub( 255 - (Uint8)( invincible_mod / 4 ), 255 - (Uint8)( invincible_mod / 2 ), 255 - (Uint8)invincible_mod, 255 );
		}
		else // default invincible
		{
			glColor4ub( 255, 255, 255, 255 - (Uint8)invincible_mod );
		}
		cSprite::Draw();
		glColor4ub( 255, 255, 255, 255 );

		invincible_mod -= Framerate.speedfactor * 20;
	}

	// draw stars if in godmode or star invincible
	if( godmode || invincible_star > 0 )
	{
		// draw stars
		invincible_starcounter += Framerate.speedfactor;

		if( invincible_star > 0 )
		{
			invincible_star -= Framerate.speedfactor;
		}

		if( invincible_starcounter > 1 )
		{
			cAnimation_3 *anim = new cAnimation_3( posx + (double)( rand() % (int)( col_rect.w * 0.9 ) ), posy + (double)( rand() % (int)( col_rect.h * 0.9 ) ), 1 );
			anim->SetFadingSpeed( 1.5 );
			anim->SetImage( GetImage( "animation/particles/star.png" ) );
			anim->SetLifetimeMod( 3.5 );
			anim->SetFading( FADE_ALPHA );
			anim->SetSpeed( 1, 1.5 );
			anim->SetScale( 0.2 );
			anim->Set_ConstRotationZ( 0.9, 10 );
			if( godmode )
			{
				anim->SetColor( lightred );
			}
			else
			{
				anim->SetColor( Color( (Uint8)255, (Uint8)200, (Uint8)100 ) );
			}

			anim->SetBlending( BLEND_ADD );
			pAnimationManager->Add( anim );

			invincible_starcounter = 0;
		}
	}
}

void cPlayer :: Draw_Animation( unsigned int item_type )
{
	Maryo_type maryo_type_old = maryo_type;

	for( unsigned int i = 0; i < 6; i++ )
	{
		if( i % 2 )
		{
			if( item_type == TYPE_MUSHROOM_DEFAULT )
			{
				maryo_type = MARYO_BIG;
			}
			else if( item_type == TYPE_FIREPLANT )
			{
				maryo_type = MARYO_FIRE;
			}
			else
			{
				printf( "Unknown Player Animation item id : %d\n", item_type );
				return;
			}
			
			LoadImages();
			maryo_type = maryo_type_old;
			
			if( maryo_type_old == MARYO_SMALL )
			{
				Move( 0, -20, 1, 1 );
			}
		}
		else
		{
			LoadImages();
		}

		pLevel->Draw( LVL_DRAW );
		Draw();
		SDL_GL_SwapBuffers();

		if( i % 2 )
		{
			if( maryo_type_old == MARYO_SMALL )
			{
				Move( 0, 20, 1, 1 );
			}
		}

		SDL_Delay( 150 );
	}

	Framerate.Reset();
}

unsigned int cPlayer :: Get_Image( void )
{
	if( state == FALL )
	{
		if( !ducked )
		{
			/*unsigned int imgnum = (unsigned int)walk_count;

			for( unsigned int i = 0; i < imgnum; i++ )
			{
				if( imgnum % 2 )
				{
					imgnum--;
				}
			}

			return MARYO_IMG_FALL + imgnum;*/
			return MARYO_IMG_FALL;
		}
		else
		{
			return MARYO_IMG_DUCK;
		}
	}
	else if( state == STAY )
	{
		if( !ducked )
		{
			return MARYO_IMG_STAND;
		}
		else
		{
			return MARYO_IMG_DUCK;
		}
	}
	else if( state == JUMP )
	{
		/*unsigned int imgnum = (unsigned int)walk_count;

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_JUMP + imgnum;*/
		return MARYO_IMG_JUMP;
	}
	else if( state == CLIMB )
	{
		if( walk_count > 4 )
		{
			return MARYO_IMG_CLIMB;
		}
		else
		{
			return MARYO_IMG_CLIMB + 1;
		}
	}
	else if( state == WALK )
	{
		if( !ducked )
		{
			unsigned int imgnum = (unsigned int)walk_count;

			for( unsigned int i = 0; i < imgnum; i++ )
			{
				if( imgnum % 2 )
				{
					imgnum--;
				}
			}

			return MARYO_IMG_WALK + imgnum;
		}
		else
		{
			return MARYO_IMG_DUCK;
		}
	}

	return MARYO_IMG_STAND;
}

void cPlayer :: LoadImages( bool preload )
{
	if( preload )
	{
		Maryo_type old_maryo_type = maryo_type;
		
		maryo_type = MARYO_SMALL;	// small
		LoadImages();
		maryo_type = MARYO_BIG;		// big
		LoadImages();
		maryo_type = MARYO_FIRE;	// fire
		LoadImages();

		maryo_type = old_maryo_type;
	}

	images.clear();

	if( maryo_type == MARYO_SMALL )
	{
		/********************* Small **************************/
		// standing
		images.push_back( GetImage( "maryo/small/left.png" ) );
		images.push_back( GetImage( "maryo/small/right.png" ) );
		// running
		images.push_back( GetImage( "maryo/small/run_left.png" ) );
		images.push_back( GetImage( "maryo/small/run_right.png" ) );
		images.push_back( GetImage( "maryo/small/run_left.png" ) ); // missing
		images.push_back( GetImage( "maryo/small/run_right.png" ) ); // missing
		// falling
		images.push_back( GetImage( "maryo/small/fall_left.png" ) );
		images.push_back( GetImage( "maryo/small/fall_right.png" ) );
		// jumping
		images.push_back( GetImage( "maryo/small/jump_left.png" ) );
		images.push_back( GetImage( "maryo/small/jump_right.png" ) );
		// dead
		images.push_back( GetImage( "maryo/small/dead_left.png" ) );
		images.push_back( GetImage( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( GetImage( "maryo/small/duck_left.png" ) );
		images.push_back( GetImage( "maryo/small/duck_right.png" ) );
		// climbing
		images.push_back( GetImage( "maryo/small/climb_left.png" ) );
		images.push_back( GetImage( "maryo/small/climb_right.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_BIG )
	{
		/********************* Big ****************************/
		// standing
		images.push_back( GetImage( "maryo/big/left.png" ) );
		images.push_back( GetImage( "maryo/big/right.png" ) );
		// running
		images.push_back( GetImage( "maryo/big/run_left_1.png" ) );
		images.push_back( GetImage( "maryo/big/run_right_1.png" ) );
		images.push_back( GetImage( "maryo/big/run_left_2.png" ) );
		images.push_back( GetImage( "maryo/big/run_right_2.png" ) );
		// falling
		images.push_back( GetImage( "maryo/big/fall_left.png" ) );
		images.push_back( GetImage( "maryo/big/fall_right.png" ) );
		// jumping
		images.push_back( GetImage( "maryo/big/jump_left.png" ) );
		images.push_back( GetImage( "maryo/big/jump_right.png" ) );
		// dead
		images.push_back( GetImage( "maryo/small/dead_left.png" ) );
		images.push_back( GetImage( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( GetImage( "maryo/big/duck_left.png" ) );
		images.push_back( GetImage( "maryo/big/duck_right.png" ) );
		// climbing
		images.push_back( GetImage( "maryo/big/climb_left.png" ) );
		images.push_back( GetImage( "maryo/big/climb_right.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_FIRE )
	{
		/********************* Fire **************************/
		// standing
		images.push_back( GetImage( "maryo/fire/left.png" ) );
		images.push_back( GetImage( "maryo/fire/right.png" ) );
		// running
		images.push_back( GetImage( "maryo/fire/run_left_1.png" ) );
		images.push_back( GetImage( "maryo/fire/run_right_1.png" ) );
		images.push_back( GetImage( "maryo/fire/run_left_2.png" ) );
		images.push_back( GetImage( "maryo/fire/run_right_2.png" ) );
		// falling
		images.push_back( GetImage( "maryo/fire/fall_left.png" ) );
		images.push_back( GetImage( "maryo/fire/fall_right.png" ) );
		// jumping
		images.push_back( GetImage( "maryo/fire/jump_left.png" ) );
		images.push_back( GetImage( "maryo/fire/fall_right.png" ) );
		// dead
		images.push_back( GetImage( "maryo/small/dead_left.png" ) );
		images.push_back( GetImage( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( GetImage( "maryo/fire/duck_left.png" ) );
		images.push_back( GetImage( "maryo/fire/duck_right.png" ) );
		// climbing
		images.push_back( GetImage( "maryo/fire/climb_left.png" ) );
		images.push_back( GetImage( "maryo/fire/climb_right.png" ) );
		/****************************************************/
	}

	curr_img = -1;
	SetImage( MARYO_IMG_STAND + DIR_RIGHT, 1 ); // standing right
}

void cPlayer :: DownGrade( bool handle /* = 0 */ )
{
	if( godmode || invincible )
	{
		return;
	}

	if( !handle )
	{
		update_downgrade = 1;
		return;
	}

	update_downgrade = 0;

	if( maryo_type != MARYO_SMALL )
	{
		ChangeSize( 0, 20 );
		maryo_type = MARYO_SMALL;
		LoadImages();

		if( !( posy - cameraposy > ( GAME_RES_H - col_rect.h ) ) ) // if not dead because of falling down
		{
			pAudio->PlaySound( SOUNDS_DIR "/powerdown.ogg", RID_MARYO_POWERDOWN );

			invincible = DESIRED_FPS * 2.5;
			invincible_mod = 0;

			Itembox->Request_Item();
			
			return;
		}
	}

	maryo_type = MARYO_DEAD;
	pointsdisplay->Clear();

	pAudio->FadeOutMusic( 1700 );
	pAudio->PlaySound( SOUNDS_DIR "/dead.ogg", RID_MARYO_DEATH );

	Fireball_clear();

	if( livedisplay )
	{
		livedisplay->AddLives( -1 );
	}

	if( debugdisplay )
	{
		debugdisplay->text = "You Died";
		debugdisplay->counter = DESIRED_FPS;
		debugdisplay->Update();
	}
	
	// Maryo's dying animation

	pLevel->Draw( LVL_DRAW ); // no Level updates
	Draw_Player( MARYO_IMG_DEAD + 1 );
	SDL_GL_SwapBuffers();
	SDL_Delay( 500 );
	Framerate.Reset();
	
	double i;

	for( i = 0; i < 7; i += Framerate.speedfactor )
	{
		Move( 0, -13, 0, 1 );
		pLevel->Draw( LVL_DRAW );
		Draw_Player( MARYO_IMG_DEAD );
		SDL_GL_SwapBuffers();
		Framerate.Update();
	}

	SDL_Delay( 300 );

	Framerate.Reset();
	walk_count = 0;
	
	for( i = 0; posy < GAME_RES_H + cameraposy + col_rect.h; i++ )
	{
		walk_count += Framerate.speedfactor;

		if( walk_count > 4 )
		{
			walk_count = 0;
		}

		Move( 0, 14, 0, 1 );
		pLevel->Draw( LVL_DRAW );	// no Level updates

		if( walk_count > 2 )
		{
			Draw_Player( MARYO_IMG_DEAD );
		}
		else
		{
			Draw_Player( MARYO_IMG_DEAD + 1 );
		}

		SDL_GL_SwapBuffers();
		
		Framerate.Update();
	}

	if( lives >= 0 )
	{
		pLevel->Unload();
		Reset();
		DrawEffect();
	}
	else
	{
		if( debugdisplay )
		{
			debugdisplay->text = "Game Over";
			debugdisplay->counter = DESIRED_FPS;
			debugdisplay->Update();
		}
		
		pLevel->Draw( LVL_DRAW );	// no Level updates
		Draw_Player( MARYO_IMG_DEAD + 1 );
		SDL_GL_SwapBuffers();

		DrawEffect();
		ResetSave();
	}
	
	Framerate.Reset();
	ClearEvents();
	maryo_type = MARYO_SMALL;
}

void cPlayer :: Get_Item( unsigned int item_type, bool force )
{
	if( item_type == TYPE_MUSHROOM_DEFAULT ) 
	{
		if( ( maryo_type == MARYO_SMALL || force ) && ChangeSize( -5, -20, 1 ) ) 
		{
			// correct position
			if( maryo_type == MARYO_SMALL ) 
			{
				ChangeSize( -5, -20 );
			}

			// draw animation if not already big
			if( maryo_type != MARYO_BIG )
			{
				pAudio->PlaySound( SOUNDS_DIR "/mushroom.ogg", RID_MUSHROOM );
				Draw_Animation( item_type );
			}

			if( maryo_type == MARYO_SMALL ) // load new images
			{
				maryo_type = MARYO_BIG;
				LoadImages();
			}
			else if( maryo_type == MARYO_BIG ) // move to itembox
			{
				Itembox->Set_Item( TYPE_MUSHROOM_DEFAULT );
			}
			else if( maryo_type == MARYO_FIRE ) // change to big
			{
				maryo_type = MARYO_BIG;
				Itembox->Set_Item( TYPE_FIREPLANT );
				LoadImages();
			}
		}
		else
		{
			Itembox->Set_Item( TYPE_MUSHROOM_DEFAULT );
		}
	}
	else if( item_type == TYPE_FIREPLANT ) 
	{
		if( ( ( maryo_type == MARYO_SMALL || force ) && !ColCheckAuto( -5, - 20 ) ) || 
			( ( maryo_type == MARYO_BIG || force ) ) )
		{
			if( maryo_type == MARYO_SMALL ) 
			{
				ChangeSize( -5, -20 );
			}
			else if( maryo_type == MARYO_FIRE ) 
			{
				Itembox->Set_Item( TYPE_FIREPLANT );
			}
			
			if( maryo_type != MARYO_FIRE )
			{
				pAudio->PlaySound( SOUNDS_DIR "/fireplant.ogg", RID_MUSHROOM );
				Draw_Animation( item_type );
			}

			maryo_type = MARYO_FIRE;
			LoadImages();
		}
		else if( Itembox->item_id == TYPE_FIREPLANT )
		{
			Fireball_add( FIREBALL_EXPLOSION );
			pointsdisplay->AddPoints( 1000, posx + ( col_rect.w / 2 ), posy + 2 );	
		}
		else
		{
			Itembox->Set_Item( TYPE_FIREPLANT );
		}
	}
	else if( item_type == TYPE_MUSHROOM_LIVE_1 ) 
	{
		pAudio->PlaySound( SOUNDS_DIR "/live_up.ogg", RID_1UP_MUSHROOM );
		livedisplay->AddLives( 1 );
	}
	else if( item_type == TYPE_MOON ) 
	{
		pAudio->PlaySound( SOUNDS_DIR "/live_up.ogg", RID_1UP_MUSHROOM );
		livedisplay->AddLives( 3 );
	}
	else if( item_type == TYPE_JSTAR ) 
	{
		pAudio->PlaySound( SOUNDS_DIR "/live_up.ogg" );
		pointsdisplay->AddPoints( 1000, posx + ( col_rect.w / 2 ), posy + 2 );
		invincible = DESIRED_FPS * 10;
		invincible_star = DESIRED_FPS * 8.5;
	}
}

void cPlayer :: Fireball_add( FireballEffect firetype )
{
	if( maryo_type != MARYO_FIRE || ducked )
	{
		return;
	}

	if( firetype == FIREBALL_DEFAULT )
	{
		if( fireballs.size() < 2 )
		{
			cFireball *temp = new cFireball( (direction == DIR_RIGHT) ? (posx + 20) : (posx + 8), posy + 45, (direction == DIR_RIGHT) ? (17) : (-17) );
			fireballs.push_back( temp );

			pAudio->PlaySound( SOUNDS_DIR "/fireball.ogg", RID_MARYO_FIREBALL );
		}
	}
	else if( firetype == FIREBALL_EXPLOSION )
	{
		if( fireballs.size() < 10 )
		{
			double fire_angle = -180;

			for( unsigned int i = 0; i < 10; i++ )
			{
				fire_angle += 18; // 10 * 18 = 180 degrees

				cFireball *temp = new cFireball( posx + col_rect.w / 2, posy + col_rect.h / 2, 0 );
				temp->SetDirection( fire_angle, 15 );
				temp->Move( temp->velx * 6, temp->vely + 6, 1 );

				fireballs.push_back( temp );
			}

			cAnimation *anim = new cAnimation_2( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 3 ), 10 );
			anim->SetFadingSpeed( 0.3 );
			pAnimationManager->Add( anim );

			pAudio->PlaySound( SOUNDS_DIR "/fireball_2.ogg", RID_MARYO_FIREBALL );
		}
	}
}

void cPlayer :: Fireball_clear( void )
{
	for( unsigned int i = 0; i < fireballs.size(); i++ )
	{
		delete fireballs[i];
	}

	fireballs.clear();
}