/***************************************************************************
 * turtle_boss.cpp  -  turtle boss enemy class
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

#include "../../enemies/bosses/turtle_boss.h"
#include "../../core/game_core.h"
#include "../../objects/box.h"
#include "../../video/animation.h"
#include "../../player/player.h"
#include "../../level/level.h"
#include "../../gui/hud.h"
#include "../../video/gl_surface.h"
#include "../../core/sprite_manager.h"
#include "../../core/i18n.h"
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

/* *** *** *** *** *** *** cTurtleBoss *** *** *** *** *** *** *** *** *** *** *** */

cTurtleBoss :: cTurtleBoss( float x, float y )
: cEnemy( x, y )
{
	cTurtleBoss::Init();
}

cTurtleBoss :: cTurtleBoss( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cTurtleBoss::Init();
	cTurtleBoss::Create_From_Stream( attributes );
}

cTurtleBoss :: ~cTurtleBoss( void )
{
	//
}

void cTurtleBoss :: Init( void )
{
	m_type = TYPE_TURTLE_BOSS;
	m_pos_z = 0.092f;

	m_state = STA_WALK;
	turtle_state = TURTLEBOSS_WALK;	// Walking
	playercounter = 0;

	color_type = COL_DEFAULT;
	Set_Color( COL_RED );
	Set_Direction( DIR_RIGHT, 1 );

	fire_resistant = 1;
	m_ice_resistance = 1;
	kill_sound = "stomp_4.ogg";

	hits = 0;
	downgrade_counts = 0;

	max_hits = 3;
	max_downgrade_counts = 3;
	shell_time = 2.5f;

	run_time_counter = 0;
}

cTurtleBoss *cTurtleBoss :: Copy( void )
{
	cTurtleBoss *turtle = new cTurtleBoss( m_start_pos_x, m_start_pos_y );
	turtle->Set_Direction( m_start_direction, 1 );
	turtle->Set_Color( color_type );
	turtle->Set_Max_Hits( max_hits );
	turtle->Set_Max_Downgrade_Counts( max_downgrade_counts );
	turtle->Set_Shell_Time( shell_time );

	return turtle;
}

void cTurtleBoss :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() )) );
	// max hits
	Set_Max_Hits( attributes.getValueAsInteger( "max_hit_count", max_hits ) );
	// max downgrade count
	Set_Max_Downgrade_Counts( attributes.getValueAsInteger( "max_downgrade_count", max_downgrade_counts ) );
	// shell time
	Set_Shell_Time( attributes.getValueAsFloat( "shell_time", shell_time ) );
}

void cTurtleBoss :: Save_To_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"turtleboss\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( m_start_direction ) << "\" />" << std::endl;
	// max hit count
	file << "\t\t<Property name=\"max_hit_count\" value=\"" << max_hits << "\" />" << std::endl;
	// max downgrade count
	file << "\t\t<Property name=\"max_downgrade_count\" value=\"" << max_downgrade_counts << "\" />" << std::endl;
	// shell time
	file << "\t\t<Property name=\"shell_time\" value=\"" << shell_time << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cTurtleBoss :: Set_Max_Hits( int nmax_hits )
{
	max_hits = nmax_hits;

	if( max_hits < 0 )
	{
		max_hits = 0;
	}
}

void cTurtleBoss :: Set_Max_Downgrade_Counts( int nmax_downgrade_counts )
{
	max_downgrade_counts = nmax_downgrade_counts;

	if( max_downgrade_counts < 0 )
	{
		max_downgrade_counts = 0;
	}
}

void cTurtleBoss :: Set_Shell_Time( float nmax_downgrade_time )
{
	shell_time = nmax_downgrade_time;

	if( shell_time < 0 )
	{
		shell_time = 0;
	}
}

void cTurtleBoss :: Set_Direction( const ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	if( dir != DIR_RIGHT && dir != DIR_LEFT )
	{
		printf( "Warning : Unknown Turtle direction set %d\n", dir );
		return;
	}

	cEnemy::Set_Direction( dir, new_start_direction );

	if( m_direction == DIR_RIGHT )
	{
		if( turtle_state == TURTLEBOSS_WALK )
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
		if( turtle_state == TURTLEBOSS_WALK )
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

	// if angry turtle
	if( turtle_state == TURTLEBOSS_STAND_ANGRY )
	{
		m_velx = 0;
	}
}

void cTurtleBoss :: Set_Color( DefaultColor col )
{
	// already set
	if( color_type == col )
	{
		return;
	}

	color_type = col;

	Clear_Images();

	if( color_type == COL_RED )
	{
		// Walk
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_0.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_1.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_2.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_1.png" ) );
		// Walk Turn
		//images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/turn_1.png" ) );
		m_images.push_back( NULL );
		// Shell
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_front.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_1.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_2.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_3.png" ) );
		m_images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_active.png" ) );

		speed_walk = 3.6f;
		speed_shell = 14;
		kill_points = 750;
	}
	// unknown color
	else
	{
		printf( "Error : Unknown TurtleBoss color : %d\n", color_type );
	}

	Set_Image_Num( 0, 1 );
}

void cTurtleBoss :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	cEnemy::Turn_Around( col_dir );

	if( turtle_state == TURTLEBOSS_WALK )
	{
		// hack : disable turn image
		//walk_count = 0;
		//Set_Image( 4 );
	}

	Update_Rotation_Hor_velx();
}

void cTurtleBoss :: DownGrade( bool force /* = 0 */ )
{
	if( !force )
	{
		// normal walking
		if( turtle_state == TURTLEBOSS_WALK )
		{
			hits++;

			speed_walk += 1.8f;

			if( m_direction == DIR_RIGHT )
			{
				m_velx = speed_walk;
			}
			else
			{
				m_velx = -speed_walk;
			}

			// state change
			if( hits >= max_hits )
			{
				counter = 0;

				hits = 0;
				downgrade_counts++;

				// die
				if( downgrade_counts == max_downgrade_counts )
				{
					Set_Dead( 1 );
				}
				// downgrade
				else
				{
					m_state = STA_STAY;
					turtle_state = TURTLEBOSS_STAND_ANGRY;

					speed_shell += 3;
					m_velx = 0;
				}
			}
		}
		// staying
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
		{
			counter = 0;
			m_state = STA_RUN;
			turtle_state = TURTLEBOSS_SHELL_RUN;
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
	}
	// falling death
	else
	{
		// die
		Set_Dead( 1 );

		if( turtle_state == TURTLEBOSS_WALK )
		{
			Move( 0, m_images[0]->m_h - m_images[5]->m_h, 1 );
		}
	}

	if( dead )
	{
		// fade out music
		pAudio->Fadeout_Music( 500 );
		// play finish music
		pAudio->Play_Music( "game/courseclear.ogg", 0, 0 );

		// set shell image
		Set_Image_Num( 5 );

		m_massive_type = MASS_PASSIVE;
		counter = 0;
		m_velx = 0;
		m_vely = 0;

		// set scaling for death animation
		Set_Scale_Affects_Rect( 1 );
	}
}

void cTurtleBoss :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.5f;

	if( m_scale_x > 0.1f )
	{
		float speed_x = pFramerate->speedfactor * 10;

		if( m_direction == DIR_LEFT )
		{
			speed_x *= -1;
		}

		Add_Rotation_Z( speed_x );
		Add_Scale( -pFramerate->speedfactor * 0.025f );

		// star animation
		if( counter >= 1 )
		{
			Generate_Stars( static_cast<unsigned int>(counter), 0.1f );
			counter -= static_cast<int>(counter);
		}

		// finished scale out animation
		if( m_scale_x <= 0.1f )
		{
			// sound
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );

			// star explosion animation
			Generate_Stars( 30 );

			// set empty image
			cMovingSprite::Set_Image( NULL, 0, 0 );
			// reset counter
			counter = 0;
		}
	}
	// after scale animation
	else
	{
		// wait some time
		if( counter > 20 )
		{
			Set_Active( 0 );
			turtle_state = TURTLEBOSS_DEAD;

			// next level
			pPlayer->Goto_Next_Level();

			// reset scaling
			Set_Scale_Affects_Rect( 0 );
		}
	}
}

void cTurtleBoss :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	// walking
	if( turtle_state == TURTLEBOSS_WALK )
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
			// force the new image
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
	else if( turtle_state == TURTLEBOSS_SHELL_STAND )
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
	else if( turtle_state == TURTLEBOSS_SHELL_RUN )
	{
		run_time_counter += pFramerate->speedfactor;
		// time = seconds since start
		float time = run_time_counter / speedfactor_fps;

		if( time > shell_time )
		{
			run_time_counter = 0;
			Stand_Up();

			return;
		}

		walk_count += pFramerate->speedfactor * 0.4f;

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

		if( walk_count >= 3 )
		{
			walk_count = 0;
		}

		Set_Image_Num( 6 + static_cast<int>(walk_count) );
	}
	else if( turtle_state == TURTLEBOSS_STAND_ANGRY )
	{
		counter += pFramerate->speedfactor;

		// angry
		if( static_cast<int>(counter) % 2 == 1 )
		{
			// slowly fade to the color
			cMovingSprite::Set_Color( Color( static_cast<Uint8>(255), 250 - static_cast<Uint8>( counter * 1.5f ), 250 - static_cast<Uint8>( counter * 4 ) ) );
		}
		// default
		else
		{
			cMovingSprite::Set_Color( white );
		}

		// randomize direction
		if( ( static_cast<int>(counter) / 10 ) % 2 == 1 )
		{
			if( m_direction == DIR_RIGHT )
			{
				Set_Direction( DIR_LEFT );
			}
		}
		else
		{
			if( m_direction == DIR_LEFT )
			{
				Set_Direction( DIR_RIGHT );
			}
		}

		// finished animation
		if( counter > 60 )
		{
			counter = 0;

			// running shell for some time
			turtle_state = TURTLEBOSS_SHELL_RUN;
			m_state = STA_RUN;

			// shell attack sound
			pAudio->Play_Sound( "enemy/boss/turtle/shell_attack.ogg" );

			// throw more fireballs with downgrades
			Throw_Fireballs( 6 + ( downgrade_counts * 2 ) );

			Col_Move( 0, m_images[0]->m_col_h - m_images[5]->m_col_h, 1, 1 );
			Set_Image_Num( 5 );

			if( m_direction == DIR_RIGHT )
			{
				m_velx = speed_shell;
			}
			else
			{
				m_velx = -speed_shell;
			}
		}
	}

	// gravity
	Update_Gravity();
}

void cTurtleBoss :: Stand_Up( void )
{
	if( turtle_state != TURTLEBOSS_SHELL_STAND && turtle_state != TURTLEBOSS_SHELL_RUN )
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

	turtle_state = TURTLEBOSS_WALK;
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

	pAudio->Play_Sound( "enemy/boss/turtle/power_up.ogg" );

	Col_Move( 0, move_y, 1, 1 );

	// set walking image
	Set_Image_Num( static_cast<int>(walk_count) );
}

bool cTurtleBoss :: Hit_Enemy( cEnemy *enemy ) const
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

	// hit enemy
	pAudio->Play_Sound( enemy->kill_sound );
	pHud_Points->Add_Points( enemy->kill_points, m_pos_x + m_image->m_w / 3, m_pos_y - 5, "", static_cast<Uint8>(255), 1 );
	enemy->DownGrade( 1 );
	pPlayer->Add_Kill_Multiplier();

	return 1;
}

void cTurtleBoss :: Throw_Fireballs( unsigned int amount /* = 6 */ )
{
	// start angle
	float ball_angle = -180;
	// move 180 degrees
	float step_size = 180 / static_cast<float>(amount);

	for( unsigned int i = 0; i < amount; i++ )
	{
		// add step size to angle
		ball_angle += step_size;

		cBall *ball = new cBall( m_pos_x + m_col_rect.m_w / 2, m_pos_y + m_col_rect.m_h / 2, this, FIREBALL_EXPLOSION );
		ball->Set_Direction( ball_angle, 15 );
		ball->Col_Move( ball->m_velx * 2, ball->m_vely * 2, 1 );
		pActive_Sprite_Manager->Add( ball );
	}

	cAnimation_Fireball *anim = new cAnimation_Fireball( m_pos_x + ( m_col_rect.m_w / 2 ), m_pos_y + ( m_col_rect.m_h / 3 ), 10 );
	anim->Set_Fading_Speed( 0.15f );
	anim->Set_Pos_Z( m_pos_z + 0.000001f );
	pAnimation_Manager->Add( anim );
}

void cTurtleBoss :: Generate_Stars( unsigned int amount /* = 1 */, float particle_scale /* = 0.4f */ ) const
{
	// animation
	cParticle_Emitter *anim = new cParticle_Emitter();
	anim->Set_Pos( m_pos_x + ( m_col_rect.m_w * 0.5f ), m_pos_y + ( m_col_rect.m_h * 0.5f ) );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
	anim->Set_Quota( amount );
	anim->Set_Pos_Z( m_pos_z + 0.000001f );
	anim->Set_Const_Rotation_Z( -6.0f, 12.0f );
	anim->Set_Time_to_Live( 1.0f );
	anim->Set_Speed( 1.0f, 4.0f );
	anim->Set_Scale( particle_scale, 0.3f );
	anim->Set_Color( orange, Color( static_cast<Uint8>(6), 60, 20, 0 ) );
	anim->Set_Blending( BLEND_ADD );
	pAnimation_Manager->Add( anim );
}

bool cTurtleBoss :: Is_Update_Valid( void )
{
	if( dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cTurtleBoss :: Validate_Collision( cSprite *obj )
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
			if( turtle_state == TURTLEBOSS_SHELL_RUN && playercounter > 0 )
			{
				return COL_VTYPE_NOT_VALID;
			}
		}
		else if( obj->m_sprite_array == ARRAY_ENEMY )
		{
			if( obj->m_type == TYPE_FLYON )
			{
				// if walking
				if( turtle_state == TURTLEBOSS_WALK )
				{
					return COL_VTYPE_NOT_VALID;
				}
				// shell
				if( turtle_state == TURTLEBOSS_SHELL_STAND || turtle_state == TURTLEBOSS_SHELL_RUN )
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
				if( turtle_state == TURTLEBOSS_SHELL_STAND || turtle_state == TURTLEBOSS_SHELL_RUN )
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
			if( turtle_state == TURTLEBOSS_SHELL_RUN )
			{
				return COL_VTYPE_INTERNAL;
			}
		}
		// ignore balls
		else if( obj->m_type == TYPE_BALL )
		{
			return COL_VTYPE_NOT_VALID;
		}

		return COL_VTYPE_BLOCKING;
	}
	if( obj->m_type == TYPE_ENEMY_STOPPER )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			return COL_VTYPE_BLOCKING;
		}

		return COL_VTYPE_NOT_VALID;
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

void cTurtleBoss :: Handle_Collision_Player( cObjectCollision *collision )
{
	if( collision->direction == DIR_UNDEFINED || ( turtle_state == TURTLEBOSS_SHELL_RUN && playercounter > 0 ) || m_state == STA_OBJ_LINKED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->m_state != STA_FLY )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			pHud_Points->Add_Points( 250, pPlayer->m_pos_x, pPlayer->m_pos_y );

			if( hits + 1 == max_hits )
			{
				pAudio->Play_Sound( "enemy/boss/turtle/big_hit.ogg" );
			}
			else
			{
				pAudio->Play_Sound( "enemy/boss/turtle/hit.ogg" );
			}
		}
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
		{
			pHud_Points->Add_Points( 100, pPlayer->m_pos_x, pPlayer->m_pos_y );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}
		else if( turtle_state == TURTLEBOSS_SHELL_RUN )
		{
			pHud_Points->Add_Points( 50, pPlayer->m_pos_x, pPlayer->m_pos_y );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( m_pos_x + ( m_col_rect.m_w / 2 ), m_pos_y + ( m_col_rect.m_h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Speed( 4, 0.8f );
		anim->Set_Scale( 0.9f );
		// add animation
		pAnimation_Manager->Add( anim );

		DownGrade();

		// if now running
		if( turtle_state == TURTLEBOSS_SHELL_RUN )
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
		if( turtle_state == TURTLEBOSS_WALK )
		{
			pPlayer->DownGrade_Player();
			Turn_Around( collision->direction );
		}
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
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
			anim->Set_Scale( 0.8f );
			anim->Set_Fading_Size( 1 );
			anim->Set_Color( Color( static_cast<Uint8>(254), 200, 100 ) );
			pAnimation_Manager->Add( anim );
			
			Update_Direction();

			playercounter = speedfactor_fps * 0.13f;

			// small upwards kick
			if( collision->direction == DIR_BOTTOM )
			{
				m_vely = -5 + (pPlayer->m_vely * 0.3f);
			}
		}
		else if( turtle_state == TURTLEBOSS_SHELL_RUN )
		{
			// bottom kicks upwards
			if( collision->direction == DIR_BOTTOM )
			{
				// small upwards kick
				if( collision->direction == DIR_BOTTOM )
				{
					m_vely = -5 + (pPlayer->m_vely * 0.3f);
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

void cTurtleBoss :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

	if( turtle_state == TURTLEBOSS_SHELL_STAND )
	{
		// if able to collide
		if( m_vely < -5 )
		{
			Hit_Enemy( enemy );
		}
	}
	else if( turtle_state == TURTLEBOSS_SHELL_RUN )
	{
		Hit_Enemy( enemy );
	}
	else if( turtle_state == TURTLEBOSS_WALK )
	{
		// turtle shell
		if( enemy->m_type == TYPE_TURTLE && enemy->m_state == STA_RUN )
		{
			Hit_Enemy( enemy );
		}
		else
		{
			Turn_Around( collision->direction );
			Send_Collision( collision );
		}
	}
}

void cTurtleBoss :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( turtle_state == TURTLEBOSS_WALK )
	{
		//
	}
	else if( turtle_state == TURTLEBOSS_SHELL_RUN )
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
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Pos_Z( col_object->m_pos_z - 0.0001f, 0.0002f );
			anim->Set_Speed( 1, 1 );
			anim->Set_Scale( 0.5f, 0.4f );
			// add animation
			pAnimation_Manager->Add( anim );
		}

		// active object collision
		if( collision->m_array == ARRAY_ACTIVE )
		{
			Send_Collision( collision );
		}
	}
	else if( turtle_state == TURTLEBOSS_SHELL_STAND )
	{
		// if able to collide
		if( m_vely < -5 )
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
					}
				}
			}
		}
	}
	
	if( collision->direction == DIR_TOP )
	{
		if( m_vely > 0 )
		{
			m_vely = 0;
		}
	}
	else if( collision->direction == DIR_BOTTOM )
	{
		if( m_vely < 0 )
		{
			m_vely = 0;
		}
	}
	else
	{
		Turn_Around( collision->direction );
	}
}

void cTurtleBoss :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_turtle_boss_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Starting direction."), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( m_start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Direction_Select, this ) );

	// max hits
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_hits" ));
	Editor_Add( UTF8_("Hits"), UTF8_("Hits until a downgrade"), editbox, 120 );

	editbox->setValidationString( "^[+]?\\d*$" );
	editbox->setText( int_to_string( max_hits ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Max_Hits_Text_Changed, this ) );

	// max downgrades
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_downgrade_count" ));
	Editor_Add( UTF8_("Downgrades"), UTF8_("Downgrades until death"), editbox, 120 );

	editbox->setValidationString( "^[+]?\\d*$" );
	editbox->setText( int_to_string( max_downgrade_counts ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Max_Downgrade_Counts_Text_Changed, this ) );

	// max shell time
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_shell_time" ));
	Editor_Add( UTF8_("Shell Time"), UTF8_("Time running as shell to rise again"), editbox, 200 );

	editbox->setValidationString( "[+]?[0-9]*\\.?[0-9]*" );
	editbox->setText( float_to_string( shell_time ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Shell_Time_Text_Changed, this ) );

	// init
	Editor_Init();
}

bool cTurtleBoss :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ), 1 );

	return 1;
}

bool cTurtleBoss :: Editor_Max_Hits_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Hits( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Max_Downgrade_Counts_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Downgrade_Counts( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Shell_Time_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Shell_Time( string_to_float( str_text ) );

	return 1;
}

void cTurtleBoss :: Create_Name( void )
{
	m_name = _("Turtle Boss");
	m_name += " " + Get_Direction_Name( m_start_direction );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
