/***************************************************************************
              gumba.cpp  -  little moving around enemy
                             -------------------
    copyright            :	(C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/gumba.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** cGumba *** *** *** *** *** *** *** *** *** *** *** */

cGumba :: cGumba( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cGumba :: cGumba( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cGumba :: ~cGumba( void )
{
	//
}

void cGumba :: Init( void )
{
	type = TYPE_GUMBA;
	posz = 0.09f;
	color_type = COL_DEFAULT;

	Set_Color( COL_BROWN );
	Set_Direction( DIR_RIGHT );
}

cGumba *cGumba :: Copy( void )
{
	cGumba *gumba = new cGumba( startposx, startposy );
	gumba->Set_Color( color_type );
	gumba->Set_Direction( start_direction );

	return gumba;
}

void cGumba :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// color
	Set_Color( (DefaultColor)Get_Color_id( attributes.getValueAsString( "color", Get_Color_name( color_type ) ).c_str() ) );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
}

void cGumba :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"gumba\" />" << std::endl;
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

void cGumba :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	Update_Rotation_velx();

	Create_Name();
}

void cGumba :: Set_Color( DefaultColor col )
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
	// red was used in pre 1.0 but was in fact blue
	else if( color_type == COL_RED || color_type == COL_BLUE )
	{
		color_type = COL_BLUE;
		filename_dir = "blue";

		velx = 4.5f;
		kill_points = 50;
		ice_resistance = 0.9f;
	}
	else
	{
		printf( "Error : Unknown Gumba Color %d\n", color_type );
	}

	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_4.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_5.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_6.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_7.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_8.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_9.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/walk_10.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gumba/" + filename_dir + "/dead.png" ) );

	Set_Image( 0, 1 );

	Create_Name();
}

void cGumba :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;
	// dead image
	Set_Image( 10 );

	// default stomp death
	if( !force )
	{
		Generate_Hit_Animation();
	}
	// falling death
	else
	{
		Set_RotationZ( 180 );
	}
}

void cGumba :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.1f;

	// stomp death
	if( rotz != 180 )
	{
		// scale out
		float speed = pFramerate->speedfactor * 0.05f;

		Add_ScaleX( -speed * 0.5f );
		Add_ScaleY( -speed );
		posx += ( image->w / 2 ) * ( speed * 0.5f );
		posy += ( image->h ) * speed;

		if( scaley < 0.1f )
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
		else if( posy < GAME_RES_H + col_rect.h )
		{
			Move( 0, 20 );

			Add_Scale( -pFramerate->speedfactor * 0.01f );
			Move( ( image->w / 2 ) * pFramerate->speedfactor * 0.01f, ( image->h ) * pFramerate->speedfactor * 0.01f, 1 );
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

void cGumba :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// walking animation
	walk_count += pFramerate->speedfactor * 0.5f;

	if( walk_count >= 10 )
	{
		walk_count = 0;
	}

	Set_Image( (int)walk_count );

	// gravity
	Update_Gravity();

	Update_Rotation_velx();
}

bool cGumba :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cGumba :: Validate_Collision( cSprite *obj )
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
	if( obj->type == TYPE_ENEMYSTOPPER )
	{
		return 2;
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

void cGumba :: Handle_Collision_Player( cObjectCollision *collision )
{
	// invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pAudio->PlaySound( kill_sound );

		DownGrade();
		pPlayer->start_enemyjump = 1;

		pointsdisplay->Add_Points( kill_points, pPlayer->posx, pPlayer->posy, "", (Uint8)255, 1 );
		pPlayer->Add_Kill_Multiplier();
	}
	else
	{
		pPlayer->DownGrade();
		Turn_Around( collision->direction );
	}
}

void cGumba :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
	Send_Collision( collision );
}

void cGumba :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( state == STA_OBJ_LINKED )
	{
		return;
	}
	
	Turn_Around( collision->direction );
}

void cGumba :: Create_Name( void )
{
	name = "Gumba";
	name += " " + Get_Color_name( color_type );
	name += " " + Get_Direction_name( start_direction );
}
