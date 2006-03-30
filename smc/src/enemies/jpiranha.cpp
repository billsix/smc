/***************************************************************************
              jpiranha.cpp  -  jprinha,jumping piranha plant
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
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

cjPiranha :: cjPiranha( double x, double y, ObjectDirection dir /* = DIR_UP */, unsigned int nmax_distance /* = 200 */ ) : cEnemy( x, y )
{
	type = TYPE_JPIRANHA;
	player_range = 6000;

	direction = dir;

	images.push_back( GetImage( "enemy/jpiranha/c1.png" ) );
	images.push_back( GetImage( "enemy/jpiranha/c2.png" ) );
	images.push_back( GetImage( "enemy/jpiranha/o1.png" ) );
	images.push_back( GetImage( "enemy/jpiranha/o2.png" ) );

	if( direction == DIR_UP )
	{
		vely = -5.8;
	}
	else if( direction == DIR_DOWN )
	{
		vely = 5.8;
		rotz = 180;
	}
	else if( direction == DIR_LEFT )
	{
		velx = -5.8;
		rotz = 270;
	}
	else if( direction == DIR_RIGHT )
	{
		velx = 5.8;
		rotz = 90;
	}
	else
	{
		dead = 1;
		visible = 0;

		printf( "jPiranha direction Error : %d\n", direction );
		return;
	}

	walk_count = rand() % 8;
	state = FLY;
	max_distance = nmax_distance;
	wait_time = rand() % 90;

	SetImage( 0 );
}

cjPiranha :: ~cjPiranha( void )
{
	//
}

void cjPiranha :: DownGrade( bool force /* = 0 */ )
{
	dead = 1;
	massivetype = MASS_PASSIVE;
	counter = 0;

	if( !force )
	{
		cAnimation_3 *anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 5, 0.6 );
		anim->SetScale( 0.8 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );
	}
	else
	{
		SetRotationZ( 180 );
	}
}

void cjPiranha :: DieStep( void )
{
	counter += Framerate.speedfactor;

	if( rotz != 180 ) // default death
	{
		visible = 0;
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

void cjPiranha :: Update( void )
{
	if( dead )
	{
		if( visible )
		{
			DieStep();
		}

		return;
	}

	// update range
	if( !isPlayer_range() )
	{
		return;
	}

	if( direction == DIR_DOWN )
	{
		startpos = -startposy;
		pos = -posy;
		vel = -vely;
	}
	else if( direction == DIR_RIGHT )
	{
		startpos = -startposx;
		pos = -posx;
		vel = -velx;
	}
	else if( direction == DIR_LEFT )
	{
		startpos = startposx;
		pos = posx;
		vel = velx;
	}
	else // up
	{
		startpos = startposy;
		pos = posy;
		vel = vely;
	}

	if( startpos - pos > ( max_distance / 3 ) && startpos - pos < max_distance && vel < -0.5 )
	{
		vel += ( -vel * 0.04 ) * Framerate.speedfactor;
	}
	else if( startpos - pos > ( max_distance - 10 ) && !wait_time )
	{
		vel = 0.5;
	}
	else if( startpos - pos > ( max_distance / 2 ) && !wait_time )
	{
		vel += 0.025 * Framerate.speedfactor;
	}
	else if( startpos - pos < 0 && vel > 0.3 )
	{
		wait_time = DESIRED_FPS * 3;
		vel = 0;
	}
	else if( wait_time > 0 )
	{
		wait_time -= Framerate.speedfactor;

		if( wait_time < 0 )
		{
			wait_time = 0;
		}
	}
	else if( startpos - pos < 0 )
	{
		GL_rect rect1 = col_rect;

		if( direction == DIR_UP )
		{
			rect1.y -= 40;
			rect1.h += 40;
		}
		else if( direction == DIR_DOWN )
		{
			rect1.y += 40;
			rect1.h -= 40;
		}
		else if( direction == DIR_RIGHT )
		{
			rect1.x += 35;
		}
		else if( direction == DIR_LEFT )
		{
			rect1.x -= 35;
		}

		if( Col_Box( &pPlayer->col_rect, &rect1 ) )
		{
			wait_time = DESIRED_FPS * 2;
		}
		else
		{
			vel = -7;
		}
	}

	// set the position from the internal calculations
	if( direction == DIR_UP )
	{
		startposy = startpos;
		posy = pos;
		vely = vel;
	}
	else if( direction == DIR_DOWN )
	{
		startposy = -startpos;
		posy = -pos;
		vely = -vel;
	}
	else if( direction == DIR_RIGHT )
	{
		startposx = -startpos;
		posx = -pos;
		velx = -vel;
	}
	else if( direction == DIR_LEFT )
	{
		startposx = startpos;
		posx = pos;
		velx = vel;
	}

	walk_count += 0.23 * Framerate.speedfactor;
	
	if( walk_count > 3.99 )
	{
		walk_count = 0;
	}
	
	SetImage( (int)walk_count );

	CollideMove();

	Draw();
}

void cjPiranha :: HandleCollision( cObjectCollision *collision )
{
	if( collision->type == CO_PLAYER && massivetype && visible )
	{
		PlayerCollision( collision->direction );
	}
}

void cjPiranha :: PlayerCollision( ObjectDirection cdirection )
{
	if( !dead && cdirection != DIR_NOTHING )
	{
		if( pPlayer->maryo_type != MARYO_SMALL && cdirection != DIR_UP )
		{
			pAudio->PlaySound( SOUNDS_DIR "/maryo_au.ogg" );
			pPlayer->start_enemyjump = 1;
		}

		pPlayer->DownGrade();
	}
}
