/***************************************************************************
              static.cpp  -  static enemy
                             -------------------
    copyright            :	(C) 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../enemies/static.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../core/obj_manager.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */

cStaticEnemy :: cStaticEnemy( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cStaticEnemy :: cStaticEnemy( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cStaticEnemy :: ~cStaticEnemy( void )
{
	//
}

void cStaticEnemy :: Init( void )
{
	type = TYPE_STATIC_ENEMY;
	posz = 0.094f;
	speed = -1.5f;

	images.push_back( pVideo->Get_Surface( "enemy/static/saw/default.png" ) );

	Set_Image( 0, 1 );
	name = "Static Enemy";
}

cStaticEnemy *cStaticEnemy :: Copy( void )
{
	cStaticEnemy *static_enemy = new cStaticEnemy( startposx, startposy );

	return static_enemy;
}

void cStaticEnemy :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
}

void cStaticEnemy :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"static\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cStaticEnemy :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;

	// falling death
	Set_RotationZ( 180 );
}

void cStaticEnemy :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.1f;

	// falling death

	// a little bit upwards first
	if( counter < 0.3 )
	{
		Move( 0, -5 );
	}
	// if not below the screen fall
	else if( posy < GAME_RES_H + col_rect.h )
	{
		Move( 0, 20 );

		Add_Scale( -pFramerate->speedfactor * 0.01f );
		Move( ( image->w / 2 ) * pFramerate->speedfactor * 0.01f, ( image->h ) * pFramerate->speedfactor * 0.01f, 1 );
	}
	// if below disable
	else
	{
		rotz = 0;
		Set_Scale( 1 );
		Set_Visible( 0 );
	}
}

void cStaticEnemy :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// update rotation
	Add_RotationZ( speed / ( image->w * 0.01f ) );
}

bool cStaticEnemy :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cStaticEnemy :: Validate_Collision( cSprite *obj )
{
	if( obj->massivetype == MASS_MASSIVE )
	{
		if( obj->type == TYPE_ROKKO )
		{
			return 0;
		}
		if( obj->type == TYPE_GEE )
		{
			return 0;
		}
		if( obj->type == TYPE_TURTLE_BOSS )
		{
			return 0;
		}

		if( obj->sprite_array == ARRAY_ENEMY )
		{
			return 1;
		}

		return 2;
	}

	return 0;
}

void cStaticEnemy :: Handle_Collision_Player( ObjectDirection cdirection )
{
	pPlayer->DownGrade();
}

void cStaticEnemy :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// invalid
	if( collision->number < 0 )
	{
		return;
	}

	cEnemy *enemy = static_cast<cEnemy *>(pObjManager->Get_Pointer( collision->number ));

	// already dead
	if( enemy->dead )
	{
		return;
	}

	// kill enemy
	pAudio->PlaySound( enemy->kill_sound );
	pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
	enemy->DownGrade( 1 );
}
