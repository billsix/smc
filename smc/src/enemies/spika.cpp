/***************************************************************************
 * spika.cpp  -  spika, spiked waiting enemy
 *
 * Copyright (C) 2006 - 2009 Florian Richter
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
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

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
	cSpika::Create_From_Stream( attributes );
}

cSpika :: ~cSpika( void )
{
	//
}

void cSpika :: Init( void )
{
	m_type = TYPE_SPIKA;
	m_pos_z = 0.09f;

	color_type = COL_DEFAULT;
	speed = 0;
	detection_size = 0;

	Set_Color( COL_ORANGE );
}

cSpika *cSpika :: Copy( void )
{
	cSpika *spika = new cSpika( m_start_pos_x, m_start_pos_y );
	spika->Set_Color( color_type );

	return spika;
}

void cSpika :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() )) );
}

void cSpika :: Save_To_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"spika\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
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
		m_images.push_back( pVideo->Get_Surface( "enemy/spika/orange.png" ) );

		speed = 3;
		detection_size = 160;
		kill_points = 50;

		fire_resistant = 0;
	}
	else if( color_type == COL_GREEN )
	{
		m_images.push_back( pVideo->Get_Surface( "enemy/spika/green.png" ) );

		speed = 4;
		detection_size = 220;
		kill_points = 200;

		fire_resistant = 0;
	}
	else if( color_type == COL_GREY )
	{
		m_images.push_back( pVideo->Get_Surface( "enemy/spika/grey.png" ) );

		speed = 7;
		detection_size = 330;
		kill_points = 500;

		fire_resistant = 1;
	}
	else
	{
		printf( "Error : Unknown Spika Color %d\n", color_type );
	}

	Set_Image_Num( 0, 1 );

	m_name = "Spika ";
	m_name += _(Get_Color_Name( color_type ).c_str());
}

void cSpika :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	m_massive_type = MASS_PASSIVE;
	counter = 0;
	m_velx = 0;
	m_vely = 0;
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
	else if( m_pos_y < game_res_h + m_col_rect.m_h )
	{
		Move( 0, 20 );

		Add_Scale( -pFramerate->speedfactor * 0.01f );
	}
	// if below disable
	else
	{
		m_rot_z = 0;
		Set_Scale( 1 );
		Set_Active( 0 );
	}
}

void cSpika :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	// update rotation
	if( m_velx != 0 )
	{
		Add_Rotation_Z( ( m_velx / ( m_image->m_w * 0.01f ) ) * pFramerate->speedfactor );
	}

	// check for player
	GL_rect player_rect = pPlayer->m_col_rect;
	player_rect.m_x += ( pPlayer->m_col_rect.m_w / 2 );
	player_rect.m_w = 1;

	// rect
	GL_rect rect_left = m_col_rect;
	rect_left.m_y -= 10;
	rect_left.m_h += 10;

	GL_rect rect_right = rect_left;
	// left
	rect_left.m_x -= detection_size + ( m_col_rect.m_w / 2 );
	rect_left.m_w += detection_size;
	// right
	rect_right.m_x -= m_col_rect.m_w / 2;
	rect_right.m_w += detection_size;


	// if player is left
	if( pPlayer->maryo_type != MARYO_GHOST && player_rect.Intersects( rect_left ) )
	{
		if( m_velx > -speed )
		{
			m_velx -= speed * 0.1f * pFramerate->speedfactor;

			if( m_velx < -speed )
			{
				m_velx = -speed;
			}
		}
	}
	// if player is right
	else if( pPlayer->maryo_type != MARYO_GHOST && player_rect.Intersects( rect_right ) )
	{
		if( m_velx < speed )
		{
			m_velx += speed * 0.1f * pFramerate->speedfactor;

			if( m_velx > speed )
			{
				m_velx = speed;
			}
		}
	}
	// out of range
	else
	{
		// slow down
		m_velx -= (m_velx * 0.03f) * pFramerate->speedfactor;
	}

	// play walking sound based on speed
	if( walk_count < m_rot_z - 30 || walk_count > m_rot_z + 30 )
	{
		pAudio->Play_Sound( "enemy/spika/move.ogg" );

		walk_count = m_rot_z;
	}

	// gravity
	Update_Gravity();
}

bool cSpika :: Is_Update_Valid( void )
{
	if( dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cSpika :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	Col_Valid_Type basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid != COL_VTYPE_NO_GHOST )
	{
		return basic_valid;
	}

	if( obj->m_massive_type == MASS_MASSIVE )
	{
		if( obj->m_type == TYPE_ROKKO )
		{
			return COL_VTYPE_NOT_VALID;
		}
		if( obj->m_type == TYPE_GEE )
		{
			return COL_VTYPE_NOT_VALID;
		}
		if( obj->m_type == TYPE_TURTLE_BOSS )
		{
			return COL_VTYPE_NOT_VALID;
		}
		if( obj->m_type == TYPE_FURBALL_BOSS )
		{
			return COL_VTYPE_NOT_VALID;
		}
		if( obj->m_type == TYPE_STATIC_ENEMY )
		{
			return COL_VTYPE_NOT_VALID;
		}

		if( obj->m_sprite_array == ARRAY_ENEMY )
		{
			// if moving collide
			if( m_velx != 0 )
			{
				// if enemy is spika and more powerful
				if( obj->m_type == TYPE_SPIKA && speed < static_cast<cSpika *>(obj)->speed )
				{
					return COL_VTYPE_BLOCKING;
				}

				return COL_VTYPE_INTERNAL;
			}

			return COL_VTYPE_NOT_VALID;
		}

		return COL_VTYPE_BLOCKING;
	}
	if( obj->m_massive_type == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( m_vely >= 0 && Is_On_Top( obj ) )
		{
			return COL_VTYPE_BLOCKING;
		}
	}

	return COL_VTYPE_NOT_VALID;
}

void cSpika :: Handle_Collision_Player( cObjectCollision *collision )
{
	pPlayer->DownGrade_Player();

	if( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT )
	{
		m_velx = 0;
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
	if( !m_velx )
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
	if( enemy->m_type == TYPE_SPIKA )
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
	if( enemy->m_type == TYPE_TURTLE_BOSS )
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
	// state change for furball boss
	else if( enemy->m_type == TYPE_FURBALL_BOSS )
	{
		enemy->DownGrade();
		// kills spika
		DownGrade( 1 );
	}
	// kill enemy
	else
	{
		pAudio->Play_Sound( enemy->kill_sound );
		pHud_Points->Add_Points( enemy->kill_points, m_pos_x + m_image->m_w / 3, m_pos_y - 5, "", static_cast<Uint8>(255), 1 );
		enemy->DownGrade( 1 );
	}
}

void cSpika :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		m_velx = 0;
	}

	Send_Collision( collision );
}

void cSpika :: Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 )
{
	if( cdirection == DIR_DOWN )
	{
		m_vely = -10;

		// left
		if( m_pos_x > r2->m_x )
		{
			m_velx += 10;
		}
		// right
		else if( m_pos_x < r2->m_x )
		{
			m_velx -= 10;
		}
	}
	// unsupported collision direction
	else
	{
		return;
	}

	Reset_on_Ground();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
