/***************************************************************************
 * star.cpp  -  jumping star class
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

#include "../objects/star.h"
#include "../player/player.h"
#include "../core/framerate.h"
#include "../video/animation.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cjStar *** *** *** *** *** *** *** *** *** *** *** */

cjStar :: cjStar( float x, float y )
: cPowerUp( x, y )
{
	type = TYPE_JSTAR;
	massivetype = MASS_PASSIVE;

	images.push_back( pVideo->Get_Surface( "game/items/star.png" ) );

	anim_counter = 0;

	glim_mod = 1;
	glim_counter = 0;

	velx = 5;

	Set_Image( 0, 1, 0 );

	name = _("Star");
}

cjStar :: ~cjStar( void )
{
	//
}

cjStar *cjStar :: Copy( void )
{
	cjStar *star = new cjStar( startposx, startposy );

	return star;
}

void cjStar :: Activate( void )
{
	// animation
	Generate_Particles( posx + col_rect.w * 0.5f, posy + col_rect.h * 0.5f, 20 );

	// activate star
	pPlayer->Get_Item( TYPE_JSTAR );

	// if spawned destroy
	if( spawned )
	{
		Destroy();
	}
	// hide
	else
	{
		Set_Visible( 0 );
	}
}

void cjStar :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		Activate();
		return;
	}

	// Add Gravitation
	if( vely < 25 )
	{
		Add_Velocity( 0, 1.8f );
	}
	
	// rotate
	if( vely < 0 )
	{
		Add_Rotation_Z( ( 5 - ( vely / 2.5f ) ) * pFramerate->speedfactor );
	}
	// rotate back to 0 if falling
	else
	{
		if( rotz > 5 && rotz <= 175 )
		{
			Add_Rotation_Z( ( 5 - ( vely / 1.2f ) ) * pFramerate->speedfactor );
		}
		else if( rotz < 355 && rotz > 185 )
		{
			Add_Rotation_Z( ( -5 + ( vely / 1.2f ) ) * pFramerate->speedfactor );
		}
	}

	// small stars
	anim_counter += 1.1f * pFramerate->speedfactor;

	while( anim_counter > 1 )
	{
		Generate_Particles();
		anim_counter--;
	}

	// glim animation
	if( glim_mod )
	{
		glim_counter += pFramerate->speedfactor * 0.2f;

		if( glim_counter > 1 ) 
		{
			glim_counter = 1;
			glim_mod = 0;
		}
	}
	else
	{
		glim_counter -= pFramerate->speedfactor * 0.2f;

		if( glim_counter < 0 ) 
		{
			glim_counter = 0;
			glim_mod = 1;
		}
	}
}

void cjStar :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	Set_Color_Combine( glim_counter / 0.8f, glim_counter / 0.9f, glim_counter, GL_ADD );

	cPowerUp::Draw();
}

void cjStar :: Generate_Particles( float x /* = 0 */, float y /* = 0 */, unsigned int quota /* = 2 */ )
{
	if( x == 0 )
	{
		x = posx + Get_Random_Float( 0, col_rect.w * 0.9f );
	}

	if( y == 0 )
	{
		y = posy + Get_Random_Float( 0, col_rect.h * 0.9f );
	}

	// set particle color
	Color particle_color = orange;
	particle_color.green += static_cast<Uint8>( glim_counter / 5 );
	particle_color.blue += static_cast<Uint8>( glim_counter / 1.5f );

	// create particles
	cParticle_Emitter *anim = new cParticle_Emitter();
	anim->Set_Pos( x, y );
	anim->Set_Quota( quota );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
	anim->Set_Time_to_Live( 0.4f );
	anim->Set_Fading_Alpha( 1 );
	anim->Set_Fading_Size( 1 );
	anim->Set_Speed( 1.5f, 0.5f );
	anim->Set_Scale( 0.15f );
	anim->Set_Color( particle_color );
	anim->Set_Blending( BLEND_ADD );
	anim->Set_Const_Rotation_Z( -5, 10 );
	pAnimation_Manager->Add( anim );
}

void cjStar :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT )
	{
		velx = -velx;
	}
	else if( collision->direction == DIR_UP )
	{
		vely = -( vely * 0.3f );
	}
	else if( collision->direction == DIR_DOWN )
	{
		if( ground_object )
		{
			Generate_Particles( posx + rect.w / 2, ground_object->posy, 10 );
		}

		vely = -25;
	}
}
