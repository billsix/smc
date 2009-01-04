/***************************************************************************
 * rokko.cpp  -  giant, slow-moving bullet
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

#include "../enemies/rokko.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../video/animation.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../core/math/utilities.h"
#include "../input/mouse.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cRokko *** *** *** *** *** *** *** *** *** *** *** */

cRokko :: cRokko( float x, float y )
: cEnemy( x, y )
{
	cRokko::Init();
}

cRokko :: cRokko( CEGUI::XMLAttributes &attributes )
: cEnemy()
{
	cRokko::Init();
	cRokko::Create_from_Stream( attributes );
}

cRokko :: ~cRokko( void )
{
	//
}

void cRokko :: Init( void  )
{
	type = TYPE_ROKKO;
	massivetype = MASS_PASSIVE;
	posz = 0.03f;
	editor_posz = 0.09f;
	can_be_on_ground = 0;
	player_range = 4000;
	Set_Visible( 0 );

	fire_resistant = 1;
	ice_resistance = 1;

	Set_Direction( DIR_LEFT );
	Set_Speed( 8.5f );
	Set_Max_Distance_Front( 1000 );
	Set_Max_Distance_Sides( 400 );
	state = STA_STAY;

	smoke_counter = 0;

	kill_sound = "stomp_1.ogg";
	kill_points = 250;
}

cRokko *cRokko :: Copy( void )
{
	cRokko *rokko = new cRokko( startposx, startposy );
	rokko->Set_Direction( start_direction );
	rokko->Set_Speed( speed );

	return rokko;
}

void cRokko :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", speed ) );
}

void cRokko :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"rokko\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cRokko :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	// Don't activate if dead
	if( dead )
	{
		return;
	}

	// activate
	if( state == STA_FLY )
	{
		Activate();
	}
}

void cRokko :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( start_direction == dir )
	{
		return;
	}

	// clear old images
	Clear_Images();

	cEnemy::Set_Direction( dir, 1 );
	name = "Rokko ";
	name += _(Get_Direction_Name(start_direction).c_str());

	if( direction == DIR_LEFT )
	{
		images.push_back( pVideo->Get_Surface( "enemy/rokko/l.png" ) );
	}
	else if( direction == DIR_RIGHT )
	{
		images.push_back( pVideo->Get_Surface( "enemy/rokko/r.png" ) );
	}
	else
	{
		printf( "Warning: Unknown Rokko direction %s\n", Get_Direction_Name( dir ).c_str() );
	}

	Update_Distance_rect();
	Set_Image( 0, 1 );
}

void cRokko :: Set_Speed( float nspeed )
{
	if( nspeed < 2 )
	{
		nspeed = 2;
	}

	speed = nspeed;
}

void cRokko :: Set_Max_Distance_Front( float distance )
{
	if( distance < 200 )
	{
		distance = 200;
	}

	max_distance_front = distance;

	Update_Distance_rect();
}

void cRokko :: Set_Max_Distance_Sides( float distance )
{
	if( distance < 50 )
	{
		distance = 50;
	}

	max_distance_sides = distance;

	Update_Distance_rect();
}

void cRokko :: Activate( void )
{
	pAudio->Play_Sound( "enemy/rokko/activate.ogg" );

	state = STA_FLY;
	walk_count = 1;
	massivetype = MASS_MASSIVE;
	Set_Visible( 1 );

	if( direction == DIR_LEFT )
	{
		Set_Velocity( -speed, 0 );
	}
	else
	{
		Set_Velocity( speed, 0 );
	}
}

void cRokko :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	vely = 0;

	if( !force )
	{
		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( pPlayer->posx + ( pPlayer->col_rect.w / 2 ), posy + 5 );
		Generate_Hit_Animation( anim );

		anim->Set_Quota( 8 );
		anim->Set_Speed( 4, 1 );
		anim->Set_Scale( 0.9f );
		// add animation
		pAnimation_Manager->Add( anim );
	}
}

void cRokko :: DieStep( void )
{
	// add gravity
	if( vely < 25 )
	{
		Add_Velocity( 0, 1.5f );
	}

	Move( velx, vely );

	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		if( rotz < 90 )
		{
			Add_Rotation_Z( pFramerate->speedfactor );
		}
	}

	// generate smoke
	smoke_counter += pFramerate->speedfactor * 10;
	if( smoke_counter >= 1 )
	{
		Generate_Smoke( static_cast<int>(smoke_counter) );
		smoke_counter -= static_cast<int>(smoke_counter);
	}

	// below ground
	if( col_rect.y - 200 > pActive_Camera->limit_rect.y + game_res_h )
	{
		rotz = 0;
		massivetype = MASS_PASSIVE;
		Set_Visible( 0 );
		velx = 0;
	}
}

void cRokko :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// if not active
	if( state != STA_FLY )
	{
		GL_rect final_distance = Get_Final_Distance_Rect();

		// if player is in front then activate
		if( pPlayer->maryo_type != MARYO_GHOST && Col_Box( &pPlayer->col_rect, &final_distance ) )
		{
			Activate();
		}
		else
		{
			return;
		}
	}

	// generate smoke
	smoke_counter += pFramerate->speedfactor * 10;
	if( smoke_counter >= 1 )
	{
		Generate_Smoke( static_cast<int>(smoke_counter) );
		smoke_counter -= static_cast<int>(smoke_counter);
	}
}

void cRokko :: Draw( cSurfaceRequest *request /* = NULL */ )
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

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cSurfaceRequest();
	}

	// Draw
	cEnemy::Draw( request );

	// alpha in debug mode
	if( editor_level_enabled )
	{
		request->color.alpha = 64;
	}

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cRokko :: Update_Distance_rect( void )
{
	if( start_direction == DIR_UP )
	{
		distance_rect.x = ((col_pos.x + col_rect.w) * 0.5f) - (max_distance_sides * 0.5f);
		distance_rect.y = -max_distance_front;
		distance_rect.w = max_distance_sides;
		distance_rect.h = max_distance_front;
	}
	else if( start_direction == DIR_DOWN )
	{
		distance_rect.x = ((col_pos.x + col_rect.w) * 0.5f) - (max_distance_sides * 0.5f);
		distance_rect.y = 0;
		distance_rect.w = max_distance_sides;
		distance_rect.h = max_distance_front;
	}
	else if( start_direction == DIR_LEFT )
	{
		distance_rect.x = -max_distance_front;
		distance_rect.y = ((col_pos.y + col_rect.h) * 0.5f) - (max_distance_sides * 0.5f);
		distance_rect.w = max_distance_front;
		distance_rect.h = max_distance_sides;
	}
	else if( start_direction == DIR_RIGHT )
	{
		distance_rect.x = 0;
		distance_rect.y = ((col_pos.y + col_rect.h) * 0.5f) - (max_distance_sides * 0.5f);
		distance_rect.w = max_distance_front;
		distance_rect.h = max_distance_sides;
	}
}

GL_rect cRokko :: Get_Final_Distance_Rect( void )
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

void cRokko :: Generate_Smoke( unsigned int quota /*= 10 */ )
{
	cParticle_Emitter *anim = NULL;

	// moving smoke particle animation
	while( quota > 0 )
	{
		anim = new cParticle_Emitter();

		// not dead
		if( !dead )
		{				
			if( direction == DIR_LEFT )
			{
				anim->Set_Pos( posx + col_rect.w - Get_Random_Float( 5, 8 ), posy + Get_Random_Float( 0, rect.h - 15 ) );
				anim->Set_Direction_Range( 320, 100 );
			}
			else
			{
				anim->Set_Pos( posx + Get_Random_Float( 5, 8 ), posy + Get_Random_Float( 0, rect.h - 15 ) );
				anim->Set_Direction_Range( 140, 100 );
			}

			anim->Set_Scale( 1.1f );
		}
		// dead
		else
		{
			anim->Set_Pos( posx + Get_Random_Float( 0, col_rect.w ), posy + Get_Random_Float( 0, rect.h ) );
			anim->Set_Scale( 0.5f );
		}

		// - 0.000001f caused a weird graphical z pos bug with an ATI card
		anim->Set_Pos_Z( posz - 0.000005f );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke_black.png" ) );
		anim->Set_Time_to_Live( 0.3f, 1 );
		anim->Set_Speed( 1, 1 );
		anim->Set_Const_Rotation_Z( -1, 2 );
		anim->Set_Color( Color( static_cast<Uint8>(155), 150, 130 ) );
		anim->Set_Fading_Alpha( 1 );
		
		// add animation
		pAnimation_Manager->Add( anim );

		if( dead )
		{
			// random sparks
			if( ( rand() % 25 ) == 0 )
			{
				// animation
				anim = new cParticle_Emitter();
				anim->Set_Pos( posx + col_rect.w * 0.2f + Get_Random_Float( 0,  col_rect.w * 0.6f ), posy + rect.h * 0.2f + Get_Random_Float( 0, rect.h * 0.6f ) );
				anim->Set_Pos_Z( posz + 0.000001f );
				anim->Set_Quota( 8 );
				anim->Set_Time_to_Live( 0.3f, 0.1f );
				anim->Set_Speed( 4, 1.5f );
				anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
				anim->Set_Color( Color( static_cast<Uint8>(255), 245, 0 ), Color( static_cast<Uint8>(0), rand() % 10, 0 ) );
				anim->Set_Scale( 0.5f );
				anim->Set_Fading_Size( 1 );
				anim->Set_Fading_Alpha( 0 );
				// add animation
				pAnimation_Manager->Add( anim );
			}
		}

		quota--;
	}
}

bool cRokko :: Is_Update_Valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

bool cRokko :: Is_Draw_Valid( void )
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

unsigned int cRokko :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		return 1;
	}
	if( obj->type == TYPE_BALL )
	{
		return 1;
	}

	return 0;
}

void cRokko :: Handle_Collision_Player( cObjectCollision *collision )
{
	// if invincible
	if( pPlayer->invincible > 0 || collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pointsdisplay->Add_Points( kill_points, pPlayer->posx + pPlayer->image->w / 3, pPlayer->posy - 5, "", static_cast<Uint8>(255), 1 );
		pAudio->Play_Sound( kill_sound );
		pPlayer->Action_Jump( 1 );

		pPlayer->Add_Kill_Multiplier();
		DownGrade();
	}
	else
	{
		pPlayer->DownGrade_Player();
	}
}

void cRokko :: Handle_out_of_Level( ObjectDirection dir )
{
	// ignore
}

void cRokko :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_rokko_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction"), combobox, 100, 75 );

	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );

	combobox->setText( Get_Direction_Name( start_direction ) );
	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cRokko::Editor_Direction_Select, this ) );

	// speed
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_rokko_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Speed when activated"), editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cRokko::Editor_Speed_Key, this ) );

	// init
	Editor_Init();
}

bool cRokko :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cRokko :: Editor_Speed_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}
