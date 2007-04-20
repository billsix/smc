/***************************************************************************
              eato.cpp  -  eating static plant :P
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../enemies/eato.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../player/player.h"

/* *** *** *** *** *** *** cEato *** *** *** *** *** *** *** *** *** *** *** */

cEato :: cEato( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cEato :: cEato( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cEato :: ~cEato( void )
{
	//
}

void cEato :: Init( void )
{
    fire_resistant = 1;

	type = TYPE_EATO;
	player_range = 1000;
	posz = 0.087f;

	state = STA_STAY;
	Set_Direction( DIR_UP_LEFT );

	images.push_back( pVideo->Get_Surface( "enemy/eato/1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/eato/2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/eato/3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/eato/2.png" ) );

	walk_count = (float)( rand() % 4 );

	kill_sound = "stomp_4.ogg";
	kill_points = 150;

	Set_Image( 0 );
}

cEato *cEato :: Copy( void )
{
	cEato *eato = new cEato( startposx, startposy );
	eato->Set_Direction( start_direction );

	return eato;
}

void cEato :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
}

void cEato :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"eato\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cEato :: Set_Direction( ObjectDirection dir )
{
	cEnemy::Set_Direction( dir, 1 );

	// clear
	Set_Rotation( 0, 0, 0, 1 );

	name = "Eato";

	if( start_direction == DIR_UP_LEFT )
	{
		Set_RotationY( 180, 1 );
		name += " Up Left";
	}
	else if( start_direction == DIR_UP_RIGHT )
	{
		// default
		name += " Up Right";
	}
	else if( start_direction == DIR_LEFT_UP )
	{
		Set_RotationZ( 90, 1 );
		Set_RotationX( 180, 1 );
		name += " Left Up";
	}
	else if( start_direction == DIR_LEFT_DOWN )
	{
		Set_RotationZ( 90, 1 );
		name += " Left Down";
	}
	else if( start_direction == DIR_RIGHT_UP )
	{
		Set_RotationZ( 270, 1 );
		name += " Right Up";
	}
	else if( start_direction == DIR_RIGHT_DOWN )
	{
		Set_RotationZ( 270, 1 );
		Set_RotationX( 180, 1 );
		name += " Right Down";
	}
	else if( start_direction == DIR_DOWN_LEFT )
	{
		Set_RotationX( 180, 1 );
		name += " Down Left";
	}
	else if( start_direction == DIR_DOWN_RIGHT )
	{
		Set_RotationZ( 180, 1 );
		name += " Down Right";
	}
}

void cEato :: DownGrade( bool force /* = 0 */ )
{
	dead = 1;
	massivetype = MASS_PASSIVE;
	counter = 0;

	if( !force )
	{
		// animation
		cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Scale( 0.8f );
		anim->Set_DirectionRange( 0, 360 );
		// add animation
		pAnimationManager->Add( anim );
	}
	else
	{
		Set_RotationZ( 180 );
	}
}

void cEato :: DieStep( void )
{
	counter += pFramerate->speedfactor;

	// default death
	if( rotz != 180 )
	{
		visible = 0;
	}
	// falling death
	else
	{
		// a little bit upwards first
		if( counter < 5 )
		{
			Move( 0, -5 );
		}
		// if not below the screen fall
		else if( posy < GAME_RES_H + col_rect.h )
		{
			Move( 0, 20 );
		}
		// if below disable
		else
		{
			rotz = 0;
			visible = 0;
		}
	}
}

void cEato :: Update( void )
{
	cEnemy::Update();

	if( dead || freeze_counter || !is_Player_range() )
	{
		return;
	}

	walk_count += 0.2f * pFramerate->speedfactor;

	if( walk_count >= 4 )
	{
		walk_count = 0;
	}

	Set_Image( (int)walk_count );

	CollideMove();
}

unsigned int cEato :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		return 2;
	}
	if( obj->type == TYPE_BALL )
	{
		return 2;
	}

	return 0;
}

void cEato :: Handle_Collision_Player( ObjectDirection cdirection )
{
	// unknown direction
	if( cdirection == DIR_UNDEFINED )
	{
		return;
	}

	// only if not invincible
	if( pPlayer->invincible <= 0 )
	{
		// if player is big and not a bottom collision
		if( pPlayer->maryo_type != MARYO_SMALL && ( cdirection != DIR_BOTTOM ) )
		{
			pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );
			pPlayer->start_enemyjump = 1;
		}

		pPlayer->DownGrade();
	}
}
