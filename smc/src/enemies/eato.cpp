/***************************************************************************
 * eato.cpp  -  eating static plant :P
 *
 * Copyright (C) 2006 - 2009 Florian Richter
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
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;
// CEGUI
#include "CEGUIXMLAttributes.h"

namespace SMC
{

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
	cEato::Create_From_Stream( attributes );
}

cEato :: ~cEato( void )
{
	//
}

void cEato :: Init( void )
{
	m_type = TYPE_EATO;
	m_player_range = 1000;
	m_pos_z = 0.087f;
	m_can_be_on_ground = 0;
	Set_Rotation_Affects_Rect( 1 );
	fire_resistant = 1;

	m_state = STA_STAY;
	Set_Image_Dir( "enemy/eato/brown/" );
	Set_Direction( DIR_UP_LEFT );

	walk_count = static_cast<float>( rand() % 4 );

	kill_sound = "enemy/eato/die.ogg";
	kill_points = 150;
}

cEato *cEato :: Copy( void )
{
	cEato *eato = new cEato( m_start_pos_x, m_start_pos_y );
	eato->Set_Image_Dir( img_dir );
	eato->Set_Direction( m_start_direction );

	return eato;
}

void cEato :: Create_From_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// image directory
	Set_Image_Dir( attributes.getValueAsString( "image_dir", img_dir ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( m_start_direction ) ).c_str() ) );
}

void cEato :: Save_To_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"eato\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(m_start_pos_x) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(m_start_pos_y) << "\" />" << std::endl;
	// image directory
	file << "\t\t<Property name=\"image_dir\" value=\"" << img_dir << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( m_start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cEato :: Set_Image_Dir( const std::string &dir )
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
	m_images.push_back( pVideo->Get_Surface( img_dir + "1.png" ) );
	m_images.push_back( pVideo->Get_Surface( img_dir + "2.png" ) );
	m_images.push_back( pVideo->Get_Surface( img_dir + "3.png" ) );
	m_images.push_back( pVideo->Get_Surface( img_dir + "2.png" ) );
	// set start image
	Set_Image_Num( 0, 1 );

	Create_Name();
}

void cEato :: Set_Direction( const ObjectDirection dir )
{
	// already set
	if( m_start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	// clear
	Set_Rotation( 0, 0, 0, 1 );

	if( m_start_direction == DIR_UP_LEFT )
	{
		Set_Rotation_Y( 180, 1 );
	}
	else if( m_start_direction == DIR_UP_RIGHT )
	{
		// default
	}
	else if( m_start_direction == DIR_LEFT_UP )
	{
		Set_Rotation_Z( 90, 1 );
		Set_Rotation_X( 180, 1 );
	}
	else if( m_start_direction == DIR_LEFT_DOWN )
	{
		Set_Rotation_Z( 90, 1 );
	}
	else if( m_start_direction == DIR_RIGHT_UP )
	{
		Set_Rotation_Z( 270, 1 );
	}
	else if( m_start_direction == DIR_RIGHT_DOWN )
	{
		Set_Rotation_Z( 270, 1 );
		Set_Rotation_X( 180, 1 );
	}
	else if( m_start_direction == DIR_DOWN_LEFT )
	{
		Set_Rotation_X( 180, 1 );
	}
	else if( m_start_direction == DIR_DOWN_RIGHT )
	{
		Set_Rotation_Z( 180, 1 );
	}

	Create_Name();
}

void cEato :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	m_massive_type = MASS_PASSIVE;
	counter = 0;

	if( !force )
	{
		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
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
	if( m_rot_z != 180 )
	{
		Set_Active( 0 );
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
		else if( m_pos_y < game_res_h + m_col_rect.m_h )
		{
			Move( 0, 20 );
		}
		// if below disable
		else
		{
			m_rot_z = 0;
			Set_Active( 0 );
		}
	}
}

void cEato :: Update( void )
{
	cEnemy::Update();

	if( !m_valid_update || !Is_In_Player_Range() )
	{
		return;
	}

	walk_count += 0.2f * pFramerate->speedfactor;

	if( walk_count >= 4 )
	{
		walk_count = 0;
	}

	Set_Image_Num( static_cast<int>(walk_count) );
}

bool cEato :: Is_Update_Valid( void )
{
	if( dead || m_freeze_counter )
	{
		return 0;
	}

	return 1;
}

Col_Valid_Type cEato :: Validate_Collision( cSprite *obj )
{
	if( obj->m_type == TYPE_PLAYER )
	{
		return COL_VTYPE_BLOCKING;
	}
	if( obj->m_type == TYPE_BALL )
	{
		return COL_VTYPE_BLOCKING;
	}

	return COL_VTYPE_NOT_VALID;
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

		pPlayer->DownGrade_Player();
	}
}

void cEato :: Editor_Activate( void )
{
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_eato_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction"), combobox, 100, 200 );

	combobox->addItem( new CEGUI::ListboxTextItem( "top_left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "top_right" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "bottom_left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "bottom_right" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left_top" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left_bottom" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right_top" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right_bottom" ) );

	combobox->setText( Get_Direction_Name( m_start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cEato::Editor_Direction_Select, this ) );

	// image dir
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_eato_image_dir" ));
	Editor_Add( UTF8_("Image directory"), UTF8_("Directory containing the images"), editbox, 200 );

	editbox->setText( img_dir.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cEato::Editor_Image_Dir_Text_Changed, this ) );
	// init
	Editor_Init();
}

bool cEato :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cEato :: Editor_Image_Dir_Text_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	std::string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Dir( str_text );

	return 1;
}

void cEato :: Create_Name( void )
{
	m_name = "Eato ";
	m_name += _(Get_Direction_Name( m_start_direction ).c_str());

	if( m_start_image && !m_start_image->m_name.empty() )
	{
		m_name += " " + m_start_image->m_name;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
