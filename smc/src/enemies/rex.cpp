/***************************************************************************
           rex.cpp  -  The little dinosaur
                             -------------------
    copyright            : (C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../enemies/rex.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../gui/hud.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"

/* *** *** *** *** *** cRex *** *** *** *** *** *** *** *** *** *** *** *** */

cRex :: cRex( float x, float y )
: cEnemy( x, y )
{
	Init();
}

cRex :: cRex( XMLAttributes &attributes )
: cEnemy()
{
	Init();
	Create_from_Stream( attributes );
}

cRex :: ~cRex( void )
{
	//
}

void cRex :: Init( void  )
{
	type = TYPE_REX;
	posz = 0.093f;

	state = STA_WALK;
	rex_state = REX_WALK;

	speed = 6;

	images.push_back( pVideo->Get_Surface( "enemy/rex/big_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/rex/big_2.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/rex/small_1.png" ) );
	images.push_back( pVideo->Get_Surface( "enemy/rex/small_2.png" ) );

	Set_Direction( DIR_RIGHT );

	Set_Image( 0, 1 );

	kill_sound = "enemy/rex/die.ogg";
	kill_points = 20;
}

cRex *cRex :: Copy( void )
{
	cRex *rex = new cRex( startposx, startposy );
	rex->Set_Direction( start_direction );

	return rex;
}

void cRex :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
}

void cRex :: Save_to_Stream( ofstream &file )
{
	// begin enemy
	file << "\t<enemy>" << std::endl;

	// name
	file << "\t\t<Property name=\"type\" value=\"rex\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;

	// end enemy
	file << "\t</enemy>" << std::endl;
}

void cRex :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	cEnemy::Load_from_Savegame( save_object );

	// rex_state
	if( save_object->exists( "rex_state" ) )
	{
		rex_state = (Rex_state)string_to_int( save_object->Get_Value( "rex_state" ) );

		if( rex_state == REX_MINI )
		{
			// todo : create a Set_State function
			// set mini image without position changes
			cSprite::Set_Image( images[2] );
			kill_points = 40;
		}
	}

	Update_Rotation_velx();
}

cSave_Level_Object *cRex :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = cEnemy::Save_to_Savegame();

	// rex_state ( only save if needed )
	if( rex_state != REX_WALK )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "rex_state", int_to_string( rex_state ) ) );
	}

	return save_object;
}

void cRex :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( start_direction == dir )
	{
		return;
	}

	cEnemy::Set_Direction( dir, 1 );

	if( start_direction == DIR_RIGHT )
	{
		velx = 2.5f;
		name = "Rex Right";
	}
	else
	{
		velx = -2.5f;
	    name = "Rex Left";
	}

	Update_Rotation_velx( 1 );
}

void cRex :: DownGrade( bool force /* = 0 */ )
{
	// default stomp downgrade
	if( !force )
	{
		// to small walking
		if( rex_state == REX_WALK )
		{
			rex_state = REX_MINI;
			state = STA_RUN;
			velx = ( direction == DIR_RIGHT ) ? (speed) : (-speed);
			counter = 2;
			kill_points = 40;

			Set_Image( static_cast<int>(counter) );
			Col_Move( 0, images[1]->col_h - images[2]->col_h, 1, 1 );
			Update_Direction();

			// animation
			cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 4 ) );
			Generate_Hit_Animation( anim );

			anim->Set_Speed( 3.5f, 0.6f );
			anim->Set_Fading_Alpha( 1 );
			// add animation
			pAnimationManager->Add( anim );
		}
		else if( rex_state == REX_MINI )
		{
			Set_Dead( 1 );

			// animation
			cParticleAnimation *anim = new cParticleAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 2 ) );
			Generate_Hit_Animation( anim );

			anim->Set_Speed( 4.5f, 1.6f );
			anim->Set_Scale( 0.6f );
			// add animation
			pAnimationManager->Add( anim );
		}
	}
	// falling death
	else
	{
		Set_Dead( 1 );
		Set_RotationZ( 180 );
	}

	if( dead )
	{
		rex_state = REX_DEAD;
		state = STA_STAY;
		massivetype = MASS_PASSIVE;
		counter = 0;
		velx = 0;
		vely = 0;
	}
}

void cRex :: DieStep( void )
{
	counter += pFramerate->speedfactor;

	// stomp death
	if( rotz != 180 )
	{
		float speed = pFramerate->speedfactor * 0.05f;

		Add_ScaleX( -speed * 0.5f );
		Add_ScaleY( -speed );
		// todo : sprite scaling should also set the position
		Move( ( image->w * 0.5f ) * ( speed * 0.5f ), ( image->h ) * speed, 1 );

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

void cRex :: Update( void )
{
	cEnemy::Update();

	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( rex_state == REX_WALK )
	{
		counter += pFramerate->speedfactor * 0.2f;

		if( counter > 1.99f )
		{
			counter = 0;
		}
	}
	else
	{
		counter += pFramerate->speedfactor * 0.4f;

		if( counter > 3.99f )
		{
			counter = 2;
		}
	}

	Set_Image( static_cast<int>(counter) );

	// gravity
	Update_Gravity();

	Update_Rotation_velx();
}

bool cRex :: is_Update_valid( void )
{
	if( dead || freeze_counter )
	{
		return 0;
	}

	return 1;
}

unsigned int cRex :: Validate_Collision( cSprite *obj )
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

void cRex :: Handle_Collision_Player( cObjectCollision *collision )
{
	// invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	if( collision->direction == DIR_TOP && pPlayer->state != STA_FLY )
	{
		pointsdisplay->Add_Points( kill_points, pPlayer->posx, pPlayer->posy, "", static_cast<Uint8>(255), 1 );
		pAudio->PlaySound( kill_sound );


		// big walking
		if( rex_state == REX_WALK )
		{
			DownGrade();
		}
		// small walking
		else if( rex_state == REX_MINI )
		{
			DownGrade();
			pPlayer->Add_Kill_Multiplier();
		}

		pPlayer->start_enemyjump = 1;
	}
	else
	{
		pPlayer->DownGrade();
		Turn_Around( collision->direction );
	}
}

void cRex :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
	Send_Collision( collision );
}

void cRex :: Handle_Collision_Massive( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
}
