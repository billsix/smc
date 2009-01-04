/***************************************************************************
 * enemy.cpp  -  base class for all enemies
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

#include "../enemies/enemy.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../user/savegame.h"
#include "../core/game_core.h"

/* *** *** *** *** *** *** cEnemy *** *** *** *** *** *** *** *** *** *** *** */

cEnemy :: cEnemy( float x /* = 0 */, float y /* = 0 */ )
: cImageObjectSprite( x, y )
{
	sprite_array = ARRAY_ENEMY;
	type = TYPE_ENEMY;

	player_range = 1500;

	massivetype = MASS_MASSIVE;
	state = STA_FALL;
	dead = 0;
	can_be_ground = 1;

	counter = 0;
	walk_count = 0;

	kill_sound = "enemy/furball/die.ogg";
	kill_points = 10;

	fire_resistant = 0;
}

cEnemy :: ~cEnemy( void )
{

}

void cEnemy :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	// direction
	if( save_object->exists( "direction" ) )
	{
		direction = (ObjectDirection)string_to_int( save_object->Get_Value( "direction" ) );
	}

	// state
	if( save_object->exists( "state" ) )
	{
		state = (Moving_state)string_to_int( save_object->Get_Value( "state" ) );
	}

	// new position x
	if( save_object->exists( "new_posx" ) )
	{
		Set_Pos_X( string_to_float( save_object->Get_Value( "new_posx" ) ) );
	}

	// new position y
	if( save_object->exists( "new_posy" ) )
	{
		Set_Pos_Y( string_to_float( save_object->Get_Value( "new_posy" ) ) );
	}

	// velocity x
	if( save_object->exists( "velx" ) )
	{
		velx = string_to_float( save_object->Get_Value( "velx" ) );
	}

	// velocity y
	if( save_object->exists( "vely" ) )
	{
		vely = string_to_float( save_object->Get_Value( "vely" ) );
	}

	// visible
	if( save_object->exists( "visible" ) )
	{
		Set_Visible( string_to_int( save_object->Get_Value( "visible" ) ) > 0 );
	}

	// dead
	if( save_object->exists( "dead" ) )
	{
		Set_Dead( string_to_int( save_object->Get_Value( "dead" ) ) > 0 );
	}
}

cSave_Level_Object *cEnemy :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->type = type;
	save_object->properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(startposx) ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(startposy) ) ) );

	// direction
	save_object->properties.push_back( cSave_Level_Object_Property( "direction", int_to_string( direction ) ) );

	// state
	save_object->properties.push_back( cSave_Level_Object_Property( "state", int_to_string( state ) ) );

	// new position ( only save if needed )
	if( startposx != posx || startposy != posy )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "new_posx", int_to_string( static_cast<int>(posx) ) ) );
		save_object->properties.push_back( cSave_Level_Object_Property( "new_posy", int_to_string( static_cast<int>(posy) ) ) );
	}

	// velocity
	save_object->properties.push_back( cSave_Level_Object_Property( "velx", float_to_string( velx ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "vely", float_to_string( vely ) ) );

	// visible ( only save if needed )
	if( !visible )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "visible", int_to_string( visible ) ) );
	}

	// dead ( only save if needed )
	if( dead )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "dead", int_to_string( dead ) ) );
	}

	return save_object;
}

void cEnemy :: Set_Dead( bool enable /* = 1 */ )
{
	dead = enable;

	Update_Valid_Update();
}

void cEnemy :: DieStep( void )
{
	// virtual
}

void cEnemy :: Update( void )
{
	cMovingSprite::Update();

	// another object controls me
	if( state == STA_OBJ_LINKED )
	{
		massivetype = MASS_MASSIVE;
		Collision_Check( &col_rect );
		Parse_Collisions();
		massivetype = MASS_PASSIVE;
		return;
	}

	// dying animation
	if( dead && visible )
	{
		DieStep();
	}

	// frozen
	if( freeze_counter )
	{
		// update gravity
		if( type == TYPE_FURBALL || type == TYPE_TURTLE || type == TYPE_KRUSH || type == TYPE_SPIKA )
		{
			Update_Gravity();
			Col_Move( 0, vely );
		}
	}
}

void cEnemy :: Update_Gravity( void )
{
	if( !ground_object )
	{
		// add gravity
		if( vely < 25 )
		{
			Add_Velocity( 0, 1.5f );
		}
		// below ground
		if( col_rect.y > pActive_Camera->limit_rect.y + game_res_h )
		{
			DownGrade( 1 );
		}
	}
	else if( ground_object )
	{
		// stop falling
		if( vely > 0 )
		{
			vely = 0;
		}
	}
}

void cEnemy :: Generate_Hit_Animation( cParticle_Emitter *anim /* = NULL */ )
{
	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticle_Emitter();
	}

	anim->Set_Emitter_Rect( col_rect.x, posy + ( col_rect.h / 3 ), col_rect.w );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
	anim->Set_Quota( 4 );
	anim->Set_Pos_Z( posz - 0.000001f );
	anim->Set_Time_to_Live( 0.3f );
	anim->Set_Color( Color( static_cast<Uint8>(150), 150, 150, 200 ), Color( static_cast<Uint8>( rand() % 155 ), rand() % 155, rand() % 155 ) );
	anim->Set_Speed( 3, 0.6f );
	anim->Set_Scale( 0.6f );
	anim->Set_Direction_Range( 180, 180 );
	anim->Set_Fading_Alpha( 1 );
	anim->Set_Fading_Size( 1 );
	anim->Set_Blending( BLEND_DRIVE );
	
	if( create_anim )
	{
		// add animation
		pAnimation_Manager->Add( anim );
	}
}

void cEnemy :: Handle_Collision( cObjectCollision *collision )
{
	if( dead )
	{
		return;
	}

	cImageObjectSprite::Handle_Collision( collision );
}

void cEnemy :: Handle_out_of_Level( ObjectDirection dir )
{
	if( dir == DIR_LEFT )
	{
		Set_Pos_X( pActive_Camera->limit_rect.x - col_pos.x );
	}
	else if( dir == DIR_RIGHT )
	{
		Set_Pos_X( pActive_Camera->limit_rect.x + pActive_Camera->limit_rect.w - col_pos.x - col_rect.w - 0.01f );
	}

	Turn_Around( dir );
}
