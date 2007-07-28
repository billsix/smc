/***************************************************************************
              rokko.cpp  -  giant, slow-moving bullet
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
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

/* *** *** *** *** *** *** cRokko *** *** *** *** *** *** *** *** *** *** *** */

cRokko :: cRokko( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cRokko :: cRokko( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
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
	Set_Visible( 0 );
	can_be_ground = 1;

	fire_resistant = 1;
	ice_resistance = 1;

	speed = 8.5f;
	Set_Direction( DIR_LEFT );
	state = STA_FLY;

	player_range = 4000;
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

void cRokko :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
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
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cRokko :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	// clear old images
	Clear_Images();

	cEnemy::Set_Direction( dir, 1 );

	if( direction == DIR_LEFT )
	{
		images.push_back( pVideo->Get_Surface( "enemy/rokko/l.png" ) );
		name = "Rokko Left";
	}
	else
	{
		images.push_back( pVideo->Get_Surface( "enemy/rokko/r.png" ) );
		name = "Rokko Right";
	}

	Set_Image( 0, 1 );
}

void cRokko :: Set_Speed( float nspeed )
{
	if( nspeed == 0 )
	{
		return;
	}

	speed = nspeed;
}

void cRokko :: Activate( void )
{
	pAudio->PlaySound( "enemy/rokko/activate.ogg" );

	walk_count = 1;
	massivetype = MASS_MASSIVE;
	Set_Visible( 1 );

	if( direction == DIR_LEFT )
	{
		Add_Velocity( -speed, 0, 1 );
	}
	else
	{
		Add_Velocity( speed, 0, 1 );
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
		cParticleAnimation *anim = new cParticleAnimation( pPlayer->posx + ( pPlayer->col_rect.w / 2 ), posy + 5 );
		Generate_Hit_Animation( anim );

		anim->Set_Quota( 8 );
		anim->Set_Speed( 4, 1 );
		anim->Set_Scale( 0.9f );
		// add animation
		pAnimationManager->Add( anim );
	}
}

void cRokko :: DieStep( void )
{
	Move( 0, 17 );

	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		if( rotz < 90 )
		{
			Add_RotationZ( pFramerate->speedfactor );
		}
	}

	// generate smoke
	smoke_counter += pFramerate->speedfactor * 10;
	Generate_Smoke( (int)smoke_counter );
	smoke_counter -= (int)smoke_counter;

	if( posy > 600 )
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

	// not active
	if( massivetype == MASS_PASSIVE )
	{
		// player is ghost
		if( pPlayer->maryo_type == MARYO_GHOST )
		{
			return;
		}

		// not in vertical range
		if( col_rect.y < pPlayer->col_rect.y - 100 || col_rect.y > pPlayer->col_rect.y + 100 )
		{
			return;
		}

		if( direction == DIR_LEFT )
		{
			// in range
			if( pPlayer->col_rect.x + 1000 > col_rect.x && pPlayer->col_rect.x + 400 < col_rect.x )
			{
				Activate();
			}
			// not in range
			else
			{
				return;
			}
		}
		// right
		else
		{
			// in range
			if( pPlayer->col_rect.x - 1000 > col_rect.x && pPlayer->col_rect.x + 400 > col_rect.x + col_rect.w )
			{
				Activate();
			}
			// not in range
			else
			{
				return;
			}
		}
	}

	// generate smoke
	smoke_counter += pFramerate->speedfactor * 10;
	Generate_Smoke( (int)smoke_counter );
	smoke_counter -= (int)smoke_counter;
}

void cRokko :: Generate_Smoke( unsigned int quota /*= 10 */ )
{
	cParticleAnimation *anim = NULL;

	// moving smoke particle animation
	while( quota > 0 )
	{
		// not dead
		if( !dead )
		{
			if( direction == DIR_LEFT )
			{
				anim = new cParticleAnimation( posx + col_rect.w - 5 - ( rand() % 3 ), posy + ( rand() % (int)( rect.h - 15 ) ) );
				
				anim->Set_DirectionRange( 320, 100 );
			}
			else
			{
				anim = new cParticleAnimation( posx - 5 - ( rand() % 3 ), posy + ( rand() % (int)( rect.h - 15 ) ) );
				anim->Set_DirectionRange( 140, 100 );
			}
		}
		// dead
		else
		{
			anim = new cParticleAnimation( posx + ( rand() % + (int)col_rect.w ), posy + ( rand() % (int)rect.h ) );
		}

		anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke_black.png" ) );
		anim->Set_Time_to_Live( 0.3f, 1 );
		anim->Set_Speed( 1, 1 );
		anim->Set_Scale( 1.3f );
		anim->Set_ConstRotationZ( -1, 2 );
		anim->Set_Color( Color( (Uint8)155, 150, 130 ) );
		anim->Set_Fading_Alpha( 1 );
		
		// add animation
		pAnimationManager->Add( anim );

		if( dead )
		{
			// random sparks
			if( ( rand() % 25 ) == 0 )
			{
				// animation
				anim = new cParticleAnimation( posx + col_rect.w * 0.2f + ( rand() % + (int)( col_rect.w * 0.6f ) ), posy + rect.h * 0.2f + ( rand() % (int)( rect.h * 0.6f ) ) );

				anim->Set_Quota( 8 );
				anim->Set_Time_to_Live( 0.3f, 0.1f );
				anim->Set_Speed( 4, 1.5f );
				anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
				anim->Set_Color( Color( (Uint8)255, 245, 0 ), Color( (Uint8)0, rand() % 10, 0 ) );
				anim->Set_Scale( 0.5f );
				anim->Set_Fading_Size( 1 );
				anim->Set_Fading_Alpha( 0 );
				// add animation
				pAnimationManager->Add( anim );
			}
		}

		quota--;
	}
}

bool cRokko :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
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

void cRokko :: Handle_Collision_Player( ObjectDirection cdirection )
{
	// if invincible
	if( pPlayer->invincible > 0 || cdirection == DIR_UNDEFINED )
	{
		return;
	}

	if( cdirection == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pointsdisplay->Add_Points( kill_points, pPlayer->posx + pPlayer->image->w / 3, pPlayer->posy - 5, "", (Uint8)255, 1 );
		pAudio->PlaySound( kill_sound );
		pPlayer->start_enemyjump = 1;

		pPlayer->Add_Kill_Multiplier();
		DownGrade();
	}
	else
	{
		pPlayer->DownGrade();
	}
}

void cRokko :: Handle_OutofLevel( ObjectDirection dir )
{
	// ignore
}

void cRokko :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// speed
	Editbox *editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_rokko_speed" );
	editbox->setTooltipText( "Speed" );
	Editor_Add( editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cRokko::Editor_Speed_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cRokko :: Editor_Speed_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}
