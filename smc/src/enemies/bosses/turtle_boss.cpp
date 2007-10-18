/***************************************************************************
           turtle_boss.cpp  -  turtle enemy class
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
							(C) 2006 by Tobias Maasland
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
#include "../../core/sprite_manager.h"
#include "../../gui/hud.h"
#include "../../video/gl_surface.h"

/* *** *** *** *** *** *** cTurtleBoss *** *** *** *** *** *** *** *** *** *** *** */

cTurtleBoss :: cTurtleBoss( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cTurtleBoss :: cTurtleBoss( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
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
	max_downgrade_time = 2.5f;

	run_time = 0;
}

cTurtleBoss *cTurtleBoss :: Copy( void )
{
	cTurtleBoss *turtle = new cTurtleBoss( startposx, startposy );
	turtle->Set_Direction( start_direction, 1 );
	turtle->Set_Color( color_type );
	turtle->Set_Max_Hits( max_hits );
	turtle->Set_Max_Downgrade_Counts( max_downgrade_counts );
	turtle->Set_Max_Downgrade_Time( max_downgrade_time );

	return turtle;
}

void cTurtleBoss :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ), 1 );
	// color
	Set_Color( (DefaultColor)Get_Color_id( attributes.getValueAsString( "color", Get_Color_name( color_type ) ).c_str() ) );

	// boss data
	Set_Max_Hits( attributes.getValueAsInteger( "max_hit_count", max_hits ) );
	Set_Max_Downgrade_Counts( attributes.getValueAsInteger( "max_downgrade_count", max_downgrade_counts ) );
	Set_Max_Downgrade_Time( attributes.getValueAsFloat( "max_downgrade_time", max_downgrade_time ) );
}

void cTurtleBoss :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"turtleboss\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	// boss data
	file << "\t\t<Property name=\"max_hit_count\" value=\"" << max_hits << "\" />" << std::endl;
	file << "\t\t<Property name=\"max_downgrade_count\" value=\"" << max_downgrade_counts << "\" />" << std::endl;
	file << "\t\t<Property name=\"max_downgrade_time\" value=\"" << max_downgrade_time << "\" />" << std::endl;

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

void cTurtleBoss :: Set_Max_Downgrade_Time( float nmax_downgrade_time )
{
	max_downgrade_time = nmax_downgrade_time;

	if( max_downgrade_time < 0 )
	{
		max_downgrade_time = 0;
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

	Update_Rotation_velx( new_start_direction );

	// angry turtle
	if( turtle_state == TURTLEBOSS_STAND_ANGRY )
	{
		velx = 0;
	}

	if( new_start_direction )
	{
		Create_Name();
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

	Update_Rotation_velx();
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
		    if( hits == max_hits )
		    {
                counter = 0;

                hits = 0;
                downgrade_counts++;

				// die
                if( downgrade_counts == max_downgrade_counts )
                {
					// fade out music
					pAudio->FadeOutMusic( 500 );
					// play finish music
					pAudio->PlayMusic( "game/courseclear.ogg", 0, 0 );

					// set shell image
					Set_Image( 5 );

					// death state
					Set_Dead( 1 );
					massivetype = MASS_PASSIVE;
					counter = 8;
					velx = 0;
					vely = 0;
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
			Update_Rotation_velx();
		}
	}
	// falling death
	else
	{
		Set_Dead( 1 );
		counter = 8;
		massivetype = MASS_PASSIVE;

		if( turtle_state == TURTLEBOSS_WALK )
		{
			Move( 0, images[0]->h - images[5]->h, 1 );
		}

		Set_Image( 5 );
	}
}

void cTurtleBoss :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.5f;

	// if not below the screen
	if( posy < GAME_RES_H + col_rect.h )
	{
		if( scalex > 0.1f )
		{
			float speed_x = pFramerate->speedfactor * 10;

			if( direction == DIR_LEFT )
			{
				speed_x *= -1;
			}

			Add_RotationZ( speed_x );
			Add_Scale( -pFramerate->speedfactor * 0.025f );

			// finished scale out animation
			if( scalex <= 0.1f )
			{
				// sound
				pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );

				// animation
				cParticleAnimation *anim = new cParticleAnimation( posx, posy );
				anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
				anim->Set_Quota( 30 );
				anim->Set_ZPos( posz + 0.000001f );
				anim->Set_ConstRotationZ( 0.9f, 12 );
				anim->Set_Time_to_Live( 1 );
				anim->Set_Speed( 1, 4 );
				anim->Set_Scale( 0.4f, 0.3f );
				anim->Set_Color( orange, Color( (Uint8)6, 60, 20 ) );
				anim->Set_Blending( BLEND_ADD );
				pAnimationManager->Add( anim );

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
				pPlayer->Goto_next_Level();
			}
		}
	}
	// if below disable
	else
	{
		state = STA_STAY;
		turtle_state = TURTLEBOSS_DEAD;
		Set_Visible( 0 );
		rotz = 0;
		velx = 0;
		vely = 0;
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

		if( walk_count >= 4 )
		{
			walk_count = 0;

			// if turn around image force the new
			if( curr_img == 4 )
			{
				Set_Image( (int)walk_count );
				Update_Rotation_velx();
			}
		}

		// if turn around image
		if( curr_img != 4 )
		{
			Set_Image( (int)walk_count );
		}
		else
		{
			// rotate the turn image
			if( walk_count >= 2 )
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
				if( (int)counter % 5 == 1 )
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
        run_time += pFramerate->speedfactor;
		// time = seconds since start
        float time = run_time / DESIRED_FPS;

        if( time > max_downgrade_time )
		{
		    run_time = 0;
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

		Set_Image( 6 + (int)walk_count );
	}
	else if( turtle_state == TURTLEBOSS_STAND_ANGRY )
	{
		counter += pFramerate->speedfactor;

		// angry
	    if( (int)counter % 2 == 1 )
        {
			// slowly fade to the color
			cMovingSprite::Set_Color( Color( (Uint8)255, 250 - (Uint8)( counter * 1.5f ), 250 - (Uint8)( counter * 4 ) ) );
        }
		// default
        else
        {
			cMovingSprite::Set_Color( white );
        }

		// randomize direction
		if( ( (int)counter / 10 ) % 2 == 1 )
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
			pAudio->PlaySound( "enemy/boss/turtle/shell_attack.ogg" );

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
	float ball_angle = -180;

	for( unsigned int i = 0; i < amount; i++ )
	{
		ball_angle += 180 / amount; // 10 * 18 = 180 degrees

		cBall *ball = new cBall( posx + col_rect.w / 2, posy + col_rect.h / 2, 0, 0, this, FIREBALL_EXPLOSION );
		ball->Set_Direction( ball_angle, 15 );
		ball->Col_Move( ball->velx * 2, ball->vely * 2, 1 );

		pSprite_Manager->Add( ball );
	}

	cFireAnimation *anim = new cFireAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 3 ), 10 );
	anim->Set_FadingSpeed( 0.15f );
	anim->Set_ZPos( posz + 0.000001f );
	pAnimationManager->Add( anim );
}

bool cTurtleBoss :: is_Update_valid( void )
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
	if( obj->type == TYPE_ENEMYSTOPPER )
	{
		if( turtle_state == TURTLEBOSS_WALK )
		{
			return 2;
		}

		return 0;
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
				pAudio->PlaySound( "enemy/boss/turtle/big_hit.ogg" );
			}
			else
			{
				pAudio->PlaySound( "enemy/turtle/hit.ogg" );
			}
		}
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
		{
			pointsdisplay->Add_Points( 100, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
		}
		else if( turtle_state == TURTLEBOSS_SHELL_RUN )
		{
			pointsdisplay->Add_Points( 50, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
		}

		// animation
		cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Speed( 4, 0.8f );
		anim->Set_Scale( 0.9f );
		// add animation
		pAnimationManager->Add( anim );

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
			pPlayer->DownGrade();
			Turn_Around( collision->direction );
		}
		else if( turtle_state == TURTLEBOSS_SHELL_STAND )
		{
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
			DownGrade();

			cParticleAnimation *anim = NULL;

			if( collision->direction == DIR_RIGHT )
			{
				anim = new cParticleAnimation( posx + col_pos.x + col_rect.w, posy + ( col_rect.h / 2 ) );
				anim->Set_DirectionRange( 90, 180 );
				velx = -speed_shell;
			}
			else if( collision->direction == DIR_LEFT )
			{
				anim = new cParticleAnimation( posx, posy + ( col_rect.h / 2 ) );
				anim->Set_DirectionRange( 270, 180 );
				velx = speed_shell;
			}
			else
			{
				anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + col_pos.y + col_rect.h );
				anim->Set_DirectionRange( 180, 180 );

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
			anim->Set_ZPos( posz + 0.0001f );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Speed( 4, 0.5f );
			anim->Set_Scale( 0.8f );
			anim->Set_Fading_Size( 1 );
			anim->Set_Color( Color( (Uint8)254, 200, 100 ) );
			pAnimationManager->Add( anim );
			
			Update_Direction();

			playercounter = DESIRED_FPS * 0.13f;

			// small upwards kick
			if( collision->direction == DIR_BOTTOM )
			{
				vely = -5 + pPlayer->vely / 3;
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
					vely = -5 + pPlayer->vely / 3;
				}
			}
			// other directions downgrade
			else
			{
				pPlayer->DownGrade();
				Turn_Around( collision->direction );
			}
		}
	}
}

void cTurtleBoss :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	cEnemy *enemy = static_cast<cEnemy *>(pSprite_Manager->Get_Pointer( collision->number ));

	if( turtle_state == TURTLEBOSS_SHELL_STAND )
	{
		// if able to collide
		if( state == STA_OBJ_LINKED || vely < -5 )
		{
			// don't collide with already dead enemies
			if( enemy->dead )
			{
				return;
			}

			// hit enemy
			pAudio->PlaySound( enemy->kill_sound );
			pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
			enemy->DownGrade( 1 );

			DownGrade( 1 );
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
		pAudio->PlaySound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
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
			pAudio->PlaySound( enemy->kill_sound );
			pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
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
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			// hack : disable turn image
			//Set_Image( 4 );

			//walk_count = 0;
			Update_Rotation_velx();
		}
	}
	else if( turtle_state == TURTLEBOSS_SHELL_RUN )
	{
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			cSprite *col_object = pSprite_Manager->Get_Pointer( collision->number );

			// animation
			cParticleAnimation *anim = NULL;
			if( collision->direction == DIR_RIGHT )
			{
				anim = new cParticleAnimation( col_object->posx + col_object->col_pos.x + 4, posy + ( col_rect.h / 1.35f ) );
				anim->Set_DirectionRange( 140, 100 );
			}
			else
			{
				anim = new cParticleAnimation( col_object->posx + col_object->col_pos.x + col_object->col_rect.w - 4, posy + ( col_rect.h / 1.35f ) );
				anim->Set_DirectionRange( 320, 100 );
			}

			anim->Set_Quota( 5 );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_ZPos( col_object->posz - 0.0001f, 0.0002f );
			anim->Set_Speed( 1, 1 );
			anim->Set_Scale( 0.5f, 0.4f );
			// add animation
			pAnimationManager->Add( anim );
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
				cSprite *col_object = pSprite_Manager->Get_Pointer( collision->number );

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
	WindowManager &wmgr = WindowManager::getSingleton();

	// max hits
	Editbox *editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_hits" );

	Editor_Add( editbox, 120 );
	editbox->setTooltipText( "Max Hits" );
	editbox->setText( int_to_string( max_hits ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cTurtleBoss::Editor_Max_Hits_Key, this ) );

	// max downgrades
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_downgrade_count" );
	editbox->setTooltipText( "Max Downgrades" );
	Editor_Add( editbox, 120 );

	editbox->setText( int_to_string( max_downgrade_counts ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cTurtleBoss::Editor_Max_Downgrade_Counts_Key, this ) );

	// max downgrade time
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_turtle_boss_max_downgrade_time" );
	editbox->setTooltipText( "Max Downgrade Time" );
	Editor_Add( editbox, 200 );

	editbox->setText( float_to_string( max_downgrade_time ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cTurtleBoss::Editor_Max_Downgrade_Time_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cTurtleBoss :: Editor_Max_Hits_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Hits( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Max_Downgrade_Counts_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Downgrade_Counts( string_to_int( str_text ) );

	return 1;
}

bool cTurtleBoss :: Editor_Max_Downgrade_Time_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Downgrade_Time( string_to_float( str_text ) );

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
	Update_Rotation_velx();

	pAudio->PlaySound( "enemy/boss/turtle/power_up.ogg" );

	if( curr_img != 4 )
	{
		Col_Move( 0, images[5]->col_h - images[0]->col_h, 1, 1 );
	}

	Set_Image( (int)walk_count );
}

void cTurtleBoss :: Create_Name( void )
{
	name = "TurtleBoss";
	name += " " + Get_Direction_name( start_direction );
}
