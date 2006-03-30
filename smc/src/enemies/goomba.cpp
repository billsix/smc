/***************************************************************************
              goomba.cpp  -  goomba, little moving around enemy
                             -------------------
    copyright            :	(C) 2003-2006 by FluXy
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

cGoomba :: cGoomba( double x, double y, int col /* = 0 */ ) : cEnemy( x, y )
{
	color = col;
	type = TYPE_GOOMBA;

	if( color == COL_BROWN )
	{
		images.push_back( GetImage( "enemy/goomba/brown/walk_1.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_2.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_3.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_4.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_5.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_6.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_7.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_8.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_9.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/walk_10.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/dead_1.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/dead_2.png" ) );
		images.push_back( GetImage( "enemy/goomba/brown/dead_3.png" ) );
		velx = 2.7;
	}
	else if( color == COL_RED )
	{
		images.push_back( GetImage( "enemy/goomba/blue/walk_1.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_2.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_3.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_4.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_5.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_6.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_7.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_8.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_9.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/walk_10.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/dead_1.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/dead_2.png" ) );
		images.push_back( GetImage( "enemy/goomba/blue/dead_3.png" ) );
		velx = 4.5;
	}
	else
	{
		velx = 0;
		dead = 1;
		visible = 0;

		printf( "Goomba Color Error : %d\n", color );
		return;
	}

	Update_Direction();
	start_direction = direction;
	SetImage( 0 );
}

cGoomba :: ~cGoomba( void )
{
	//
}

void cGoomba :: DownGrade( bool force /* = 0 */ )
{
	dead = 1;
	massivetype = MASS_PASSIVE;
	counter = 0;

	if( !force ) // default stomp death
	{
		SetImage( 10 );
		Move( 0, images[1]->h - images[10]->h, 1, 1 );

		cAnimation_3 *anim = new cAnimation_3( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 3, 0.6 );
		anim->SetScale( 0.5 );
		anim->SetDirectionRange( 180, 180 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );
	}
	else // falling death
	{
		SetRotationZ( 180 );
	}
}

void cGoomba :: DieStep( void )
{
	counter += Framerate.speedfactor;

	if( rotz != 180 ) // stomp death
	{
		if( counter < DESIRED_FPS / 6 )
		{
			if( curr_img != 11 )
			{
				SetImage( 11 );
				Move( 0, images[10]->h - images[11]->h, 1, 1 );
			}
		}
		else if( counter < DESIRED_FPS / 5 )
		{
			if( curr_img != 12 )
			{
				SetImage( 12 );
				Move( 0, images[11]->h - images[12]->h, 1, 1 );
			}
		}
		else if( counter > DESIRED_FPS )
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

void cGoomba :: Update( void )
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

	walk_count += Framerate.speedfactor;
	
	if( walk_count > 10 )
	{
		walk_count = 0;
	}
	
	SetImage( (int)walk_count );

	if( !groundObject && vely < 25 )
	{
		AddVel( 0,2 + ( vely * 0.05 ) );
	}
	else if( groundObject && vely > 0 )
	{
		vely = 0;
	}
	
	CollideMove();

	if( velx < 0 )
	{
		roty = 0;
	}
	else
	{
		roty = 180;
	}

	Draw();
}

void cGoomba :: HandleCollision( cObjectCollision *collision )
{
	if( collision->type != CO_NOTHING && collision->type != CO_PLAYER )
	{
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			Turn_Around();
		}
	}

	if( collision->type == CO_PLAYER && massivetype )
	{
		PlayerCollision( collision->direction );

		if( collision->direction != DIR_NOTHING ) 
		{
			Turn_Around();
		}
	}
}

void cGoomba :: PlayerCollision( ObjectDirection cdirection )
{
	if( cdirection == DIR_DOWN )
	{
		pAudio->PlaySound( SOUNDS_DIR "/goomba_hit.ogg" );

		DownGrade();
		pPlayer->start_enemyjump = 1;

		if( color == COL_BROWN )
		{
			pointsdisplay->AddPoints( 10, pPlayer->posx, pPlayer->posy );
		}
		else if( color == COL_RED )
		{
			pointsdisplay->AddPoints( 50, pPlayer->posx, pPlayer->posy );
		}
	}
	else if( !dead && ( cdirection == DIR_LEFT || cdirection == DIR_RIGHT || cdirection == DIR_UP ) )
	{
		pPlayer->DownGrade();
	}
}

void cGoomba :: EnemyCollision( ObjectDirection cdirection )
{
	if( ( cdirection == DIR_LEFT && velx > 0 ) || ( cdirection == DIR_RIGHT && velx < 0 ) )
	{
		Turn_Around();
	}
}

