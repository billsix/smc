/***************************************************************************
           ball.cpp  -  ball class
                             -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/ball.h"
#include "../core/game_core.h"
#include "../enemies/enemy.h"
#include "../objects/goldpiece.h"
#include "../enemies/gee.h"
#include "../enemies/spika.h"
#include "../player/player.h"
#include "../video/animation.h"
#include "../core/obj_manager.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** cBall *** *** *** *** *** *** *** *** *** *** *** */

cBall :: cBall( float x, float y, float nvelx, float nvely /* = 0 */, cMovingSprite *norigin /* = NULL */, ball_effect btype /* = FIREBALL_DEFAULT */ )
: cMovingSprite( NULL, x, y )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_BALL;
	posz = 0.095f;

	spawned = 1;
	player_range = 2000;

	massivetype = MASS_MASSIVE;
	counter = 0;

	glim_mod = 1;
	glim_counter = 0;

	velx = nvelx;
	vely = nvely;

	origin = norigin;

	// if not set
	if( !origin )
	{
		printf( "Warning : Ball origin not set\n" );
		origin = pPlayer;
	}

	if( btype == FIREBALL_DEFAULT || btype == FIREBALL_EXPLOSION )
	{
		Set_Image( pVideo->Get_Surface( "animation/fireball/1.png" ) );
		ball_type = FIREBALL_DEFAULT;
	}
	else if( btype == ICEBALL_DEFAULT || btype == ICEBALL_EXPLOSION )
	{
		Set_Image( pVideo->Get_Surface( "animation/iceball/1.png" ) );
		ball_type = ICEBALL_DEFAULT;
	}
	else
	{
		printf( "Warning : Ball unknown type %d\n", btype );
		cMovingSprite::Destroy();
		return;
	}

	if( origin->type == TYPE_PLAYER )
	{
		if( ball_type == FIREBALL_DEFAULT )
		{
			pPlayer->fireball_count++;
		}
		else if( ball_type == ICEBALL_DEFAULT )
		{
			pPlayer->iceball_count++;
		}
	}
}

cBall :: ~cBall( void )
{

}

void cBall :: Destroy( void )
{
	if( destroy )
	{
		return;
	}

	if( ball_type == FIREBALL_DEFAULT )
	{
		pAnimationManager->Add( new cFireAnimation( posx + col_rect.w / 2, posy + col_rect.h / 2 ) );

		if( origin->type == TYPE_PLAYER )
		{
			pPlayer->fireball_count--;
		}
	}
	else if( ball_type == ICEBALL_DEFAULT )
	{
		// create animation
		cParticleAnimation *anim = new cParticleAnimation();
		Generate_Particles( anim );
		anim->Set_Quota( 15 );
		// add animation
		pAnimationManager->Add( anim );

		if( origin->type == TYPE_PLAYER )
		{
			pPlayer->iceball_count--;
		}
	}

	cMovingSprite::Destroy();
}

void cBall :: Update( void )
{
	if( !valid_update )
	{
		return;
	}

	// right
	if( velx > 0 )
	{
		rotz += pFramerate->speedfactor * 40;
	}
	// left
	else
	{
		rotz -= pFramerate->speedfactor * 40;
	}

	if( vely < 30 )
	{
		Add_Velocity( 0, 1 );
	}

	// if ball is out of Player Range
	if( !is_Player_range() )
	{
		Destroy();
	}

	// glim animation
	if( glim_mod )
	{
		glim_counter += pFramerate->speedfactor * 0.1f;

		if( glim_counter > 1 )
		{
			glim_counter = 1;
			glim_mod = 0;
		}
	}
	else
	{
		glim_counter -= pFramerate->speedfactor * 0.1f;

		if( glim_counter < 0 )
		{
			glim_counter = 0;
			glim_mod = 1;
		}
	}

	// Particle animation
	Generate_Particles();
}

void cBall :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// don't draw if leveleditor mode
	if( editor_level_enabled )
	{
		return;
	}

	if( ball_type == FIREBALL_DEFAULT )
	{
		Set_Color_Combine( glim_counter, glim_counter / 1.2f, glim_counter / 2, GL_ADD );
	}
	else if( ball_type == ICEBALL_DEFAULT )
	{
		Set_Color_Combine( glim_counter / 2.5f, glim_counter / 2.5f, glim_counter / 1.7f, GL_ADD );
	}

	cMovingSprite::Draw( request );
}

void cBall :: Generate_Particles( cParticleAnimation *anim /* = NULL */ )
{
	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticleAnimation();
	}

	anim->Set_Emitter_Rect( col_rect );
	anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
	anim->Set_ZPos( posz + 0.0001f );
	if( ball_type == FIREBALL_DEFAULT )
	{
		anim->Set_Time_to_Live( 0.2f );
		anim->Set_Color( Color( (Uint8)250, 140, 90 ) );
	}
	else if( ball_type == ICEBALL_DEFAULT )
	{
		anim->Set_Time_to_Live( 0.5f );
		anim->Set_Color( Color( (Uint8)90, 90, 255 ) );
	}
	anim->Set_Blending( BLEND_ADD );
	anim->Set_Speed( 0.35f, 0.3f );
	anim->Set_Scale( 0.4f, 0.2f );
	
	if( create_anim )
	{
		// add animation
		pAnimationManager->Add( anim );
	}
}

unsigned int cBall :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	int basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid > -1 )
	{
		return basic_valid;
	}

	// player
	if( obj->type == TYPE_PLAYER )
	{
		if( origin->type != TYPE_PLAYER )
		{
			return 2;
		}

		return 0;
	}
	// massive
	if( obj->massivetype == MASS_MASSIVE )
	{
		if( obj->type == TYPE_BALL )
		{
			return 0;
		}
		else if( obj->sprite_array == ARRAY_ENEMY && origin->sprite_array == ARRAY_ENEMY )
		{
			return 0;
		}

		return 2;
	}
	else if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cBall :: Handle_Collision( cObjectCollision *collision )
{
	// already destroyed
	if( destroy )
	{
		return;
	}

	cMovingSprite::Handle_Collision( collision );
}

void cBall :: Handle_Collision_Player( ObjectDirection cdirection )
{
	// velocity hit
	if( direction == DIR_LEFT )
	{
		pPlayer->velx -= 5;
	}
	else if( direction == DIR_RIGHT )
	{
		pPlayer->velx += 5;
	}
	else if( direction == DIR_UP )
	{
		pPlayer->vely -= 5;
	}
	else if( direction == DIR_DOWN )
	{
		pPlayer->vely += 5;
	}

	Destroy();
}

void cBall :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	cEnemy *enemy = static_cast<cEnemy *>(pObjManager->Get_Pointer( collision->number ));

	// if enemy is not destroyable
	if( ( ball_type == FIREBALL_DEFAULT && enemy->fire_resistant ) || ( ball_type == ICEBALL_DEFAULT && enemy->ice_resistance >= 1 ) )
	{
		pAudio->PlaySound( "tock.ogg" );
	}
	// destroy enemy
	else
	{
		// enemy rect particle animation
		for( unsigned int w = 0; w < enemy->col_rect.w; w += 15 )
		{
			for( unsigned int h = 0; h < enemy->col_rect.h; h += 15 )
			{
				// animation
				cParticleAnimation *anim = new cParticleAnimation( enemy->posx + w, enemy->posy + h );

				anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
				anim->Set_Time_to_Live( 0.2f, 0.4f );
				Color anim_color, anim_color_rand;
				if( ball_type == FIREBALL_DEFAULT )
				{
					anim_color = Color( (Uint8)250, 170, 150 );
					anim_color_rand = Color( (Uint8)( rand() % 5 ), rand() % 85, rand() % 25 );
				}
				else
				{
					anim_color = Color( (Uint8)150, 150, 240 );
					anim_color_rand = Color( (Uint8)( rand() % 80 ), rand() % 80, rand() % 10 );
				}
				anim->Set_Color( anim_color, anim_color_rand );
				anim->Set_Fading_Alpha( 1 );
				anim->Set_Fading_Size( 1 );
				anim->Set_Speed( 0.5f, 2.2f );
				anim->Set_Blending( BLEND_DRIVE );
				// add animation
				pAnimationManager->Add( anim );
			}
		}

		// play enemy kill sound
		pAudio->PlaySound( enemy->kill_sound );

		if( ball_type == FIREBALL_DEFAULT )
		{
			// get points
			pointsdisplay->Add_Points( enemy->kill_points, posx, posy, "", (Uint8)255, 1 );

			// create goldpiece
			cMovingSprite *goldpiece = new cFGoldpiece( enemy->col_rect.x, enemy->col_rect.y + enemy->col_rect.h, collision->direction );
			// set optimal position
			goldpiece->Col_Move( -( ( goldpiece->col_rect.w - enemy->col_rect.w ) / 2 ), -( goldpiece->col_pos.y + goldpiece->col_rect.h ), 1, 1 );
			// add goldpiece
			pObjManager->Add( goldpiece );

			enemy->Set_Visible( 0 );
			enemy->DownGrade( 1 );
			pPlayer->Add_Kill_Multiplier();
		}
		else if( ball_type == ICEBALL_DEFAULT )
		{
			enemy->Freeze();
		}
	}

	Destroy();
}

void cBall :: Handle_Collision_Massive( cObjectCollision *collision )
{
	if( collision->direction == DIR_DOWN )
	{
		// if directly hitting the ground
		if( velx < 0.1f && velx > -0.1f )
		{
			Destroy();
			return;
		}

		if( ball_type == FIREBALL_DEFAULT )
		{
			vely = -10;	
			
			// create animation
			cFireAnimation *anim = new cFireAnimation( posx + col_rect.w / 2, posy + col_rect.h / 2 );
			anim->Set_FadingSpeed( 3 );
			pAnimationManager->Add( anim );
		}
		else if( ball_type == ICEBALL_DEFAULT )
		{
			vely = -5;

			// create animation
			cParticleAnimation *anim = new cParticleAnimation( posx + col_rect.w / 2, posy + col_rect.h / 2 );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/cloud.png" ) );
			anim->Set_DirectionRange( 0, 180 );
			anim->Set_Quota( 3 );
			anim->Set_Time_to_Live( 0.8f );
			anim->Set_ZPos( posz + 0.0001f );
			anim->Set_Color( Color( (Uint8)50, 50, 250 ) );
			anim->Set_Blending( BLEND_ADD );
			anim->Set_Speed( 0.5f, 0.4f );
			anim->Set_Scale( 0.3f, 0.4f );
			// add animation
			pAnimationManager->Add( anim );
		}
	}
	// other directions
	else
	{
		Destroy();
	}
}

void cBall :: Handle_OutofLevel( ObjectDirection dir )
{
	// ignore top
	if( dir == DIR_TOP )
	{
		return;
	}

	Destroy();
}
