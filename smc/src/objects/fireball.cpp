/***************************************************************************
           fireball.cpp  -  fireball class
                             -------------------
    copyright            :	(C) 2006 by FluXy
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

cFireball :: cFireball( double x, double y, double velocity_x, double velocity_y ) : cSprite( NULL, x, y )
{
	array = ARRAY_ACTIVE;
	type = TYPE_FIREBALL;

	player_range = 2000;

	massivetype = MASS_MASSIVE;
	counter = 0;

	velx = velocity_x;
	vely = velocity_y;

	images.push_back( GetImage( "animation/fireball/1.png" ) );
	SetImage( images[0] );
}

cFireball :: ~cFireball( void )
{
	//
}

void cFireball :: SetDirection( double angle, double speed )
{
	velx = (double)cos( angle * 0.017453292519943295 ) * speed;
	vely = (double)sin( angle * 0.017453292519943295 ) * speed;
}

void cFireball :: Update( void )
{
	rotz += Framerate.speedfactor * 40;

	if( posx <= 10 )
	{
		destroy = 1;
	}
	
	if( vely < 30 )
	{
		AddVel( 0, 1.0 );
	}

	// collision handling
	CollideMove();

	if( !isPlayer_range() )	// If Fireball is out of Range
	{
		destroy = 1;
	}
}

void cFireball :: HandleCollision( cObjectCollision *collision )
{
	if( collision->type > CO_NOTHING && collision->type != CO_PLAYER ) // no Player
	{
		if( collision->type == CO_ENEMY ) // Enemy
		{
			if( pObjManager->items[collision->number]->type != TYPE_BANZAI_BILL )
			{
				pointsdisplay->AddPoints( 30, posx, posy );

				cSprite *tmp = NULL;
				
				if( pObjManager->items[collision->number]->ColCheckAuto( 0, 0 ) == CO_NOTHING )
				{
					tmp = new cGoldPiece( pObjManager->items[collision->number]->posx, pObjManager->items[collision->number]->posy );
				}
				else
				{
					tmp = new cGoldPiece( pObjManager->items[collision->number]->posx + pObjManager->items[collision->number]->col_rect.w / 2, pObjManager->items[collision->number]->posy + pObjManager->items[collision->number]->col_rect.h / 2 );
				}

				tmp->spawned = 1;
				pObjManager->Add( tmp );

				pObjManager->items[collision->number]->visible = 0;
				pObjManager->items[collision->number]->DownGrade();
				
				destroy = 1;
			}
			else
			{
				pAudio->PlaySound( SOUNDS_DIR "/tock.ogg" );
				destroy = 1;
			}
		}
		else if( collision->type == CO_MASSIVE || collision->type == CO_ACTIVE )
		{
			if( collision->direction == DIR_DOWN )
			{
				if( velx < 0.1 && velx > -0.1 ) // if directly hitting the ground
				{
					destroy = 1;
				}

				vely = -10;
				cAnimation_2 *anim = new cAnimation_2( posx + col_rect.w / 2, posy + col_rect.h / 2 );
				anim->SetFadingSpeed( 3 );
				pAnimationManager->Add( anim );
			}
			else
			{
				destroy = 1;
			}
		}
		else
		{
			destroy = 1;
		}		
	}
}