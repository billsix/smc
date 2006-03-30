/***************************************************************************
         goldpiece.cpp  -  goldpiece class
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

cGoldPiece :: cGoldPiece( double x, double y, DefaultColor goldcolor /* = COL_YELLOW */ ) : cActiveSprite( x, y )
{
	type = TYPE_GOLDPIECE;
	massivetype = MASS_PASSIVE;

	if( goldcolor == COL_RED )
	{
		color = COL_RED;

		images.push_back( GetImage( "animation/goldpiece/red/1.png" ) );
		images.push_back( GetImage( "animation/goldpiece/red/2.png" ) );
		images.push_back( GetImage( "animation/goldpiece/red/3.png" ) );
		images.push_back( GetImage( "animation/goldpiece/red/4.png" ) );
	}
	else	// default = yellow
	{
		color = COL_YELLOW;

		images.push_back( GetImage( "animation/goldpiece/yellow/1.png" ) );
		images.push_back( GetImage( "animation/goldpiece/yellow/2.png" ) );
		images.push_back( GetImage( "animation/goldpiece/yellow/3.png" ) );
		images.push_back( GetImage( "animation/goldpiece/yellow/4.png" ) );
	}

	// small random counter
	counter = rand() % 3;

	SetImage( 3, 1 );
}

cGoldPiece :: ~cGoldPiece( void )
{
	//
}

void cGoldPiece :: Update( void )
{
	if( !visible || !isVisible_onScreen() )
	{
		return;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		pAudio->PlaySound( SOUNDS_DIR "/goldpiece_1.ogg" );
		Activate();
		return;
	}

	counter += 0.3 * Framerate.speedfactor;

	if( counter > 4 )
	{
		counter = 0;
	}
	
	SetImage( (int)counter );
	Draw();
}

void cGoldPiece :: Activate( void )
{
	// animation
	cAnimation_1 *anim = new cAnimation_1( posx + ( col_rect.w / 10 ), posy + ( col_rect.h / 10 ) );
	
	// gold
	unsigned int points = 0;

	if( color == COL_RED )
	{
		golddisplay->AddGold( 5 );
		points = 100;
		anim->SetColor( Get_Color( 255, 240, 20, 200 ) );
	}
	else
	{
		golddisplay->AddGold( 1 );
		points = 5;
	}

	pAnimationManager->Add( anim );
	if( type == TYPE_BOUNCINGGOLDPIECE ) // if bouncing double the points
	{
		points *= 2;
	}

	pointsdisplay->AddPoints( points, posx + col_rect.w / 2, posy + 2 );
	// disable
	visible = 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cBouncingGoldPiece :: cBouncingGoldPiece( double x, double y, DefaultColor goldcolor /* = COL_YELLOW */ ) : cGoldPiece( x + 5, y, goldcolor )
{
	type = TYPE_BOUNCINGGOLDPIECE;
	
	counter = ( rand() % 4 ) + 1;
	massivetype = MASS_PASSIVE;
	
	vely = -18;
}

cBouncingGoldPiece :: ~cBouncingGoldPiece( void )
{
	//
}

void cBouncingGoldPiece :: Update( void )
{
	if( !visible )
	{
		return;
	}

	Move( velx, vely, 0, 1 );

	counter += 0.5 * Framerate.speedfactor;

	if( counter > 4 )
	{
		counter = 0;
	}

	SetImage( (int)counter );
	Draw();
}
