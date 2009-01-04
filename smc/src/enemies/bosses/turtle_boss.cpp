/***************************************************************************
 * turtle_boss.cpp  -  turtle enemy class
 *
 * Copyright (C) 2003 - 2008 Florian Richter
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
	cTurtleBoss::Create_from_Stream( attributes );
}

cTurtleBoss :: ~cTurtleBoss( void )
{
	//
}

void cTurtleBoss :: Init( void )
{
	type = TYPE_TURTLE_BOSS;
	posz = 0.092f;

	state = STA_WALK;
	turtle_state = TURTLEBOSS_WALK;	// Walking
	playercounter = 0;

	color_type = COL_DEFAULT;
	Set_Color( COL_RED );
	Set_Direction( DIR_RIGHT, 1 );

	fire_resistant = 1;
	ice_resistance = 1;
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
	cTurtleBoss *turtle = new cTurtleBoss( startposx, startposy );
	turtle->Set_Direction( start_direction, 1 );
	turtle->Set_Color( color_type );
	turtle->Set_Max_Hits( max_hits );
	turtle->Set_Max_Downgrade_Counts( max_downgrade_counts );
	turtle->Set_Shell_Time( shell_time );

	return turtle;
}

void cTurtleBoss :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ), 1 );
	// color
	Set_Color( static_cast<DefaultColor>(Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() )) );
	// max hits
	Set_Max_Hits( attributes.getValueAsInteger( "max_hit_count", max_hits ) );
	// max downgrade count
	Set_Max_Downgrade_Counts( attributes.getValueAsInteger( "max_downgrade_count", max_downgrade_counts ) );
	// shell time
	Set_Shell_Time( attributes.getValueAsFloat( "shell_time", shell_time ) );
}

void cTurtleBoss :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"turtleboss\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
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

void cTurtleBoss :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	if( dir != DIR_RIGHT && dir != DIR_LEFT )
	{
		printf( "Warning : Unknown Turtle direction set %d\n", dir );
		return;
	}

	cEnemy::Set_Direction( dir, new_start_direction );

	if( direction == DIR_RIGHT )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			velx = speed_walk;
		}
		else
		{
			velx = speed_shell;
		}
	}
	else
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			velx = -speed_walk;
		}
		else
		{
			velx = -speed_shell;
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
		velx = 0;
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
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_0.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_1.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_2.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/walk_1.png" ) );
		// Walk Turn
		//images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/turn_1.png" ) );
		images.push_back( NULL );
		// Shell
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_front.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_1.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_2.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_move_3.png" ) );
		images.push_back( pVideo->Get_Surface( "enemy/bosses/turtle/shell_active.png" ) );

		speed_walk = 3.6f;
		speed_shell = 14;
		kill_points = 750;
	}
	// unknown color
	else
	{
		printf( "Error : Unknown TurtleBoss color : %d\n", color_type );
	}

	Set_Image( 0, 1 );
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

            if( direction == DIR_RIGHT )
            {
                velx = speed_walk;
            }
            else
            {
                velx = -speed_walk;
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
                    state = STA_STAY;
                    turtle_state = TURTLEBOSS_STAND_ANGRY;

                    speed_shell += 3;
                    velx = 0;
                }
		    }
		}
		// staying
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
		{
			counter = 0;
			state = STA_RUN;
			turtle_state = TURTLEBOSS_SHELL_RUN;
			player_range = 5000;

			// move into random direction
			if( rand() % 2 != 1 )
			{
				// right
				velx = speed_shell;
			}
			else
			{
				// left
				velx = -speed_shell;
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
			Move( 0, images[0]->h - images[5]->h, 1 );
		}
	}

	if( dead )
	{
		// fade out music
		pAudio->Fadeout_Music( 500 );
		// play finish music
		pAudio->Play_Music( "game/courseclear.ogg", 0, 0 );

		// set shell image
		Set_Image( 5 );

		massivetype = MASS_PASSIVE;
		counter = 8;
		velx = 0;
		vely = 0;
	}
}

void cTurtleBoss :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.5f;

	if( scalex > 0.1f )
	{
		float speed_x = pFramerate->speedfactor * 10;

		if( direction == DIR_LEFT )
		{
			speed_x *= -1;
		}

		Add_Rotation_Z( speed_x );
		Add_Scale( -pFramerate->speedfactor * 0.025f );

		// finished scale out animation
		if( scalex <= 0.1f )
		{
			// sound
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );

			// animation
			cParticle_Emitter *anim = new cParticle_Emitter();
			anim->Set_Pos( posx, posy );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
			anim->Set_Quota( 30 );
			anim->Set_Pos_Z( posz + 0.000001f );
			anim->Set_Const_Rotation_Z( -6, 12 );
			anim->Set_Time_to_Live( 1 );
			anim->Set_Speed( 1, 4 );
			anim->Set_Scale( 0.4f, 0.3f );
			anim->Set_Color( orange, Color( static_cast<Uint8>(6), 60, 20 ) );
			anim->Set_Blending( BLEND_ADD );
			pAnimation_Manager->Add( anim );

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
		if( counter > 25 )
		{
			// next level
			pPlayer->Goto_Next_Level();
		}
	}
}

void cTurtleBoss :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// walking
	if( turtle_state == TURTLEBOSS_WALK )
	{
		walk_count += pFramerate->speedfactor * 0.3f;

		// if not turn around image
		if( curr_img != 4 )
		{
			if( walk_count >= 4 )
			{
				walk_count = 0;
			}

			Set_Image( static_cast<int>(walk_count) );
		}
		// turn around image
		else
		{
			// force the new image
			if( walk_count >= 4 )
			{
				Set_Image( static_cast<int>(walk_count) );
				Update_Rotation_Hor_velx();
			}
			// rotate the turn image
			else if( walk_count >= 2 )
			{
				if( velx < 0 )
				{
					roty = 180;
				}
				else
				{
					roty = 0;
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
			Set_Image( 5 ); // front
		}
		else
		{
			// animation
			if( counter < 192 )
			{
				if( static_cast<int>(counter) % 5 == 1 )
				{
					Set_Image( 9 ); // active
				}
				else
				{
					Set_Image( 5 ); // front
				}
			}
			// activate
			else
			{
				counter = 0;
                Activate_Turtle();
			}
		}


		if( velx != 0 )
		{
			Add_Velocity( -velx * 0.2f, 0 );

			if( velx < 0.3f && velx > -0.3f )
			{
				velx = 0;
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
            Activate_Turtle();

            return;
        }

		walk_count += pFramerate->speedfactor * 0.4f;

		if( playercounter > 0 )
		{
			playercounter -= pFramerate->speedfactor;

			if( playercounter <= 0 )
			{
				// do not start collision detection if in maryo
				if( Col_Box( &pPlayer->col_rect, &col_rect ) )
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

		Set_Image( 6 + static_cast<int>(walk_count) );
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
			if( direction == DIR_RIGHT )
			{
				Set_Direction( DIR_LEFT );
			}
		}
		else
		{
			if( direction == DIR_LEFT )
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
		    state = STA_RUN;

			// shell attack sound
			pAudio->Play_Sound( "enemy/boss/turtle/shell_attack.ogg" );

			// throw more fireballs with downgrades
		    Throw_Fireballs( 6 + ( downgrade_counts * 2 ) );

            Col_Move( 0, images[0]->col_h - images[5]->col_h, 1, 1 );
            Set_Image( 5 );

			if( direction == DIR_RIGHT )
			{
				velx = speed_shell;
			}
			else
			{
				velx = -speed_shell;
			}
		}
	}

	// gravity
	Update_Gravity();
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

		cBall *ball = new cBall( posx + col_rect.w / 2, posy + col_rect.h / 2, this, FIREBALL_EXPLOSION );
		ball->Set_Direction( ball_angle, 15 );
		ball->Col_Move( ball->velx * 2, ball->vely * 2, 1 );
		pActive_Sprite_Manager->Add( ball );
	}

	cAnimation_Fireball *anim = new cAnimation_Fireball( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 3 ), 10 );
	anim->Set_Fading_Speed( 0.15f );
	anim->Set_Pos_Z( posz + 0.000001f );
	pAnimation_Manager->Add( anim );
}

bool cTurtleBoss :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cTurtleBoss :: Validate_Collision( cSprite *obj )
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
		if( obj->type == TYPE_PLAYER )
		{
			// player is invincible
			if( pPlayer->invincible )
			{
				return 0;
			}
			// player counter is active
			if( turtle_state == TURTLEBOSS_SHELL_RUN && playercounter > 0 )
			{
				return 0;
			}
		}
		else if( obj->sprite_array == ARRAY_ENEMY )
		{
			if( obj->type == TYPE_JPIRANHA )
			{
				// if walking
				if( turtle_state == TURTLEBOSS_WALK )
				{
					return 0;
				}
				// shell
				if( turtle_state == TURTLEBOSS_SHELL_STAND || turtle_state == TURTLEBOSS_SHELL_RUN )
				{
					return 1;
				}
			}
			if( obj->type == TYPE_ROKKO )
			{
				return 0;
			}
			if( obj->type == TYPE_GEE )
			{
				if( turtle_state == TURTLEBOSS_SHELL_STAND || turtle_state == TURTLEBOSS_SHELL_RUN )
				{
					return 1;
				}

				return 0;
			}
			if( obj->type == TYPE_STATIC_ENEMY )
			{
				return 0;
			}

			// if moving shell don't collide with enemies
			if( turtle_state == TURTLEBOSS_SHELL_RUN )
			{
				return 1;
			}
		}
		// ignore balls
		else if( obj->type == TYPE_BALL )
		{
		    return 0;
		}

		return 2;
	}
	if( obj->type == TYPE_ENEMY_STOPPER )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			return 2;
		}

		return 0;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and the object is on bottom
		if( vely >= 0 && Is_on_Top( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cTurtleBoss :: Handle_Collision_Player( cObjectCollision *collision )
{
	if( collision->direction == DIR_UNDEFINED || ( turtle_state == TURTLEBOSS_SHELL_RUN && playercounter > 0 ) || state == STA_OBJ_LINKED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			pointsdisplay->Add_Points( 250, pPlayer->posx, pPlayer->posy );

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
			pointsdisplay->Add_Points( 100, pPlayer->posx, pPlayer->posy );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}
		else if( turtle_state == TURTLEBOSS_SHELL_RUN )
		{
			pointsdisplay->Add_Points( 50, pPlayer->posx, pPlayer->posy );
			pAudio->Play_Sound( "enemy/turtle/shell/hit.ogg" );
		}

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
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
			if( ( pPlayer->col_rect.w / 2 ) + pPlayer->posx < ( col_rect.w / 2 ) + posx )
			{
				velx = speed_shell;
			}
			// on the right side
			else
			{
				velx = -speed_shell;
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
				anim->Set_Pos( posx + col_pos.x + col_rect.w, posy + ( col_rect.h / 2 ) );
				anim->Set_Direction_Range( 90, 180 );
				velx = -speed_shell;
			}
			else if( collision->direction == DIR_LEFT )
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( posx, posy + ( col_rect.h / 2 ) );
				anim->Set_Direction_Range( 270, 180 );
				velx = speed_shell;
			}
			else
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + col_pos.y + col_rect.h );
				anim->Set_Direction_Range( 180, 180 );

				// if player is on the left side
				if( ( pPlayer->col_rect.w / 2 ) + pPlayer->posx < ( col_rect.w / 2 ) + posx )
				{
					velx = speed_shell;
				}
				// on the right side
				else
				{
					velx = -speed_shell;
				}
			}

			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			anim->Set_Quota( 4 );
			anim->Set_Pos_Z( posz + 0.0001f );
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
				vely = -5 + (pPlayer->vely * 0.3f);
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
					vely = -5 + (pPlayer->vely * 0.3f);
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
		if( vely < -5 )
		{
			// don't collide with already dead enemies
			if( enemy->dead )
			{
				return;
			}

			// hit enemy
			pAudio->Play_Sound( enemy->kill_sound );
			pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
			enemy->DownGrade( 1 );

			pPlayer->Add_Kill_Multiplier();
		}
	}
	else if( turtle_state == TURTLEBOSS_SHELL_RUN )
	{
		// don't collide with already dead enemies
		if( enemy->dead )
		{
			return;
		}

		// hit enemy
		pAudio->Play_Sound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
		enemy->DownGrade( 1 );

		pPlayer->Add_Kill_Multiplier();
	}
	else if( turtle_state == TURTLEBOSS_WALK )
	{
		// turtle shell
		if( enemy->type == TYPE_TURTLE && enemy->state == STA_RUN )
		{
			// don't collide with already dead enemies
			if( enemy->dead )
			{
				return;
			}

			// hit enemy
			pAudio->Play_Sound( enemy->kill_sound );
			pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
			enemy->DownGrade( 1 );
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
				anim->Set_Pos( col_object->posx + col_object->col_pos.x + 4, posy + ( col_rect.h / 1.35f ) );
				anim->Set_Direction_Range( 140, 100 );
			}
			else
			{
				anim = new cParticle_Emitter();
				anim->Set_Pos( col_object->posx + col_object->col_pos.x + col_object->col_rect.w - 4, posy + ( col_rect.h / 1.35f ) );
				anim->Set_Direction_Range( 320, 100 );
			}

			anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke.png" ) );
			anim->Set_Quota( 5 );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Pos_Z( col_object->posz - 0.0001f, 0.0002f );
			anim->Set_Speed( 1, 1 );
			anim->Set_Scale( 0.5f, 0.4f );
			// add animation
			pAnimation_Manager->Add( anim );
		}

		// active object collision
		if( collision->type == CO_ACTIVE )
		{
			Send_Collision( collision );
		}
	}
	else if( turtle_state == TURTLEBOSS_SHELL_STAND )
	{
		// if able to collide
		if( vely < -5 )
		{
			// active object box collision
			if( collision->type == CO_ACTIVE )
			{
				// get colliding object
				cSprite *col_object = pActive_Sprite_Manager->Get_Pointer( collision->number );

				if( col_object->type == TYPE_BONUSBOX || col_object->type == TYPE_SPINBOX )
				{
					// get basebox
					cBaseBox *box = static_cast<cBaseBox *>(col_object);

					// if useable
					if( box->useable_count != 0 )
					{
						pointsdisplay->Add_Points( 50, posx + image->w / 3, posy - 5 );
						Send_Collision( collision );
						DownGrade( 1 );
					}
				}
			}
		}
	}
	
	if( collision->direction == DIR_TOP )
	{
		if( vely > 0 )
		{
			vely = 0;
		}
	}
	else if( collision->direction == DIR_BOTTOM )
	{
		if( vely < 0 )
		{
			vely = 0;
		}
	}
	else
	{
		Turn_Around( collision->direction );
	}
}

void cTurtleBoss :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_turtle_boss_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction"), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Direction_Select, this ) );

	// max hits
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_hits" ));
	Editor_Add( UTF8_("Hits"), UTF8_("Hits until a downgrade"), editbox, 120 );

	editbox->setText( int_to_string( max_hits ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Max_Hits_Key, this ) );

	// max downgrades
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_downgrade_count" ));
	Editor_Add( UTF8_("Downgrades"), UTF8_("Downgrades until death"), editbox, 120 );

	editbox->setText( int_to_string( max_downgrade_counts ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Max_Downgrade_Counts_Key, this ) );

	// max shell time
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_shell_time" ));
	Editor_Add( UTF8_("Shell Time"), UTF8_("Time running as shell to rise again"), editbox, 200 );

	editbox->setText( float_to_string( shell_time ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cTurtleBoss::Editor_Shell_Time_Key, this ) );

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

bool cTurtleBoss :: Editor_Max_Hits_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Hits( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Max_Downgrade_Counts_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Downgrade_Counts( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Shell_Time_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Shell_Time( string_to_float( str_text ) );

	return 1;
}

void cTurtleBoss :: Activate_Turtle( void )
{
	turtle_state = TURTLEBOSS_WALK;
	state = STA_WALK;

	if( direction == DIR_RIGHT )
	{
		velx = speed_walk;
	}
	else
	{
		velx = -speed_walk;
	}

	Update_Direction();
	Update_Rotation_Hor_velx();

	pAudio->Play_Sound( "enemy/boss/turtle/power_up.ogg" );

	if( curr_img != 4 )
	{
		Col_Move( 0, images[5]->col_h - images[0]->col_h, 1, 1 );
	}

	Set_Image( static_cast<int>(walk_count) );
}

void cTurtleBoss :: Create_Name( void )
{
	name = _("Turtle Boss");
	name += " " + Get_Direction_Name( start_direction );
}
