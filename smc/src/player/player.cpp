/***************************************************************************
           player.cpp  -  Level Maryo class
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

#include "../core/globals.h"
#include "../player/player.h"
#include "../core/main.h"
#include "../video/animation.h"
#include "../core/game_core.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../core/sprite_manager.h"
#include "../core/framerate.h"
#include "../audio/audio.h"
#include "../enemies/turtle.h"
#include "../overworld/overworld.h"
#include "../level/level.h"
#include "../gui/menu.h"
#include "../core/camera.h"
#include "../objects/levelexit.h"
#include "../objects/level_entry.h"
#include "../objects/box.h"
#include "../input/keyboard.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** *** cPlayer *** *** *** *** *** *** *** *** *** */

cPlayer :: cPlayer( float x /* = 120 */, float y /* = 0 */ )
: cImageObjectSprite( x, y )
{
	type = TYPE_PLAYER;
	massivetype = MASS_MASSIVE;
	state = STA_FALL;

	maryo_type = MARYO_SMALL;
	maryo_type_temp_power = MARYO_DEAD;
	name = "Maryo";	

	images.reserve( 20 );
	posz = 0.0999f;
	ducked = 0;
	parachute = 0;
	throwing_counter = 0;

	// invincible data
	invincible = 0;
	invincible_star = 0;
	invincible_mod = 0;
	invincible_starcounter = 0;

	walk_count = 0;
	running_particle_counter = 0;

	godmode = 0;

	walk_time = 0;
	ghost_time = 0;
	ghost_time_mod = 0;

	// Starting with 3 lives 
	lives = 3;
	goldpieces = 0;
	points = 0;
	kill_multiplier = 1;
	last_kill_counter = 0;

	// jump data
	UpKeytime = 0;
	start_enemyjump = 0;
	jump_power = 0;
	jump_accel_up = 4.5f;
	jump_vel_deaccel = 0.06f;

	// no movement timer
	no_velx_counter = 0;
	no_vely_counter = 0;

	fireball_count = 0;
	iceball_count = 0;
	active_object = NULL;

	Load_Images();
	// default direction : right
	Set_Direction( DIR_RIGHT, 1 );
}

cPlayer :: ~cPlayer( void )
{
	Ball_clear();
}

cPlayer *cPlayer :: Copy( void )
{
	// non copyable
	return NULL;
}

void cPlayer :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx", 50 )), static_cast<float>(attributes.getValueAsInteger( "posy", 300 )), 1 );
	// direction
	ObjectDirection dir = Get_Direction_id( attributes.getValueAsString( "direction" ).c_str() );

	// if invalid set default
	if( dir != DIR_LEFT && dir != DIR_RIGHT )
	{
		dir = DIR_RIGHT;
	}
	Set_Direction( dir, 1 );

	// ## change position to the current state height
	// level engine version 2 and lower
	if( pLevel->engine_version <= 2 )
	{
		Set_Pos( posx, posy - col_rect.h, 1 );
	}
	// higher as 2
	else
	{
		Col_Move( 0, start_image->col_h - col_rect.h, 1, 1 );
	}
}

void cPlayer :: Save_to_Stream( ofstream &file )
{
	// begin player
	file << "\t<player>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;

	// end player
	file << "\t</player>" << std::endl;
}

void cPlayer :: Hold( void )
{
	if( !ground_object || ( state != STA_WALK && state != STA_RUN ) )
	{
		return;
	}
	
	Set_MovingState( STA_STAY );
}

void cPlayer :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	// set start image
	if( new_start_direction )
	{
		cMovingSprite::Set_Image( pVideo->Get_Surface( "maryo/small/stand_" + Get_Direction_name( dir ) + ".png" ), 1 );

		// set back current image
		curr_img = -1;
		Set_Image( Get_Image() + direction );
	}

	cImageObjectSprite::Set_Direction( dir, new_start_direction );
}

void cPlayer :: Set_onGround( cSprite *obj )
{
	cMovingSprite::Set_onGround( obj );

	// if valid ground
	if( ground_object )
	{
		// if moving
		if( velx )
		{
			if( state != STA_STAY && state != STA_RUN )
			{
				Set_MovingState( STA_WALK );
			}
		}
		// not moving
		else if( !velx )
		{
			Set_MovingState( STA_STAY );
		}

		// if massive ground and ducking key is pressed
		if( ground_object->massivetype == MASS_MASSIVE && pKeyboard->keys[pPreferences->key_down] )
		{
			Start_ducking();
		}
	}
}

void cPlayer :: DownGrade( bool delayed /* = 1 */, bool force /* = 0 */ )
{
	if( godmode || invincible )
	{
		return;
	}

	if( delayed )
	{
		Game_Action = GA_DOWNGRADE_PLAYER;
		Game_Action_Data.add( "force", CEGUI::PropertyHelper::boolToString( force ) );
		return;
	}

	// if not weakest state or not forced 
	if( maryo_type != MARYO_SMALL && !force )
	{
		pAudio->PlaySound( "player/powerdown.ogg", RID_MARYO_POWERDOWN );

		// power down
		Set_Type( MARYO_SMALL );

		invincible = DESIRED_FPS * 2.5f;
		invincible_mod = 0;

		Itembox->Request_Item();
		
		return;
	}

	Set_Type( MARYO_DEAD, 0, 0 );
	pointsdisplay->Clear();
	Ball_clear();
	livedisplay->AddLives( -1 );
	pAudio->FadeOutMusic( 1700 );

	// lost a live
	if( lives >= 0 )
	{
		pAudio->PlaySound( "player/dead.ogg", RID_MARYO_DEATH );
	}
	// game over
	else
	{
		pAudio->PlaySound( DATA_DIR "/" GAME_MUSIC_DIR "/game/lost_1.ogg", RID_MARYO_DEATH );
	}
	
	// dying animation
	Set_Image( MARYO_IMG_DEAD + 1 );

	// draw
	Draw_Game();
	// render
	pVideo->Render();

	// wait
	SDL_Delay( 500 );
	pFramerate->Reset();
	
	Set_Image( MARYO_IMG_DEAD );

	float i;

	for( i = 0; i < 7; i += pFramerate->speedfactor )
	{
		// move up
		Move( 0, -13 );
		// draw
		Draw_Game();
		// render
		pVideo->Render();
		pFramerate->Update();
	}

	// very small delay until falling animation
	SDL_Delay( 300 );

	pFramerate->Reset();
	walk_count = 0;
	
	for( i = 0; posy < GAME_RES_H + pCamera->y + col_rect.h; i++ )
	{
		walk_count += pFramerate->speedfactor * 0.75f;

		if( walk_count > 4 )
		{
			walk_count = 0;
		}
		
		// move down
		Move( 0, 14 );

		if( walk_count > 2 )
		{
			Set_Image( MARYO_IMG_DEAD );
		}
		else
		{
			Set_Image( MARYO_IMG_DEAD + 1 );
		}
		
		// draw
		Draw_Game();
		// render
		pVideo->Render();
		pFramerate->Update();
	}

	// lost a live
	if( lives >= 0 )
	{
		// unload level
		pLevel->Unload( 1 );
	}
	// game over
	else
	{
		GL_Surface *game_over = pVideo->Get_Surface( "game/game_over.png" );
		cSprite *sprite = new cSprite( game_over, ( GAME_RES_W * 0.5f ) - ( game_over->w * 0.5f ), ( GAME_RES_H * 0.5f ) - ( game_over->h * 0.5f ) );
		sprite->posz = 0.8f;
		sprite->spawned = 1;
		sprite->Set_Ignore_Camera( 1 );
		pLevel->pSprite_Manager->Add( sprite );

		cParticleAnimation *anim = new cParticleAnimation();
		anim->Set_Emitter_Rect( sprite->rect.x + pCamera->x, sprite->rect.y + pCamera->y, sprite->rect.w ,sprite->rect.h );
		anim->Set_Emitter_Time_to_Live( 24 );
		anim->Set_Emitter_Iteration_Interval( 0.1f );
		anim->Set_Quota( 1 );
		anim->Set_ZPos( 0.79f );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/axis.png" ) );
		anim->Set_Time_to_Live( 10 );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Speed( 2, 0.5f );
		anim->Set_Scale( 0.9f );
		anim->Set_Color( Color( static_cast<Uint8>(250), 140, 90 ), Color( static_cast<Uint8>(5), 100, 0 ) );
		anim->Set_ConstRotationZ( 0.1f, 5 );

		anim->Init();

		for( float i = 1; i > 0; i -= 0.0011f * pFramerate->speedfactor )
		{
			// update pressed keys
			SDL_PumpEvents();
			Uint8 *keys = SDL_GetKeyState( NULL );
			// Escape stops
			if( keys[SDLK_ESCAPE] || keys[SDLK_RETURN] || keys[SDLK_SPACE] || keys[pPreferences->key_action] )
			{
				break;
			}

			// if joystick enabled and exit pressed
			if( pPreferences->joy_enabled && SDL_JoystickGetButton( pJoystick->joystick, pPreferences->joy_button_exit ) )
			{
				 break;
			}

			// update animation
			anim->Update();
			
			// draw
			Draw_Game();
			anim->Draw();

			pVideo->Render();
			pFramerate->Update();
		}

		delete anim;
		pAudio->StopSounds();
	}
	
	// clear
	Set_Type( MARYO_SMALL, 0, 0 );
	ClearInputEvents();
	pFramerate->Reset();

	// game over - reset
	if( lives < 0 )
	{
		ResetSave();
		Game_Action = GA_ENTER_MENU;
	}
	// custom level
	else if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
	{
		Game_Action = GA_ENTER_MENU;
	}
	// back to overworld
	else
	{
		Game_Action = GA_ENTER_WORLD;
	}
}

void cPlayer :: Move_Player( float velocity, float vel_wrongway )
{
	if( direction == DIR_LEFT )
	{
		velocity *= -1;
		vel_wrongway *= -1;
	}

	// collision count
	unsigned int col_count = Collision_Check_Relative( velocity, 0, 0, 0, COLLIDE_ALLOW_BLOCKING ).size();
	// if collision with a blocking object
	bool is_col = 0;

	// check collisions
	for( unsigned int i = 0; i < col_count; i++ )
	{
		cObjectCollision *collision = Collision_Get_last();

		// massive or active object is blocking
		if( collision->type == CO_MASSIVE || collision->type == CO_ACTIVE )
		{
			is_col = 1;
		}
		// special enemies
		else if( collision->type == CO_ENEMY )
		{
			cEnemy *enemy = static_cast<cEnemy *>(pLevel->pSprite_Manager->Get_Pointer( collision->number ));

			// blocking enemies
			if( enemy->type == TYPE_EATO )
			{
				is_col = 1;
			}
		}

		Collision_Delete( collision );
	}

	// don't move if colliding
	if( is_col )
	{
		if( !velx )
		{
			Set_MovingState( STA_STAY );
		}

		return;
	}

	// move right
	if( direction == DIR_RIGHT )
	{
		if( velx > 0 )
		{
			Add_Velocity( velocity, 0 );

			if( velx > 10 * Get_Vel_mod() )
			{
				velx = 10 * Get_Vel_mod();
			}
		}
		else
		{
			// small smoke clouds under foots
			if( velx < 0 )
			{
				Generate_feet_clouds();
			}

			// slow down
			Add_Velocity( vel_wrongway, 0 );
		}
	}
	// move left
	else if( direction == DIR_LEFT )
	{
		if( velx < 0 )
		{
			Add_Velocity( velocity, 0 );

			if( velx < -10 * Get_Vel_mod() )
			{
				velx = -10 * Get_Vel_mod();
			}
		}
		else
		{
			// small smoke clouds under foots
			if( velx > 0 )
			{
				Generate_feet_clouds();
			}

			// slow down
			Add_Velocity( vel_wrongway, 0 );
		}
	}

	// start walking
	if( state == STA_STAY )
	{
		Set_MovingState( STA_WALK );
	}
}

void cPlayer :: Generate_feet_clouds( cParticleAnimation *anim /* = NULL */ )
{
	// check if valid
	if( !ground_object || !ground_object->image || ground_object->image->ground_type == GROUND_NORMAL )
	{
		return;
	}

	bool create_anim = 0;

	if( !anim )
	{
		create_anim = 1;
		// create animation
		anim = new cParticleAnimation();
	}

	anim->Set_Emitter_Rect( col_rect.x, col_rect.y + col_rect.h - 2, col_rect.w );
	anim->Set_ZPos( posz - 0.000001f );

	float vel = ( velx > 0 ? velx : -velx );

	// ground type
	switch( ground_object->image->ground_type )
	{
		case GROUND_EARTH:
		{
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/dirt.png" ) );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Scale( 0.5f );
			anim->Set_Speed( 0.08f + vel * 0.1f, vel * 0.05f );
			break;
		}
		case GROUND_ICE:
		{
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/ice_1.png" ) );
			anim->Set_Time_to_Live( 0.6f );
			anim->Set_Scale( 0.3f );
			anim->Set_Speed( 0.1f + vel * 0.05f, vel * 0.04f );
			break;
		}
		case GROUND_SAND:
		{
			anim->Set_Emitter_Iteration_Interval( 4 );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/cloud.png" ) );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Scale( 0.2f );
			anim->Set_Color( lightorange );
			anim->Set_Speed( 0.2f + vel * 0.15f, vel * 0.15f );
			break;
		}
		case GROUND_STONE:
		{
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke_black.png" ) );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Scale( 0.3f );
			anim->Set_Speed( vel * 0.08f, 0.1f + vel * 0.1f );
			break;
		}
		case GROUND_PLASTIC:
		{
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			anim->Set_Time_to_Live( 0.2f );
			anim->Set_Scale( 0.2f );
			anim->Set_Color( lightgrey );
			anim->Set_Speed( 0.05f, vel * 0.05f );
			break;
		}
		default:
		{
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/smoke.png" ) );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Speed( 0.1f + vel * 0.1f, vel * 0.1f );
			break;
		}
	}

	if( direction == DIR_RIGHT )
	{
		anim->Set_DirectionRange( 180, 90 );
	}
	else
	{
		anim->Set_DirectionRange( 270, 90 );
	}

	if( create_anim )
	{
		// add animation
		pAnimationManager->Add( anim );
	}
}

void cPlayer :: Update_walking( void )
{
	if( ducked || !ground_object || ( state != STA_STAY && state != STA_WALK && state != STA_RUN ) )
	{
		return;
	}

	// validate ground object
	if( ( ground_object->type == TYPE_EATO || ground_object->type == TYPE_SPIKA || ground_object->type == TYPE_ROKKO ) && invincible <= 0 )
	{
		Reset_onGround();
	}

	// only if left or right is pressed
	if( pKeyboard->keys[pPreferences->key_left] || pKeyboard->keys[pPreferences->key_right] || pJoystick->left || pJoystick->right )
	{
		float ground_mod = 1;

		if( ground_object && ground_object->image )
		{
			// ground type
			switch( ground_object->image->ground_type )
			{
				case GROUND_ICE:
				{
					ground_mod = 0.5f;
					break;
				}
				case GROUND_SAND:
				{
					ground_mod = 0.7f;
					break;
				}
				case GROUND_PLASTIC:
				{
					ground_mod = 0.85f;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		Move_Player( ( 0.6f * ground_mod * Get_Vel_mod() ), ( 1.2f * ground_mod * Get_Vel_mod() ) );
	}

	// update walking time
	if( state == STA_WALK )
	{
		// update
		if( velx > 10 || velx < -10 )
		{
			walk_time += pFramerate->speedfactor;

			if( walk_time > DESIRED_FPS )
			{
				Set_MovingState( STA_RUN );
			}
		}
		// reset
		else if( walk_time )
		{
			walk_time = 0;
		}
	}
}

void cPlayer :: Update_running( void )
{
	if( ducked || !ground_object || state != STA_RUN )
	{
		return;
	}

	running_particle_counter += pFramerate->speedfactor * 0.5f;

	// create particles
	while( running_particle_counter > 1 )
	{
		float vel = ( velx > 0 ? velx : -velx );

		// start on high velocity
		if( vel > 17 )
		{
			// light particles
			cParticleAnimation *anim = new cParticleAnimation( col_rect.x + col_rect.w * 0.25f, col_rect.y + col_rect.h * 0.1f, col_rect.w * 0.5f, col_rect.h * 0.8f );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			anim->Set_ZPos( posz + 0.000001f );
			anim->Set_Time_to_Live( 0.1f + vel * 0.03f );
			anim->Set_Fading_Alpha( 1 );
			anim->Set_Fading_Size( 1 );
			anim->Set_Speed( vel * 0.9f, 0 );
			// right
			if( velx > 0 )
			{
				anim->Set_DirectionRange( 0, 0 );
			}
			// left
			else
			{
				anim->Set_DirectionRange( 180, 0 );
			}
			anim->Set_Scale( 0.15f );

			anim->Set_Blending( BLEND_ADD );
			pAnimationManager->Add( anim );
		}

		// smoke clouds under foots
		Generate_feet_clouds();

		running_particle_counter--;
	}
}

void cPlayer :: Update_staying( void )
{
	// only if player is onground
	if( !ground_object || ducked || state == STA_JUMP )
	{
		return;
	}

	// if left and right is not pressed
	if( !pKeyboard->keys[pPreferences->key_left] && !pKeyboard->keys[pPreferences->key_right] && !pJoystick->left && !pJoystick->right )
	{
		// walking
		if( velx )
		{
			if( ground_object->image && ground_object->image->ground_type == GROUND_ICE )
			{
				Auto_SlowDown( 1.1f / 5.0f );
			}
			else
			{
				Auto_SlowDown( 1.1f );
			}

			// stopped walking
			if( !velx )
			{
				Set_MovingState( STA_STAY );
			}
		}

		// walk on spika
		if( ground_object->type == TYPE_SPIKA )
		{
			cMovingSprite *moving_ground_object = static_cast<cMovingSprite *>(ground_object);

			if( moving_ground_object->velx < 0 )
			{
				walk_count -= moving_ground_object->velx * pFramerate->speedfactor * 0.1f;
			}
			else if( moving_ground_object->velx > 0 )
			{
				walk_count += moving_ground_object->velx * pFramerate->speedfactor * 0.1f;
			}
		}
		// if not moving anymore
		else if( velx == 0 )
		{
			walk_count = 0;
		}
	}

	// if staying don't move vertical
	if( vely > 0 )
	{
		vely = 0;
	}
}

void cPlayer :: Update_flying( void )
{
	/* only if not onground
	 * if jumping update jump is already used
	*/
	if( ground_object || state == STA_CLIMB || state == STA_JUMP )
	{
		return;
	}

	// flying
	if( state == STA_FLY )
	{
		if( direction == DIR_LEFT )
		{
			if( velx > -15 * Get_Vel_mod() )
			{
				Add_Velocity( -1.1f, 0 );
			}
		}
		else
		{
			if( velx < 15 * Get_Vel_mod() )
			{
				Add_Velocity( 1.1f, 0 );
			}
		}

		// move down 
		if( pKeyboard->keys[pPreferences->key_down] || pJoystick->down )
		{
			if( vely < 5 * Get_Vel_mod() )
			{
				Add_Velocity( 0, 0.6f );
			}
		}
		// move up
		else if( pKeyboard->keys[pPreferences->key_up] || pJoystick->up )
		{
			if( vely > -5 * Get_Vel_mod() )
			{
				Add_Velocity( 0, -0.6f );
			}
		}
		// slow down if not pressed
		else
		{
			Auto_SlowDown( 0, 0.4f );
		}

		// set flying rotation
		float fly_rot = vely * 2.5f;

		// limit angle
		if( fly_rot > 70 )
		{
			fly_rot = 70;
		}
		else if( fly_rot < -70 )
		{
			fly_rot = -70;
		}

		Set_RotationZ( fly_rot );
	}
	// falling
	else
	{
		// move left
		if( ( pKeyboard->keys[pPreferences->key_left] || pJoystick->left ) && !ducked )
		{
			if( !parachute )
			{
				if( velx > -10 * Get_Vel_mod() )
				{
					Add_Velocity( -1.2f, 0 );
				}
			}
			// slow fall/parachute
			else
			{
				if( velx > -5 * Get_Vel_mod() )
				{
					Add_Velocity( -0.6f, 0 );
				}
			}

		}
		// move right
		else if( ( pKeyboard->keys[pPreferences->key_right] || pJoystick->right ) && !ducked )
		{
			if( !parachute )
			{
				if( velx < 10 * Get_Vel_mod() )
				{
					Add_Velocity( 1.2f, 0 );
				}
			}
			// slow fall/parachute
			else
			{
				if( velx < 5 * Get_Vel_mod() )
				{
					Add_Velocity( 0.6f, 0 );
				}
			}
		}
		// slow down if not pressed
		else
		{
			Auto_SlowDown( 0.4f );
		}
	}
}

void cPlayer :: Stop_flying( bool nparachute /* = 1 */ )
{
	if( state != STA_FLY )
	{
		return;
	}

	if( nparachute )
	{
		parachute = 1;
	}

	Set_MovingState( STA_FALL );
}

void cPlayer :: Start_falling( void )
{
	Move( 0, 1.9f, 1 );
	Set_MovingState( STA_FALL );
}

void cPlayer :: Update_falling( void )
{
	// only if not on ground
	if( ground_object || state == STA_CLIMB || state == STA_FLY )
	{
		return;
	}

	// Add Gravitation
	if( !parachute )
	{
		if( vely < 25 )
		{
			Add_Velocity( 0, 2.8f );
		}
	}
	// slow fall/parachute
	else
	{
		if( vely < 10 )
		{
			Add_Velocity( 0, 0.6f );
		}
	}


	if( state != STA_JUMP && state != STA_FALL )
	{
		Set_MovingState( STA_FALL );
	}
}

void cPlayer :: Start_ducking( void )
{ 
	// only if massive ground
	if( ( state != STA_STAY && state != STA_WALK && state != STA_RUN ) || state == STA_CLIMB || !ground_object || ground_object->massivetype == MASS_HALFMASSIVE || ducked )
	{
		return;
	}

	Release_item( 1, 1 );

	// set ducking image ( without Check_OutofLevel from cMovingSprite )
	cSprite::Move( 0, image->col_h - images[MARYO_IMG_DUCK]->col_h, 1 );
	Set_Image( MARYO_IMG_DUCK + direction );

	ducked = 1;
	Set_MovingState( STA_STAY );
}

void cPlayer :: Stop_ducking( void )
{
	if( !ducked )
	{
		return;
	}

	// unset ducking image ( without Check_OutofLevel from cMovingSprite )
	cSprite::Move( 0, -( images[MARYO_IMG_STAND]->col_h - image->col_h ), 1 );
	Set_Image( MARYO_IMG_STAND + direction );

	ducked = 0;
	Set_MovingState( STA_STAY );
}

void cPlayer :: Update_ducking( void )
{
	if( !ducked )
	{
		return;
	}

	// lost ground object
	if( !ground_object )
	{
		Stop_ducking();
		return;
	}

	// slow down
	if( velx )
	{
		Auto_SlowDown( 0.6f );
	}
}

void cPlayer :: Start_climbing( void )
{
	if( ducked || state == STA_CLIMB )
	{
		return;
	}

	Release_item( 1, 1 );
	Set_MovingState( STA_CLIMB );
}

void cPlayer :: Update_climbing( void )
{
	if( state != STA_CLIMB )
	{
		return;
	}

	velx = 0;
	vely = 0;

	// collision testing
	Collision_Check( &col_rect );

	for( ObjectCollisionList::iterator itr = collisions.begin(), itr_end = collisions.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cObjectCollision *col = (*itr);

		// collides with the climbable object
		if( col && col->type == CO_ACTIVE && pLevel->pSprite_Manager->Get_Pointer( col->number )->massivetype == MASS_CLIMBABLE )
		{
			Collisions_Clear();

			// set velocity
			if( pKeyboard->keys[pPreferences->key_left] || pJoystick->left )
			{
				velx = -2 * Get_Vel_mod();
			}
			else if( pKeyboard->keys[pPreferences->key_right] || pJoystick->right )
			{
				velx = 2 * Get_Vel_mod();
			}

			if( pKeyboard->keys[pPreferences->key_up] || pJoystick->up )
			{
				vely = -4 * Get_Vel_mod();
			}
			else if( pKeyboard->keys[pPreferences->key_down] || pJoystick->down )
			{
				vely = 4 * Get_Vel_mod();
			}

			// in contact testing
			Collision_Check_Relative( 0, -15 );
			col = Collision_Get_first();

			// do not climb further upwards if maryo will loose contact
			if( !( col && col->type == CO_ACTIVE && pLevel->pSprite_Manager->Get_Pointer( col->number )->massivetype == MASS_CLIMBABLE ) )
			{
				// only stop if loosing contact upwards
				if( vely < 0 )
				{
					vely = 0;
				}
			}

			Collisions_Clear();
			return;
		}
	}

	// lost contact
	Set_MovingState( STA_FALL );
	Collisions_Clear();
}

void cPlayer :: Start_jump_keytime( void )
{
	if( godmode || state == STA_STAY || state == STA_WALK || state == STA_RUN || state == STA_FALL || state == STA_FLY || state == STA_JUMP || ( state == STA_CLIMB && !pKeyboard->keys[pPreferences->key_up] ) )
	{
		UpKeytime = DESIRED_FPS / 4;
	}
}

void cPlayer :: Update_jump_keytime( void )
{
	// handle jumping start
	if( UpKeytime && ( ground_object || godmode || state == STA_CLIMB ) )
	{
		Start_jump();
	}
	else if( start_enemyjump )
	{
		Start_jump( 18, 4.1f );
	}
}

void cPlayer :: Start_jump( float power /* = 17 */, float accel /* = 4 */, float deaccel /* = 0.08f */ )
{
	// play sound
	if( !start_enemyjump )
	{
		// small
		if( maryo_type == MARYO_SMALL )
		{
			pAudio->PlaySound( "player/jump_small.ogg", RID_MARYO_JUMP );
		}
		// ghost
		else if( maryo_type == MARYO_GHOST )
		{
			pAudio->PlaySound( "player/jump_ghost.ogg", RID_MARYO_JUMP );
		}
		// big
		else
		{
			pAudio->PlaySound( "player/jump_big.ogg", RID_MARYO_JUMP );
		}
	}

	bool jump_key = 0;

	// if jump key pressed
	if( pKeyboard->keys[pPreferences->key_jump] || ( pJoystick->up && pPreferences->joy_analog_jump ) || pJoystick->Button( pPreferences->joy_button_jump ) )
	{
		jump_key = 1;
	}

	// avoid setting onground again
	Col_Move( 0, -1, 1, 1 );

	// fly
	if( maryo_type == MARYO_CAPE && !start_enemyjump && state == STA_RUN && jump_key && ( ( direction == DIR_RIGHT && velx > 14 ) || ( direction == DIR_LEFT && velx < -14 ) ) )
	{
		Set_MovingState( STA_FLY );
		power *= 0.5f;
	}
	// jump
	else
	{
		jump_accel_up = accel;
		jump_vel_deaccel = deaccel;

		if( jump_key )
		{
			jump_power = 10;
		}
		else
		{
			jump_power = 2;
		}
		
		Set_MovingState( STA_JUMP );
	}

	UpKeytime = 0;
	start_enemyjump = 0;

	vely = -power;

	// jump higher when running
	if( velx < 0 )
	{
		vely += velx / 9.5f;
	}
	else if( velx > 0 )
	{
		vely -= velx / 9.5f;
	}

	// slow down if running
	velx = velx * 0.9f;
}

void cPlayer :: Update_jump( void )
{
	// jumping keytime
	if( UpKeytime )
	{
		UpKeytime -= pFramerate->speedfactor;

		if( UpKeytime < 0 )
		{
			UpKeytime = 0;
		}
	}
	
	// only if jumping
	if( state != STA_JUMP )
	{
		return;
	}

	// jumping physics
	if( pKeyboard->keys[pPreferences->key_jump] || ( pPreferences->joy_analog_jump && pJoystick->up ) || pJoystick->Button( pPreferences->joy_button_jump ) )
	{
		Add_Velocity( 0, -( jump_accel_up + ( vely * jump_vel_deaccel ) / Get_Vel_mod() ) );
		jump_power -= pFramerate->speedfactor;
	}
	else
	{
		Add_Velocity( 0, 0.5f );
		jump_power -= 6 * pFramerate->speedfactor;
	}
	
	// left right physics
	if( ( pKeyboard->keys[pPreferences->key_left] || pJoystick->left ) && !ducked )
	{
		if( velx > -10 * Get_Vel_mod() )
		{
			Add_Velocity( ( -1.1f * Get_Vel_mod() ) - ( velx / 100 ), 0 );
		}
		
	}	
	else if( ( pKeyboard->keys[pPreferences->key_right] || pJoystick->right ) && !ducked && state != STA_STAY )
	{
		if( velx < 10 * Get_Vel_mod() )
		{
			Add_Velocity( ( 1.1f * Get_Vel_mod() ) + ( velx / 100 ), 0 );
		}
	}
	// slow down
	else 
	{
		Auto_SlowDown( 0.2f );
	}

	// if no more jump power set to falling
	if( jump_power <= 0 )
	{
		Set_MovingState( STA_FALL );
	}
}

void cPlayer :: Update_item( void )
{
	// if active object item is available
	if( active_object )
	{
		// collision with something or activated
		if( active_object->type == TYPE_TURTLE && static_cast<cTurtle *>(active_object)->dead )
		{
			Release_item( 0 );
		}
		else
		{
			// update the item position
			float pos_hor = 0;

			if( maryo_type == MARYO_SMALL )
			{
				if( direction == DIR_LEFT )
				{
					pos_hor = rect.w * 0.13f - active_object->col_rect.w;
				}
				else
				{
					pos_hor = rect.w * 0.7f;
				}
			}
			// big
			else
			{
				if( direction == DIR_LEFT )
				{
					pos_hor = rect.w * 0.15f - active_object->col_rect.w;
				}
				else
				{
					pos_hor = rect.w * 0.70f;
				}
			}

			active_object->Set_Pos( posx + pos_hor, posy + ( rect.h * 0.1f ) );
			active_object->Move( velx, vely );
		}

		return;
	}

	if( state == STA_FLY )
	{
		return;
	}

	// if control is pressed search for items in front of the player
	if( ( pKeyboard->keys[pPreferences->key_action] || pJoystick->Button( pPreferences->joy_button_action ) ) && !ducked && state != STA_CLIMB )
	{
		// next position velocity with extra size
		float check_x = ( velx > 0 ) ? (velx + 5) : ( velx - 5);
		
		// if wrong direction return
		if( ( direction == DIR_LEFT && check_x >= 0 ) || ( direction == DIR_RIGHT && check_x <= 0 ) )
		{
			return;
		}

		// check the next player position for objects
		unsigned int object_count = Collision_Check_Relative( ( direction == DIR_LEFT ) ? (check_x) : (col_rect.w), 0, ( direction == DIR_LEFT ) ? (-check_x) : (check_x) ).size();

		// possible objects
		for( unsigned int i = 0; i < object_count; i++ )
		{
			cObjectCollision *collision = Collision_Get_last();

			// if valid collision
			if( !collision )
			{
				continue;
			}

			// collision found
			if( !active_object )
			{
				// enemy item
				if( collision->type == CO_ENEMY )
				{
					cEnemy *enemy = static_cast<cEnemy *>(pLevel->pSprite_Manager->Get_Pointer( collision->number ));

					if( enemy->type == TYPE_TURTLE && enemy->state == STA_STAY )
					{
						Get_Item( TYPE_TURTLE, 0, enemy );
					}
				}
				// other items here
			}
			
			Collision_Delete( collision );
		}
	}
}

void cPlayer :: Release_item( bool set_position /* = 1 */, bool no_action /* = 0 */ )
{
	if( !active_object )
	{
		return;
	}

	ObjectDirection kick_direction = DIR_UNDEFINED;

	// if flying : opposite direction
	if( state == STA_FLY )
	{
		kick_direction = Get_OppositeDirection( direction );
	}
	// if jumping : upwards
	else if( jump_power > 0 )
	{
		kick_direction = DIR_UP;
	}
	// use current direction
	else
	{
		kick_direction = direction;
	}

	// if frozen
	if( active_object->freeze_counter )
	{
		no_action = 1;
	}

	// add back to level
	if( active_object->type == TYPE_TURTLE )
	{
		cTurtle *turtle = static_cast<cTurtle *>(active_object);
		
		// play kick sound if not dead
		if( !turtle->dead )
		{
			pAudio->PlaySound( "enemy/turtle/shell/hit.ogg" );
		}

		// if object got kicked upwards use Stay
		if( kick_direction == DIR_UP || no_action )
		{
			turtle->state = STA_STAY;
			turtle->turtle_state = TURTLE_SHELL_STAND;
			
			// small direction acceleration if no object action
			if( kick_direction != DIR_UP && no_action )
			{
				active_object->velx = ( kick_direction == DIR_LEFT ) ? (static_cast<float>(-5)) : (static_cast<float>(5));
			}
			// if jumping kick the object upwards
			else if( kick_direction == DIR_UP && !no_action )
			{
				active_object->vely = -10 + vely;
			}
		}
		// default object horizontal kicking
		else
		{
			turtle->state = STA_RUN;
			turtle->turtle_state = TURTLE_SHELL_RUN;
			turtle->velx = ( kick_direction == DIR_LEFT ) ? (-turtle->speed_shell) : (turtle->speed_shell);
		}

		if( !turtle->dead )
		{
			turtle->massivetype = MASS_MASSIVE;
			turtle->direction = kick_direction;

			if( !turtle->freeze_counter )
			{
				turtle->playercounter = DESIRED_FPS * 0.5f;
			}
		}

		turtle->Update_valid_update();
	}

	// if the new object position is in the player ( e.g. add velx to move slowly out of the player position )
	bool position_in_player = 0;

	// set position
	if( set_position )
	{
		// middle the object
		float obj_diff_x = ( active_object->col_rect.w / 2 ) - ( col_rect.w / 2 );

		// set start position
		float start_posx = ( posx + obj_diff_x );
		// put in front if left or right
		if( kick_direction == DIR_LEFT || kick_direction == DIR_RIGHT )
		{
			start_posx += ( ( kick_direction == DIR_LEFT ) ? (-rect.w) : (rect.w) * 1.2f );
		}
		float start_posy = posy;
		// put on top if up
		if( kick_direction == DIR_UP )
		{
			start_posy -= rect.h * 0.9f;
		}
		active_object->Set_Pos( start_posx, start_posy );
		// set step size
		float step_x = 0;
		float step_y = 0;
	
		if( kick_direction == DIR_UP )
		{
			step_x = 2;
		}
		else if( kick_direction == DIR_LEFT )
		{
			step_x = 2;
		}
		else
		{
			step_x = -2;
		}

		// check for a valid position to release
		while( start_posx - 50 < active_object->posx && start_posx + 50 > active_object->posx && 
			   start_posy - 50 < active_object->posy && start_posy + 50 > active_object->posy )
		{
			cObjectCollisionType col_list = active_object->Collision_Check_Relative( step_x, step_y, 0, 0, COLLIDE_ALLOW_BLOCKING );
			
			// check for blocking objects
			if( col_list.size() )
			{
				// collides with a blocking object
				if( col_list.find( ARRAY_MASSIVE ) || col_list.find( ARRAY_ACTIVE ) || col_list.find( ARRAY_ENEMY ) )
				{
					// move into the opposite kick direction
					active_object->Move( step_x, step_y, 1 );
					continue;
				}
			}

			break;
		}

		cObjectCollisionType col_list = active_object->Collision_Check( &active_object->col_rect, COLLIDE_ALLOW_BLOCKING );

		// check for blocking objects on the final position
		if( col_list.size() )
		{
			// still collides with a blocking object
			if( col_list.find( ARRAY_MASSIVE ) || col_list.find( ARRAY_ACTIVE ) || col_list.find( ARRAY_ENEMY ) )
			{
				if( active_object->type == TYPE_TURTLE )
				{
					// downgrade if collision with static blocking objects
					if( active_object->state == STA_RUN && ( col_list.find( ARRAY_MASSIVE ) || col_list.find( ARRAY_ACTIVE ) ) )
					{
						active_object->DownGrade( 1 );
					}
					// downgrade always
					else if( active_object->state == STA_STAY )
					{
						active_object->DownGrade( 1 );
					}
				}
			}
			// collides with player
			else if( col_list.find( TYPE_PLAYER ) )
			{
				position_in_player = 1;
			}
		}
	}

	// if found position is in the player
	if( position_in_player )
	{
		if( active_object->type == TYPE_TURTLE )
		{
			cTurtle *turtle = static_cast<cTurtle *>(active_object);
			
			// if staying
			if( turtle->state == STA_STAY )
			{
				// if not moving fast enough
				if( active_object->velx > -5 && active_object->velx <= 0 )
				{
					active_object->velx -= 5;
				}
				else if( active_object->velx < 5 && active_object->velx >= 0 )
				{
					active_object->velx += 5;
				}
			}
			else if( turtle->state == STA_RUN )
			{
				turtle->playercounter = 10;
			}
		}
	}

	active_object->Collisions_Clear();
	active_object->Reset_onGround();
	active_object->Update_Direction();

	// unset link
	active_object = NULL;
	// load default images
	Load_Images();
}

void cPlayer :: Set_Type( SpriteType item_type, bool animation /* = 1 */, bool sound /* = 1 */ )
{
	if( item_type == TYPE_PLAYER )
	{
		Set_Type( MARYO_SMALL, animation, sound );
	}
	else if( item_type == TYPE_MUSHROOM_DEFAULT )
	{
		Set_Type( MARYO_BIG, animation, sound );
	}
	else if( item_type == TYPE_FIREPLANT )
	{
		Set_Type( MARYO_FIRE, animation, sound );
	}
}

void cPlayer :: Set_Type( Maryo_type new_type, bool animation /* = 1 */, bool sound /* = 1 */ )
{
	// already set
	if( maryo_type == new_type )
	{
		return;
	}

	// play sound
	if( sound )
	{
		if( new_type == MARYO_BIG )
		{
			pAudio->PlaySound( "item/mushroom.ogg", RID_MUSHROOM );
		}
		else if( new_type == MARYO_FIRE )
		{
			pAudio->PlaySound( "item/fireplant.ogg", RID_FIREPLANT );
		}
		else if( new_type == MARYO_ICE )
		{
			pAudio->PlaySound( "item/mushroom_blue.ogg", RID_MUSHROOM_BLUE );
		}
		else if( new_type == MARYO_CAPE )
		{
			pAudio->PlaySound( "item/feather.ogg", RID_FEATHER );
		}
		else if( new_type == MARYO_GHOST )
		{
			pAudio->PlaySound( "item/mushroom_ghost.ogg", RID_MUSHROOM_GHOST );
		}
	}

	// was flying
	if( maryo_type == MARYO_CAPE )
	{
		Stop_flying( 0 );
	}

	// remember old type
	Maryo_type old_type = maryo_type;

	// draw animation and set new type
	if( animation )
	{
		Draw_Animation( new_type );
	}
	// only set type
	else
	{
		maryo_type = new_type;
		Load_Images();
	}

	// to ghost
	if( maryo_type == MARYO_GHOST )
	{
		ghost_time = DESIRED_FPS * 10;
		maryo_type_temp_power = old_type;
	}
	// was ghost
	else if( old_type == MARYO_GHOST )
	{
		ghost_time = 0;
		ghost_time_mod = 0;
		maryo_type_temp_power = MARYO_DEAD;

		// check for ghost ground
		if( ground_object )
		{
			cBaseBox *box = dynamic_cast<cBaseBox *>( ground_object );

			if( box )
			{
				// ghost box
				if( box->box_invisible == 2 )
				{
					Reset_onGround();
				}
			}
		}
	}
}

void cPlayer :: Set_MovingState( Moving_state new_state )
{
	if( new_state == state )
	{
		return;
	}

	// was flying
	if( state == STA_FLY )
	{
		ChangeSize( 0, -( images[MARYO_IMG_FALL]->h - images[MARYO_IMG_FLY]->h ) );
		Set_Image( MARYO_IMG_FALL + direction );

		// reset flying rotation
		rotz = 0;
	}
	// was falling
	else if( state == STA_FALL )
	{
		// reset slow fall/parachute
		parachute = 0;
	}
	// was running
	else if( state == STA_RUN )
	{
		walk_time = 0;
		running_particle_counter = 0;
	}

	// # before new state is set
	if( new_state == STA_STAY )
	{
		if( ground_object )
		{
			vely = 0;
		}
	}
	else if( new_state == STA_FALL )
	{
		Reset_onGround();
		walk_count = 0;
		jump_power = 0;
	}
	else if( new_state == STA_FLY )
	{
		Reset_onGround();
	}
	else if( new_state == STA_JUMP )
	{
		Reset_onGround();
	}
	else if( new_state == STA_CLIMB )
	{
		Reset_onGround();
		velx = 0;
		vely = 0;
	}

	state = new_state;

	// # after new state is set
	if( state == STA_FLY )
	{
		Release_item( 1 );
	}
}

bool cPlayer :: ChangeSize( float x, float y, bool only_check /* = 0 */ )
{
	bool valid_hor = 0;
	float check_pos = x;

	// no value
	if( x == 0 )
	{
		valid_hor = 1;
	}

	while( !valid_hor )
	{
		if( !Collision_Check_Relative( check_pos, 0, 0, 0, COLLIDE_ONLY_CHECK ).size() )
		{
			if( !only_check )
			{
				Col_Move( check_pos, 0, 1, 1 );
			}

			valid_hor = 1;
		}

		// move to opposite direction
		if( x > 0 )
		{
			check_pos--;

			// nothing found
			if( check_pos < -x )
			{
				break;
			}
		}
		else
		{
			check_pos++;

			// nothing found
			if( check_pos > x )
			{
				break;
			}
		}
	}

	bool valid_ver = 0;
	check_pos = y;

	// no value
	if( y == 0 )
	{
		valid_ver = 1;
	}

	while( !valid_ver )
	{
		if( !Collision_Check_Relative( 0, check_pos, 0, 0, COLLIDE_ONLY_CHECK ).size() )
		{
			if( !only_check )
			{
				Col_Move( 0, check_pos, 1, 1 );
			}

			valid_ver = 1;
		}

		// move to opposite direction
		if( y > 0 )
		{
			check_pos--;

			// nothing found
			if( check_pos < -y )
			{
				break;
			}
		}
		else
		{
			check_pos++;

			// nothing found
			if( check_pos > y )
			{
				break;
			}
		}
	}


	// if both directions valid
	if( valid_hor && valid_ver )
	{
		return 1;
	}

	return 0;
}

void cPlayer :: ResetSave( void )
{
	pOverworld_manager->Reset();

	pLevel->Unload();
	Reset();
	Ball_clear();

	lives = 3;
	goldpieces = 0;
	points = 0;

	timedisplay->counter = 0;

	Set_Type( MARYO_SMALL, 0, 0 );

	Itembox->Reset();
}

void cPlayer :: Reset( bool full /* = 1 */ )
{
	// reset level states
	Set_Visible( 1 );
	Set_Pos( startposx, startposy + start_image->col_h - col_rect.h );
	Set_Direction( start_direction );
	ducked = 0;
	Set_MovingState( STA_FALL );
	Set_Image( Get_Image() + direction );
	jump_power = 0;
	start_enemyjump = 0;
	jump_accel_up = 0;
	jump_vel_deaccel = 0;
	no_velx_counter = 0;
	no_vely_counter = 0;
	UpKeytime = 0;
	velx = 0;
	vely = 0;
	Collisions_Clear();
	walk_count = 0;
	Reset_onGround();
	if( active_object )
	{
		active_object = NULL;
		Load_Images();
	}
	kill_multiplier = 1;
	last_kill_counter = 0;

	// reset item and camera
	if( full )
	{
		pCamera->Center();
		invincible = 0;
		invincible_mod = 0;
		invincible_star = 0;
		Set_Color_Combine( 0, 0, 0, 0 );
		Itembox->Push_back();
	}
}

void cPlayer :: Goto_next_Level( void )
{
	// custom level
	if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
	{
		// delay unload level
		pLevel->Unload( 1 );
		// Enter Menu
		Game_Action = GA_ENTER_MENU;
	}
	else
	{
		// Finish level
		pActive_Overworld->Goto_next_Level();
		// Enter World
		Game_Action = GA_ENTER_WORLD;
	}
}

void cPlayer :: Goto_SubLevel( string str_level, string str_entry )
{
	// same level - scroll to the position
	if( str_level.compare( Get_filename( pLevel->data_file, 0, 0 ) ) == 0 )
	{
		Reset( 0 );
		
		cLevel_Entry *entry = NULL;

		// entry position
		if( !str_entry.empty() )
		{
			// Search for the entry
			for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
			{
				cSprite *obj = (*itr);

				// only level entry
				if( obj->type != TYPE_LEVEL_ENTRY )
				{
					continue;
				}
				
				cLevel_Entry *level_entry = static_cast<cLevel_Entry *>(obj);

				// found
				if( level_entry->entry_name.compare( str_entry ) == 0 )
				{
					entry = level_entry;
					break;
				}
			}
		}

		// use entry position
		if( entry )
		{
			Set_Pos( entry->posx, entry->posy );
			// set invisible for warp animation
			Set_Visible( 0 );
		}
		// not found
		else if( !str_entry.empty() )
		{
			printf( "Warning : Level entry %s not found\n", str_entry.c_str() );
		}

		pFramerate->speedfactor = 1;

		// move camera to new position
		for( unsigned int i = 0; i < 200; i++ )
		{
			Update_Camera( 1 );

			// draw
			Draw_Game();

			pVideo->Render();
		}

		// warp animation
		if( entry )
		{
			// set back visible
			Set_Visible( 1 );
			// activate entry
			entry->Activate();
		}
	}
	// another level - load the level
	else
	{
		Game_Action = GA_ENTER_LEVEL;
		Game_Action_Data.add( "level", str_level.c_str() );
	}
}

void cPlayer :: Update( void )
{
	if( editor_enabled )
	{
		return;
	}

	// check if got stuck
	if( !ducked )
	{
		Update_antistuck();
	}

	// check if starting a jump is possible
	Update_jump_keytime();

	// update states
	Update_jump();
	Update_climbing();
	Update_falling();
	Update_walking();
	Update_running();
	Update_ducking();
	Update_staying();
	Update_flying();
	// throwing
	if( throwing_counter > 0 )
	{
		throwing_counter -= pFramerate->speedfactor;

		if( throwing_counter < 0 )
		{
			throwing_counter = 0;
		}
	}
	// ghost
	if( ghost_time > 0 )
	{
		ghost_time -= pFramerate->speedfactor;

		// ended
		if( ghost_time <= 0 )
		{
			pAudio->PlaySound( "player/ghost_end.ogg", RID_MUSHROOM_GHOST );
			Set_Type( maryo_type_temp_power, 1, 0 );
		}
		// near end
		else if( ghost_time * 0.5f < DESIRED_FPS * 3 )
		{
			ghost_time_mod = ( DESIRED_FPS * 3 ) - ( ghost_time * 0.5f );
		}
	}
	// invincible
	if( invincible > 0 )
	{
		invincible -= pFramerate->speedfactor;

		if( invincible < 0 )
		{
			invincible = 0;
		}

		if( invincible_mod < 0 )
		{
			invincible_mod = invincible * 3;

			if( invincible_mod > 180 )
			{
				invincible_mod = 180;
			}
		}
		
		invincible_mod -= pFramerate->speedfactor * 20;
	}
	// draw stars if in godmode or star invincible
	if( godmode || invincible_star > 0 )
	{
		if( editor_level_enabled )
		{
			Set_Color_Combine( 0, 0, 0, 0 );
		}
		else
		{
			// draw stars
			invincible_starcounter += pFramerate->speedfactor;

			if( invincible_star > 0 )
			{
				invincible_star -= pFramerate->speedfactor;

				if( invincible_star <= 0 )
				{
					Set_Color_Combine( 0, 0, 0, 0 );
					invincible_star = 0;
					pAudio->FadeOutMusic( 500 );
				}
			}

			while( invincible_starcounter > 1 )
			{
				// set particle color
				Color particle_color = orange;
				particle_color.green += static_cast<Uint8>( invincible_mod / 5 );
				particle_color.blue += static_cast<Uint8>( invincible_mod / 1.5f );

				// create particle
				cParticleAnimation *anim = new cParticleAnimation( posx + static_cast<float>( rand() % static_cast<int>( col_rect.w * 0.9f ) ), posy + static_cast<float>( rand() % static_cast<int>( col_rect.h * 0.9f ) ) );
				anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
				anim->Set_ZPos( posz - 0.000001f );
				anim->Set_Time_to_Live( 0.3f );
				anim->Set_Fading_Alpha( 1 );
				anim->Set_Fading_Size( 1 );
				anim->Set_ConstRotationZ( 0.9f, 10 );
				// godmode stars
				if( godmode )
				{
					anim->Set_DirectionRange( 180, 180 );
					anim->Set_Speed( 3.5f, 2.5f );
					anim->Set_Scale( 0.3f );
					anim->Set_Color( lightblue );
				}
				// default
				else
				{
					anim->Set_Time_to_Live( 2 );
					anim->Set_Speed( 1, 1.5f );
					anim->Set_Scale( 0.2f );
					anim->Set_Color( particle_color );
				}

				anim->Set_Blending( BLEND_ADD );
				pAnimationManager->Add( anim );

				invincible_starcounter--;
			}
		}
	}

	// update active item
	Update_item();

	// image counter
	if( state == STA_WALK || ( maryo_type != MARYO_CAPE && state == STA_RUN ) )
	{
		// 4 frames
		if( maryo_type == MARYO_SMALL )
		{
			walk_count += pFramerate->speedfactor * 0.35f;
		}
		// 4 frames
		else if( maryo_type == MARYO_BIG )
		{
			walk_count += pFramerate->speedfactor * 0.3f;
		}
		// 4 frames
		else
		{
			walk_count += pFramerate->speedfactor * 0.3f;
		}

		// ground type modification
		float vel = ( velx > 0 ? velx : -velx );

		if( vel && ground_object && ground_object->image)
		{
			float ground_mod = 0;

			switch( ground_object->image->ground_type )
			{
				case GROUND_ICE:
				{
					ground_mod = 0.125f;
					break;
				}
				case GROUND_SAND:
				{
					ground_mod = 0.075f;
					break;
				}
				case GROUND_PLASTIC:
				{
					ground_mod = 0.03f;
					break;
				}
				default:
				{
					break;
				}
			}

			if( ground_mod )
			{
				walk_count += ground_mod * ( 5 / vel ) * pFramerate->speedfactor;
			}
		}
	}
	else if( state == STA_RUN )
	{
		// ? frames
		if( maryo_type == MARYO_SMALL )
		{
			//walk_count += pFramerate->speedfactor * 0.35f;
		}
		// ? frames
		else if( maryo_type == MARYO_BIG )
		{
			//walk_count += pFramerate->speedfactor * 0.3f;
		}
		// 2 frames
		else if( maryo_type == MARYO_CAPE )
		{
			walk_count += pFramerate->speedfactor * 0.35f;
		}
		// ? frames
		else
		{
			//walk_count += pFramerate->speedfactor * 0.3f;
		}
	}
	/*else if( state == FALL || state == JUMP )
	{
		walk_count += pFramerate->speedfactor * 0.3f;
	}*/
	else if( state == STA_CLIMB )
	{
		if( vely != 0 || velx != 0 )
		{
			walk_count += pFramerate->speedfactor * 0.7f;
		}
	}
	else if( state == STA_FLY )
	{
		walk_count += pFramerate->speedfactor * 0.6f;
	}

	// add x velocity
	if( state == STA_WALK || state == STA_RUN || state == STA_CLIMB )
	{
		if( velx > 0 )
		{
			walk_count += ( velx * 0.05f ) * pFramerate->speedfactor;
		}
		else if( velx < 0 )
		{
			walk_count += ( -velx * 0.05f ) * pFramerate->speedfactor;
		}
	}

	if( state == STA_WALK || state == STA_STAY || ( maryo_type != MARYO_CAPE && state == STA_RUN ) )
	{
		// 4 frames
		if( maryo_type == MARYO_SMALL )
		{
			if( walk_count >= 8 )
			{
				walk_count = 0;
			}
		}
		// 4 frames
		else if( maryo_type == MARYO_BIG )
		{
			if( walk_count >= 8 )
			{
				walk_count = 0;
			}
		}
		// 4 frames
		else
		{
			if( walk_count >= 8 )
			{
				walk_count = 0;
			}
		}
	}
	else if( state == STA_RUN )
	{
		// ? frames
		if( maryo_type == MARYO_SMALL )
		{
		}
		// ? frames
		else if( maryo_type == MARYO_BIG )
		{
		}
		// 2 frames
		else if( maryo_type == MARYO_CAPE )
		{
			if( walk_count >= 4 )
			{
				walk_count = 0;
			}
		}
		// ? frames
		else
		{
		}
	}
	/*else if( state == STA_FALL || state == STA_JUMP )
	{
		if( walk_count > 6 )
		{
			walk_count = 6;
		}
	}*/
	else if( state == STA_FLY )
	{
		// 4 frames
		if( walk_count > 8 )
		{
			walk_count = 0;
		}
	}
	else
	{
		// 4 frames
		if( walk_count >= 8 )
		{
			walk_count = 0;
		}
	}

	// Set image
	if( state != STA_CLIMB )
	{
		Set_Image( Get_Image() + direction );
	}
	else
	{
		Set_Image( Get_Image() );
	}

	// Special
	Update_Kill_Multiplier();
}

void cPlayer :: Update_Camera( bool smooth /* = 0 */ )
{
	// if smooth scrolling
	if( smooth )
	{
		float vel = 0;

		if( posx - pCamera->x > ( GAME_RES_W / 6 ) * 3 )
		{
			vel = ( ( posx - pCamera->x - ( GAME_RES_W / 6 ) * 3 ) / 30 );

			if( vel < 2 )
			{
				vel = 2;
			}

			pCamera->Move( vel * pFramerate->speedfactor, 0 );
		}
		else if( posx - pCamera->x < ( GAME_RES_W / 6 ) * 2.5f )
		{
			vel = ( ( posx - pCamera->x - ( GAME_RES_W / 6 ) * 3 ) / 20 );

			if( vel > -2 )
			{
				vel = -2;
			}

			pCamera->Move( vel * pFramerate->speedfactor, 0 );
		}

		if( posy - pCamera->y > ( GAME_RES_H / 6 ) * 3 )
		{
			vel = ( ( posy - pCamera->y - ( GAME_RES_H / 6 ) * 3 ) / 30 );

			if( vel < 2 )
			{
				vel = 2;
			}

			pCamera->Move( 0, vel * pFramerate->speedfactor );
		}
		else if( posy - pCamera->y < ( GAME_RES_H / 6 ) * 2.5f )
		{
			vel = ( ( posy - pCamera->y - ( GAME_RES_H / 6 ) * 3 ) / 20 );

			if( vel > -2 )
			{
				vel = -2;
			}

			pCamera->Move( 0, vel * pFramerate->speedfactor );
		}
	}
	else
	{
		pCamera->Update();
	}
}

void cPlayer :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// invincible
	if( invincible > 0 )
	{
		// star invincible
		if( invincible_star > 0 )
		{
			Set_Color_Combine( invincible_mod / 130, invincible_mod / 130, invincible_mod / 180, GL_ADD );
		}
		// default invincible
		else
		{
			Set_Color( 255, 255, 255, 255 - static_cast<Uint8>(invincible_mod) );
		}
	}
	// ghost
	if( ghost_time > 0 )
	{
		// ghost shadows
		color.alpha = 32 + static_cast<int>(ghost_time_mod);
		float old_posx = posx, old_posy = posy, old_posz = posz;

		for( unsigned int i = 0; i < 5; i++ )
		{
			color.alpha -= 5;
			posx -= velx * 0.2f + ( ( rand() % 1000 ) * 0.001f );
			posy -= vely * 0.2f + ( ( rand() % 1000 ) * 0.001f );
			posz -= 0.000001f;

			cMovingSprite::Draw( request );
		}

		// set original values
		posx = old_posx;
		posy = old_posy;
		posz = old_posz;
		color.alpha = 48 + static_cast<int>(ghost_time_mod);
	}

	cMovingSprite::Draw( request );

	if( invincible > 0 || ghost_time > 0 )
	{
		Set_Color( white );
	}
}

void cPlayer :: Draw_Animation( Maryo_type new_mtype )
{
	// already set or invalid
	if( new_mtype == maryo_type || maryo_type == MARYO_DEAD || new_mtype == MARYO_DEAD )
	{
		return;
	}

	Maryo_type maryo_type_old = maryo_type;
	bool parachute_old = parachute;

	float posx_old = posx;
	float posy_old = posy;

	// ChangeSize needs new state size
	maryo_type = maryo_type_old;
	parachute = parachute_old;
	Load_Images();

	// correct position for bigger maryo
	if( maryo_type_old == MARYO_SMALL && ( new_mtype == MARYO_BIG || new_mtype == MARYO_FIRE || new_mtype == MARYO_ICE || new_mtype == MARYO_CAPE || new_mtype == MARYO_GHOST ) )
	{
		ChangeSize( -5, -12 );
		//Col_Move( 0, 12, 1, 1 );
	}
	// correct position for small maryo
	else if( ( maryo_type_old == MARYO_BIG || maryo_type_old == MARYO_FIRE || maryo_type_old == MARYO_ICE || new_mtype == MARYO_CAPE || maryo_type_old == MARYO_GHOST ) && new_mtype == MARYO_SMALL )
	{
		ChangeSize( 5, 12 );
		//Col_Move( 0, -12, 1, 1 );
	}

	float posx_new = posx;
	float posy_new = posy;

	for( unsigned int i = 0; i < 7; i++ )
	{
		// draw current type
		if( i % 2 )
		{
			maryo_type = maryo_type_old;
			parachute = parachute_old;
			Load_Images();
			
			Set_Pos( posx_old, posy_old );
		}
		// draw animation/new type
		else
		{
			maryo_type = new_mtype;
			if( new_mtype != MARYO_CAPE )
			{
				parachute = 0;
			}
			Load_Images();
			
			Set_Pos( posx_new, posy_new );
		}

		// draw
		Draw_Game();
		pVideo->Render();

		// frame delay
		SDL_Delay( 120 );
	}

	pFramerate->Reset();
}

unsigned int cPlayer :: Get_Image( void )
{
	// throwing
	if( throwing_counter && ( maryo_type == MARYO_FIRE || maryo_type == MARYO_ICE ) && !ducked && ( state == STA_FALL || state == STA_STAY || state == STA_WALK || state == STA_RUN || state == STA_JUMP ) )
	{
		int imgnum = 0;

		if( throwing_counter < DESIRED_FPS * 0.2f )
		{
			imgnum = 2;
		}

		return MARYO_IMG_THROW + imgnum;
	}

	// ducked
	if( ducked && ( state == STA_STAY || state == STA_WALK || state == STA_RUN || state == STA_JUMP || state == STA_FALL ) )
	{
		return MARYO_IMG_DUCK;
	}

	// parachute
	if( parachute && state == STA_FALL )
	{
		return MARYO_IMG_SPECIAL_1;
	}

	if( state == STA_STAY || state == STA_WALK || ( maryo_type != MARYO_CAPE && state == STA_RUN ) )
	{
		unsigned int imgnum = static_cast<unsigned int>(walk_count);

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_WALK + imgnum;
	}
	else if( state == STA_RUN )
	{
		unsigned int imgnum = static_cast<unsigned int>(walk_count);

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_RUN + imgnum;
	}
	else if( state == STA_FALL )
	{
		/*unsigned int imgnum = static_cast<unsigned int>(walk_count);

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_FALL + imgnum;*/
		return MARYO_IMG_FALL;
	}
	/*else if( state == STAY )
	{
		return MARYO_IMG_STAND;
	}*/
	else if( state == STA_JUMP )
	{
		/*unsigned int imgnum = static_cast<unsigned int>(walk_count);

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_JUMP + imgnum;*/
		return MARYO_IMG_JUMP;
	}
	else if( state == STA_FLY )
	{
		unsigned int imgnum = static_cast<unsigned int>(walk_count);

		for( unsigned int i = 0; i < imgnum; i++ )
		{
			if( imgnum % 2 )
			{
				imgnum--;
			}
		}

		return MARYO_IMG_FLY + imgnum;
	}
	else if( state == STA_CLIMB )
	{
		if( walk_count > 4 )
		{
			return MARYO_IMG_CLIMB;
		}
		else
		{
			return MARYO_IMG_CLIMB + 1;
		}
	}

	return MARYO_IMG_STAND;
}

void cPlayer :: Load_Images( void )
{
	// not valid
	if( maryo_type == MARYO_DEAD )
	{
		return;
	}

	Clear_Images();

	// special maryo images state
	string special_state;
	// if holding item
	if( active_object )
	{
		special_state = "_holding";
	}

	if( maryo_type == MARYO_SMALL )
	{
		/********************* Small **************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/small/stand_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/stand_right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/small/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/small/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/jump_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/small/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/small/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/climb_right.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_BIG )
	{
		/********************* Big ****************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/big/stand_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/stand_right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/big/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/big/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/jump_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/big/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/big/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/big/climb_right.png" ) );
		// throwing
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		/****************************************************/
	}
	else if( maryo_type == MARYO_FIRE )
	{
		/********************* Fire **************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/fire/stand_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/stand_right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/fire/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/fire/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/fall_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/fire/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/fire/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/climb_right.png" ) );
		// throwing
		images.push_back( pVideo->Get_Surface( "maryo/fire/throw_left_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/throw_right_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/throw_left_2.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/fire/throw_right_2.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_ICE )
	{
		/********************* Ice **************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/ice/stand_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/stand_right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/ice/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/ice/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/fall_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/ice/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/ice/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/climb_right.png" ) );
		// throwing
		images.push_back( pVideo->Get_Surface( "maryo/ice/throw_left_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/throw_right_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/throw_left_2.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ice/throw_right_2.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_CAPE )
	{
		/********************* Cape **************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/flying/left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( pVideo->Get_Surface( "maryo/flying/run_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/run_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/run_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/run_right_2" + special_state + ".png" ) );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/flying/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/flying/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fall_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/flying/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/flying/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/climb_right.png" ) );
		// throwing
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// flying
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_left_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_right_1.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_left_2.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_right_2.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_left_3.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_right_3.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_left_4.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/fly_right_4.png" ) );
		// slow fall/parachute
		images.push_back( pVideo->Get_Surface( "maryo/flying/slow_fall_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/flying/slow_fall_right.png" ) );
		/****************************************************/
	}
	else if( maryo_type == MARYO_GHOST )
	{
		/********************* Ghost **************************/
		// standing
		images.push_back( pVideo->Get_Surface( "maryo/ghost/stand_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/stand_right" + special_state + ".png" ) );
		// walking
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_right_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_left_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_right_2" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_left_1" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/walk_right_1" + special_state + ".png" ) );
		// running
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		// falling
		images.push_back( pVideo->Get_Surface( "maryo/ghost/fall_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/fall_right" + special_state + ".png" ) );
		// jumping
		images.push_back( pVideo->Get_Surface( "maryo/ghost/jump_left" + special_state + ".png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/fall_right" + special_state + ".png" ) );
		// dead
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/small/dead_right.png" ) );
		// ducked
		images.push_back( pVideo->Get_Surface( "maryo/ghost/duck_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/duck_right.png" ) );
		// climbing
		images.push_back( pVideo->Get_Surface( "maryo/ghost/climb_left.png" ) );
		images.push_back( pVideo->Get_Surface( "maryo/ghost/climb_right.png" ) );
		// throwing
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		images.push_back( NULL );
		/****************************************************/
	}

	// set image
	Set_Image( Get_Image() + direction );
}

void cPlayer :: Get_Item( SpriteType item_type, bool force /* = 0 */, cMovingSprite *base /* = NULL */ )
{
	// Default Mushroom
	if( item_type == TYPE_MUSHROOM_DEFAULT ) 
	{
		if( ( maryo_type == MARYO_SMALL || force ) && ChangeSize( -5, -12, 1 ) ) 
		{
			// change to big
			if( maryo_type == MARYO_SMALL )
			{
				// set type
				Set_Type( MARYO_BIG, 1, 1 );
			}
			// move item to itembox
			else if( maryo_type == MARYO_BIG )
			{
				// item to itembox
				Itembox->Set_Item( TYPE_MUSHROOM_DEFAULT );
			}
			// change to big
			else if( maryo_type == MARYO_FIRE )
			{
				// set type
				Set_Type( MARYO_BIG, 0, 1 );
				// old item to itembox
				Itembox->Set_Item( TYPE_FIREPLANT );
			}
		}
		// move item to itembox
		else
		{
			Itembox->Set_Item( TYPE_MUSHROOM_DEFAULT );
		}
	}
	// Fireplant
	else if( item_type == TYPE_FIREPLANT ) 
	{
		if( ( ( maryo_type == MARYO_SMALL || force ) && ChangeSize( -5, -12, 1 ) ) || 
			( ( maryo_type == MARYO_BIG || maryo_type == MARYO_ICE || force ) ) )
		{
			// move item to itembox
			if( maryo_type == MARYO_FIRE ) 
			{
				// move item to itembox
				Itembox->Set_Item( TYPE_FIREPLANT );
			}
			// change to fire
			else
			{
				// set type
				Set_Type( MARYO_FIRE, 1, 1 );
			}
		}
		// fire explosion
		else if( maryo_type == MARYO_FIRE && Itembox->item_id == TYPE_FIREPLANT )
		{
			Ball_add( FIREBALL_EXPLOSION );
			pointsdisplay->Add_Points( 1000, posx + ( col_rect.w / 2 ), posy + 2 );	
		}
		// move item to itembox
		else
		{
			Itembox->Set_Item( TYPE_FIREPLANT );
		}
	}
	// Blue Mushroom
	else if( item_type == TYPE_MUSHROOM_BLUE ) 
	{
		if( ( ( maryo_type == MARYO_SMALL || force ) && ChangeSize( -5, -20, 1 ) ) || 
			( ( maryo_type == MARYO_BIG || maryo_type == MARYO_FIRE || force ) ) )
		{
			// move item to itembox
			if( maryo_type == MARYO_ICE ) 
			{
				// move item to itembox
				Itembox->Set_Item( TYPE_MUSHROOM_BLUE );
			}
			// change to ice
			else
			{
				// set type
				Set_Type( MARYO_ICE, 1, 1 );
			}
		}
		// ice explosion
		else if( maryo_type == MARYO_ICE && Itembox->item_id == TYPE_MUSHROOM_BLUE )
		{
			Ball_add( ICEBALL_EXPLOSION );
			pointsdisplay->Add_Points( 1000, posx + ( col_rect.w / 2 ), posy + 2 );	
		}
		// move item to itembox
		else
		{
			Itembox->Set_Item( TYPE_MUSHROOM_BLUE );
		}
	}
	// Ghost Mushroom
	else if( item_type == TYPE_MUSHROOM_GHOST ) 
	{
		if( ( ( maryo_type == MARYO_SMALL || force ) && ChangeSize( -5, -20, 1 ) ) || 
			maryo_type != MARYO_SMALL )
		{
			// set back ghost time
			if( maryo_type == MARYO_GHOST )
			{
				ghost_time = DESIRED_FPS * 10;
				ghost_time_mod = 0;
			}
			// change to ghost
			else
			{
				// set type
				Set_Type( MARYO_GHOST, 1, 1 );
			}
		}
		// move item to itembox
		else
		{
			Itembox->Set_Item( TYPE_MUSHROOM_GHOST );
		}
	}
	// Mushroom 1-UP
	else if( item_type == TYPE_MUSHROOM_LIVE_1 ) 
	{
		pAudio->PlaySound( "item/live_up.ogg", RID_1UP_MUSHROOM );
		livedisplay->AddLives( 1 );
	}
	// Mushroom Poison
	else if( item_type == TYPE_MUSHROOM_POISON ) 
	{
		DownGrade();
	}
	// Moon
	else if( item_type == TYPE_MOON ) 
	{
		pAudio->PlaySound( "item/moon.ogg", RID_MOON );
		livedisplay->AddLives( 3 );
	}
	// Jumping Star
	else if( item_type == TYPE_JSTAR ) 
	{
		// todo : check if music is already playing
		pAudio->PlayMusic( "game/star.ogg", 0, 1, 500 );
		pAudio->PlayMusic( pLevel->musicfile, -1, 0 );
		pointsdisplay->Add_Points( 1000, posx + ( col_rect.w / 2 ), posy + 2 );
		invincible = DESIRED_FPS * 16;
		invincible_star = DESIRED_FPS * 15;
	}
	// Turtle Shell
	else if( item_type == TYPE_TURTLE ) 
	{
		active_object = base;
		active_object->massivetype = MASS_PASSIVE;
		active_object->state = STA_OBJ_LINKED;
		active_object->Update_valid_update();
		active_object->Reset_onGround();
		active_object->velx = 0;
		active_object->vely = 0;
		
		
		cTurtle *turtle = static_cast<cTurtle *>(active_object);
		// clear the standing counter
		turtle->counter = 0;
		// clear player counter
		turtle->playercounter = 0;
		turtle->Set_Image( 5 );

		// load holding images
		Load_Images();
	}
}

float cPlayer :: Get_Vel_mod( void )
{
	float vel_mod = 1;

	// if running key is pressed or always run
	if( pPreferences->always_run || pKeyboard->keys[pPreferences->key_action] || pJoystick->Button( pPreferences->joy_button_action ) )
	{
		vel_mod = 1.5f;
	}

	if( invincible_star > 0 )
	{
		vel_mod *= 1.2f;
	}

	if( state == STA_RUN )
	{
		vel_mod *= 1.2f;
	}

	return vel_mod;
}

void cPlayer :: Action_Jump( bool enemy_jump /* = 0 */ )
{
	if( ducked )
	{
		return;
	}

	// enemy jump
	if( enemy_jump )
	{
		start_enemyjump = 1;
	}
	// default jump
	else
	{
		// start keytime
		Start_jump_keytime();
	}

	// check if starting a jump is possible
	Update_jump_keytime();
}

void cPlayer :: Action_Interact( SDLKey key_type )
{
	// Up
	if( key_type == pPreferences->key_up )
	{
		// Search for colliding level exit Objects
		for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			// levelexit
			if( obj->type == TYPE_LEVELEXIT && Col_Box( &col_rect, &obj->col_rect ) )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(obj);

				// beam
				if( level_exit->exit_type == LEVELEXIT_BEAM )
				{
					// needs to be on ground
					if( ground_object )
					{
						Game_Action = GA_ACTIVATE_LEVEL_EXIT;
						Game_Action_ptr = level_exit;
						return;
					}
				}
				// warp
				else if( level_exit->exit_type == LEVELEXIT_WARP )
				{
					if( level_exit->direction == DIR_UP )
					{
						if( vely <= 0 )
						{
							Game_Action = GA_ACTIVATE_LEVEL_EXIT;
							Game_Action_ptr = level_exit;
							return;
						}
					}
				}
			}
		}
	}
	// Down
	else if( key_type == pPreferences->key_down )
	{
		// Search for colliding Levelexit Objects
		for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			// levelexit
			if( obj->type == TYPE_LEVELEXIT && Col_Box( &col_rect, &obj->col_rect ) )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(obj);

				// warp
				if( level_exit->exit_type == LEVELEXIT_WARP )
				{
					if( level_exit->direction == DIR_DOWN )
					{
						// needs to be on ground
						if( ground_object )
						{
							Game_Action = GA_ACTIVATE_LEVEL_EXIT;
							Game_Action_ptr = level_exit;
							return;
						}
					}
				}
			}
		}

		// ducking / falling
		if( state != STA_FLY )
		{
			if( ground_object )
			{
				if( ground_object->massivetype == MASS_MASSIVE )
				{
					Start_ducking();
				}
				else if( ground_object->massivetype == MASS_HALFMASSIVE )
				{
					Start_falling();
				}
			}
		}
	}
	// Left
	else if( key_type == pPreferences->key_left )
	{
		// Search for colliding Levelexit Objects
		for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			// levelexit
			if( obj->type == TYPE_LEVELEXIT && Col_Box( &col_rect, &obj->col_rect ) )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(obj);

				// warp
				if( level_exit->exit_type == LEVELEXIT_WARP )
				{
					if( level_exit->direction == DIR_LEFT )
					{
						if( velx >= 0 )
						{
							Game_Action = GA_ACTIVATE_LEVEL_EXIT;
							Game_Action_ptr = level_exit;
							return;
						}
					}
				}
			}
		}

		// direction
		if( state != STA_FLY )
		{
			if( direction != DIR_LEFT )
			{
				// play stop sound if already running
				if( velx > 12 && ground_object )
				{
					pAudio->PlaySound( "player/maryo_stop.ogg", RID_MARYO_STOP );
				}

				direction = DIR_LEFT;
			}
		}
	}
	// Right
	else if( key_type == pPreferences->key_right )
	{
		// Search for colliding Levelexit Objects
		for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			cSprite *obj = (*itr);

			// levelexit
			if( obj->type == TYPE_LEVELEXIT && Col_Box( &col_rect, &obj->col_rect ) )
			{
				cLevel_Exit *level_exit = static_cast<cLevel_Exit *>(obj);

				// warp
				if( level_exit->exit_type == LEVELEXIT_WARP )
				{
					if( level_exit->direction == DIR_RIGHT )
					{
						if( velx <= 0 )
						{
							Game_Action = GA_ACTIVATE_LEVEL_EXIT;
							Game_Action_ptr = level_exit;
							return;
						}
					}
				}
			}
		}

		// direction
		if( state != STA_FLY )
		{
			if( direction != DIR_RIGHT )
			{
				// play stop sound if already running
				if( velx < -12 && ground_object )
				{
					pAudio->PlaySound( "player/maryo_stop.ogg", RID_MARYO_STOP );
				}

				direction = DIR_RIGHT;
			}
		}
	}
}

void cPlayer :: Action_Shoot( void )
{
	// add fire or ice-ball
	ball_effect ball_type = FIREBALL_DEFAULT;
	// if ice maryo
	if( maryo_type == MARYO_ICE )
	{
		ball_type = ICEBALL_DEFAULT;
	}
	// if added ball
	if( Ball_add( ball_type ) )
	{
		throwing_counter = DESIRED_FPS * 0.3f;
	}
}

void cPlayer :: Action_Stop_Jump( void )
{
	Stop_flying();
	UpKeytime = 0;
}

void cPlayer :: Action_Stop_Interact( SDLKey key_type )
{
	if( key_type == pPreferences->key_action )
	{
		Release_item();
	}
}

void cPlayer :: Action_Stop_Shoot( void )
{
	// nothing
}

bool cPlayer :: Ball_add( ball_effect effect_type /* = FIREBALL_DEFAULT */ )
{
	if( ( maryo_type != MARYO_FIRE && maryo_type != MARYO_ICE ) || ducked )
	{
		return 0;
	}

	// default fire/ice-ball
	if( effect_type == FIREBALL_DEFAULT || effect_type == ICEBALL_DEFAULT )
	{
		// if maximum allowed balls
		if( ( effect_type == FIREBALL_DEFAULT && fireball_count >= 2 ) || ( effect_type == ICEBALL_DEFAULT && iceball_count >= 2 ) )
		{
			return 0;
		}

		// horizontal speed
		float speed = 17;

		// iceball is slower
		if( effect_type == ICEBALL_DEFAULT )
		{
			speed = 12;
		}
		// add ball
		cBall *ball = new cBall( (direction == DIR_RIGHT) ? (posx + 20) : (posx + 8), posy + 45, (direction == DIR_RIGHT) ? (speed) : (-speed), 0, this, effect_type );
		pLevel->pSprite_Manager->Add( ball );

		// play sound
		if( effect_type == FIREBALL_DEFAULT )
		{
			pAudio->PlaySound( "item/fireball.ogg", RID_MARYO_BALL );
		}
		else
		{
			pAudio->PlaySound( "item/iceball.ogg", RID_MARYO_BALL );
		}

	}
	// half circle fire/ice-ball explosion
	else if( effect_type == FIREBALL_EXPLOSION || effect_type == ICEBALL_EXPLOSION )
	{
		float ball_angle = -180;

		for( unsigned int i = 0; i < 10; i++ )
		{
			ball_angle += 18; // 10 * 18 = 180 degrees

			// add ball
			cBall *ball = new cBall( posx + col_rect.w / 2, posy + col_rect.h / 2, 0, 0, this, effect_type );
			ball->Set_Direction( ball_angle, 15 );
			ball->Col_Move( ball->velx * 2, ball->vely * 2, 1 );
			pLevel->pSprite_Manager->Add( ball );
		}

		// explosion animation and sound
		if( effect_type == FIREBALL_EXPLOSION )
		{
			cFireAnimation *anim = new cFireAnimation( posx + ( col_rect.w / 2 ), posy + ( col_rect.h / 3 ), 10 );
			anim->Set_FadingSpeed( 0.3f );
			pAnimationManager->Add( anim );

			pAudio->PlaySound( "item/fireball_2.ogg", RID_MARYO_BALL );
		}
		else
		{
			// create animation
			cParticleAnimation *anim = new cParticleAnimation( posx + col_rect.w / 2, posy + col_rect.h / 2 );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			anim->Set_Quota( 10 );
			anim->Set_Time_to_Live( 1.5f );
			anim->Set_ZPos( posz + 0.0001f );
			anim->Set_Color( Color( static_cast<Uint8>(50), 50, 250 ) );
			anim->Set_Blending( BLEND_ADD );
			anim->Set_Speed( 0.8f, 0.7f );
			anim->Set_Scale( 0.4f, 0.2f );
			// add animation
			pAnimationManager->Add( anim );

			pAudio->PlaySound( "item/iceball_2.ogg", RID_MARYO_BALL );
		}
	}
	// unknown type
	else
	{
		return 0;
	}

	return 1;
}

void cPlayer :: Ball_clear( void )
{
	// destroy all fireballs from the player
	for( SpriteList::iterator itr = pLevel->pSprite_Manager->objects.begin(), itr_end = pLevel->pSprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		if( obj->type == TYPE_BALL )
		{
			cBall *ball = static_cast<cBall *>(obj);

			// if from player
			if( ball->origin->type == TYPE_PLAYER )
			{
				obj->Destroy();
			}
		}
	}
}

void cPlayer :: Add_Kill_Multiplier( void )
{
	kill_multiplier += 0.1f;

	// give lives after 10 continuous kills
	if( kill_multiplier >= 2 )
	{
		// only every fifth kill
		if( static_cast<int>( kill_multiplier * 10 ) % 5 == 0 )
		{
			// add 1 live
			golddisplay->AddGold( 100 );
		}
	}

	last_kill_counter = 0;
}

void cPlayer :: Update_Kill_Multiplier( void )
{
	last_kill_counter += pFramerate->speedfactor;

	if( kill_multiplier > 1 )
	{
		// if longer than a second reset the multiplier
		if( last_kill_counter > DESIRED_FPS )
		{
			kill_multiplier = 1;
		}
	}
}

unsigned int cPlayer :: Validate_Collision( cSprite *obj )
{
	// don't collide with anything if dead
	if( maryo_type == MARYO_DEAD )
	{
		return 0;
	}

	if( obj->massivetype == MASS_MASSIVE )
	{
		if( obj->sprite_array == ARRAY_ENEMY )
		{
			// if invincible
			if( invincible )
			{
				// jpiranha
				if( obj->type == TYPE_JPIRANHA )
				{
					if( invincible_star > 0 )
					{
						return 1;
					}

					return 0;
				}
				// gee
				else if( obj->type == TYPE_GEE )
				{
					if( invincible_star > 0 )
					{
						return 1;
					}

					// block if on top
					if( vely >= 0 && is_onTop( obj ) )
					{
						return 2;
					}

					return 0;
				}
				// walk on spika and rokko
				else if( obj->type == TYPE_SPIKA || obj->type == TYPE_ROKKO )
				{
					// block if on top
					if( vely >= 0 && is_onTop( obj ) )
					{
						return 2;
					}

					if( invincible_star > 0 )
					{
						return 1;
					}

					return 0;
				}
				// eato
				else if( obj->type == TYPE_EATO )
				{
					// block if on top
					if( vely >= 0 && is_onTop( obj ) )
					{
						return 2;
					}

					if( invincible_star > 0 )
					{
						return 1;
					}
				}
				// turtle
				else if( obj->type == TYPE_TURTLE )
				{
					// block if on top
					if( vely >= 0 && is_onTop( obj ) )
					{
						return 2;
					}

					if( invincible_star > 0 )
					{
						return 1;
					}
				}
			}

			// turtle
			if( obj->type == TYPE_TURTLE )
			{
				cTurtle *turtle = static_cast<cTurtle *>(obj);

				// if not on top
				if( !( vely >= 0 && is_onTop( obj ) ) )
				{
					// if player counter active or invincible
					if( turtle->playercounter || invincible )
					{
						return 0;
					}
				}
			}
		}
		else if( obj->type == TYPE_BALL )
		{
			return 0;
		}
		else if( obj->type == TYPE_BONUSBOX || obj->type == TYPE_SPINBOX )
		{
			cBaseBox *box = static_cast<cBaseBox *>(obj);

			// ghost
			if( box->box_invisible == 2 )
			{
				// maryo is not ghost
				if( maryo_type != MARYO_GHOST )
				{
					return 0;
				}
			}
		}

		// don't collide with ground
		if( ground_object == obj )
		{
			return 0;
		}

		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// fall through
		if( pKeyboard->keys[pPreferences->key_down] )
		{
			return 0;
		}

		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			// don't collide with ground
			if( ground_object == obj )
			{
				return 0;
			}

			return 2;
		}

		return 0;
	}
	// allow climbing
	if( obj->massivetype == MASS_CLIMBABLE )
	{
		// if not climbing and player wants to climb
		if( state != STA_CLIMB && state != STA_FLY && ( ( pKeyboard->keys[pPreferences->key_up] || pJoystick->up ) || ( ( pKeyboard->keys[pPreferences->key_down] || pJoystick->down ) && !ground_object ) ) )
		{
			GL_rect climb_rect = obj->col_rect;
			climb_rect.x += climb_rect.w * 0.25f;
			climb_rect.y += climb_rect.h * 0.25f;
			climb_rect.w *= 0.5f;
			climb_rect.h *= 0.5f;

			// check if core collision
			if( Col_Box( &climb_rect, &col_rect ) )
			{
				// start climbing
				Start_climbing();
			}
		}

		return 1;
	}
	// warp levelexit key check
	if( obj->type == TYPE_LEVELEXIT )
	{
		cLevel_Exit *levelexit = static_cast<cLevel_Exit *>(obj);

		if( levelexit->exit_type == LEVELEXIT_WARP )
		{
			if( pKeyboard->keys[pPreferences->key_up] && levelexit->start_direction == DIR_UP )
			{
				Action_Interact( pPreferences->key_up );
			}
			else if( pKeyboard->keys[pPreferences->key_down] && levelexit->start_direction == DIR_DOWN )
			{
				Action_Interact( pPreferences->key_down );
			}
			else if( pKeyboard->keys[pPreferences->key_right] && levelexit->start_direction == DIR_RIGHT )
			{
				Action_Interact( pPreferences->key_right );
			}
			else if( pKeyboard->keys[pPreferences->key_left] && levelexit->start_direction == DIR_LEFT )
			{
				Action_Interact( pPreferences->key_left );
			}
		}

		return 0;
	}

	return 0;
}

void cPlayer :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// if invalid
	if( collision->direction == DIR_UNDEFINED || maryo_type == MARYO_DEAD || !visible )
	{
		return;
	}

	cEnemy *enemy = static_cast<cEnemy *>(pLevel->pSprite_Manager->Get_Pointer( collision->number ));

	// if enemy already dead
	if( enemy->dead )
	{
		return;
	}

	// if invincible
	if( invincible_star > 0 )
	{
		bool hit_enemy = 1;

		// spika and eato ignore top collisions
		if( ( enemy->type == TYPE_SPIKA || enemy->type == TYPE_EATO || enemy->type == TYPE_THROMP ) && collision->direction == DIR_DOWN )
		{
			hit_enemy = 0;
		}
		// rokko
		else if( enemy->type == TYPE_ROKKO )
		{
			hit_enemy = 0;
		}
		// turtle boss
		else if( enemy->type == TYPE_TURTLE_BOSS )
		{
			hit_enemy = 0;
		}

		// hit
		if( hit_enemy )
		{
			pAudio->PlaySound( "item/star_kill.ogg" );
			pointsdisplay->Add_Points( static_cast<unsigned int>( enemy->kill_points * 1.2f ), posx, posy, "", yellow, 1 );
			// force complete downgrade
			enemy->DownGrade( 1 );
			Add_Kill_Multiplier();
			return;
		}
		// no hit
		else
		{
			// eato blocks
			if( enemy->type == TYPE_EATO )
			{
				if( collision->direction == DIR_LEFT )
				{
					velx = 0;
				}
				else if( collision->direction == DIR_RIGHT )
				{
					velx = 0;
				}
			}
		}
	}

	// enemy is frozen
	if( enemy->freeze_counter )
	{
		// enemy rect particle ice animation
		for( unsigned int w = 0; w < enemy->col_rect.w; w += 10 )
		{
			for( unsigned int h = 0; h < enemy->col_rect.h; h += 10 )
			{
				// animation
				cParticleAnimation *anim = new cParticleAnimation( enemy->posx + w, enemy->posy + h );

				anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
				anim->Set_Time_to_Live( 0.6f, 0.4f );
				anim->Set_Color( Color( static_cast<Uint8>(160), 160, 240 ), Color( static_cast<Uint8>( rand() % 80 ), rand() % 80, rand() % 10 ) );
				anim->Set_Fading_Alpha( 1 );
				anim->Set_Fading_Size( 1 );
				anim->Set_Speed( 0.5f, 0.2f );
				anim->Set_Blending( BLEND_DRIVE );
				// add animation
				pAnimationManager->Add( anim );
			}
		}

		// ice sound
		pAudio->PlaySound( "item/ice_kill.ogg" );

		// get points
		pointsdisplay->Add_Points( enemy->kill_points, enemy->posx, enemy->posy, "", static_cast<Uint8>(255), 1 );
		
		// kill enemy
		enemy->DownGrade( 1 );
		Add_Kill_Multiplier();

		// slow down
		velx *= 0.7f;
		vely *= 0.7f;
		return;
	}

	if( collision->direction == DIR_TOP )
	{
		vely = 0;

		if( state != STA_FLY )
		{
			if( state != STA_CLIMB )
			{
				Set_MovingState( STA_FALL );
				UpKeytime = 0;
				jump_power = 0;
			}
		}
	}

	// send collision
	Send_Collision( collision );
}

void cPlayer :: Handle_Collision_Massive( cObjectCollision *collision )
{
	// if invalid
	if( collision->direction == DIR_UNDEFINED )
	{
		return;
	}

	cSprite *col_obj = pLevel->pSprite_Manager->Get_Pointer( collision->number );

	// ignore climbable
	if( col_obj->massivetype == MASS_CLIMBABLE )
	{
		return;
	}

	if( collision->direction == DIR_UP )
	{
		vely = 0;

		if( state != STA_FLY )
		{
			if( state != STA_CLIMB )
			{
				Set_MovingState( STA_FALL );
				UpKeytime = 0;
				jump_power = 0;

				cMovingSprite *moving_object = dynamic_cast<cMovingSprite *>(col_obj);

				// if valid moving sprite
				if( moving_object )
				{
					if( moving_object->vely > 0 )
					{
						vely += moving_object->vely;
					}
				}

				if( collision->type == CO_MASSIVE )
				{
					pAudio->PlaySound( "tock.ogg", RID_MARYO_TOCK );
				}
			}
		}
		// flying
		else
		{
			velx -= velx * 0.05f * pFramerate->speedfactor;

			// too slow
			if( velx > -5 && velx < 5 )
			{
				Stop_flying();
			}
		}
	}
	else if( collision->direction == DIR_DOWN )
	{
		vely = 0;

		// flying
		if( state == STA_FLY )
		{
			velx -= velx * 0.05f * pFramerate->speedfactor;

			// too slow
			if( velx > -5 && velx < 5 )
			{
				Stop_flying();
			}
		}
	}
	else if( collision->direction == DIR_LEFT )
	{
		velx = 0;
		vely -= vely * 0.01f * pFramerate->speedfactor;

		// flying box collision
		if( ( col_obj->type == TYPE_BONUSBOX || col_obj->type == TYPE_SPINBOX ) && state == STA_FLY )
		{
			cBaseBox *box = static_cast<cBaseBox *>(col_obj);
			box->Activate_collision( collision->direction );
		}

		Stop_flying();
	}
	else if( collision->direction == DIR_RIGHT )
	{
		velx = 0;
		vely -= vely * 0.01f * pFramerate->speedfactor;

		// flying box collision
		if( ( col_obj->type == TYPE_BONUSBOX || col_obj->type == TYPE_SPINBOX ) && state == STA_FLY )
		{
			cBaseBox *box = static_cast<cBaseBox *>(col_obj);
			box->Activate_collision( collision->direction );
		}

		Stop_flying();
	}

	if( collision->type == CO_ACTIVE )
	{
		// send collision
		Send_Collision( collision );
	}
}

void cPlayer :: Handle_OutofLevel( ObjectDirection dir )
{
	if( dir == DIR_LEFT )
	{
		Set_PosX( pCamera->limit_rect.x - col_pos.x );
		velx = 0;
		Stop_flying();
	}
	else if( dir == DIR_RIGHT )
	{
		Set_PosX( pCamera->limit_rect.x + pCamera->limit_rect.w - col_pos.x - col_rect.w - 0.01f );
		velx = 0;
		Stop_flying();
	}
	else if( dir == DIR_TOP )
	{
		/*if( pLevel->limit_top_blocks )
		{
			Set_PosY( pCamera->limit_rect.y + pCamera->limit_rect.h + 0.01f );
			vely = 0;
		}*/
	}
	else if( dir == DIR_BOTTOM )
	{
		if( godmode )
		{
			vely = -35;
		}
		else
		{
			/*if( pLevel->limit_bottom_blocks )
			{
				Set_PosY( pCamera->limit_rect.y + GAME_RES_H - col_pos.x - col_rect.w + 0.01f );
				vely = 0;
			}
			// falling below ground
			else
			{*/
				invincible = 0;
				DownGrade( 1, 1 );
			//}
		}
	}
}

void cPlayer :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// direction
	Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_player_direction" ));
	combobox->getEditbox()->setTooltipText( "Direction" );
	Editor_Add( combobox, 100, 75 );

	combobox->addItem( new ListboxTextItem( "right" ) );
	combobox->addItem( new ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_name( start_direction ) );

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cPlayer::Editor_Direction_Select, this ) );

	// set position
	Editor_pos_update();
}

bool cPlayer :: Editor_Direction_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_id( item->getText().c_str() ), 1 );

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayer *pPlayer = NULL;
