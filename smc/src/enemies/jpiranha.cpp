/***************************************************************************
 * jpiranha.cpp  -  jumping piranha plant
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

#include "../enemies/jpiranha.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../player/player.h"
#include "../core/obj_manager.h"
#include "../video/animation.h"
#include "../user/savegame.h"
#include "../core/math/utilities.h"
#include "../input/mouse.h"
#include "../video/gl_surface.h"
#include "../core/i18n.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;

/* *** *** *** *** *** *** cjPiranha *** *** *** *** *** *** *** *** *** *** *** */

cjPiranha :: cjPiranha( float x, float y )
: cEnemy( x, y )
{
	cjPiranha::Init();
}

cjPiranha :: cjPiranha( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cjPiranha::Init();
	cjPiranha::Create_from_Stream( attributes );
}

cjPiranha :: ~cjPiranha( void )
{
	//
}

void cjPiranha :: Init( void  )
{
	type = TYPE_JPIRANHA;
	posz = 0.06f;
	Set_Rotation_affects_Rect( 1 );
	editor_posz = 0.089f;
	player_range = 1000;
	can_be_on_ground = 0;
	can_be_ground = 0;

	state = STA_STAY;
	Set_Direction( DIR_UP );
	Set_Image_Dir( "enemy/jpiranha/orange/" );
	Set_Max_Distance( 200 );
	Set_Speed( 5.8f );

	kill_sound = "enemy/jpiranha/die.ogg";
	kill_points = 100;

	walk_count = Get_Random_Float( 0, 4 );
	wait_time = Get_Random_Float( 0, 70 );
	move_back = 0;
}

cjPiranha *cjPiranha :: Copy( void )
{
	cjPiranha *jpiranha = new cjPiranha( startposx, startposy );
	jpiranha->Set_Direction( start_direction );
	jpiranha->Set_Image_Dir( img_dir );
	jpiranha->Set_Max_Distance( max_distance );
	jpiranha->Set_Speed( speed );
	return jpiranha;
}

void cjPiranha :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
	// image directory
	Set_Image_Dir( attributes.getValueAsString( "image_dir", img_dir ).c_str() );
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
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
	// image directory
	file << "\t\t<Property name=\"image_dir\" value=\"" << img_dir << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << static_cast<int>(max_distance) << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cjPiranha :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	// move_back
	if( save_object->exists( "move_back" ) )
	{
		move_back = string_to_int( save_object->Get_Value( "move_back" ) ) > 0;
	}
}

cSave_Level_Object *cjPiranha :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_to_Savegame();

	// move_back ( only save if needed )
	if( move_back )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "move_back", int_to_string( move_back ) ) );
	}

	return save_object;
}

void cjPiranha :: Set_Image_Dir( string dir )
{
	if( dir.empty() )
	{
		return;
	}

	// remove pixmaps dir
	if( img_dir.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_dir.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	// if directory does not exist
	if( !fs::exists( fs::path( DATA_DIR "/" GAME_PIXMAPS_DIR "/" + dir, fs::native ) ) )
	{
		return;
	}

	img_dir = dir;

	// clear images
	Clear_Images();
	// set images
	images.push_back( pVideo->Get_Surface( img_dir + "closed_1.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "closed_2.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "open_1.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "open_2.png" ) );
	// set start image
	Set_Image( 0, 1 );

	Create_Name();
}

void cjPiranha :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( dir == direction )
	{
		return;
	}

	if( dir != DIR_UP && dir != DIR_DOWN && dir != DIR_LEFT && dir != DIR_RIGHT )
	{
		printf( "Error : Unknown jPiranha direction %d\n", direction );
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	// clear
	Set_Rotation( 0, 0, 0, 1 );

	if( start_direction == DIR_UP )
	{
		// default
	}
	else if( start_direction == DIR_LEFT )
	{
		Set_Rotation_Z( 270, 1 );
	}
	else if( start_direction == DIR_RIGHT )
	{
		Set_Rotation_Z( 90, 1 );
	}
	else if( start_direction == DIR_DOWN )
	{
		Set_Rotation_Z( 180, 1 );
	}

	Create_Name();
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

	Update_Dest_Vel();
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
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
		Generate_Hit_Animation( anim );

		anim->Set_Speed( 5, 0.6f );
		anim->Set_Scale( 0.8f );
		// add animation
		pAnimation_Manager->Add( anim );
	}
	else
	{
		Set_Rotation_Z( 180 );
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
				wait_time = speedfactor_fps * 2;
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
			break;
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
			break;
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
			break;
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
			break;
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

		Set_Image( static_cast<int>(walk_count) );

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
				wait_time = speedfactor_fps * 2;
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
			pVideo->Draw_Rect( startposx - pActive_Camera->x, startposy + ( rect.h * 0.5f ) - 5 - pActive_Camera->y, max_distance + rect.w, 10, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_LEFT )
		{
			pVideo->Draw_Rect( startposx - pActive_Camera->x + rect.w, startposy + ( rect.h * 0.5f ) - 5 - pActive_Camera->y, -( rect.w + max_distance ), 10, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_DOWN )
		{
			pVideo->Draw_Rect( startposx + ( rect.w * 0.5f ) - 5 - pActive_Camera->x, startposy - pActive_Camera->y, 10, max_distance + rect.h, editor_posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_UP )
		{
			pVideo->Draw_Rect( startposx + ( rect.w * 0.5f ) - 5 - pActive_Camera->x, startposy - pActive_Camera->y + rect.h, 10, -( rect.h + max_distance ), editor_posz - 0.000001f, &whitealpha128 );
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

void cjPiranha :: Update_Dest_Vel( void )
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

bool cjPiranha :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

bool cjPiranha :: Is_Draw_Valid( void )
{
	bool valid = cEnemy::Is_Draw_Valid();

	// if editor enabled
	if( editor_enabled )
	{
		// if active mouse object
		if( pMouseCursor->active_object == this )
		{
			return 1;
		}
	}


	return valid;
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

void cjPiranha :: Handle_Collision_Player( cObjectCollision *collision )
{
	// unknown direction
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( pPlayer->maryo_type != MARYO_SMALL && !pPlayer->invincible && collision->direction == direction )
	{
		// todo : create again
		//pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );
		pPlayer->Action_Jump( 1 );
	}

	pPlayer->DownGrade_Player();
}

void cjPiranha :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_jpiranha_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction"), combobox, 100, 110 );

	combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cjPiranha::Editor_Direction_Select, this ) );

	// image dir
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_jpiranha_image_dir" ));
	Editor_Add( UTF8_("Image directory"), UTF8_("Directory containing the images"), editbox, 200 );

	editbox->setText( img_dir.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cjPiranha::Editor_Image_Dir_Key, this ) );

	// max distance
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_jpiranha_max_distance" ));
	Editor_Add( UTF8_("Distance"), _("Movable Distance into its direction"), editbox, 90 );

	editbox->setText( int_to_string( static_cast<int>(max_distance) ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cjPiranha::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_jpiranha_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Initial speed when jumping out"), editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cjPiranha::Editor_Speed_Key, this ) );

	// init
	Editor_Init();
}

bool cjPiranha :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cjPiranha :: Editor_Image_Dir_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Dir( str_text );

	return 1;
}

bool cjPiranha :: Editor_Max_Distance_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( static_cast<float>(string_to_int( str_text )) );

	return 1;
}

bool cjPiranha :: Editor_Speed_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

void cjPiranha :: Create_Name( void )
{
	name = "jPiranha ";
	name += _(Get_Direction_Name( start_direction ).c_str());

	if( start_image && !start_image->name.empty() )
	{
		name += " " + start_image->name;
	}
}
