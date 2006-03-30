/***************************************************************************
           turtle.cpp  -  turtle enemy class
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cTurtle :: cTurtle( double x, double y, unsigned int color ) : cEnemy( x, y )
{
	type = TYPE_TURTLE;

	velx = 4;
	direction = DIR_RIGHT;
	state = WALK;
	turtle_state = TURTLE_WALK;	// Walking

	playercounter = 0;

	speed = 15;

	/*	Turtle Colors
	 *	
	 *	1 = Red
	 *	2 = Green
	 *	3 = Yellow
	 *	4 = Blue
	 */
	if( color == 1 ) // Red
	{
		// Turtle
		images.push_back( GetImage( "enemy/turtle/red/walk_1.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/walk_2.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/turn_1.png" ) );
		// Shell
		images.push_back( GetImage( "enemy/turtle/red/shell_front.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/shell_move_1.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/shell_move_2.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/shell_move_3.png" ) );
		images.push_back( GetImage( "enemy/turtle/red/shell_active.png" ) );
	}
	else // unsupported/invalid color
	{
		dead = 1;
		visible = 0;

		printf( "Error : Invalid Turtle color : %d\n", color );
		return;
	}
	
	SetImage( 0, 1 ); // Turtle walking first
}

cTurtle :: ~cTurtle( void )
{
	//
}

void cTurtle :: DownGrade( bool force /* = 0 */ )
{
	dead = 1;
	massivetype = MASS_PASSIVE;
	SetImage( 3 );
	counter = 0;

	SetRotationZ( 180 );

	if( !force )
	{
		cAnimation_3 *anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 4, 0.8 );
		anim->SetScale( 0.6 );
		anim->SetDirectionRange( 180, 180 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );
	}
}

void cTurtle :: DieStep( void )
{
	counter += Framerate.speedfactor;

	if( counter < 5 ) // a little bit upwards first
	{
		Move( 0, -5, 0, 1 );
	}
	else if( posy < GAME_RES_H + col_rect.h ) // if not below the screen fall
	{
		Move( 0, 20, 0, 1 );
	}
	else // if below disable
	{
		visible = 0;
		rotz = 0;
		state = STAY;
		turtle_state = TURTLE_DEAD;
	}

	Draw();
}

void cTurtle :: Update( void )
{
	if( dead )
	{
		if( visible )
		{
			DieStep();
		}

		return;
	}

	if( !isPlayer_range() )
	{
		return;
	}

	if( turtle_state == TURTLE_WALK ) // walking
	{
		if( velx >= 0 )
		{
			velx = 4;
		}
		else
		{
			velx = -4;
		}

		walk_count += Framerate.speedfactor * 0.2;
		
		if( walk_count > 2 )
		{
			walk_count = 0;

			// if turn around image force the new
			if( curr_img == 2 )
			{
				SetImage( (int)walk_count );
			}
		}
		
		// if turn around image
		if( curr_img != 2 )
		{
			SetImage( (int)walk_count );
		}
		else
		{
			// rotate the turn image
			if( walk_count > 1 )
			{
				if( velx < 0 )
				{
					roty = 180;
				}
				else
				{
					roty = 0;
				}
			}
		}
	}
	else if( turtle_state == TURTLE_STAND ) // standing shell
	{
		counter += Framerate.speedfactor;
		
		if( counter < 160 ) // waiting
		{
			SetImage( 3 ); // front
		}
		else
		{
			if( counter < 192 ) // animation
			{
				if( (int)counter % 5 == 1 )
				{
					SetImage( 7 ); // active
				}
				else
				{
					SetImage( 3 ); // front
				}
			}
			else // activate
			{
				counter = 0;
				turtle_state = TURTLE_WALK;
				state = WALK;

				pAudio->PlaySound( SOUNDS_DIR "/turtle_up.ogg" );

				if( curr_img != 2 )
				{
					Move( 0, -( images[0]->h - images[3]->h ) - 1, 1, 1 );
				}

				SetImage( 3 ); // turtle
			}
		}
		
		
		if( velx != 0 )
		{
			AddVel( -velx * 0.2, 0 );

			if( velx < 0.3 && velx > -0.3 ) 
			{
				velx = 0;
			}
		}
	}
	else if( turtle_state == TURTLE_SHELL ) // moving shell
	{
		if( velx != speed && velx != -speed )
		{
			if( ( pPlayer->col_rect.w / 2 ) + pPlayer->posx < ( col_rect.w / 2 ) + posx )
			{
				velx = speed;
			}
			else 
			{
				velx = -speed;
			}
		}

		walk_count += Framerate.speedfactor * 0.4;

		if( playercounter > 0 )
		{
			playercounter -= Framerate.speedfactor;
			
			if( playercounter <= 0 )
			{
				// do not start collision detection if in maryo
				if( Col_Box( &pPlayer->col_rect, &col_rect ) )
				{
					playercounter = 5;
				}
				else
				{
					playercounter = 0;
				}
			}
		}

		if( walk_count > 3 )
		{
			walk_count = 0;
		}

		SetImage( 4 + (int)walk_count );
	}

	if( !groundObject && vely < 25 )
	{
		AddVel( 0, 1.5 + vely * 0.1 );
	}
	else if( groundObject && vely != 0 )
	{
		vely = 0;
	}

	CollideMove();

	// only auto set rotation if not turn image
	if( curr_img != 2 )
	{
		Update_Rotation_velx();
	}


	Draw();
}

void cTurtle :: HandleCollision( cObjectCollision *collision )
{
	if( turtle_state == TURTLE_WALK )
	{
		if( collision->type != CO_PLAYER && collision->direction != DIR_DOWN )
		{
			if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
			{
				SetImage( 2 );

				Turn_Around();
				walk_count = 0;
				Update_Rotation_velx();
			}
		}
		else if( collision->type == CO_PLAYER && ( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT ) )
		{
			PlayerCollision( collision->direction );
		}
	}
	else if( turtle_state == TURTLE_STAND )
	{
		//
	}
	else if( turtle_state == TURTLE_SHELL )
	{
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			if( collision->type > CO_NOTHING && collision->type != CO_ENEMY && collision->type != CO_PLAYER )
			{
				Turn_Around();

				// animation
				cAnimation_3 *anim = NULL;
				if( collision->direction == DIR_RIGHT )
				{
					anim = new cAnimation_3( posx + col_rect.w + 10, posy + ( col_rect.h / 1.35 ), 5 );
					anim->SetDirectionRange( 140, 100 );
				}
				else
				{
					anim = new cAnimation_3( posx - 15, posy + ( col_rect.h / 1.35 ), 5 );
					anim->SetDirectionRange( 320, 100 );
				}
				anim->SetSpeed( 3 );
				anim->SetScale( 0.5 );
				pAnimationManager->Add( anim );
			}
		}
		
		if( collision->type == CO_ACTIVE && ( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT ) ) // Active Object Collision
		{
			pObjManager->items[collision->number]->EnemyCollision( collision->direction );
		}
		else if( collision->type == CO_ENEMY ) // killed an enemy
		{
			if( pObjManager->items[collision->number]->type == TYPE_GOOMBA )
			{
				pAudio->PlaySound( SOUNDS_DIR "/goomba_hit.ogg" );
			}
			else
			{
				pAudio->PlaySound( SOUNDS_DIR "/shell_smash_default.ogg" );
			}
			
			pointsdisplay->AddPoints( 35, posx + image->w / 3, posy - 5 );
			pObjManager->items[collision->number]->DownGrade( 1 );

			Move( velx, vely );
		}
		else if( collision->type == CO_PLAYER ) // Player
		{
			if( playercounter == 0 ) // collision
			{
				PlayerCollision( collision->direction );
				
				if( pObjManager->Get_Size_array( ARRAY_ENEMY ) ) 
				{
					if( collision->direction == DIR_LEFT ) 
					{
						velx = speed;
					}
					else if( collision->direction == DIR_RIGHT ) 
					{
						velx = -speed;
					}
				}
			}
		}
	}
}

void cTurtle :: PlayerCollision( ObjectDirection cdirection )
{
	if( ( turtle_state == TURTLE_SHELL && playercounter ) || dead ) 
	{
		return;
	}

	if( cdirection == DIR_DOWN )
	{
		if( turtle_state == TURTLE_WALK )
		{
			pointsdisplay->AddPoints( 25, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( SOUNDS_DIR "/koopa_hit.ogg" );
		}
		else if( turtle_state == TURTLE_STAND )
		{
			pointsdisplay->AddPoints( 10, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( SOUNDS_DIR "/shell_hit.ogg" );
		}
		else if( turtle_state == TURTLE_SHELL )
		{
			pointsdisplay->AddPoints( 5, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( SOUNDS_DIR "/shell_hit.ogg" );
		}
		else
		{
			printf( "Turtle state error : %d\n", state );
		}

		cAnimation_3 *anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + 5, 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 3, 0.8 );
		anim->SetScale( 0.4 );
		anim->SetDirectionRange( 180, 180 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );

		if( turtle_state == TURTLE_WALK )  // normal walking
		{
			counter = 0;
			state = STAY;
			turtle_state = TURTLE_STAND;

			Move( 0, ( images[0]->h - images[3]->h ) - 5, 1, 1 );
		}
		else if( turtle_state == TURTLE_STAND ) // to running
		{
			counter = 0;
			state = RUN;
			turtle_state = TURTLE_SHELL;
			player_range = 10000;
		}
		else if( turtle_state == TURTLE_SHELL ) // to standing
		{
			counter = 0;
			state = STAY;
			turtle_state = TURTLE_STAND;
			player_range = 2500;
		}

		massivetype = MASS_MASSIVE;
		pPlayer->start_enemyjump = 1;
		
	}
	else if( turtle_state == TURTLE_STAND )
	{
		if( cdirection == DIR_LEFT || cdirection == DIR_RIGHT || cdirection == DIR_UP )
		{
			pAudio->PlaySound( SOUNDS_DIR "/shell_hit.ogg" );
			
			cAnimation_3 *anim = NULL;
			if( cdirection == DIR_RIGHT )
			{
				anim = new cAnimation_3( posx + 5, posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
				anim->SetDirectionRange( 90, 180 );
			}

			else if( cdirection == DIR_LEFT )
			{
				anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
				anim->SetDirectionRange( 270, 180 );
			}
			else
			{
				anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + 5, 4, GetImage( "animation/particles/light.png" ) );
				anim->SetDirectionRange( 180, 180 );
			}

			anim->SetSpeed( 4, 0.5 );
			anim->SetScale( 0.6 );
			anim->SetFading( FADE_SIZE );
			anim->SetColor( Get_Color( 254, 200, 100 ) );
			pAnimationManager->Add( anim );

			playercounter = DESIRED_FPS * 0.13;

			state = RUN;
			turtle_state = TURTLE_SHELL;
			player_range = 10000;
		}
		else if( cdirection == DIR_DOWN )
		{
			DownGrade();
		}
	} 
	else if( cdirection == DIR_LEFT || cdirection == DIR_RIGHT || cdirection == DIR_DOWN )
	{
		pPlayer->DownGrade();
	}
}

void cTurtle :: EnemyCollision( ObjectDirection cdirection )
{
	if( ( cdirection == DIR_LEFT && velx > 0 && state != RUN ) || ( cdirection == DIR_RIGHT && velx < 0 && state != RUN ) )
	{
		Turn_Around();
	}
}
