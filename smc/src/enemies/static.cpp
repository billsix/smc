/***************************************************************************
 * static.cpp  -  static enemy
 *
 * Copyright (C) 2007 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/static.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */

cStaticEnemy :: cStaticEnemy( float x, float y )
: cEnemy( x, y )
{
	cStaticEnemy::Init();
}

cStaticEnemy :: cStaticEnemy( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cStaticEnemy::Init();
	cStaticEnemy::Create_from_Stream( attributes );
}

cStaticEnemy :: ~cStaticEnemy( void )
{
	//
}

void cStaticEnemy :: Init( void )
{
	type = TYPE_STATIC_ENEMY;
	posz = 0.094f;
	can_be_ground = 0;
	can_be_on_ground = 0;

	Set_Rotation_Speed( 0 );
	Set_Static_Image( "enemy/static/blocks/spike_1/2_grey.png" );
	Create_Name();
}

cStaticEnemy *cStaticEnemy :: Copy( void )
{
	cStaticEnemy *static_enemy = new cStaticEnemy( startposx, startposy );
	static_enemy->Set_Static_Image( img_filename );
	static_enemy->Set_Rotation_Speed( rotation_speed );
	return static_enemy;
}

void cStaticEnemy :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// rotation speed
	Set_Rotation_Speed( static_cast<float>( attributes.getValueAsFloat( "rotation_speed", -7.5f ) ) );
	// image
	Set_Static_Image( attributes.getValueAsString( "image", "enemy/static/saw/default.png" ).c_str() );
}

void cStaticEnemy :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"static\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// rotation speed
	file << "\t\t<Property name=\"rotation_speed\" value=\"" << rotation_speed << "\" />" << std::endl;
	// image
	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cStaticEnemy :: Set_Rotation_Speed( float speed )
{
	rotation_speed = speed;
}

void cStaticEnemy :: Set_Static_Image( string filename )
{
	if( filename.empty() )
	{
		return;
	}

	Clear_Images();

	img_filename = filename;

	// remove pixmaps dir
	if( img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	images.push_back( pVideo->Get_Surface( filename ) );
	Set_Image( 0, 1 );
	Create_Name();
}

void cStaticEnemy :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;
	Set_Scale_Directions( 1, 1, 1, 1 );

	// falling death
	Set_Rotation_Z( 180 );
}

void cStaticEnemy :: DieStep( void )
{
	counter += pFramerate->speedfactor * 0.1f;

	// falling death

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

void cStaticEnemy :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( rotation_speed )
	{
		// update rotation
		Add_Rotation_Z( rotation_speed * pFramerate->speedfactor );
	}
}

bool cStaticEnemy :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cStaticEnemy :: Validate_Collision( cSprite *obj )
{
	if( obj->massivetype == MASS_MASSIVE )
	{
		if( obj->type == TYPE_ROKKO )
		{
			return 0;
		}
		if( obj->type == TYPE_GEE )
		{
			return 0;
		}
		if( obj->type == TYPE_TURTLE_BOSS )
		{
			return 0;
		}

		if( obj->sprite_array == ARRAY_ENEMY )
		{
			return 1;
		}

		return 2;
	}

	return 0;
}

void cStaticEnemy :: Handle_Collision_Player( cObjectCollision *collision )
{
	pPlayer->DownGrade_Player();
}

void cStaticEnemy :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// invalid
	if( collision->number < 0 )
	{
		return;
	}

	cEnemy *enemy = static_cast<cEnemy *>(pActive_Sprite_Manager->Get_Pointer( collision->number ));

	// already dead
	if( enemy->dead )
	{
		return;
	}

	// kill enemy
	pAudio->Play_Sound( enemy->kill_sound );
	pointsdisplay->Add_Points( enemy->kill_points, posx + image->w / 3, posy - 5, "", static_cast<Uint8>(255), 1 );
	enemy->DownGrade( 1 );
}

void cStaticEnemy :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// image
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_image" ));
	Editor_Add( UTF8_("Image"), UTF8_("Image filename"), editbox, 200 );

	editbox->setText( img_filename.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Image_Key, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_static_enemy_rotation_speed" ));
	Editor_Add( UTF8_("Rotation Speed"), UTF8_("Rotation Speed"), editbox, 120 );

	editbox->setText( float_to_string( rotation_speed ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cStaticEnemy::Editor_Rotation_Speed_Key, this ) );

	// init
	Editor_Init();
}

bool cStaticEnemy :: Editor_Image_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Static_Image( str_text );

	return 1;
}

bool cStaticEnemy :: Editor_Rotation_Speed_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Rotation_Speed( string_to_float( str_text ) );

	return 1;
}

void cStaticEnemy :: Create_Name( void )
{
	name = "Static Enemy";

	if( start_image && !start_image->name.empty() )
	{
		name += " " + start_image->name;
	}
}
