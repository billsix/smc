/***************************************************************************
 * eato.cpp  -  eating static plant :P
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

#include "../enemies/eato.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cEato *** *** *** *** *** *** *** *** *** *** *** */

cEato :: cEato( float x, float y )
: cEnemy( x, y )
{
	cEato::Init();
}

cEato :: cEato( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cEato::Init();
	cEato::Create_from_Stream( attributes );
}

cEato :: ~cEato( void )
{
	//
}

void cEato :: Init( void )
{
	type = TYPE_EATO;
	player_range = 1000;
	posz = 0.087f;
	Set_Rotation_affects_Rect( 1 );
	can_be_ground = 1;
	fire_resistant = 1;

	state = STA_STAY;
	Set_Image_Dir( "enemy/eato/brown/" );
	Set_Direction( DIR_UP_LEFT );

	walk_count = static_cast<float>( rand() % 4 );

	kill_sound = "enemy/eato/die.ogg";
	kill_points = 150;
}

cEato *cEato :: Copy( void )
{
	cEato *eato = new cEato( startposx, startposy );
	eato->Set_Image_Dir( img_dir );
	eato->Set_Direction( start_direction );

	return eato;
}

void cEato :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// image directory
	Set_Image_Dir( attributes.getValueAsString( "image_dir", img_dir ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
}

void cEato :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"eato\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// image directory
	file << "\t\t<Property name=\"image_dir\" value=\"" << img_dir << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cEato :: Set_Image_Dir( string dir )
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

	// clear images
	Clear_Images();
	// set images
	images.push_back( pVideo->Get_Surface( img_dir + "1.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "2.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "3.png" ) );
	images.push_back( pVideo->Get_Surface( img_dir + "2.png" ) );
	// set start image
	Set_Image( 0, 1 );

	Create_Name();
}

void cEato :: Set_Direction( ObjectDirection dir )
{
	cEnemy::Set_Direction( dir, 1 );

	// clear
	Set_Rotation( 0, 0, 0, 1 );

	if( start_direction == DIR_UP_LEFT )
	{
		Set_Rotation_Y( 180, 1 );
	}
	else if( start_direction == DIR_UP_RIGHT )
	{
		// default
	}
	else if( start_direction == DIR_LEFT_UP )
	{
		Set_Rotation_Z( 90, 1 );
		Set_Rotation_X( 180, 1 );
	}
	else if( start_direction == DIR_LEFT_DOWN )
	{
		Set_Rotation_Z( 90, 1 );
	}
	else if( start_direction == DIR_RIGHT_UP )
	{
		Set_Rotation_Z( 270, 1 );
	}
	else if( start_direction == DIR_RIGHT_DOWN )
	{
		Set_Rotation_Z( 270, 1 );
		Set_Rotation_X( 180, 1 );
	}
	else if( start_direction == DIR_DOWN_LEFT )
	{
		Set_Rotation_X( 180, 1 );
	}
	else if( start_direction == DIR_DOWN_RIGHT )
	{
		Set_Rotation_Z( 180, 1 );
	}

	Create_Name();
}

void cEato :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;

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

void cEato :: DieStep( void )
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

void cEato :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	walk_count += 0.2f * pFramerate->speedfactor;

	if( walk_count >= 4 )
	{
		walk_count = 0;
	}

	Set_Image( static_cast<int>(walk_count) );
}

bool cEato :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cEato :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		return 2;
	}
	if( obj->type == TYPE_BALL )
	{
		return 2;
	}

	return 0;
}

void cEato :: Handle_Collision_Player( cObjectCollision *collision )
{
	// unknown direction
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	// only if not invincible
	if( pPlayer->invincible <= 0 )
	{
		// if player is big and not a bottom collision
		if( pPlayer->maryo_type != MARYO_SMALL && ( collision->direction != DIR_BOTTOM ) )
		{
			// todo : create again
			//pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );
			pPlayer->Action_Jump( 1 );
		}

		pPlayer->DownGrade();
	}
}

void cEato :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// image dir
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_eato_image_dir" ));
	Editor_Add( UTF8_("Image directory"), UTF8_("Directory containing the images"), editbox, 200 );

	editbox->setText( img_dir.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cEato::Editor_Image_Dir_Key, this ) );
	// init
	Editor_Init();
}

bool cEato :: Editor_Image_Dir_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Dir( str_text );

	return 1;
}

void cEato :: Create_Name( void )
{
	name = "Eato ";
	name += _(Get_Direction_Name( start_direction ).c_str());

	if( start_image && !start_image->name.empty() )
	{
		name += " " + start_image->name;
	}
}
