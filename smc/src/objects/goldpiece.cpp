/***************************************************************************
         goldpiece.cpp  -  goldpiece class
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../objects/goldpiece.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../video/animation.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** cGoldpiece *** *** *** *** *** *** *** *** *** *** *** */

cGoldpiece :: cGoldpiece( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cGoldpiece :: cGoldpiece( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cGoldpiece :: ~cGoldpiece( void )
{
	//
}

void cGoldpiece :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	massivetype = MASS_PASSIVE;
	type = TYPE_GOLDPIECE;
	posz = 0.041f;

	Set_Goldcolor( COL_YELLOW );

	// small random counter
	counter = (float)(rand() % 5);
}

cGoldpiece *cGoldpiece :: Copy( void )
{
	cGoldpiece *goldpiece = new cGoldpiece( startposx, startposy );
	goldpiece->Set_Goldcolor( color_type );

	return goldpiece;
}

void cGoldpiece :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// gold color
	Set_Goldcolor( Get_Color_id( attributes.getValueAsString( "color", Get_Color_name( color_type ) ).c_str() ) );
}

void cGoldpiece :: Save_to_Stream( ofstream &file )
{
	// begin item
	file << "\t<item>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"goldpiece\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_name( color_type ) << "\" />" << std::endl;

	// end item
	file << "\t</item>" << std::endl;
}

void cGoldpiece :: Set_Goldcolor( DefaultColor ncolor )
{
	color_type = ncolor;

	// clear images
	Clear_Images();

	if( color_type == COL_RED )
	{
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/1.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/2.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/3.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/4.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/5.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/6.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/7.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/8.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/9.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/10.png" ) );

		name = "Red Gold";
	}
	// default = yellow
	else
	{
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/2.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/3.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/4.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/5.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/6.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/7.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/8.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/9.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/10.png" ) );

		name = "Gold";
	}

	Set_Image( 0, 1, 0 );
}

void cGoldpiece :: Activate( void )
{
	// animation
	cBlinkAnimation *anim = new cBlinkAnimation( posx + ( col_rect.w / 10 ), posy + ( col_rect.h / 10 ) );

	// gold
	unsigned int points = 0;

	if( color_type == COL_RED )
	{
		golddisplay->AddGold( 5 );
		points = 100;
		anim->Set_Color( Color( (Uint8)255, 240, 20, 200 ) );
	}
	else
	{
		golddisplay->AddGold( 1 );
		points = 5;
	}

	pAnimationManager->Add( anim );

	// if jumping double the points
	if( type == TYPE_JGOLDPIECE )
	{
		points *= 2;
	}

	pointsdisplay->Add_Points( points, posx + col_rect.w / 2, posy + 2 );
	// disable
	Set_Visible( 0 );
}

void cGoldpiece :: Update( void )
{
	if( !valid_update || !is_Visible_onScreen() )
	{
		return;
	}

	// todo : move to internal collision
	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		pAudio->PlaySound( "item/goldpiece_1.ogg" );
		Activate();
		return;
	}

	counter += 0.4f * pFramerate->speedfactor;

	if( counter >= 10 )
	{
		counter = 0;
	}

	Set_Image( (int)counter, 0, 0 );
}

void cGoldpiece :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// don't draw in leveleditor if spawned ingame
	if( editor_level_enabled && spawned )
	{
		return;
	}

	cImageObjectSprite::Draw( request );
}

bool cGoldpiece :: is_Update_valid( void )
{
	if( !visible )
	{
		return 0;
	}

	return 1;
}

/* *** *** *** *** *** *** cJGoldpiecee *** *** *** *** *** *** *** *** *** *** *** */

cJGoldpiece :: cJGoldpiece( float x, float y )
: cGoldpiece( x, y )
{
	type = TYPE_JGOLDPIECE;
	spawned = 1;

	Set_Goldcolor( COL_YELLOW );

	counter = (float)( rand() % 5 );
	massivetype = MASS_PASSIVE;

	vely = -18;
}

cJGoldpiece :: ~cJGoldpiece( void )
{
	//
}

void cJGoldpiece :: Update( void )
{
	if( !visible )
	{
		return;
	}

	// add velocity downwards
	if( vely < 8 )
	{
		AddVel( 0, 1.62f );
	}
	// finished animation
	else
	{
		Activate();
		Destroy();
	}

	counter += pFramerate->speedfactor * 0.5f;

	if( counter >= 10 )
	{
		counter = 0;
	}

	Set_Image( (int)counter, 0, 0 );
}

unsigned int cJGoldpiece :: Validate_Collision( cSprite *obj )
{
	return 0;
}

/* *** *** *** *** *** *** cFGoldpiecee *** *** *** *** *** *** *** *** *** *** *** */

cFGoldpiece :: cFGoldpiece( float x, float y, ObjectDirection dir /* = DIR_NOTHING */ )
: cGoldpiece( x, y )
{
	type = TYPE_FGOLDPIECE;
	spawned = 1;
	player_range = 2000;
	counter = (float)( rand() % 5 );
	massivetype = MASS_PASSIVE;

	// set a random direction
	if( dir != DIR_LEFT && dir != DIR_RIGHT )
	{
		if( rand() % 2 != 1 )
		{
			dir = DIR_LEFT;
		}
		else
		{
			dir = DIR_RIGHT;
		}
	}

	direction = dir;

	if( direction == DIR_RIGHT )
	{
		velx = 5;
	}
	else
	{
		velx = -5;
	}

	Set_Goldcolor( COL_YELLOW );
}

cFGoldpiece :: ~cFGoldpiece( void )
{
	//
}

void cFGoldpiece :: Set_Goldcolor( DefaultColor ncolor )
{
	color_type = ncolor;

	// clear images
	Clear_Images();

	if( color_type == COL_RED )
	{
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/1_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/2_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/3_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/4_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/5_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/6_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/7_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/8_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/9_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/red/10_falling.png" ) );
	}
	// default = yellow
	else
	{
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/1_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/2_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/3_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/4_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/5_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/6_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/7_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/8_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/9_falling.png" ) );
		images.push_back( pVideo->Get_Surface( "game/items/goldpiece/yellow/10_falling.png" ) );
	}

	Set_Image( 0, 1, 0 );
}

void cFGoldpiece :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		pAudio->PlaySound( "item/goldpiece_1.ogg" );
		Activate();
		return;
	}

	// Add Gravitation
	if( !ground_object && vely < 25 )
	{
		AddVel( 0, 1.2f );
	}

	counter += 0.5f * pFramerate->speedfactor;

	if( counter >= 10 )
	{
		counter = 0;
	}

	Set_Image( (int)counter, 0, 0 );
}

unsigned int cFGoldpiece :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	int basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid > -1 )
	{
		return basic_valid;
	}

	if( obj->massivetype == MASS_MASSIVE )
	{
		// ignore player
		if( obj->type == TYPE_PLAYER )
		{
			return 0;
		}
		// ignore enemies
		if( obj->sprite_array == ARRAY_ENEMY )
		{
			return 0;
		}
		// ignore fireballs
		if ( obj->type == TYPE_BALL )
		{
		    return 0;
		}

		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cFGoldpiece :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		Turn_Around( collision->direction );
	}
	else if( collision->direction == DIR_UP )
	{
		vely = -( vely * 0.3f );
	}
	else if( collision->direction == DIR_DOWN )
	{
		// minimal value for a jump
		if( vely > 0.5f )
		{
			vely = -( vely * 0.5f );

			// maximum value for a jump
			if( vely > 10 )
			{
				vely = 10;
			}
		}
		else
		{
			vely = 0;
		}
	}
}

void cFGoldpiece :: Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 )
{
	// if unsupported collision direction
	if( cdirection != DIR_DOWN && cdirection != DIR_LEFT && cdirection != DIR_RIGHT )
	{
		return;
	}

	if( cdirection == DIR_DOWN )
	{
		vely = -30;

		// left
		if( posx > r2->x && velx < 0 )
		{
			Turn_Around( DIR_LEFT );
		}
		// right
		else if( posx < r2->x && velx > 0 )
		{
			Turn_Around( DIR_RIGHT );
		}
	}
	else if( cdirection == DIR_LEFT || cdirection == DIR_RIGHT )
	{
		vely = -13;
		Turn_Around( cdirection );
	}

	Reset_onGround();
}
