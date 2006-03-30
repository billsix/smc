/***************************************************************************
           star.cpp  -  jumping star class
                             -------------------
    copyright            :	(C) 2006 FluXy
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

cjStar :: cjStar( double x, double y ) : cPowerUp( x, y )
{
	type = TYPE_JSTAR;
	massivetype = MASS_PASSIVE;

	images.push_back( GetImage( "game/items/star.png" ) );

	anim_counter = 0;
	vely = 0;
	velx = 5;

	SetImage( 0 );
}

cjStar :: ~cjStar( void )
{
	//
}


void cjStar :: Update( void )
{
	if( !visible || !isPlayer_range() )
	{
		return;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		Activate();
		return;
	}

	// Add Gravitation
	if( vely < 25.0 )
	{
		AddVel( 0, 1.8 );
	}

	CollideMove();
	
	// rotate
	if( vely < 0 )
	{
		AddRotationZ( ( 5 - ( vely / 2.5 ) ) * Framerate.speedfactor );
	}
	// rotate back to 0 if falling
	else
	{
		if( rotz > 5 && rotz <= 175 )
		{
			AddRotationZ( ( 5 - ( vely / 1.2 ) ) * Framerate.speedfactor );
		}
		else if( rotz < 355 && rotz > 185 )
		{
			AddRotationZ( ( -5 + ( vely / 1.2 ) ) * Framerate.speedfactor );
		}
	}


	anim_counter += 1.1 * Framerate.speedfactor;

	if( anim_counter > 1 )
	{
		Generate_Particles();

		anim_counter = 0;
	}

	Draw();
}

void cjStar :: Activate( void )
{
	// animation
	cAnimation_1 *anim = new cAnimation_1( posx + ( col_rect.w / 10 ), posy + ( col_rect.h / 10 ) );
	anim->SetColor( Get_Color( 255, 240, 20, 200 ) );
	pAnimationManager->Add( anim );

	// disable
	visible = 0;

	// activate star
	pPlayer->Get_Item( TYPE_JSTAR );
}

void cjStar :: Generate_Particles( double x /* = 0 */, double y /* = 0 */, unsigned int power /* = 2 */ )
{
	if( x == 0 )
	{
		x = posx + (double)( rand() % (int)( col_rect.w * 0.9 ) );
	}

	if( y == 0 )
	{
		y = posy + (double)( rand() % (int)( col_rect.h * 0.9 ) );
	}

	cAnimation_3 *anim = new cAnimation_3( x, y, power );
	anim->SetFadingSpeed( 1.5 );
	anim->SetImage( GetImage( "animation/particles/star.png" ) );
	anim->SetLifetimeMod( 3 );
	anim->SetFading( FADE_ALPHA );
	anim->SetSpeed( 1.5, 0.5 );
	anim->SetScale( 0.15 );
	anim->SetColor( orange );
	anim->SetBlending( BLEND_ADD );
	anim->Set_ConstRotationZ( 1.0, 10 );
	pAnimationManager->Add( anim );
}

void cjStar :: HandleCollision( cObjectCollision *collision )
{
	if( collision->type > CO_NOTHING && collision->type != CO_PLAYER ) // no Player
	{
		if( collision->type == CO_MASSIVE || collision->type == CO_ACTIVE )
		{
			if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
			{
				velx = -velx;
			}
			else if( collision->direction == DIR_UP )
			{
				vely = -( vely * 0.3 );
			}
			else if( collision->direction == DIR_DOWN )
			{
				if( groundObject )
				{
					Generate_Particles( posx + rect.w / 2, groundObject->posy, 10 );
				}

				vely = -25;
			}
		}
	}
}