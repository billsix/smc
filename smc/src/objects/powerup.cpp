/***************************************************************************
       powerup.cpp  -  classes for powerups
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

cPowerUp :: cPowerUp( double x, double y ) : cActiveSprite( x, y )
{
	type = TYPE_POWERUP;
	massivetype = MASS_PASSIVE;

	counter = 0;
}

cPowerUp :: ~cPowerUp( void )
{
	//
}

void cPowerUp :: Update( void )
{
	Draw();
}

void cPowerUp :: PlayerCollision( ObjectDirection direction )
{
	// virtual
}

void cPowerUp :: BoxCollision( ObjectDirection direction, GL_rect *r2 )
{
	// virtual
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMushroom :: cMushroom( double x, double y, SpriteType ntype ) : cPowerUp( x, y )
{
	velx = 3;
	type = ntype;
	player_range = 2500;

	if( type == TYPE_MUSHROOM_DEFAULT )
	{
		images.push_back( GetImage( "game/items/mushroom_red.png" ) );
	}
	else if( type == TYPE_MUSHROOM_LIVE_1 )
	{
		images.push_back( GetImage( "game/items/mushroom_green.png" ) );
	}
	else
	{
		printf( "Warning Unknown Mushroom type : %d\n", ntype );
		visible = 0;
		return;
	}

	SetImage( 0 );
}

cMushroom :: ~cMushroom( void )
{
	//
}

void cMushroom :: Update( void )
{
	if( !visible || !isPlayer_range() )
	{
		return;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		visible = 0;
		pPlayer->Get_Item( type );
		
		if( type == TYPE_MUSHROOM_DEFAULT )
		{
			pointsdisplay->AddPoints( 500, posx + image->w / 2, posy + 3 );
		}
		else if( type == TYPE_MUSHROOM_LIVE_1 )
		{
			pointsdisplay->AddPoints( 1000, posx + image->w/ 2, posy + 3 );
		}
	}

	if( !groundObject && vely < 50 )
	{
		if( vely > 0 )
		{
			AddVel( 0, 0.5 + ( vely * 0.05 ) );
		}
		else
		{
			AddVel( 0, 2.5 );
		}
	}
	else
	{
		vely = 0;
	}
	
	// collision handling
	CollideMove();

	// particles
	if( type == TYPE_MUSHROOM_LIVE_1 )
	{
		counter += Framerate.speedfactor;

		if( counter > 1 )
		{
			if( ( rand() % 5 ) == 0 )
			{
				cAnimation_3 *anim = new cAnimation_3( posx + 9 + ( rand() % 30 ), posy + 8 + ( rand() % 20 ), 1 );
				anim->SetDirectionRange( 180, 180 );
				anim->SetSpeed( 0.5, 0.5 );
				anim->SetScale( 0.5 );
				anim->SetColor( green );
				anim->SetBlending( BLEND_ADD );
				anim->SetImage( GetImage( "animation/particles/light.png" ) );
				pAnimationManager->Add( anim );
			}

			counter = 0;
		}
	}

   	Draw();
}

void cMushroom :: HandleCollision( cObjectCollision *collision )
{
	if( ( collision->type == CO_MASSIVE || collision->type == CO_ACTIVE ) && ( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT ) )
	{
		velx *= -1;
	}
}

void cMushroom :: BoxCollision( ObjectDirection cdirection, GL_rect *r2 )
{
	if( cdirection == DIR_DOWN )
	{
		if( posx > r2->x && velx < 0 ) // left
		{
			velx *= -1;
		}
		else if( posx < r2->x && velx > 0 ) // right
		{
			velx *= -1;
		}
		
		vely = -30;
		Move( velx, vely, 0, 1, 0 );
		Reset_onGround();
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFirePlant :: cFirePlant( double x, double y ) : cPowerUp( x, y )
{
	type = TYPE_FIREPLANT;
	
	images.push_back( GetImage( "game/items/flower_red_1.png" ) );
	images.push_back( GetImage( "game/items/flower_red_2.png" ) );

	SetImage( 0 );
}

cFirePlant :: ~cFirePlant( void )
{
	//
}

void cFirePlant :: Update( void )
{
	if( !visible || !isVisible_onScreen() )
	{
		return;
	}

	counter += Framerate.speedfactor;
	
	if( (int)counter < (int)( DESIRED_FPS / 2 ) )
	{
		SetImage( 1 );
	}
	else if( (int)counter < DESIRED_FPS )
	{
		SetImage( 0 );
	}
	else
	{
		counter = 0;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		visible = 0;
		pPlayer->Get_Item( TYPE_FIREPLANT );

		pointsdisplay->AddPoints( 700, posx + image->w / 2, posy );
	}

	Draw();
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMoon :: cMoon( double x, double y ) : cPowerUp( x, y )
{
	type = TYPE_MOON;

	images.push_back( GetImage( "game/items/moon_1.png" ) );
	images.push_back( GetImage( "game/items/moon_2.png" ) );

	SetImage( 0 );
}

cMoon :: ~cMoon( void )
{
	//
}

void cMoon :: Update( void )
{
	if( !visible || !isVisible_onScreen() )
	{
		return;
	}

	counter += Framerate.speedfactor;
	
	if( (int)counter > DESIRED_FPS * 2.8 )
	{
		SetImage( 1 );
	}
	else if( (int)counter > 0 )
	{
		SetImage( 0 );
	}
	
	if( counter > DESIRED_FPS * 3.3 ) 
	{
		counter = 0;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		visible = 0;
		pPlayer->Get_Item( TYPE_MOON );

		pointsdisplay->AddPoints( 4000, posx + image->w / 2, posy );
	}

	particle_counter += Framerate.speedfactor;

	// particles
	if( particle_counter > 5 )
	{
		if( ( rand() % 5 ) == 0 )
		{
			cAnimation_3 *anim = new cAnimation_3( posx + 12 + ( rand() % 30 ), posy + 10 + ( rand() % 20 ), 1 );
			anim->SetSpeed( 0.2, 0.2 );
			anim->SetScale( 0.2 );
			anim->SetColor( Color( (Uint8)200, 200, 0 ) );
			anim->SetBlending( BLEND_ADD );
			anim->SetImage( GetImage( "animation/particles/light.png" ) );
			pAnimationManager->Add( anim );
		}

		particle_counter = 0;
	}

	Draw();
}
