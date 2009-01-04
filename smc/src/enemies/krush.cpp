/***************************************************************************
 * krush.cpp  -  The little dinosaur
 *
 * Copyright (C) 2004 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/krush.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../gui/hud.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/i18n.h"

/* *** *** *** *** *** cKrush *** *** *** *** *** *** *** *** *** *** *** *** */

cKrush :: cKrush( float x, float y )
: cEnemy( x, y )
{
	cKrush::Init();
}

cKrush :: cKrush( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cKrush::Init();
	cKrush::Create_from_Stream( attributes );
}

cKrush :: ~cKrush( void )
{
	//
}

void cKrush :: Init( void  )
{
	type = TYPE_KRUSH;
	posz = 0.093f;

	state = STA_WALK;
	krush_state = KRUSH_WALK;

	speed = 6;

	images.push_back( pVideo->Get_Surface( "enemy/krush/big_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/big_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/big_3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/big_4.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/small_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/small_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/small_3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/krush/small_4.png" ) );

	Set_Direction( DIR_RIGHT );

	Set_Image( 0, 1 );

	kill_sound = "enemy/krush/die.ogg";
	kill_points = 20;
}

cKrush *cKrush :: Copy( void )
{
	cKrush *krush = new cKrush( startposx, startposy );
	krush->Set_Direction( start_direction );

	return krush;
}

void cKrush :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
}

void cKrush :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"krush\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cKrush :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	// krush_state
	if( save_object->exists( "krush_state" ) )
	{
		krush_state = static_cast<Krush_state>(string_to_int( save_object->Get_Value( "krush_state" ) ));

		if( krush_state == KRUSH_SMALL )
		{
			// todo : create a Set_State function
			// set small image without position changes
			cSprite::Set_Image( images[4] );
			kill_points = 40;
		}
	}

	Update_Rotation_Hor_velx();
}

cSave_Level_Object *cKrush :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_to_Savegame();

	// krush_state ( only save if needed )
	if( krush_state != KRUSH_WALK )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "krush_state", int_to_string( krush_state ) ) );
	}

	return save_object;
}

void cKrush :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	name = "Krush ";
	name += _(Get_Direction_Name( start_direction ).c_str());

	if( start_direction == DIR_RIGHT )
	{
		velx = 2.5f;
	}
	else
	{
		velx = -2.5f;
	}

	Update_Rotation_Hor_velx( 1 );
}

void cKrush :: DownGrade( bool force /* = 0 */ )
{
	// default stomp downgrade
	if( !force )
	{
		// big to small walking
		if( krush_state == KRUSH_WALK )
		{
			krush_state = KRUSH_SMALL;
			state = STA_RUN;
			velx = ( direction == DIR_RIGHT ) ? (speed) : (-speed);
			counter = 4;
			kill_points = 40;

			Set_Image( static_cast<int>(counter) );
			Col_Move( 0, images[3]->col_h - images[4]->col_h, 1, 1 );
			Update_Direction();

			// animation
			cParticle_Emitter *anim = new cParticle_Emitter();
			anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 4 ) );
			Generate_Hit_Animation( anim );

			anim->Set_Speed( 3.5f, 0.6f );
			anim->Set_Fading_Alpha( 1 );
			// add animation
			pAnimation_Manager->Add( anim );
		}
		else if( krush_state == KRUSH_SMALL )
		{
			Set_Scale_Directions( 1, 0, 1, 1 );
			Set_Dead( 1 );

			// animation
			cParticle_Emitter *anim = new cParticle_Emitter();
			anim->Set_Pos( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
			Generate_Hit_Animation( anim );

			anim->Set_Speed( 4.5f, 1.6f );
			anim->Set_Scale( 0.6f );
			// add animation
			pAnimation_Manager->Add( anim );
		}
	}
	// falling death
	else
	{
		Set_Dead( 1 );
		Set_Rotation_Z( 180 );
	}

	if( dead )
	{
		krush_state = KRUSH_DEAD;
		state = STA_STAY;
		massivetype = MASS_PASSIVE;
		counter = 0;
		velx = 0;
		vely = 0;
	}
}

void cKrush :: DieStep( void )
{
	counter += pFramerate->speedfactor;

	// stomp death
	if( rotz != 180 )
	{
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

void cKrush :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( krush_state == KRUSH_WALK )
	{
		counter += pFramerate->speedfactor * 0.3f;

		if( counter >= 4 )
		{
			counter = 0;
		}
	}
	else
	{
		counter += pFramerate->speedfactor * 0.5f;

		if( counter >= 8 )
		{
			counter = 4;
		}
	}

	Set_Image( static_cast<int>(counter) );

	// gravity
	Update_Gravity();

	Update_Rotation_Hor_velx();
}

bool cKrush :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cKrush :: Validate_Collision( cSprite *obj )
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
		// if moving downwards and the object is on bottom
		if( vely >= 0 && Is_on_Top( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cKrush :: Handle_Collision_Player( cObjectCollision *collision )
{
	// invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pointsdisplay->Add_Points( kill_points, pPlayer->posx, pPlayer->posy, "", static_cast<Uint8>(255), 1 );
		pAudio->Play_Sound( kill_sound );


		// big walking
		if( krush_state == KRUSH_WALK )
		{
			DownGrade();
		}
		// small walking
		else if( krush_state == KRUSH_SMALL )
		{
			DownGrade();
			pPlayer->Add_Kill_Multiplier();
		}

		pPlayer->Action_Jump( 1 );
	}
	else
	{
		pPlayer->DownGrade_Player();
		Turn_Around( collision->direction );
	}
}

void cKrush :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
	Send_Collision( collision );
}

void cKrush :: Handle_Collision_Massive( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
	Send_Collision( collision );
}

void cKrush :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_krush_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction"), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( start_direction ) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cKrush::Editor_Direction_Select, this ) );

	// init
	Editor_Init();
}

bool cKrush :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}