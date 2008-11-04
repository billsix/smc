/***************************************************************************
 * spika.cpp  -  spika, spiked waiting enemy
 *
 * Copyright (C) 2006 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/spika.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"
#include "../enemies/bosses/turtle_boss.h"

/* *** *** *** *** *** *** cSpika *** *** *** *** *** *** *** *** *** *** *** */

cSpika :: cSpika( float x, float y )
: cEnemy( x, y )
{
	cSpika::Init();
}

cSpika :: cSpika( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cSpika::Init();
	cSpika::Create_from_Stream( attributes );
}

cSpika :: ~cSpika( void )
{
	//
}

void cSpika :: Init( void )
{
	type = TYPE_SPIKA;
	posz = 0.09f;
	can_be_ground = 1;

	color_type = COL_DEFAULT;
	speed = 0;
	detection_size = 0;

	Set_Color( COL_ORANGE );
}

cSpika *cSpika :: Copy( void )
{
	cSpika *spika = new cSpika( startposx, startposy );
	spika->Set_Color( color_type );

	return spika;
}

void cSpika :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() )) );
}

void cSpika :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"spika\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( color_type ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cSpika :: Set_Color( DefaultColor col )
{
	// already set
	if( color_type == col )
	{
		return;
	}

	// clear old images
	Clear_Images();

	color_type = col;

	if( color_type == COL_ORANGE )
	{
		images.push_back( pVideo->Get_Surface( "enemy/spika/orange.png" ) );

		speed = 3;
		detection_size = 160;
		kill_points = 50;

		fire_resistant = 0;
	}
	else if( color_type == COL_GREEN )
	{
		images.push_back( pVideo->Get_Surface( "enemy/spika/green.png" ) );

		speed = 4;
		detection_size = 220;
		kill_points = 200;

		fire_resistant = 0;
	}
	else if( color_type == COL_GREY )
	{
		images.push_back( pVideo->Get_Surface( "enemy/spika/grey.png" ) );

		speed = 7;
		detection_size = 330;
		kill_points = 500;

		fire_resistant = 1;
	}
	else
	{
		printf( "Error : Unknown Spika Color %d\n", color_type );
	}

	Set_Image( 0, 1 );

	name = "Spika ";
	name += _(Get_Color_Name( color_type ).c_str());
}

void cSpika :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;
	Set_Scale_Directions( 1, 1, 1, 1 );

	// default stomp death
	if( !force )
	{
		Generate_Hit_Animation();
	}
	// falling death
	else
	{
		Set_Rotation_Z( 180 );
	}
}

void cSpika :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.1f;

	// falling death

	// a little bit upwards first
	if( counter < 0.3f )
	{
		Move( 0, -5 );
	}
	// if not below the screen fall
	else if( posy < game_res_h + col_rect.h )
	{
		Move( 0, 20 );

		Add_Scale( -pFramerate->speedfactor * 0.01f );
	}
	// if below disable
	else
	{
		rotz = 0;
		Set_Scale( 1 );
		Set_Visible( 0 );
	}
}

void cSpika :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// update rotation
	if( velx != 0 )
	{
		Add_Rotation_Z( ( velx / ( image->w * 0.01f ) ) * pFramerate->speedfactor );
	}

	// check for player
	GL_rect player_rect = pPlayer->col_rect;
	player_rect.x += ( pPlayer->col_rect.w / 2 );
	player_rect.w = 1;

	// rect
	GL_rect rect_left = col_rect;
	rect_left.y -= 10;
	rect_left.h += 10;

	GL_rect rect_right = rect_left;
	// left
	rect_left.x -= detection_size + ( col_rect.w / 2 );
	rect_left.w += detection_size;
	// right
	rect_right.x -= col_rect.w / 2;
	rect_right.w += detection_size;


	// if player is left
	if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &player_rect, &rect_left ) )
	{
		if( velx > -speed )
		{
			velx -= speed * 0.1f * pFramerate->speedfactor;

			if( velx < -speed )
			{
				velx = -speed;
			}
		}
	}
	// if player is right
	else if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &player_rect, &rect_right ) )
	{
		if( velx < speed )
		{
			velx += speed * 0.1f * pFramerate->speedfactor;

			if( velx > speed )
			{
				velx = speed;
			}
		}
	}
	// out of range
	else
	{
		// slow down
		velx -= velx * 0.03f * pFramerate->speedfactor;
	}

	// play walking sound based on speed
	if( walk_count < rotz - 30 || walk_count > rotz + 30 )
	{
		pAudio->Play_Sound( "enemy/spika/move.ogg" );

		walk_count = rotz;
	}

	// gravity
	Update_Gravity();
}

bool cSpika :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cSpika :: Validate_Collision( cSprite *obj )
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
		if( obj->type == TYPE_STATIC_ENEMY )
		{
			return 0;
		}

		if( obj->sprite_array == ARRAY_ENEMY )
		{
			// if moving collide
			if( velx != 0 )
			{
				// if enemy is spika and more powerful
				if( obj->type == TYPE_SPIKA && speed < static_cast<cSpika *>(obj)->speed )
				{
					return 2;
				}

				return 1;
			}

			return 0;
		}

		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && Is_on_Top( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cSpika :: Handle_Collision_Player( cObjectCollision *collision )
{
	pPlayer->DownGrade();

	if( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT )
	{
		velx = 0;
	}
}

void cSpika :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// invalid
	if( collision->number < 0 )
	{
		return;
	}

	// only if moving
	if( !velx )
	{
		return;
	}

	cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

	// already dead
	if( enemy->dead )
	{
		return;
	}

	// check spika power
	if( enemy->type == TYPE_SPIKA )
	{
		cSpika *spika = static_cast<cSpika *>(enemy);

		// if colliding spika is more powerful
		if( speed < spika->speed )
		{
			//enemy->Send_Collision( collision );
			return;
		}
	}

	// state change for turtle boss
	if( enemy->type == TYPE_TURTLE_BOSS )
	{
		// todo : remove hackiness and implement a better generic downgrade handler
		cTurtleBoss *turtle_boss = static_cast<cTurtleBoss *>(enemy);

		// downgrade until state change
		for( int i = turtle_boss->hits; i < turtle_boss->max_hits; i++ )
		{
			turtle_boss->DownGrade();
		}

		// turtle kills spika
		DownGrade( 1 );
	}
	// kill enemy
	else
	{
		pAudio->Play_Sound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
		enemy->DownGrade( 1 );
	}
}

void cSpika :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		velx = 0;
	}
}

void cSpika :: Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 )
{
	if( cdirection == DIR_DOWN )
	{
		vely = -10;

		// left
		if( posx > r2->x )
		{
			velx += 10;
		}
		// right
		else if( posx < r2->x )
		{
			velx -= 10;
		}
	}
	// unsupported collision direction
	else
	{
		return;
	}

	Reset_on_Ground();
}
