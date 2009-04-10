/***************************************************************************
 * turtle.cpp  -  turtle enemy class
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/turtle.h"
#include "../core/game_core.h"
#include "../objects/box.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** cTurtle *** *** *** *** *** *** *** *** *** *** *** */

cTurtle :: cTurtle( float x, float y )
: cEnemy( x, y )
{
	cTurtle::Init();
}

cTurtle :: cTurtle( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cTurtle::Init();
	cTurtle::Create_From_Stream( attributes );
}

cTurtle :: ~cTurtle( void )
{
	//
}

void cTurtle :: Init( void )
{
	m_type = TYPE_TURTLE;
	m_pos_z = 0.091f;

	m_state = STA_WALK;
	turtle_state = TURTLE_WALK;	// Walking
	playercounter = 0;

	color_type = COL_DEFAULT;
	Set_Color( COL_RED );
	Set_Direction( DIR_RIGHT, 1 );

	kill_sound = "stomp_4.ogg";
}

cTurtle *cTurtle :: Copy( void )
{
	cTurtle *turtle = new cTurtle( m_start_pos_x, m_start_pos_y );
	turtle->Set_Direction( m_start_direction, 1 );
	turtle->Set_Color( color_type );

	return turtle;
}

void cTurtle :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() )) );
}

void cTurtle :: Save_To_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"turtle\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( m_start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cTurtle :: Load_From_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_From_Savegame( save_object );

	// turtle_state
	if( save_object->exists( "turtle_state" ) )
	{
		turtle_state = static_cast<Turtle_state>(string_to_int( save_object->Get_Value( "turtle_state" ) ));

		if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
		{
			// todo : create a Set_State function
			// set shell image without position changes
			cSprite::Set_Image( m_images[5] );
		}
	}
	
	Update_Rotation_Hor_velx();
}

cSave_Level_Object *cTurtle :: Save_To_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_To_Savegame();

	// turtle_state ( only save if needed )
	if( turtle_state != TURTLE_WALK )
	{
		save_object->m_properties.push_back( cSave_Level_Object_Property( "turtle_state", int_to_string( turtle_state ) ) );
	}

	return save_object;
}

void cTurtle :: Set_Direction( const ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	if( dir != DIR_RIGHT && dir != DIR_LEFT )
	{
		printf( "Warning : Unknown Turtle direction set %d\n", dir );
		return;
	}

	cEnemy::Set_Direction( dir, new_start_direction );

	// update velocity
	if( m_direction == DIR_RIGHT )
	{
		if( turtle_state == TURTLE_WALK )
		{
			m_velx = speed_walk;
		}
		else
		{
			m_velx = speed_shell;
		}
	}
	else
	{
		if( turtle_state == TURTLE_WALK )
		{
			m_velx = -speed_walk;
		}
		else
		{
			m_velx = -speed_shell;
		}
	}

	Update_Rotation_Hor_velx( new_start_direction );

	if( new_start_direction )
	{
		Create_Name();
	}
}

void cTurtle :: Set_Color( DefaultColor col )
{
	// already set
	if( color_type == col )
	{
		return;
	}

	color_type = col;

	std::string filename_dir;

	if( color_type == COL_RED )
	{
		filename_dir = "red";

		speed_walk = 3.6f;
		speed_shell = 14;
		kill_points = 50;
	}
	else if( color_type == COL_GREEN )
	{
		filename_dir = "green";

		speed_walk = 4.5f;
		speed_shell = 17;
		kill_points = 150;
	}
	// unknown color
	else
	{
		printf( "Error : Unknown Turtle color : %d\n", color_type );
	}

	Clear_Images();

	// Walk
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_0.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_1.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_2.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_1.png" ) );
	// Walk Turn
	//images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/turn_1.png" ) );
	m_images.push_back( NULL );
	// Shell
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_front.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_1.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_2.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_3.png" ) );
	m_images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_active.png" ) );

	Set_Image_Num( 0, 1 );
	Create_Name();
}

void cTurtle :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	cEnemy::Turn_Around( col_dir );

	if( turtle_state == TURTLE_WALK )
	{
		// hack : disable turn image
		//walk_count = 0;
		//Set_Image( 4 );
	}

	Update_Rotation_Hor_velx();
}

void cTurtle :: DownGrade( bool force /* = 0 */ )
{
	if( !force )
	{
		// normal walking
		if( turtle_state == TURTLE_WALK )
		{
			counter = 0;
			m_state = STA_STAY;
			turtle_state = TURTLE_SHELL_STAND;

			Set_Image_Num( 5 );
			Col_Move( 0, m_images[0]->m_col_h - m_images[5]->m_col_h, 1, 1 );
		}
		// staying
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			counter = 0;
			m_state = STA_RUN;
			turtle_state = TURTLE_SHELL_RUN;
			m_player_range = 5000;

			// move into random direction
			if( rand() % 2 != 1 )
			{
				// right
				m_velx = speed_shell;
			}
			else
			{
				// left
				m_velx = -speed_shell;
			}

			Update_Direction();
			Update_Rotation_Hor_velx();
		}
		// running shell
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			counter = 0;
			m_state = STA_STAY;
			turtle_state = TURTLE_SHELL_STAND;
			m_player_range = 2000;
		}
	}
	// falling death
	else
	{
		counter = 8;
		Set_Dead( 1 );
		m_massive_type = MASS_PASSIVE;
		m_velx = 0;
		m_vely = 0;

		if( turtle_state == TURTLE_WALK )
		{
			Move( 0, m_images[0]->m_h - m_images[5]->m_h, 1 );
		}

		Set_Image_Num( 5 );
	}
}

void cTurtle :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.5f;

	// if not below the screen fall
	if( m_pos_y < game_res_h + m_col_rect.m_h )
	{
		float speed_y = counter;

		// first a little bit upwards
		if( speed_y < 10 )
		{
			speed_y *= -1;
		}
		// limit falling speed
		else if( speed_y > 25 )
		{
			speed_y = 25;
		}

		float speed_x = pFramerate->speedfactor;

		if( m_direction == DIR_LEFT )
		{
			speed_x *= -1;
		}

		Add_Rotation_Z( speed_x );

		Move( speed_x * 15, speed_y );
	}
	// if below disable
	else
	{
		Set_Active( 0 );
		m_rot_z = 0;
		m_state = STA_STAY;
		turtle_state = TURTLE_DEAD;
	}
}

void cTurtle :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	// walking
	if( turtle_state == TURTLE_WALK )
	{
		walk_count += pFramerate->speedfactor * 0.3f;

		// if not turn around image
		if( m_curr_img != 4 )
		{
			if( walk_count >= 4 )
			{
				walk_count = 0;
			}

			Set_Image_Num( static_cast<int>(walk_count) );
		}
		// turn around image
		else
		{
			// set normal image back
			if( walk_count >= 4 )
			{
				Set_Image_Num( static_cast<int>(walk_count) );
				Update_Rotation_Hor_velx();
			}
			// rotate the turn image
			else if( walk_count >= 2 )
			{
				if( m_velx < 0 )
				{
					m_rot_y = 180;
				}
				else
				{
					m_rot_y = 0;
				}
			}
		}
	}
	// standing shell
	else if( turtle_state == TURTLE_SHELL_STAND )
	{
		counter += pFramerate->speedfactor;

		// waiting
		if( counter < 160 )
		{
			Set_Image_Num( 5 ); // front
		}
		else
		{
			// animation
			if( counter < 192 )
			{
				if( static_cast<int>(counter) % 5 == 1 )
				{
					Set_Image_Num( 9 ); // active
				}
				else
				{
					Set_Image_Num( 5 ); // front
				}
			}
			// activate
			else
			{
				counter = 0;
				Stand_Up();
			}
		}


		if( m_velx != 0 )
		{
			Add_Velocity( -m_velx * 0.2f, 0 );

			if( m_velx < 0.3f && m_velx > -0.3f )
			{
				m_velx = 0;
			}
		}
	}
	// moving shell
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		walk_count += pFramerate->speedfactor * 0.4f;

		if( walk_count >= 3 )
		{
			walk_count = 0;
		}

		Set_Image_Num( 6 + static_cast<int>(walk_count) );
	}

	if( playercounter > 0 )
	{
		playercounter -= pFramerate->speedfactor;

		if( playercounter <= 0 )
		{
			// do not start collision detection if in maryo
			if( pPlayer->m_col_rect.Intersects( m_col_rect ) )
			{
				playercounter = 5;
			}
			else
			{
				playercounter = 0;
			}
		}
	}

	// gravity
	Update_Gravity();
}

void cTurtle :: Stand_Up( void )
{
	if( turtle_state != TURTLE_SHELL_STAND && turtle_state != TURTLE_SHELL_RUN )
	{
		return;
	}

	// get space needed to stand up
	float move_y = m_image->m_col_h - m_images[0]->m_col_h;

	cObjectCollisionType *col_list = Collision_Check_Relative( 0, move_y, 0, 0, COLLIDE_ONLY_BLOCKING );

	// failed to stand up because something is blocking
	if( !col_list->empty() )
	{
		delete col_list;
		return;
	}

	delete col_list;

	turtle_state = TURTLE_WALK;
	m_state = STA_WALK;

	if( m_direction == DIR_RIGHT )
	{
		m_velx = speed_walk;
	}
	else
	{
		m_velx = -speed_walk;
	}

	Update_Direction();
	Update_Rotation_Hor_velx();

	pAudio->Play_Sound( "enemy/turtle/stand_up.wav" );

	Col_Move( 0, move_y, 1, 1 );

	// set walking image
	Set_Image_Num( static_cast<int>(walk_count) );
}

bool cTurtle :: Hit_Enemy( cEnemy *enemy ) const
{
	// invalid
	if( !enemy )
	{
		return 0;
	}

	// don't collide with already dead enemies
	if( enemy->dead )
	{
		return 0;
	}

	// shells can't hit these bosses
	if( enemy->m_type == TYPE_TURTLE_BOSS )
	{
		return 0;
	}
	if( enemy->m_type == TYPE_FURBALL_BOSS )
	{
		return 0;
	}

	// if red shell check if colliding turtle is green and also running
	if( color_type == COL_RED && enemy->m_type == TYPE_TURTLE )
	{
		cTurtle *turtle = static_cast<cTurtle *>(enemy);

		// red shell can't kill the green shell
		if( turtle->color_type == COL_GREEN && turtle->m_state == STA_RUN )
		{
			return 0;
		}
	}

	// hit enemy
	pAudio->Play_Sound( enemy->kill_sound );
	pHud_Points->Add_Points( enemy->kill_points, m_pos_x + m_image->m_w / 3, m_pos_y - 5.0f, "", static_cast<Uint8>(255), 1 );
	enemy->DownGrade( 1 );
	pPlayer->Add_Kill_Multiplier();

	return 1;
}

bool cTurtle :: Is_Update_Valid( void )
{
	if( dead || m_freeze_counter || m_state == STA_OBJ_LINKED )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cTurtle :: Validate_Collision( cSprite *obj )
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
		if( obj->m_type == TYPE_PLAYER )
		{
			// player is invincible
			if( pPlayer->invincible )
			{
				return COL_VTYPE_NOT_VALID;
			}
			// player counter is active
			if( turtle_state == TURTLE_SHELL_RUN && playercounter > 0.0f )
			{
				return COL_VTYPE_NOT_VALID;
			}
		}
		else if( obj->m_sprite_array == ARRAY_ENEMY )
		{
			if( obj->m_type == TYPE_FLYON )
			{
				// if walking
				if( turtle_state == TURTLE_WALK )
				{
					return COL_VTYPE_NOT_VALID;
				}
				// shell
				if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
				{
					return COL_VTYPE_INTERNAL;
				}
			}
			if( obj->m_type == TYPE_ROKKO )
			{
				return COL_VTYPE_NOT_VALID;
			}
			if( obj->m_type == TYPE_GEE )
			{
				if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
				{
					return COL_VTYPE_INTERNAL;
				}

				return COL_VTYPE_NOT_VALID;
			}
			if( obj->m_type == TYPE_STATIC_ENEMY )
			{
				return COL_VTYPE_NOT_VALID;
			}

			// if moving shell don't collide with enemies
			if( turtle_state == TURTLE_SHELL_RUN )
			{
				return COL_VTYPE_INTERNAL;
			}
		}

		return COL_VTYPE_BLOCKING;
	}
	else if( obj->m_massive_type == MASS_PASSIVE )
	{
		if( obj->m_type == TYPE_ENEMY_STOPPER )
		{
			if( turtle_state == TURTLE_WALK )
			{
				return COL_VTYPE_BLOCKING;
			}

			return COL_VTYPE_NOT_VALID;
		}
		else if( obj->m_type == TYPE_BONUSBOX || obj->m_type == TYPE_SPINBOX )
		{
			// if shell
			if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
			{
				cBaseBox *box = static_cast<cBaseBox *>(obj);

				// invisible semi massive
				if( box->box_invisible == BOX_INVISIBLE_SEMI_MASSIVE )
				{
					// if moving upwards and the object is on top
					if( m_vely <= 0.0f && obj->Is_On_Top( this ) )
					{
						return COL_VTYPE_BLOCKING;
					}
				}
			}
		}
	}
	else if( obj->m_massive_type == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( m_vely >= 0.0f && Is_On_Top( obj ) )
		{
			return COL_VTYPE_BLOCKING;
		}
	}

	return COL_VTYPE_NOT_VALID;
}

void cTurtle :: Handle_Collision_Player( cObjectCollision *collision )
{
	if( collision->direction == DIR_UNDEFINED || ( turtle_state == TURTLE_SHELL_RUN && playercounter > 0 ) || m_state == STA_OBJ_LINKED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->m_state != STA_FLY )
	{
		if( turtle_state == TURTLE_WALK )
		{
			pHud_Points->Add_Points( 25, pPlayer->m_pos_x, pPlayer->m_pos_y );
			pAudio->Play_Sound( "enemy/turtle/hit.ogg" );
		}
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			pHud_Points->Add_Points( 10, pPlayer->m_pos_x, pPlayer->m_pos_y );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			pHud_Points->Add_Points( 5, pPlayer->m_pos_x, pPlayer->m_pos_y );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		Generate_Hit_Animation( anim );
		anim->Set_Speed( 4, 0.8f );
		anim->Set_Scale( 0.6f );
		// add animation
		pAnimation_Manager->Add( anim );

		DownGrade();

		// if now running
		if( turtle_state == TURTLE_SHELL_RUN )
		{
			// if player is on the left side
			if( ( pPlayer->m_col_rect.m_w / 2 ) + pPlayer->m_pos_x < ( m_col_rect.m_w / 2 ) + m_pos_x )
			{
				m_velx = speed_shell;
			}
			// on the right side
			else
			{
				m_velx = -speed_shell;
			}

			Update_Direction();
		}

		pPlayer->Action_Jump( 1 );
	}
	else
	{
		if( turtle_state == TURTLE_WALK )
		{
			pPlayer->DownGrade_Player();
			Turn_Around( collision->direction );
		}
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
			DownGrade();

			cParticle_Emitter *anim = NULL;
			if( collision->direction == DIR_RIGHT )
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( m_pos_x + m_col_pos.m_x + m_col_rect.m_w, m_pos_y + ( m_col_rect.m_h / 2 ) );
				anim->Set_Direction_Range( 90, 180 );
				m_velx = -speed_shell;
			}
			else if( collision->direction == DIR_LEFT )
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( m_pos_x, m_pos_y + ( m_col_rect.m_h / 2 ) );
				anim->Set_Direction_Range( 270, 180 );
				m_velx = speed_shell;
			}
			else
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( m_pos_x + ( m_col_rect.m_w / 2 ), m_pos_y + m_col_pos.m_y + m_col_rect.m_h );
				anim->Set_Direction_Range( 180, 180 );

				// if player is on the left side
				if( ( pPlayer->m_col_rect.m_w / 2 ) + pPlayer->m_pos_x < ( m_col_rect.m_w / 2 ) + m_pos_x )
				{
					m_velx = speed_shell;
				}
				// on the right side
				else
				{
					m_velx = -speed_shell;
				}
			}
			
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			anim->Set_Quota( 4 );
			anim->Set_Pos_Z( m_pos_z + 0.0001f );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Speed( 4, 0.5f );
			anim->Set_Scale( 0.6f );
			anim->Set_Fading_Size( 1 );
			anim->Set_Color( Color( static_cast<Uint8>(254), 200, 100 ) );
			pAnimation_Manager->Add( anim );

			Update_Direction();

			playercounter = speedfactor_fps * 0.13f;

			// small upwards kick
			if( collision->direction == DIR_BOTTOM )
			{
				m_vely = -5.0f + (pPlayer->m_vely * 0.3f);
			}
		}
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			// bottom kicks upwards
			if( collision->direction == DIR_BOTTOM )
			{
				// small upwards kick
				if( collision->direction == DIR_BOTTOM )
				{
					m_vely = -5.0f + (pPlayer->m_vely * 0.3f);
				}
			}
			// other directions downgrade
			else
			{
				pPlayer->DownGrade_Player();
				Turn_Around( collision->direction );
			}
		}
	}
}

void cTurtle :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	if( turtle_state == TURTLE_SHELL_STAND )
	{
		cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

		// if able to collide
		if( m_state == STA_OBJ_LINKED || m_vely < -5.0f )
		{
			Hit_Enemy( enemy );
			DownGrade( 1 );
		}
	}
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

		Hit_Enemy( enemy );

		// create animation
		cParticle_Emitter *anim = new cParticle_Emitter();

		anim->Set_Emitter_Rect( m_col_rect.m_x + ( m_col_rect.m_w * 0.2f ), m_col_rect.m_y + ( m_col_rect.m_h * 0.2f ), m_col_rect.m_w * 0.6f, m_col_rect.m_h * 0.8f );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Quota( 5 );
		anim->Set_Pos_Z( m_pos_z + 0.000001f );
		anim->Set_Time_to_Live( 0.3f );
		anim->Set_Speed( 1.2f, 0.8f );
		anim->Set_Scale( 0.7f );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Blending( BLEND_ADD );

		if( collision->direction == DIR_RIGHT )
		{
			anim->Set_Direction_Range( 0.0f );
		}
		else if( collision->direction == DIR_LEFT )
		{
			anim->Set_Direction_Range( 180.0f );
		}
		
		// add animation
		pAnimation_Manager->Add( anim );
	}
	else if( turtle_state == TURTLE_WALK )
	{
		Turn_Around( collision->direction );
		Send_Collision( collision );
	}
}

void cTurtle :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( turtle_state == TURTLE_WALK )
	{
		Send_Collision( collision );
	}
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			cSprite *col_object = pActive_Sprite_Manager->Get_Pointer( collision->number );

			// animation
			cParticle_Emitter *anim = NULL;
			if( collision->direction == DIR_RIGHT )
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( col_object->m_pos_x + col_object->m_col_pos.m_x + 4, m_pos_y + ( m_col_rect.m_h / 1.35f ) );
				anim->Set_Direction_Range( 140, 100 );
			}
			else
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( col_object->m_pos_x + col_object->m_col_pos.m_x + col_object->m_col_rect.m_w - 4, m_pos_y + ( m_col_rect.m_h / 1.35f ) );
				anim->Set_Direction_Range( 320, 100 );
			}

			anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke.png" ) );
			anim->Set_Quota( 5 );
			anim->Set_Pos_Z( col_object->m_pos_z - 0.0001f, 0.0002f );
			anim->Set_Time_to_Live( 0.2f, 0.2f );
			anim->Set_Speed( 1, 1 );
			anim->Set_Scale( 0.5f, 0.4f );
			pAnimation_Manager->Add( anim );
		}

		// active object collision
		if( collision->m_array == ARRAY_ACTIVE )
		{
			Send_Collision( collision );
		}
	}
	else if( turtle_state == TURTLE_SHELL_STAND )
	{
		// if able to collide
		if( m_state == STA_OBJ_LINKED || m_vely < -5 )
		{
			// active object box collision
			if( collision->m_array == ARRAY_ACTIVE )
			{
				// get colliding object
				cSprite *col_object = pActive_Sprite_Manager->Get_Pointer( collision->number );

				if( col_object->m_type == TYPE_BONUSBOX || col_object->m_type == TYPE_SPINBOX )
				{
					// get basebox
					cBaseBox *box = static_cast<cBaseBox *>(col_object);

					// if useable
					if( box->useable_count != 0 )
					{
						pHud_Points->Add_Points( 50, m_pos_x + m_image->m_w / 3, m_pos_y - 5 );
						Send_Collision( collision );
						DownGrade( 1 );
						return;
					}
				}
			}
		}
	}

	if( m_state == STA_OBJ_LINKED )
	{
		return;
	}
	
	if( collision->direction == DIR_TOP )
	{
		if( m_vely > 0.0f )
		{
			m_vely = 0;
		}
	}
	else if( collision->direction == DIR_BOTTOM )
	{
		if( m_vely < 0.0f )
		{
			m_vely = 0;
		}
	}
	else
	{
		Turn_Around( collision->direction );
	}
}

void cTurtle :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_turtle_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Starting direction."), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( m_start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cTurtle::Editor_Direction_Select, this ) );

	// init
	Editor_Init();
}

bool cTurtle :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ), 1 );

	return 1;
}

void cTurtle :: Create_Name( void )
{
	m_name = "Turtle ";
	m_name += _(Get_Color_Name( color_type ).c_str());
	m_name += " ";
	m_name += _(Get_Direction_Name( m_start_direction ).c_str());
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
