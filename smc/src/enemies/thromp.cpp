/***************************************************************************
 * thromp.cpp  -  falling stone
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

#include "../enemies/thromp.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/sprite_manager.h"
#include "../core/math/utilities.h"
#include "../input/mouse.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cThromp *** *** *** *** *** *** *** *** *** *** *** */

cThromp :: cThromp( float x, float y )
: cEnemy( x, y )
{
	cThromp::Init();
}

cThromp :: cThromp( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cThromp::Init();
	cThromp::Create_from_Stream( attributes );
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
	move_back = 0;
	dest_velx = 0;
	dest_vely = 0;
	img_dir = "enemy/thromp/";
	Set_Direction( DIR_DOWN );
	Set_Speed( 7 );
	Set_Max_Distance( 200 );

	kill_sound = "enemy/thromp/die.ogg";
	kill_points = 200;
}

cThromp *cThromp :: Copy( void )
{
	cThromp *thromp = new cThromp( startposx, startposy );
	thromp->Set_Image_Dir( img_dir );
	thromp->Set_Direction( start_direction );
	thromp->Set_Max_Distance( max_distance );
	thromp->Set_Speed( speed );

	return thromp;
}

void cThromp :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// image directory
	Set_Image_Dir( attributes.getValueAsString( "image_dir", img_dir ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( static_cast<float>(attributes.getValueAsInteger( "max_distance", static_cast<int>(max_distance) )) );
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
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// image directory
	file << "\t\t<Property name=\"image_dir\" value=\"" << img_dir << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << static_cast<int>(max_distance) << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cThromp :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	// Don't activate if dead
	if( dead )
	{
		return;
	}

	// move_back
	if( save_object->exists( "move_back" ) )
	{
		move_back = string_to_int( save_object->Get_Value( "move_back" ) ) > 0;
	}
}

cSave_Level_Object *cThromp :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_to_Savegame();

	// move_back ( only save if needed )
	if( move_back )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "move_back", int_to_string( move_back ) ) );
	}

	return save_object;
}

void cThromp :: Set_Image_Dir( string dir )
{
	if( dir.empty() )
	{
		return;
	}

	img_dir = dir;

	// remove pixmaps dir
	if( img_dir.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_dir.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	Update_Images();
}

void cThromp :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	Update_Distance_rect();
	Update_Images();
}

void cThromp :: Set_Max_Distance( float nmax_distance )
{
	max_distance = nmax_distance;

	if( max_distance < 0 )
	{
		max_distance = 0;
	}

	Update_Distance_rect();
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

	// active image
	Set_Image( 1 );
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

	// default image
	Set_Image( 0 );
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
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Scale( 0.8f );
		anim->Set_Direction_Range( 0, 360 );
		// add animation
		pAnimation_Manager->Add( anim );
	}
	else
	{
		Set_Rotation_Z( 180 );
	}
}

void cThromp :: DieStep( void )
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
		else if( posy < game_res_h + col_rect.h )
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
		GL_rect final_distance = Get_Final_Distance_Rect();

		// if player is in front then activate
		if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &pPlayer->col_rect, &final_distance ) )
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
			Set_Pos( startposx, startposy );
			// unset velocity
			Set_Velocity( 0, 0 );

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
		GL_rect final_distance = Get_Final_Distance_Rect();
		final_distance.x -= pActive_Camera->x;
		final_distance.y -= pActive_Camera->y;

		pVideo->Draw_Rect( &final_distance, posz - 0.000001f, &whitealpha128 );
	}

	cEnemy::Draw( request );
}

void cThromp :: Update_Images( void )
{
	// clear images
	Clear_Images();
	// set images
	images.push_back( pVideo->Get_Surface( img_dir + Get_Direction_Name( start_direction ) + ".png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + Get_Direction_Name( start_direction ) + "_active.png" ) );
	// set start image
	Set_Image( 0, 1 );

	// set active image
	if( state == STA_FLY )
	{
		Set_Image( 1 );
	}

	Create_Name();
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

void cThromp :: Update_Distance_rect( void )
{
	if( start_direction == DIR_UP )
	{
		distance_rect.x = col_pos.x;
		distance_rect.y = -max_distance;
		distance_rect.w = col_rect.w;
		distance_rect.h = max_distance;
	}
	else if( start_direction == DIR_DOWN )
	{
		distance_rect.x = col_pos.x;
		distance_rect.y = 0;
		distance_rect.w = col_rect.w;
		distance_rect.h = max_distance;
	}
	else if( start_direction == DIR_LEFT )
	{
		distance_rect.x = -max_distance;
		distance_rect.y = col_pos.y;
		distance_rect.w = max_distance;
		distance_rect.h = col_rect.h;
	}
	else if( start_direction == DIR_RIGHT )
	{
		distance_rect.x = 0;
		distance_rect.y = col_pos.y;
		distance_rect.w = max_distance;
		distance_rect.h = col_rect.h;
	}
}

GL_rect cThromp :: Get_Final_Distance_Rect( void )
{
	GL_rect final_distance = distance_rect;

	final_distance.x += rect.x;
	final_distance.y += rect.y;

	if( start_direction == DIR_LEFT || start_direction == DIR_RIGHT )
	{
		final_distance.x += rect.w;
		final_distance.w -= rect.w;
	}
	else if( start_direction == DIR_UP || start_direction == DIR_DOWN )
	{
		final_distance.y += rect.h;
		final_distance.h -= rect.h;
	}

	return final_distance;
}

bool cThromp :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

bool cThromp :: Is_Draw_Valid( void )
{
	if( cEnemy::Is_Draw_Valid() == 1 )
	{
		return 1;
	}

	// if not editor enabled or not active mouse object
	if( !editor_enabled || pMouseCursor->active_object != this )
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
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( smoke_x + Get_Random_Float( 0, smoke_width ), smoke_y + Get_Random_Float( 0, smoke_height ) );

		anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke.png" ) );
		anim->Set_Pos_Z( posz + 0.000001f );
		anim->Set_Time_to_Live( 1, 1 );
		anim->Set_Direction_Range( 0, 360 );
		anim->Set_Speed( 0.5f, 0.2f );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Const_Rotation_Z( -2, 4 );
		// add animation
		pAnimation_Manager->Add( anim );
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
		if( vely >= 0 && Is_on_Top( obj ) )
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
	if( collision->direction == Get_Opposite_Direction( direction ) )
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
			pAudio->Play_Sound( "enemy/thromp/hit.ogg" );
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
		cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

		// kill enemy
		pAudio->Play_Sound( enemy->kill_sound );
		pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
		enemy->DownGrade( 1 );

		if( !move_back )
		{
			Generate_Smoke();
		}
	}
}

void cThromp :: Handle_Collision_Massive( cObjectCollision *collision )
{
	// if not active or already moving back
	if( state != STA_FLY || move_back )
	{
		return;
	}
	
	pAudio->Play_Sound( "enemy/thromp/hit.ogg" );
	Generate_Smoke();
	Move_Back();
}

void cThromp :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// image dir
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_image_dir" ));
	Editor_Add( UTF8_("Image directory"), UTF8_("Directory containing the images"), editbox, 200 );

	editbox->setText( img_dir.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cThromp::Editor_Image_Dir_Key, this ) );

	// max distance
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_max_distance" ));
	Editor_Add( UTF8_("Distance"), UTF8_("Detection distance into its direction"), editbox, 90 );

	editbox->setText( int_to_string( static_cast<int>(max_distance) ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cThromp::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_thromp_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Speed when activated"), editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cThromp::Editor_Speed_Key, this ) );

	// init
	Editor_Init();
}

bool cThromp :: Editor_Image_Dir_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Dir( str_text );

	return 1;
}

bool cThromp :: Editor_Max_Distance_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( static_cast<float>(string_to_int( str_text )) );

	return 1;
}

bool cThromp :: Editor_Speed_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

void cThromp :: Create_Name( void )
{
	name = "Thromp ";
	name += _(Get_Direction_Name( start_direction ).c_str());

	if( start_image && !start_image->name.empty() )
	{
		name += " " + start_image->name;
	}
}
