/***************************************************************************
              banzai_bill.cpp  -  banzai bill, a giant, slow-moving bullet
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

cbanzai_bill :: cbanzai_bill( double x, double y, ObjectDirection fdirection ) : cEnemy( x, y )
{
	type = TYPE_BANZAI_BILL;
	massivetype = MASS_PASSIVE;
	visible = 0;

	direction = fdirection;
	state = FLY;

	player_range = 800;

	if( direction == DIR_LEFT )
	{
		images.push_back( GetImage( "enemy/banzai_bill/l.png" ) );
	}
	else
	{
		images.push_back( GetImage( "enemy/banzai_bill/r.png" ) );
	}
	
	SetImage( 0 );
}

cbanzai_bill :: ~cbanzai_bill( void )
{
	//
}

void cbanzai_bill :: Activate( void )
{
	pAudio->PlaySound( SOUNDS_DIR "/cannon_1.ogg" );
	
	walk_count = 1;
	massivetype = MASS_MASSIVE;
	visible = 1;

	if( direction == DIR_LEFT )
	{
		AddVel( -6.5, 0, 1 );
	}
	else
	{
		AddVel( 6.5, 0, 1 );
	}
}

void cbanzai_bill :: DownGrade( bool force /* = 0 */ )
{
	dead = 1;
	massivetype = MASS_PASSIVE;
	visible = 1;
	
	if( !force )
	{
		cAnimation_3 *anim = new cAnimation_3( pPlayer->posx + ( pPlayer->col_rect.w / 2 ), posy + 5, 4, GetImage( "animation/particles/light.png" ) );
		anim->SetSpeed( 4, 1 );
		anim->SetScale( 0.9 );
		anim->SetDirectionRange( 180, 180 );
		anim->SetLifetimeMod( 1.6 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );
	}
}

void cbanzai_bill :: DieStep( void )
{
	Move( 0, 17, 0, 1 );
	Draw();
	
	if( posy > 600 )
	{
		massivetype = MASS_PASSIVE;
		visible = 0;
	}
}

void cbanzai_bill :: Update( void )
{
	if( dead )
	{
		if( visible )
		{
			DieStep();
		}

		return;
	}

	// not active and player in y range
	if( massivetype == MASS_PASSIVE && ( posy + 300 > pPlayer->posy && posy - 300 < pPlayer->posy ) )
	{
		if( direction == DIR_LEFT )
		{
			if( pPlayer->posx + 1000 > posx && pPlayer->posx < posx )
			{
				Activate();
			}
		}
		else
		{
			if( pPlayer->posx - 1000 > posx && pPlayer->posx > posx + col_rect.w )
			{
				Activate();
			}
		}
	}

	if( massivetype == MASS_PASSIVE )
	{
		return;
	}

	CollideMove();
	
	cAnimation_3 *anim = NULL;

	// moving particle animation
	for( unsigned int i = 0; i < 20; i++ )
	{
		if( direction == DIR_LEFT )
		{
			anim = new cAnimation_3( posx + col_rect.w - 5 - ( rand() % 3 ), posy - 5 + ( rand() % (int)( rect.h - 10 ) ), 1 );
			anim->SetDirectionRange( 320, 100 );
		}
		else
		{
			anim = new cAnimation_3( posx - 5 - ( rand() % 3 ), posy - 5 + ( rand() % (int)( rect.h - 10 ) ), 1 );
			anim->SetDirectionRange( 140, 100 );
		}
		anim->SetSpeed( 3, 1.6 );
		anim->SetScale( 1.3 );
		anim->SetLifetimeMod( 1.3 );
		anim->SetColor( Get_Color( 255, 250, 230 ) );
		anim->SetFading( FADE_ALPHA );
		anim->SetBlending( BLEND_ADD );
		pAnimationManager->Add( anim );
	}

	Draw();
}

void cbanzai_bill :: HandleCollision( cObjectCollision *collision )
{
	Move( velx, vely, 0, 1, 0 );

	if( collision->type == CO_PLAYER )
	{
		PlayerCollision( collision->direction );
	}
}

void cbanzai_bill :: PlayerCollision( ObjectDirection cdirection )
{
	if( cdirection == DIR_DOWN )
	{
		pointsdisplay->AddPoints( 250, pPlayer->posx + pPlayer->image->w / 3, pPlayer->posy - 5 );
		pAudio->PlaySound( SOUNDS_DIR "/stomp_1.ogg" );
		pPlayer->start_enemyjump = 1;
		DownGrade();
	}
	else if( !dead && ( cdirection == DIR_LEFT || cdirection == DIR_RIGHT ) )
	{
		pPlayer->DownGrade();
	}
}
