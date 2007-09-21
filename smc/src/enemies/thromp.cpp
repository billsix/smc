/***************************************************************************
              thromp.cpp  -  falling stone
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/thromp.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../core/sprite_manager.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** cThromp *** *** *** *** *** *** *** *** *** *** *** */

cThromp :: cThromp( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cThromp :: cThromp( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cThromp :: ~cThromp( void )
{
	//
}

void cThromp :: Init( void  )
{
	type = TYPE_THROMP;
	posz = 0.093f;
	player_range = 1000;
	can_be_ground = 1;

	fire_resistant = 1;

	state = STA_STAY;
	Set_Direction( DIR_DOWN );
	speed = 7;
	dest_velx = 0;
	dest_vely = 0;

	max_distance = 200;
	move_back = 0;

	kill_sound = "enemy/thromp/die.ogg";
	kill_points = 200;

	Update_dest_vel();
}

cThromp *cThromp :: Copy( void )
{
	cThromp *thromp = new cThromp( startposx, startposy );
	thromp->Set_Direction( start_direction );
	thromp->Set_Max_Distance( max_distance );
	thromp->Set_Speed( speed );

	return thromp;
}

void cThromp :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( attributes.getValueAsInteger( "max_distance", max_distance ) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", speed ) );
}

void cThromp :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"thromp\" />" << std::endl;
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

void cThromp :: Set_Direction( ObjectDirection dir )
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

	if( direction == DIR_UP )
	{
		filename_dir = "up";
		name = "Thromp Up";
	}
	else if( direction == DIR_DOWN )
	{
		filename_dir = "down";
		name = "Thromp Down";
	}
	else if( direction == DIR_LEFT )
	{
		filename_dir = "left";
		name = "Thromp Left";
	}
	else if( direction == DIR_RIGHT )
	{
		filename_dir = "right";
		name = "Thromp Right";
	}
	else
	{
		printf( "Error : Unknown Thromp direction %d\n", direction );
		return;
	}

	images.push_back( pVideo->Get_Surface( "enemy/thromp/" + filename_dir + "_default.png" ) );

	Set_Image( 0, 1 );
}

void cThromp :: Set_Max_Distance( int nmax_distance )
{
	max_distance = nmax_distance;

	if( max_distance < 0 )
	{
		max_distance = 0;
	}
}

void cThromp :: Set_Speed( float val )
{
	if( speed < 0.1f )
	{
		speed = 0.1f;
	}

	speed = val;

	Update_dest_vel();
}

void cThromp :: Activate( void )
{
	if( state == STA_FLY )
	{
		return;
	}

	state = STA_FLY;

	velx = dest_velx;
	vely = dest_vely;
	move_back = 0;
}

void cThromp :: Move_Back( void )
{
	if( state == STA_STAY || move_back )
	{
		return;
	}

	velx = -dest_velx * 0.01f;
	vely = -dest_vely * 0.01f;

	move_back = 1;
}

void cThromp :: DownGrade( bool force /* = 0 */ )
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

void cThromp :: DieStep( void )
{
	counter += pFramerate->speedfactor;

	if( rotz != 180 ) // default death
	{
		Set_Visible( 0 );
	}
	else // falling death
	{
		if( counter < 5 ) // a little bit upwards first
		{
			Move( 0, -5 );
		}
		else if( posy < GAME_RES_H + col_rect.h ) // if not below the screen fall
		{
			Move( 0, 20 );
		}
		else // if below disable
		{
			rotz = 0;
			Set_Visible( 0 );
		}
	}
}

void cThromp :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// standing ( waiting )
	if( state == STA_STAY )
	{
		GL_rect rect_distance = Get_Distance_rect();

		rect_distance.x += col_rect.x;
		rect_distance.y += col_rect.y;
		rect_distance.w += col_rect.w;
		rect_distance.h += col_rect.h;

		// if player is in front jump out
		if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &pPlayer->col_rect, &rect_distance ) )
		{
			Activate();
		}
	}
	// flying ( moving into the destination direction )
	else
	{
		// distance to final position
		float dist_to_final_pos = 0;
		// multiplier for the minimal velocity
		float vel_mod_min = 1;

		/* slow down
		 * only if the velocity is not too small for the given distance to the final position
		 * final velocity should not get smaller on the last 10% to the final position
		*/
		if( direction == DIR_LEFT )
		{
			dist_to_final_pos = max_distance - ( startposx - posx );

			// move back
			if( move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( max_distance * 0.1f ) ) / max_distance;
				if( -velx > dest_velx * vel_mod_min )
				{
					velx *= 1 + ( 0.2f * pFramerate->speedfactor );
				}
			}

		}
		else if( direction == DIR_RIGHT )
		{
			dist_to_final_pos = max_distance + ( startposx - posx );

			// move back
			if( move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( max_distance * 0.1f ) ) / max_distance;
				if( -velx < dest_velx * vel_mod_min )
				{
					velx *= 1 + ( 0.2f * pFramerate->speedfactor );
				}
			}
		}
		else if( direction == DIR_UP )
		{
			dist_to_final_pos = max_distance - ( startposy - posy );

			// move back
			if( move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( max_distance * 0.1f ) ) / max_distance;
				if( -vely > dest_vely * vel_mod_min )
				{
					vely *= 1 + ( 0.2f * pFramerate->speedfactor );
				}
			}
		}
		else if( direction == DIR_DOWN )
		{
			dist_to_final_pos = max_distance + ( startposy - posy );

			// move back
			if( move_back )
			{
				vel_mod_min = ( dist_to_final_pos + ( max_distance * 0.1f ) ) / max_distance;
				if( -vely < dest_vely * vel_mod_min )
				{
					vely *= 1 + ( 0.2f * pFramerate->speedfactor );
				}
			}
		}

		// reached final position move back
		if( !move_back && dist_to_final_pos < 0 )
		{
			Move_Back();
		}
		// reached original position
		else if( move_back && dist_to_final_pos > max_distance )
		{
			state = STA_STAY;
			posx = startposx;
			posy = startposy;
			velx = 0;
			vely = 0;

			move_back = 0;
		}
	}
}

void cThromp :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// draw distance rect
	if( editor_level_enabled )
	{
		GL_rect rect_distance = Get_Distance_rect();
		rect_distance.x += rect.x - pCamera->x;
		rect_distance.y += rect.y - pCamera->y;

		if( start_direction == DIR_LEFT || start_direction == DIR_RIGHT )
		{
			rect_distance.y += ( rect.h * 0.5f ) - 5;
			rect_distance.h = 10;
			rect_distance.w += rect.w;
		}
		else if( start_direction == DIR_UP || start_direction == DIR_DOWN )
		{
			rect_distance.x += ( rect.w * 0.5f ) - 5;
			rect_distance.h += rect.h;
			rect_distance.w = 10;
		}

		pVideo->Draw_Rect( &rect_distance, posz - 0.000001f, &whitealpha128 );
	}

	cEnemy::Draw( request );
}

void cThromp :: Update_dest_vel( void )
{
	if( start_direction == DIR_UP )
	{
		dest_velx = 0;
		dest_vely = -speed;
	}
	else if( start_direction == DIR_DOWN )
	{
		dest_velx = 0;
		dest_vely = speed;
	}
	else if( start_direction == DIR_LEFT )
	{
		dest_velx = -speed;
		dest_vely = 0;
	}
	else if( start_direction == DIR_RIGHT )
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

GL_rect cThromp :: Get_Distance_rect( void )
{
	GL_rect rect_distance( 0, 0, 0, 0 );

	if( start_direction == DIR_UP )
	{
		rect_distance.y = (float)-max_distance;
		rect_distance.h = (float)max_distance;
	}
	else if( start_direction == DIR_DOWN )
	{
		rect_distance.h = (float)max_distance;
	}
	else if( start_direction == DIR_LEFT )
	{
		rect_distance.x = (float)-max_distance;
		rect_distance.w = (float)max_distance;
	}
	else if( start_direction == DIR_RIGHT )
	{
		rect_distance.w = (float)max_distance;
	}

	return rect_distance;
}

bool cThromp :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

void cThromp :: Generate_Smoke( unsigned int power /* = 10 */ )
{
	// smoke on the destination direction
	float smoke_x;
	float smoke_y;
	float smoke_width;
	float smoke_height;

	if( direction == DIR_DOWN )
	{
		smoke_x = posx;
		smoke_y = posy + rect.h;
		smoke_width = col_rect.w;
		smoke_height = 1;
	}
	else if( direction == DIR_UP )
	{
		smoke_x = posx;
		smoke_y = posy;
		smoke_width = col_rect.w;
		smoke_height = 1;
	}
	else if( direction == DIR_LEFT )
	{
		smoke_x = posx;
		smoke_y = posy;
		smoke_width = 1;
		smoke_height = col_rect.h;
	}
	else if( direction == DIR_RIGHT )
	{
		smoke_x = posx + rect.w;
		smoke_y = posy;
		smoke_width = 1;
		smoke_height = col_rect.h;
	}
	else
	{
		return;
	}

	for( unsigned int i = 0 ; i < power; i++ )
	{
		// animation
		cParticleAnimation *anim = new cParticleAnimation( smoke_x + ( rand() % (int)smoke_width ), smoke_y + ( rand() % (int)smoke_height ) );

		anim->Set_ZPos( posz + 0.000001f );
		anim->Set_Time_to_Live( 1, 1 );
		anim->Set_DirectionRange( 0, 360 );
		anim->Set_Speed( 0.5f, 0.2f );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_ConstRotationZ( 0, 5 );
		// add animation
		pAnimationManager->Add( anim );
	}
}

unsigned int cThromp :: Validate_Collision( cSprite *obj )
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
			return 1;
		}
		if( obj->type == TYPE_STATIC_ENEMY )
		{
			return 0;
		}

		// if moving back collide with nothing
		if( move_back )
		{
			return 0;
		}
		else
		{
			return 2;
		}
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			// if moving back collide with nothing
			if( !move_back )
			{
				return 2;
			}
		}
	}

	return 0;
}

void cThromp :: Handle_Collision_Player( cObjectCollision *collision )
{
	// behind
	if( collision->direction == Get_OppositeDirection( direction ) )
	{
		if( collision->direction == DIR_LEFT || collision->direction == DIR_RIGHT )
		{
			pPlayer->Col_Move( velx, 0, 1 );
		}
	}
	// front
	else if( collision->direction == direction )
	{
		pPlayer->DownGrade();

		if( !move_back )
		{
			pAudio->PlaySound( "enemy/thromp/hit.ogg" );
			Generate_Smoke();
			Move_Back();
		}
	}
	// else is left/right of front direction which doesn't harm
}

void cThromp :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// only destination direction collisions
	if( collision->direction != direction )
	{
		return;
	}

	// if active
	if( state == STA_FLY )
	{
		cEnemy *enemy = static_cast<cEnemy *>(pSprite_Manager->Get_Pointer( collision->number ));

		// kill enemy
		pAudio->PlaySound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", (Uint8)255, 1 );
		enemy->DownGrade( 1 );

		if( !move_back )
		{
			Generate_Smoke();
		}
	}
}

void cThromp :: Handle_Collision_Massive( cObjectCollision *collision )
{
	// already moving back
	if( move_back )
	{
		return;
	}
	
	pAudio->PlaySound( "enemy/thromp/hit.ogg" );
	Generate_Smoke();
	Move_Back();
}

void cThromp :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// max distance
	Editbox *editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_max_distance" );
	editbox->setTooltipText( "Max Distance" );
	Editor_Add( editbox, 90 );

	editbox->setText( int_to_string( max_distance ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cThromp::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_speed" );
	editbox->setTooltipText( "Speed" );
	Editor_Add( editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cThromp::Editor_Speed_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cThromp :: Editor_Max_Distance_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( string_to_int( str_text ) );

	return 1;
}

bool cThromp :: Editor_Speed_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

