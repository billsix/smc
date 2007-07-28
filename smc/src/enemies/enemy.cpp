/***************************************************************************
           enemy.cpp  -  base class for all enemies
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
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

	counter = 0;
	walk_count = 0;

	kill_sound = "enemy/gumba/die.ogg";
	kill_points = 10;

	fire_resistant = 0;
}

cEnemy :: ~cEnemy( void )
{

}

void cEnemy :: Set_Dead( bool enable /* = 1 */ )
{
	dead = enable;

	Update_valid_update();
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
		if( type == TYPE_GUMBA || type == TYPE_TURTLE || type == TYPE_REX || type == TYPE_SPIKA )
		{
			Update_Gravity();
			Col_Move( 0, vely );
		}
	}
}

void cEnemy :: Update_Gravity( void )
{
	if( !ground_object && vely < 25 )
	{
		Add_Velocity( 0, 1.5f );
	}
	else if( ground_object && vely > 0 )
	{
		vely = 0;
	}
}

void cEnemy :: Generate_Hit_Animation( cParticleAnimation *anim /* = NULL */ )
{
	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticleAnimation();
	}

	anim->Set_Emitter_Rect( col_rect.x, posy + ( col_rect.h / 3 ), col_rect.w );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
	anim->Set_Quota( 4 );
	anim->Set_ZPos( posz - 0.000001f );
	anim->Set_Time_to_Live( 0.3f );
	anim->Set_Color( Color( (Uint8)150, 150, 150, 200 ), Color( (Uint8)( rand() % 155 ), rand() % 155, rand() % 155 ) );
	anim->Set_Speed( 3, 0.6f );
	anim->Set_Scale( 0.6f );
	anim->Set_DirectionRange( 180, 180 );
	anim->Set_Fading_Alpha( 1 );
	anim->Set_Fading_Size( 1 );
	anim->Set_Blending( BLEND_DRIVE );
	
	if( create_anim )
	{
		// add animation
		pAnimationManager->Add( anim );
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

void cEnemy :: Handle_OutofLevel( ObjectDirection dir )
{
	if( dir == DIR_LEFT )
	{
		Set_PosX( pCamera->limit_rect.x - col_pos.x );
	}
	else if( dir == DIR_RIGHT )
	{
		Set_PosX( pCamera->limit_rect.x + pCamera->limit_rect.w - col_pos.x - col_rect.w - 0.01f );
	}

	Turn_Around( dir );
}
