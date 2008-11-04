/***************************************************************************
 * furball.cpp  -  little moving around enemy
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

#include "../enemies/furball.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cFurball *** *** *** *** *** *** *** *** *** *** *** */

cFurball :: cFurball( float x, float y )
: cEnemy( x, y )
{
	cFurball::Init();
}

cFurball :: cFurball( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cFurball::Init();
	cFurball::Create_from_Stream( attributes );
}

cFurball :: ~cFurball( void )
{
	//
}

void cFurball :: Init( void )
{
	type = TYPE_FURBALL;
	posz = 0.09f;
	color_type = COL_DEFAULT;

	Set_Color( COL_BROWN );
	Set_Direction( DIR_RIGHT );
}

cFurball *cFurball :: Copy( void )
{
	cFurball *furball = new cFurball( startposx, startposy );
	furball->Set_Color( color_type );
	furball->Set_Direction( start_direction );

	return furball;
}

void cFurball :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// color
	Set_Color( (DefaultColor)Get_Color_Id( attributes.getValueAsString( "color", Get_Color_Name( color_type ) ).c_str() ) );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
}

void cFurball :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"furball\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_Name( color_type ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cFurball :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	Update_Rotation_Hor_velx();
}

void cFurball :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	Update_Rotation_Hor_velx( 1 );

	Create_Name();
}

void cFurball :: Set_Color( DefaultColor col )
{
	// already set
	if( color_type == col )
	{
		return;
	}

	// clear old images
	Clear_Images();

	color_type = col;
	string filename_dir;

	if( color_type == COL_BROWN )
	{
		filename_dir = "brown";

		velx = 2.7f;
		kill_points = 10;
		ice_resistance = 0;
	}
	else if( color_type == COL_BLUE )
	{
		filename_dir = "blue";

		velx = 4.5f;
		kill_points = 50;
		ice_resistance = 0.9f;
	}
	else
	{
		printf( "Error : Unknown Furball Color %d\n", color_type );
	}

	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_4.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_5.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_6.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_7.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/walk_8.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/turn.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/furball/" + filename_dir + "/dead.png" ) );

	Set_Image( 0, 1 );

	Create_Name();
}

void cFurball :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	cEnemy::Turn_Around( col_dir );

	if( col_dir == DIR_LEFT || col_dir == DIR_RIGHT )
	{
		// set turn around image
		walk_count = 0;
		Set_Image( 8 );
	}
}

void cFurball :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;
	// dead image
	Set_Image( 9 );

	// default stomp death
	if( !force )
	{
		Generate_Hit_Animation();
		Set_Scale_Directions( 1, 0, 1, 1 );
	}
	// falling death
	else
	{
		Set_Rotation_Z( 180 );
		Set_Scale_Directions( 1, 1, 1, 1 );
	}
}

void cFurball :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.1f;

	// stomp death
	if( rotz != 180 )
	{
		// scale out
		float speed = pFramerate->speedfactor * 0.05f;

		Add_Scale_X( -speed * 0.5f );
		Add_Scale_Y( -speed );

		if( scaley < 0.01f )
		{
			Set_Scale( 1 );
			Set_Visible( 0 );
		}
	}
	// falling death
	else
	{
		// a little bit upwards first
		if( counter < 0.3 )
		{
			Move( 0, -5 );
		}
		// if not below the screen fall
		else if( posy < game_res_h + col_rect.h )
		{
			Move( 0, 20 );

			Add_Scale( -pFramerate->speedfactor * 0.01f );
		}
		// if below disable
		else
		{
			rotz = 0;
			Set_Scale( 1 );
			Set_Visible( 0 );
		}
	}
}

void cFurball :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// walking animation
	walk_count += pFramerate->speedfactor * 0.5f;

	// if not turn around image
	if( curr_img != 8 )
	{
		if( walk_count >= 8 )
		{
			walk_count = 0;
		}

		Set_Image( static_cast<int>(walk_count) );
		Update_Rotation_Hor_velx();
	}
	else
	{
		// set normal image back
		if( walk_count >= 4 )
		{
			Set_Image( static_cast<int>(walk_count) );
			Update_Rotation_Hor_velx();
		}
		// rotate the turn image
		else if( walk_count >= 2 )
		{
			if( velx > 0 )
			{
				roty = 180;
			}
			else
			{
				roty = 0;
			}
		}
	}

	// gravity
	Update_Gravity();
}

bool cFurball :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cFurball :: Validate_Collision( cSprite *obj )
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
		if( obj->type == TYPE_JPIRANHA )
		{
			return 0;
		}
		if( obj->type == TYPE_ROKKO )
		{
			return 0;
		}
		if( obj->type == TYPE_GEE )
		{
			return 0;
		}

		return 2;
	}
	if( obj->type == TYPE_ENEMY_STOPPER )
	{
		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && Is_on_Top( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cFurball :: Handle_Collision_Player( cObjectCollision *collision )
{
	// invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pAudio->Play_Sound( kill_sound );

		DownGrade();
		pPlayer->Action_Jump( 1 );

		pointsdisplay->Add_Points( kill_points, pPlayer->posx, pPlayer->posy, "", static_cast<Uint8>(255), 1 );
		pPlayer->Add_Kill_Multiplier();
	}
	else
	{
		pPlayer->DownGrade();
		Turn_Around( collision->direction );
	}
}

void cFurball :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
	Send_Collision( collision );
}

void cFurball :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( state == STA_OBJ_LINKED )
	{
		return;
	}
	
	Turn_Around( collision->direction );
}

void cFurball :: Create_Name( void )
{
	name = "Furball ";
	name += _(Get_Color_Name( color_type ).c_str());
	name += " ";
	name += _(Get_Direction_Name( start_direction ).c_str());
}
