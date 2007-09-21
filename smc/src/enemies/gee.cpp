/***************************************************************************
              gee.cpp  -  Electro, Lava or Gift monster
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

#include "../enemies/gee.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** cGee *** *** *** *** *** *** *** *** *** *** *** */

cGee :: cGee( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cGee :: cGee( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cGee :: ~cGee( void )
{
	//
}

void cGee :: Init( void  )
{
	type = TYPE_GEE;
	player_range = 1000;
	posz = 0.088f;

	state = STA_STAY;
	dest_velx = 0;
	dest_vely = 0;
	Set_Max_Distance( 400 );
	always_fly = 0;
	wait_time = 2;
	fly_distance = 400;

	Set_Direction( DIR_HORIZONTAL );
	color_type = COL_DEFAULT;
	Set_Color( COL_YELLOW );

	kill_sound = "enemy/gee/die.ogg";

	walk_count = (float)( rand() % 4 );

	wait_time_counter = 0;
	fly_distance_counter = 0;
	clouds_counter = 0;
}

cGee *cGee :: Copy( void )
{
	cGee *gee = new cGee( startposx, startposy );
	gee->Set_Direction( start_direction );
	gee->Set_Max_Distance( max_distance );
	gee->always_fly = always_fly;
	gee->wait_time = wait_time;
	gee->fly_distance = fly_distance;
	gee->Set_Color( color_type );

	return gee;
}

void cGee :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( attributes.getValueAsInteger( "max_distance", max_distance ) );
	// always fly
	always_fly = attributes.getValueAsBool( "always_fly", always_fly );
	// wait time
	wait_time = attributes.getValueAsFloat( "wait_time", wait_time );
	// fly distance
	fly_distance = attributes.getValueAsInteger( "fly_distance", fly_distance );
	// color
	Set_Color( (DefaultColor)Get_Color_id( attributes.getValueAsString( "color", Get_Color_name( color_type ) ).c_str() ) );
}

void cGee :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"gee\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << (int)max_distance << "\" />" << std::endl;
	// always fly
	file << "\t\t<Property name=\"always_fly\" value=\"" << always_fly << "\" />" << std::endl;
	// wait time
	file << "\t\t<Property name=\"wait_time\" value=\"" << wait_time << "\" />" << std::endl;
	// fly distance
	file << "\t\t<Property name=\"fly_distance\" value=\"" << (int)fly_distance << "\" />" << std::endl;
	// color
	file << "\t\t<Property name=\"color\" value=\"" << Get_Color_name( color_type ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cGee :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	// horizontal
	if( dir == DIR_HORIZONTAL || dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		dir = DIR_HORIZONTAL;
		dest_vely = 0;
		dest_velx = 3;
	}
	// vertical
	else
	{
		dir = DIR_VERTICAL;
		dest_vely = 3;
		dest_velx = 0;
	}

	cEnemy::Set_Direction( dir, 1 );

	Create_Name();
}

void cGee :: Set_Max_Distance( int nmax_distance )
{
	max_distance = nmax_distance;

	if( max_distance < 0 )
	{
		max_distance = 0;
	}
}

void cGee :: Set_Color( DefaultColor col )
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

	// Electro
	if( color_type == COL_YELLOW )
	{
		filename_dir = "electro";
		kill_points = 50;

		fire_resistant = 0;
	}
	// Lava
	else if( color_type == COL_RED )
	{
		filename_dir = "lava";

		if( dest_velx )
		{
			dest_velx = 4;
		}
		if( dest_vely )
		{
			dest_vely = 4;
		}
		kill_points = 100;

		fire_resistant = 1;
	}
	// Venom
	else if( color_type == COL_GREEN )
	{
		filename_dir = "venom";
		if( dest_velx )
		{
			dest_velx = 5;
		}
		if( dest_vely )
		{
			dest_vely = 5;
		}
		kill_points = 200;

		fire_resistant = 0;
	}

	Create_Name();

	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/3.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/4.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/5.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/6.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/7.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/8.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/9.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/gee/" + filename_dir + "/10.png" ) );

	Set_Image( 5, 1 );
}

void cGee :: DownGrade( bool force /* = 0 */ )
{
	Set_Dead( 1 );
	massivetype = MASS_PASSIVE;
	counter = 0;
	velx = 0;
	vely = 0;

	if( !force )
	{
		for( unsigned int i = 0; i < 20; i++ )
		{
			Generate_Particles();
		}
	}
	else
	{
		Set_RotationZ( 180 );
	}
}

void cGee :: DieStep( void )
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
		else if( posy < GAME_RES_H + col_rect.h )
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

void cGee :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// staying
	if( state == STA_STAY )
	{
		wait_time_counter += pFramerate->speedfactor;

		// if wait time reached or always fly
		if( wait_time_counter > wait_time * DESIRED_FPS || always_fly )
		{
			Activate();
		}
	}
	// moving
	else
	{
		Col_Move( velx, vely, 0, 1, 0 );

		// update fly distance counter
		if( velx > 0 )
		{
			fly_distance_counter += velx;
		}
		else if( velx < 0 )
		{
			fly_distance_counter -= velx;
		}
		if( vely > 0 )
		{
			fly_distance_counter += vely;
		}
		else if( vely < 0 )
		{
			fly_distance_counter -= vely;
		}

		// check out of map screen
		if( direction == DIR_DOWN )
		{
			// if below screen move back
			if( rect.y + col_rect.h > GAME_RES_H )
			{
				Turn_Around( DIR_DOWN );
			}
		}
		// check out of max distance
		if( !Check_Max_Distance() )
		{
			Stop();
		}

		// generate particle clouds
		clouds_counter += pFramerate->speedfactor * 0.4f;

		while( clouds_counter > 0 )
		{
			Generate_Particles();

			clouds_counter--;
		}

		// walk_distance reached
		if( !always_fly && fly_distance_counter > fly_distance )
		{
			Stop();
		}
	}

	walk_count += pFramerate->speedfactor * 0.3f;

	if( walk_count >= 10 )
	{
		walk_count = 0;
	}

	Set_Image( (int)walk_count );
}

void cGee :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// draw distance rect
	if( editor_level_enabled )
	{
		if( start_direction == DIR_HORIZONTAL )
	    {
            pVideo->Draw_Rect( startposx - pCamera->x - max_distance, startposy + ( rect.h * 0.5f ) - 5 - pCamera->y, col_rect.w + ( max_distance * 2 ), 10, posz - 0.000001f, &whitealpha128 );
	    }
		else if( start_direction == DIR_VERTICAL )
	    {
            pVideo->Draw_Rect( startposx + ( rect.w * 0.5f ) - 5 - pCamera->x, startposy - pCamera->y - max_distance, 10, col_rect.h + ( max_distance * 2 ), posz - 0.000001f, &whitealpha128 );
	    }
	}

	cEnemy::Draw( request );
}

void cGee :: Activate( void )
{
	// if empty maximum or walk distance
	if( !max_distance || !fly_distance )
	{
		return;
	}

	wait_time_counter = 0;
	state = STA_WALK;

	// set velocity
	velx = dest_velx;
	vely = dest_vely;

	// random direction
	if( rand() % 2 != 1 )
	{
		velx *= -1;
	}
	if( rand() % 2 != 1 )
	{
		vely *= -1;
	}

	Update_Direction();

	// check max distance
	if( !Check_Max_Distance() )
	{
		Turn_Around();
	}

	Update_Rotation_velx();
}

void cGee :: Stop( void )
{
	fly_distance_counter = 0;
	state = STA_STAY;
	velx = 0;
	vely = 0;
}

void cGee :: Generate_Particles( void )
{
	cParticleAnimation *anim = new cParticleAnimation( posx + 15 + ( rand() % (int)( col_rect.w / 2 ) ), posy + ( ( rand() % (int)( col_rect.h / 2 ) ) ) );
	anim->Set_Quota( 4 );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/cloud.png" ) );

	if( !dead )
	{
		anim->Set_Speed( 0.3f, 0.2f );

		// direction
		if( direction == DIR_LEFT )
		{
			anim->Set_DirectionRange( 90, 180 );
		}
		else
		{
			anim->Set_DirectionRange( 270, 180 );
		}
	}
	else
	{
		anim->Set_Speed( 0.1f, 0.6f );
	}

	anim->Set_Scale( 0.5f, 0.3f );

	// color
	if( color_type == COL_YELLOW )
	{
		anim->Set_Color( yellow );
	}
	else if( color_type == COL_RED )
	{
		anim->Set_Color( lightred );
	}
	else if( color_type == COL_GREEN )
	{
		anim->Set_Color( lightgreen );
	}
	anim->Set_Time_to_Live( 2 );
	anim->Set_Fading_Alpha( 1 );
	anim->Set_Blending( BLEND_ADD );
	pAnimationManager->Add( anim );
}

bool cGee :: Check_Max_Distance( void )
{
	if( direction == DIR_UP )
	{
		if( posy - startposy < -max_distance )
		{
			return 0;
		}
	}
	else if( direction == DIR_DOWN )
	{
		if( posy - startposy > max_distance )
		{
			return 0;
		}
	}
	else if( direction == DIR_LEFT )
	{
		if( posx - startposx < -max_distance )
		{
			return 0;
		}
	}
	else if( direction == DIR_RIGHT )
	{
		if( posx - startposx > max_distance )
		{
			return 0;
		}
	}

	return 1;
}

bool cGee :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cGee :: Validate_Collision( cSprite *obj )
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

void cGee :: Handle_Collision_Player( cObjectCollision *collision )
{
	// unknown direction
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
	else if( !pPlayer->invincible )
	{
		if( pPlayer->maryo_type != MARYO_SMALL )
		{
			// todo : create again
			//pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );

			if( collision->direction == DIR_BOTTOM  )
			{
				pPlayer->start_enemyjump = 1;
			}
			else if( collision->direction == DIR_LEFT )
			{
				pPlayer->velx = -7;
			}
			else if( collision->direction == DIR_RIGHT )
			{
				pPlayer->velx = 7;
			}
		}

		pPlayer->DownGrade();
		Turn_Around( collision->direction );
	}
}

void cGee :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// max distance
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_gee_max_distance" ));
	editbox->setTooltipText( "Max Distance" );
	Editor_Add( editbox, 90 );

	editbox->setText( int_to_string( max_distance ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cGee::Editor_Max_Distance_Key, this ) );

	// always fly
	Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_gee_always_fly" ));
	combobox->getEditbox()->setTooltipText( "Always Fly" );
	Editor_Add( combobox, 120, 80 );

	combobox->addItem( new ListboxTextItem( "Enabled" ) );
	combobox->addItem( new ListboxTextItem( "Disabled" ) );

	if( always_fly )
	{
		combobox->setText( "Enabled" );
	}
	else
	{
		combobox->setText( "Disabled" );
	}

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cGee::Editor_Always_Fly_Select, this ) );

	// wait time
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_gee_wait_time" ));
	editbox->setTooltipText( "Wait Time" );
	Editor_Add( editbox, 90 );

	editbox->setText( float_to_string( wait_time ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cGee::Editor_Wait_Time_Key, this ) );

	// fly distance
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_gee_fly_distance" ));
	editbox->setTooltipText( "Fly Distance" );
	Editor_Add( editbox, 90 );

	editbox->setText( int_to_string( fly_distance ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cGee::Editor_Fly_Distance_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cGee :: Editor_Max_Distance_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( string_to_int( str_text ) );

	return 1;
}

bool cGee :: Editor_Always_Fly_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( "Enabled" ) == 0 )
	{
		always_fly = 1;
	}
	else
	{
		always_fly = 0;
	}

	return 1;
}

bool cGee :: Editor_Wait_Time_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	wait_time = string_to_float( str_text );

	return 1;
}

bool cGee :: Editor_Fly_Distance_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	fly_distance = string_to_int( str_text );

	return 1;
}

void cGee :: Create_Name( void )
{
    name = "Gee";

    if( color_type == COL_YELLOW )
    {
        name += "lectro";
    }
    else if( color_type == COL_RED )
    {
        name += "lava";
    }
    else if( color_type == COL_GREEN )
    {
        name += "venom";
    }

    if( start_direction == DIR_HORIZONTAL )
    {
        name += " Hor";
    }
    else if( start_direction == DIR_VERTICAL )
    {
        name += " Ver";
    }
}
