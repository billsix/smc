/***************************************************************************
              jpiranha.cpp  -  jumping piranha plant
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../enemies/jpiranha.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../player/player.h"
#include "../core/obj_manager.h"
#include "../video/animation.h"

/* *** *** *** *** *** *** cjPiranha *** *** *** *** *** *** *** *** *** *** *** */

cjPiranha :: cjPiranha( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cjPiranha :: cjPiranha( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cjPiranha :: ~cjPiranha( void )
{
	//
}

void cjPiranha :: Init( void  )
{
	type = TYPE_JPIRANHA;
	posz = 0.06f;
	editor_posz = 0.089f;
	player_range = 1000;

	state = STA_STAY;
	Set_Direction( DIR_UP );
	Set_Max_Distance( 200 );
	Set_Speed( 5.8f );

	kill_sound = "enemy/jpiranha/die.ogg";
	kill_points = 100;

	walk_count = (float)( rand() % 4 );
	wait_time = (float)( rand() % 70 );
	move_back = 0;
}

cjPiranha *cjPiranha :: Copy( void )
{
	cjPiranha *jpiranha = new cjPiranha( startposx, startposy );
	jpiranha->Set_Direction( start_direction );
	jpiranha->Set_Max_Distance( max_distance );
	jpiranha->Set_Speed( speed );
	return jpiranha;
}

void cjPiranha :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( attributes.getValueAsFloat( "max_distance", max_distance ) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", speed ) );
}

void cjPiranha :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"jpiranha\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << (int)max_distance << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cjPiranha :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	// clear old images
	Clear_Images();

	cEnemy::Set_Direction( dir, 1 );

	string filename_dir;
	name = "jPiranha";

	if( direction == DIR_UP )
	{
		filename_dir = "up";
		name += " Up";
	}
	else if( direction == DIR_DOWN )
	{
		filename_dir = "down";
		name += " Down";
	}
	else if( direction == DIR_LEFT )
	{
		filename_dir = "left";
		name += " Left";
	}
	else if( direction == DIR_RIGHT )
	{
		filename_dir = "right";
		name += " Right";
	}
	else
	{
		Set_Dead( 1 );
		Set_Visible( 0 );

		printf( "Error : Unknown jPiranha direction %d\n", direction );
		return;
	}

	images.push_back( pVideo->Get_Surface( "enemy/jpiranha/" + filename_dir + "_closed_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/jpiranha/" + filename_dir + "_closed_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/jpiranha/" + filename_dir + "_open_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/jpiranha/" + filename_dir + "_open_2.png" ) );

	Set_Image( 0, 1 );
}

void cjPiranha :: Set_Max_Distance( float nmax_distance )
{
	max_distance = nmax_distance;

	if( max_distance < 0 )
	{
		max_distance = 0;
	}
}

void cjPiranha :: Set_Speed( float val )
{
	if( speed < 0.1f )
	{
		speed = 0.1f;
	}

	speed = val;

	Update_dest_vel();
}

void cjPiranha :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;

	if( !force )
	{
		// animation
		cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Speed( 5, 0.6f );
		anim->Set_Scale( 0.8f );
		// add animation
		pAnimationManager->Add( anim );
	}
	else
	{
		Set_RotationZ( 180 );
	}
}

void cjPiranha :: DieStep( void )
{
	counter += pFramerate->speedfactor;

	// default death
	if( rotz != 180 )
	{
		Set_Visible( 0 );
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
			Set_Visible( 0 );
		}
	}
}

void cjPiranha :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// standing ( waiting )
	if( state == STA_STAY )
	{
		// if waiting time
		if( wait_time > 0 )
		{
			wait_time -= pFramerate->speedfactor;

			if( wait_time < 0 )
			{
				wait_time = 0;
			}
		}
		// no more waiting try to jump out
		else
		{
			GL_rect rect1 = col_rect;

			if( direction == DIR_UP )
			{
				rect1.y -= 40;
				rect1.h += 40;
			}
			else if( direction == DIR_DOWN )
			{
				rect1.y += 40;
				rect1.h -= 40;
			}
			else if( direction == DIR_LEFT )
			{
				rect1.x -= 35;
				rect1.w += 35;
			}
			else if( direction == DIR_RIGHT )
			{
				rect1.x += 35;
				rect1.w += 35;
			}

			// if player is in front wait again
			if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &pPlayer->col_rect, &rect1 ) )
			{
				wait_time = DESIRED_FPS * 2;
			}
			// if not jump out
			else
			{
				state = STA_FLY;

				velx = dest_velx;
				vely = dest_vely;
				move_back = 0;
			}
		}
	}
	// flying ( moving into the destination direction )
	else
	{
		// distance to final position
		float dist_to_final_pos = Get_End_Distance();
		// multiplier for the minimal velocity
		float vel_mod_min = ( dist_to_final_pos + ( max_distance * 0.1f ) ) / max_distance;

		// if behind max distance
		if( vel_mod_min <= 0.1f )
		{
			vel_mod_min = 0.1f;
		}

		/* slow down
		 * with the velocity mod which is calculated from the distance to the final position
		*/
		switch( direction )
		{
		case DIR_LEFT:
		{
			// move forward
			if( !move_back )
			{
				velx = dest_velx * vel_mod_min;
			}
			// move back
			else
			{
				velx = -dest_velx * vel_mod_min;
			}
		}
		case DIR_RIGHT:
		{
			// move forward
			if( !move_back )
			{
				velx = dest_velx * vel_mod_min;
			}
			// move back
			else
			{
				velx = -dest_velx * vel_mod_min;
			}
		}
		case DIR_UP:
		{
			// move forward
			if( !move_back )
			{
				vely = dest_vely * vel_mod_min;
			}
			// move back
			else
			{
				vely = -dest_vely * vel_mod_min;
			}
		}
		case DIR_DOWN:
		{
			// move forward
			if( !move_back )
			{
				vely = dest_vely * vel_mod_min;
			}
			// move back
			else
			{
				vely = -dest_vely * vel_mod_min;
			}
		}
		default:
		{
			break;
		}
		}

		walk_count += 0.23f * pFramerate->speedfactor;

		if( walk_count >= 4 )
		{
			walk_count = 0;
		}

		Set_Image( (int)walk_count );

		// moving forward
		if( !move_back )
		{
			// reached final position move back
			if( dist_to_final_pos < 0 )
			{
				velx = -dest_velx * 0.01f;
				vely = -dest_vely * 0.01f;

				move_back = 1;
			}

		}
		// moving back
		else if( move_back )
		{
			// reached original position
			if( dist_to_final_pos > max_distance )
			{
				state = STA_STAY;
				Set_Pos( startposx, startposy );
				velx = 0;
				vely = 0;

				move_back = 0;
				wait_time = DESIRED_FPS * 2;
				Set_Image( 0 );
			}
		}
	}
}

void cjPiranha :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// draw distance rect
	if( editor_level_enabled )
	{
		if( start_direction == DIR_RIGHT )
		{
			pVideo->Draw_Rect( startposx - pCamera->x, startposy + ( rect.h * 0.5f ) - 5 - pCamera->y, max_distance + rect.w, 10, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_LEFT )
		{
			pVideo->Draw_Rect( startposx - pCamera->x + rect.w, startposy + ( rect.h * 0.5f ) - 5 - pCamera->y, -( rect.w + max_distance ), 10, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_DOWN )
		{
			pVideo->Draw_Rect( startposx + ( rect.w * 0.5f ) - 5 - pCamera->x, startposy - pCamera->y, 10, max_distance + rect.h, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_UP )
		{
			pVideo->Draw_Rect( startposx + ( rect.w * 0.5f ) - 5 - pCamera->x, startposy - pCamera->y + rect.h, 10, -( rect.h + max_distance ), editor_posz - 0.000001f, &whitealpha128 );
		}
	}

	cEnemy::Draw( request );
}

float cjPiranha :: Get_End_Distance(void )
{
	switch( direction )
	{
	case DIR_LEFT:
	{
		return max_distance - ( startposx - posx );
	}
	case DIR_RIGHT:
	{
		return max_distance + ( startposx - posx );
	}
	case DIR_UP:
	{
		return max_distance - ( startposy - posy );
	}
	case DIR_DOWN:
	{
		return max_distance + ( startposy - posy );
	}
	default:
	{
		break;
	}
	}
	
	return 0;
}

void cjPiranha :: Update_dest_vel( void )
{
	if( direction == DIR_UP )
	{
		dest_velx = 0;
		dest_vely = -speed;
	}
	else if( direction == DIR_DOWN )
	{
		dest_velx = 0;
		dest_vely = speed;
	}
	else if( direction == DIR_LEFT )
	{
		dest_velx = -speed;
		dest_vely = 0;
	}
	else if( direction == DIR_RIGHT )
	{
		dest_velx = speed;
		dest_vely = 0;
	}
	else
	{
		dest_velx = 0;
		dest_vely = 0;
	}
}

bool cjPiranha :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cjPiranha :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		return 1;
	}
	if( obj->type == TYPE_BALL )
	{
		return 2;
	}

	return 0;
}

void cjPiranha :: Handle_Collision_Player( ObjectDirection cdirection )
{
	// unknown direction
	if( cdirection == DIR_UNDEFINED )
	{
		return;
	}

	if( pPlayer->maryo_type != MARYO_SMALL && ( cdirection != DIR_BOTTOM ) )
	{
		pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );
		pPlayer->start_enemyjump = 1;
	}

	pPlayer->DownGrade();
}

void cjPiranha :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// max distance
	Editbox *editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_jpiranha_max_distance" );
	editbox->setTooltipText( "Max Distance" );
	Editor_Add( editbox, 90 );

	editbox->setText( int_to_string( (int)max_distance ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cjPiranha::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_jpiranha_speed" );
	editbox->setTooltipText( "Speed" );
	Editor_Add( editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cjPiranha::Editor_Speed_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cjPiranha :: Editor_Max_Distance_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( (float)string_to_int( str_text ) );

	return 1;
}

bool cjPiranha :: Editor_Speed_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}
