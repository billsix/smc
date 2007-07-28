/***************************************************************************
           turtle.cpp  -  turtle enemy class
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
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
#include "../core/obj_manager.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** cTurtle *** *** *** *** *** *** *** *** *** *** *** */

cTurtle :: cTurtle( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cTurtle :: cTurtle( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cTurtle :: ~cTurtle( void )
{
	//
}

void cTurtle :: Init( void )
{
	type = TYPE_TURTLE;
	posz = 0.091f;

	state = STA_WALK;
	turtle_state = TURTLE_WALK;	// Walking
	playercounter = 0;

	color_type = COL_DEFAULT;
	Set_Color( COL_RED );
	Set_Direction( DIR_RIGHT, 1 );

	kill_sound = "stomp_4.ogg";
}

cTurtle *cTurtle :: Copy( void )
{
	cTurtle *turtle = new cTurtle( startposx, startposy );
	turtle->Set_Direction( start_direction, 1 );
	turtle->Set_Color( color_type );

	return turtle;
}

void cTurtle :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ), 1 );
	// color
	Set_Color( (DefaultColor)Get_Color_id( attributes.getValueAsString( "color", Get_Color_name( color_type ) ).c_str() ) );
}

void cTurtle :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"turtle\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cTurtle :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	if( dir != DIR_RIGHT && dir != DIR_LEFT )
	{
		printf( "Warning : Unknown Turtle direction set %d\n", dir );
		return;
	}

	cEnemy::Set_Direction( dir, new_start_direction );

	if( direction == DIR_RIGHT )
	{
		if( turtle_state == TURTLE_WALK )
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
		if( turtle_state == TURTLE_WALK )
		{
			velx = -speed_walk;
		}
		else
		{
			velx = -speed_shell;
		}
	}

	Update_Rotation_velx( new_start_direction );

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

	string filename_dir;

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
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_0.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/walk_1.png" ) );
	// Walk Turn
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/turn_1.png" ) );
	// Shell
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_front.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_move_3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/turtle/" + filename_dir + "/shell_active.png" ) );

	Set_Image( 0, 1 );
}

void cTurtle :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	cEnemy::Turn_Around( col_dir );

	Update_Rotation_velx();
}

void cTurtle :: DownGrade( bool force /* = 0 */ )
{
	if( !force )
	{
		// normal walking
		if( turtle_state == TURTLE_WALK )
		{
			counter = 0;
			state = STA_STAY;
			turtle_state = TURTLE_SHELL_STAND;

			Set_Image( 5 );
			Col_Move( 0, images[0]->col_h - images[5]->col_h, 1, 1 );
		}
		// staying
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			counter = 0;
			state = STA_RUN;
			turtle_state = TURTLE_SHELL_RUN;
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
		// running shell
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			counter = 0;
			state = STA_STAY;
			turtle_state = TURTLE_SHELL_STAND;
			player_range = 2000;
		}
	}
	// falling death
	else
	{
		counter = 8;
		Set_Dead( 1 );
		massivetype = MASS_PASSIVE;
		velx = 0;
		vely = 0;

		if( turtle_state == TURTLE_WALK )
		{
			Move( 0, images[0]->h - images[5]->h, 1 );
		}

		Set_Image( 5 );
	}
}

void cTurtle :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.5f;

	// if not below the screen fall
	if( posy < GAME_RES_H + col_rect.h )
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

		if( direction == DIR_LEFT )
		{
			speed_x *= -1;
		}

		Add_RotationZ( speed_x );

		Move( speed_x * 15, speed_y );
	}
	// if below disable
	else
	{
		Set_Visible( 0 );
		rotz = 0;
		state = STA_STAY;
		turtle_state = TURTLE_DEAD;
	}
}

void cTurtle :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// walking
	if( turtle_state == TURTLE_WALK )
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

		// if not turn around image
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
	else if( turtle_state == TURTLE_SHELL_STAND )
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
				turtle_state = TURTLE_WALK;
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

				// todo : create again
				//pAudio->PlaySound( "enemy/turtle/power_up.ogg" );

				if( curr_img > 4 )
				{
					Col_Move( 0, images[5]->col_h - images[0]->col_h, 1, 1 );
				}

				Set_Image( 0 ); // walk
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
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		walk_count += pFramerate->speedfactor * 0.4f;



		if( walk_count >= 3 )
		{
			walk_count = 0;
		}

		Set_Image( 6 + (int)walk_count );
	}

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

	// gravity
	Update_Gravity();
}

bool cTurtle :: is_Update_valid( void )
{
	if( dead || freeze_counter || state == STA_OBJ_LINKED )
	{
		return 0;
	}

	return 1;
}

unsigned int cTurtle :: Validate_Collision( cSprite *obj )
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
			if( turtle_state == TURTLE_SHELL_RUN && playercounter > 0 )
			{
				return 0;
			}
		}
		else if( obj->sprite_array == ARRAY_ENEMY )
		{
			if( obj->type == TYPE_JPIRANHA )
			{
				// if walking
				if( turtle_state == TURTLE_WALK )
				{
					return 0;
				}
				// shell
				if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
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
				if( turtle_state == TURTLE_SHELL_STAND || turtle_state == TURTLE_SHELL_RUN )
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
			if( turtle_state == TURTLE_SHELL_RUN )
			{
				return 1;
			}
		}

		return 2;
	}
	if( obj->type == TYPE_ENEMYSTOPPER )
	{
		if( turtle_state == TURTLE_WALK )
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

void cTurtle :: Handle_Collision_Player( ObjectDirection cdirection )
{
	if( cdirection == DIR_UNDEFINED || ( turtle_state == TURTLE_SHELL_RUN && playercounter > 0 ) || state == STA_OBJ_LINKED )
	{
		return;
	}

	if( cdirection == DIR_TOP && pPlayer->state != STA_FLY )
	{
		if( turtle_state == TURTLE_WALK )
		{
			pointsdisplay->Add_Points( 25, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( "enemy/turtle/hit.ogg" );
		}
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			pointsdisplay->Add_Points( 10, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
		}
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			pointsdisplay->Add_Points( 5, pPlayer->posx, pPlayer->posy );
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
		}

		// animation
		cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Speed( 4, 0.8f );
		anim->Set_Scale( 0.6f );
		// add animation
		pAnimationManager->Add( anim );

		DownGrade();

		// if now running
		if( turtle_state == TURTLE_SHELL_RUN )
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
		if( turtle_state == TURTLE_WALK )
		{
			pPlayer->DownGrade();
			Turn_Around( cdirection );
		}
		else if( turtle_state == TURTLE_SHELL_STAND )
		{
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
			DownGrade();

			cParticleAnimation *anim = NULL;
			if( cdirection == DIR_RIGHT )
			{
				anim = new cParticleAnimation( posx + col_pos.x + col_rect.w, posy + ( col_rect.h / 2 ) );
				anim->Set_DirectionRange( 90, 180 );
				velx = -speed_shell;
			}
			else if( cdirection == DIR_LEFT )
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
			anim->Set_Scale( 0.6f );
			anim->Set_Fading_Size( 1 );
			anim->Set_Color( Color( (Uint8)254, 200, 100 ) );
			pAnimationManager->Add( anim );

			Update_Direction();

			playercounter = DESIRED_FPS * 0.13f;

			// small upwards kick
			if( cdirection == DIR_BOTTOM )
			{
				vely = -5 + pPlayer->vely / 3;
			}
		}
		else if( turtle_state == TURTLE_SHELL_RUN )
		{
			// bottom kicks upwards
			if( cdirection == DIR_BOTTOM )
			{
				// small upwards kick
				if( cdirection == DIR_BOTTOM )
				{
					vely = -5 + pPlayer->vely / 3;
				}
			}
			// other directions downgrade
			else
			{
				pPlayer->DownGrade();
				Turn_Around( cdirection );
			}
		}
	}
}

void cTurtle :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	if( turtle_state == TURTLE_SHELL_STAND )
	{
		cEnemy *enemy = static_cast<cEnemy *>(pObjManager->Get_Pointer( collision->number ));

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
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		cEnemy *enemy = static_cast<cEnemy *>(pObjManager->Get_Pointer( collision->number ));

		
		// don't collide with already dead enemies
		if( enemy->dead )
		{
			return;
		}

		// Shells can't kill the boss
		if( enemy->type == TYPE_TURTLE_BOSS )
		{
		    return;
		}

		// if red shell check if colliding turtle is green and also running
		if( color_type == COL_RED && enemy->type == TYPE_TURTLE )
		{
			cTurtle *turtle = static_cast<cTurtle *>(enemy);

			// red shell can't kill green shells
			if( turtle->color_type == COL_GREEN && turtle->state == STA_RUN )
			{
				return;
			}
		}

		// hit enemy
		pAudio->PlaySound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
		enemy->DownGrade( 1 );

		// create animation
		cParticleAnimation *anim = new cParticleAnimation();

		anim->Set_Emitter_Rect( col_rect.x + ( col_rect.w * 0.2f ), col_rect.y + ( col_rect.h * 0.2f ), col_rect.w * 0.6f, col_rect.h * 0.8f );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Quota( 5 );
		anim->Set_ZPos( posz + 0.000001f );
		anim->Set_Time_to_Live( 0.3f );
		anim->Set_Speed( 1.2f, 0.8f );
		anim->Set_Scale( 0.7f );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Blending( BLEND_ADD );

		if( collision->direction == DIR_RIGHT )
		{
			anim->Set_DirectionRange( 0 );
		}
		else if( collision->direction == DIR_LEFT )
		{
			anim->Set_DirectionRange( 180 );
		}
		
		// add animation
		pAnimationManager->Add( anim );

		pPlayer->Add_Kill_Multiplier();
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
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			// hack : disable turn image
			//Set_Image( 4 );

			//walk_count = 0;
			Update_Rotation_velx();
		}
	}
	else if( turtle_state == TURTLE_SHELL_RUN )
	{
		if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
		{
			cSprite *col_object = pObjManager->Get_Pointer( collision->number );

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
			anim->Set_ZPos( col_object->posz - 0.0001f, 0.0002f );
			anim->Set_Time_to_Live( 0.2f, 0.2f );
			anim->Set_Speed( 1, 1 );
			anim->Set_Scale( 0.5f, 0.4f );
			pAnimationManager->Add( anim );
		}

		// active object collision
		if( collision->type == CO_ACTIVE )
		{
			Send_Collision( collision );
		}
	}
	else if( turtle_state == TURTLE_SHELL_STAND )
	{
		// if able to collide
		if( state == STA_OBJ_LINKED || vely < -5 )
		{
			// active object box collision
			if( collision->type == CO_ACTIVE )
			{
				// get colliding object
				cSprite *col_object = pObjManager->Get_Pointer( collision->number );

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
						return;
					}
				}
			}
		}
	}

	if( state == STA_OBJ_LINKED )
	{
		return;
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

void cTurtle :: Create_Name( void )
{
	name = "Turtle";
	name += " " + Get_Color_name( color_type );
	name += " " + Get_Direction_name( start_direction );
}
