/***************************************************************************
           cloud.cpp  -  moving clouds
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

cCloud :: cCloud( double x, double y ) : cActiveSprite( x, y )
{
	type = TYPE_CLOUD;
	massivetype = MASS_HALFMASSIVE;

	player_range = 2000;

	speed = rand() % 5 + 4;
	moving = 0;

	counter = rand() % ( DESIRED_FPS * 4 );

	images.push_back( GetImage( "clouds/default_1/1_left.png" ) );
	images.push_back( GetImage( "clouds/default_1/1_right.png" ) );
	images.push_back( GetImage( "clouds/default_1/1_middle.png" ) );

	SetImage( 1 );
}

cCloud :: ~cCloud( void )
{
	//
}

void cCloud :: Update( void )
{
	if( !visible || !isPlayer_range() )
	{
		return;
	}

	counter += Framerate.speedfactor;

	if( counter > DESIRED_FPS * 2 )
	{
		if( counter < DESIRED_FPS * 4 )
		{
			if( !moving )
			{
				velx = speed;

				if( rand() % 2 != 1 )
				{
					velx *= -1;
				}

				if( velx > 0 )
				{
					SetImage( 1 );
				}
				else
				{
					SetImage( 0 );
				}

				moving = 1;
			}
			
			Move( velx, vely, 0, 1 );
		}
		else
		{
			SetImage( 2 );
			
			counter = 0;
			moving = 0;
			velx = 0;
		}
	}

	if( ( rand() % 50 ) == 1 )
	{
		cAnimation_3 *anim = new cAnimation_3( posx + 15 + ( rand() % (int)( col_rect.w - 30 ) ), posy + ( col_rect.h / 2 ), 4, GetImage( "animation/particles/cloud.png" ) );
		anim->SetSpeed( 0.3, 0.2 );
		anim->SetScale( 0.5 );
		//anim->SetDirectionRange( 180, 180 );
		anim->SetLifetimeMod( 10 );
		anim->SetFading( FADE_ALPHA );
		pAnimationManager->Add( anim );
	}

	Draw();
}
