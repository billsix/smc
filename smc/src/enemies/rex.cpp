/***************************************************************************
           rex.cpp  -  the little dinosaur
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cRex :: cRex( double x, double y ) : cEnemy( x, y )
{
	type = TYPE_REX;

	velx = 2.5;
	direction = DIR_RIGHT;

	state = WALK;
	rex_state = REX_WALK;

	speed = 6;

	images.push_back( GetImage( "enemy/rex/l1.png" ) );		// Walking left 1
	images.push_back( GetImage( "enemy/rex/l2.png" ) );		// Walking left 2
	images.push_back( GetImage( "enemy/rex/l3.png" ) );		// small Walking left 1
	images.push_back( GetImage( "enemy/rex/l4.png" ) );		// small Walking left 2
	images.push_back( GetImage( "enemy/rex/ld.png" ) );		// dead left
	images.push_back( GetImage( "enemy/rex/r1.png" ) );		// Walking right 1
	images.push_back( GetImage( "enemy/rex/r2.png" ) );		// Walking right 2
	images.push_back( GetImage( "enemy/rex/r3.png" ) );		// small Walking right 1
	images.push_back( GetImage( "enemy/rex/r4.png" ) );		// small Walking right 2
	images.push_back( GetImage( "enemy/rex/rd.png" ) );		// dead right
	
	SetImage( 5 ); // walking right 1
}

cRex :: ~cRex( void )
{
	//
}

void cRex :: DownGrade( bool force /* = 0 */ )
{
	massivetype = MASS_PASSIVE;
	dead = 1;
	state = STAY;
	counter = 0;

	if( !force ) // default stomp death
	{
		if( rex_state == REX_WALK ) 
		{
			Move( 0, images[1]->h - images[4]->h, 1, 1 );
		}
		else if( rex_state == REX_MINI )
		{
			Move( 0, images[3]->h - images[4]->h, 1, 1 );
		}

		if( velx > 0 )
		{
			SetImage( 9 );
		}
		else
		{
			SetImage( 4 );
		}
	}
	else // falling death
	{
		SetRotationZ( 180 );
	}
	
	rex_state = REX_DEAD;
}

void cRex :: DieStep( void )
{
	counter += Framerate.speedfactor;

	if( rotz != 180 ) // stomp death
	{
		if( counter > DESIRED_FPS * 2 )
		{
			visible = 0;
		}
	}
	else // falling death
	{
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
			rotz = 0;
			visible = 0;
		}
	}

	Draw();
}

void cRex :: Update( void )
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

	counter += Framerate.speedfactor;
	
	if( rex_state == REX_WALK )
	{
		if( counter > (double)( DESIRED_FPS / 7 * 2 ) )
		{
			counter = 0;
		}
	}
	else
	{
		if( counter > (double)( DESIRED_FPS / 10 * 2 ) )
		{
			counter = 0;
		}
	}


	if( velx < 0 )
	{
		if( rex_state == REX_WALK )
		{
			if( counter < (double)( DESIRED_FPS / 7 ) )
			{
				SetImage( 0 ); // walking left 1
			}
			else
			{
				SetImage( 1 ); // walking left 2
			}
		}
		else
		{
			if( counter < (double)( DESIRED_FPS / 10 ) )
			{
				SetImage( 2 ); // small walking left 1
			}
			else
			{
				SetImage( 3 ); // small walking left 2
			}
		}
	}
	else
	{
		if( rex_state == REX_WALK )
		{
			if( counter < (double)( DESIRED_FPS / 7 ) )
			{
				SetImage( 5 ); // walking right 1
			}
			else 
			{
				SetImage( 6 ); // walking right 2
			}
		}
		else
		{
			if( counter < (double)( DESIRED_FPS / 10 ) )
			{
				SetImage( 7 ); // small walking right 1
			}
			else
			{
				SetImage( 8 ); // small walking right 2
			}
		}
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

	Draw();
}

void cRex :: HandleCollision( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		Turn_Around();
	}
	
	if( collision->type == CO_PLAYER )
	{
		PlayerCollision( collision->direction );
	}
}

void cRex :: PlayerCollision( ObjectDirection cdirection )
{
	if( dead )
	{
		return;
	}

	if( cdirection == DIR_DOWN )
	{
		if( rex_state == REX_WALK )
		{
			pointsdisplay->AddPoints( 20, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( SOUNDS_DIR "/stomp_5.ogg" );
		}
		else if( rex_state == REX_MINI )
		{
			pointsdisplay->AddPoints( 40, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( SOUNDS_DIR "/stomp_1.ogg" );
		}
		
		cAnimation_3 *anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 3, 0.8 );
		anim->SetScale( 0.4 );
		anim->SetDirectionRange( 180, 180 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );

		if( rex_state == REX_WALK )
		{
			counter = 0;
			rex_state = REX_MINI; // small walking
			state = RUN;

			if( velx < 0 ) 
			{
				velx = -speed;
				Move( 0, images[1]->h - images[2]->h - 1, 1, 1 );
			}
			else
			{
				velx = speed;
				Move( 0, images[6]->h - images[7]->h - 1, 1, 1 );
			}
		}
		else if( rex_state == REX_MINI )
		{
			DownGrade();
		}

		pPlayer->start_enemyjump = 1;
	}
	else if( cdirection == DIR_LEFT || cdirection == DIR_RIGHT || cdirection == DIR_DOWN )
	{
		pPlayer->DownGrade();
	}
}

void cRex :: EnemyCollision( ObjectDirection cdirection )
{
	if( ( cdirection == DIR_LEFT && velx > 0 ) || ( cdirection == DIR_RIGHT && velx < 0 ) ) 
	{
		Turn_Around();
	}
}
