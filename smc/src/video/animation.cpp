/***************************************************************************
 * animation.cpp  -  Animation and Particle classes
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

#include "../video/animation.h"
#include "../core/framerate.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

cAnimation :: cAnimation( float x /* = 0 */, float y /* = 0 */ )
: cImageObjectSprite( x, y )
{
	sprite_array = ARRAY_ANIM;
	type = TYPE_ACTIVESPRITE;
	massivetype = MASS_PASSIVE;
	spawned = 1;

	posz = 0.07000f;
	posz_rand = 0;
	time_to_live = 0;
	time_to_live_rand = 0;

	fading_speed = 1;
	animtype = ANIM_UNDEFINED;
}

cAnimation :: ~cAnimation( void )
{
	//
}

void cAnimation :: Init_Anim( void )
{
	// virtual
}

void cAnimation :: Update( void )
{
	// virtual
}

void cAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
}


void cAnimation :: Set_Time_to_Live( float time, float time_rand /* = 0 */ )
{
	time_to_live = time;
	time_to_live_rand = time_rand;
}

void cAnimation :: Set_Fading_Speed( float speed )
{
	fading_speed = speed;

	if( fading_speed <= 0 )
	{
		fading_speed = 0.1f;
	}
}

void cAnimation :: Set_Pos_Z( float pos, float pos_rand /* = 0 */ )
{
	posz = pos;
	posz_rand = pos_rand;
}

/* *** *** *** *** *** *** *** cAnimation_Goldpiece *** *** *** *** *** *** *** *** *** *** */

cAnimation_Goldpiece :: cAnimation_Goldpiece( float posx, float posy, float height /* = 40 */, float width /* = 20 */ )
: cAnimation( posx, posy )
{
	animtype = BLINKING_POINTS;

	images.push_back( pVideo->Get_Surface( "animation/light_1/1.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/light_1/2.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/light_1/3.png" ) );

	for( unsigned int i = 0; i < 4; i++ )
	{
		cSprite *obj = new cSprite( NULL, Get_Random_Float( 0, width ), Get_Random_Float( 0, height ) );
		objects.push_back( obj );
	}
}

cAnimation_Goldpiece :: ~cAnimation_Goldpiece( void )
{
	// clear
	for( BlinkPointList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();
}

void cAnimation_Goldpiece :: Update( void )
{
	time_to_live += pFramerate->speedfactor * fading_speed;

	// update the fixed points
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		switch( i ) 
		{
		case 0:
		{
			if( time_to_live < 3 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 6 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 9 )
			{
				Set_Image( 1 );
			}
			else if( time_to_live < 12 )
			{
				Set_Image( 0 );
			}
			break;
		}
		case 1:
		{
			if( time_to_live < 3 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 6 )
			{
				Set_Image( 1 );
			}
			else if( time_to_live < 9 )
			{
				Set_Image( 2 );
			}
			else if( time_to_live < 12 )
			{
				Set_Image( 1 );
			}
			break;
		}
		case 2:
		{
			if( time_to_live < 3 )
			{
				Set_Image( 1 );
			}
			else if( time_to_live < 6 )
			{
				Set_Image( 2 );
			}
			else if( time_to_live < 9 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 12 )
			{
				Set_Image( -1 );
			}
			break;			
		}
		case 3:
		{
			if( time_to_live < 3 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 6 )
			{
				Set_Image( 1 );
			}
			else if( time_to_live < 9 )
			{
				Set_Image( 0 );
			}
			else if( time_to_live < 12 )
			{
				Set_Image( 0 );
			}
			break;
		}
		default:
		{
			break;
		}
		}

		objects[i]->Set_Scale( 1.1f - ( time_to_live / 12 ) );
	}
}

void cAnimation_Goldpiece :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	if( !image )
	{
		Set_Visible( 0 );
		return;
	}

	// draw the fixed points
	for( BlinkPointList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// create request
		cSurfaceRequest *request = new cSurfaceRequest();
		image->Blit( obj->posx - ( pActive_Camera->x - posx ), obj->posy - ( pActive_Camera->y - posy ), posz, request );

		// scale
		request->scale_x = obj->scalex;
		request->scale_y = obj->scaley;

		// color
		request->color = color;

		// add request
		pRenderer->Add( request );
	}
	
	if( time_to_live > 11 || time_to_live < 0 )
	{
		Set_Visible( 0 );
	}
}

/* *** *** *** *** *** *** *** cAnimation_Fireball *** *** *** *** *** *** *** *** *** *** */

cAnimation_Fireball :: cAnimation_Fireball( float posx, float posy, unsigned int power /* = 5 */ )
: cAnimation( posx, posy )
{
	animtype = FIRE_EXPLOSION;

	// create objects
	for( unsigned int i = 0; i < power; i++ )
	{
		cAnimation_Fireball_Item *obj = new cAnimation_Fireball_Item();
		objects.push_back( obj );
	}
}

cAnimation_Fireball :: ~cAnimation_Fireball( void )
{
	// clear
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();
}

void cAnimation_Fireball :: Init_Anim( void )
{
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cAnimation_Fireball_Item *obj = (*itr);

		// images
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/4.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/3.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/2.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/1.png" ) );
		obj->Set_Image( 0 );

		// velocity
		obj->velx = Get_Random_Float( -2.5f, 5 );
		obj->vely = Get_Random_Float( -2.5f, 5 );

		// Z position
		obj->posz = posz;
		if( posz_rand > 0 )
		{
			obj->posz += Get_Random_Float( 0, posz_rand );
		}

		// lifetime
		obj->counter = Get_Random_Float( 8, 13 );
	}
}

void cAnimation_Fireball :: Update( void )
{
	if( !visible )
	{
		return;
	}

	time_to_live += pFramerate->speedfactor * fading_speed;

	// update objects
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cAnimation_Fireball_Item *obj = (*itr);

		if( obj->counter > 8 )
		{
			obj->Set_Image( 0 );
		}
		else if( obj->counter > 5 )
		{
			obj->Set_Image( 1 );
		}
		else if( obj->counter > 3 )
		{
			obj->Set_Image( 2 );
		}
		else
		{
			obj->Set_Image( 3 );
		}

		obj->counter -= pFramerate->speedfactor * fading_speed;

		obj->Set_Scale( obj->counter / 10 );
		obj->Move( obj->velx, obj->vely );
	}

	if( time_to_live > 12 || time_to_live < 0 )
	{
		Set_Visible( 0 );
	}
}

void cAnimation_Fireball :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	// draw objects
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cAnimation_Fireball_Item *obj = (*itr);

		// create request
		cSurfaceRequest *request = new cSurfaceRequest();
		obj->image->Blit( obj->posx - ( pActive_Camera->x - posx ), obj->posy - ( pActive_Camera->y - posy ), obj->posz, request );

		// scale
		request->scale_x = obj->scalex;
		request->scale_y = obj->scaley;

		// color
		request->color = color;

		// add request
		pRenderer->Add( request );
	}
}

/* *** *** *** *** *** *** *** cParticle *** *** *** *** *** *** *** *** *** *** */

cParticle :: cParticle( void )
: cMovingSprite()
{
	// scale centered
	Set_Scale_Directions( 1, 1, 1, 1 );
	time_to_live = 0;
	const_rotx = 0;
	const_roty = 0;
	const_rotz = 0;
	gravity_x = 0;
	gravity_y = 0;

	fade_pos = 1;
}

cParticle :: ~cParticle( void )
{

}

void cParticle :: Update( void )
{
	// update fade modifier
	fade_pos -= ( pFramerate->speedfactor * ( static_cast<float>(speedfactor_fps) * 0.001f ) ) / time_to_live;

	// finished fading
	if( fade_pos <= 0 )
	{
		fade_pos = 0;
		Set_Visible( 0 );
		return;
	}

	// move
	Move( velx, vely );
	// add gravity velocity
	Add_Velocity( gravity_x, gravity_y );

	// constant rotation
	if( const_rotx != 0 )
	{
		Add_Rotation_X( const_rotx * pFramerate->speedfactor );
	}
	if( const_roty != 0 )
	{
		Add_Rotation_Y( const_roty * pFramerate->speedfactor );
	}
	if( const_rotz != 0 )
	{
		Add_Rotation_Z( const_rotz * pFramerate->speedfactor );
	}
}

void cParticle :: Draw( cParticle_Emitter *origin )
{
	if( !image )
	{
		return;
	}

	// create request
	cSurfaceRequest *request = new cSurfaceRequest();

	image->Blit( posx - pActive_Camera->x, posy - pActive_Camera->y, posz, request );

	// blending
	if( origin->blending == BLEND_ADD )
	{
		request->blend_sfactor = GL_SRC_ALPHA;
		request->blend_dfactor = GL_ONE;
	}
	else if( origin->blending == BLEND_DRIVE )
	{
		request->blend_sfactor = GL_SRC_COLOR;
		request->blend_dfactor = GL_DST_ALPHA;
	}

	// rotation
	request->rotx += rotx;
	request->roty += roty;
	request->rotz += rotz;

	// scale
	request->scale_x = scalex;
	request->scale_y = scaley;

	// size fading
	if( origin->fade_size )
	{
		request->scale_x *= fade_pos;
		request->scale_y *= fade_pos;
	}

	// color
	request->color = color;

	// color fading
	if( origin->fade_color )
	{
		request->color.red = static_cast<Uint8>(request->color.red * fade_pos);
		request->color.green = static_cast<Uint8>(request->color.green * fade_pos);
		request->color.blue = static_cast<Uint8>(request->color.blue * fade_pos);
	}
	// alpha fading
	if( origin->fade_alpha )
	{
		request->color.alpha = static_cast<Uint8>(request->color.alpha * fade_pos);
	}

	// add request
	pRenderer->Add( request );
}

void cParticle :: Set_Gravity( float x, float y)
{
	gravity_x = x;
	gravity_y = y;
}

/* *** *** *** *** *** *** *** cParticle_Emitter *** *** *** *** *** *** *** *** *** *** */

cParticle_Emitter :: cParticle_Emitter( void )
: cAnimation()
{
	Init();
}

cParticle_Emitter :: cParticle_Emitter( CEGUI::XMLAttributes &attributes )
: cAnimation()
{
	Init();
	Create_from_Stream( attributes );
}

cParticle_Emitter :: ~cParticle_Emitter( void )
{
	Clear();
}

void cParticle_Emitter :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// filename
	Set_Filename( attributes.getValueAsString( "file" ).c_str() );
	// position z
	Set_Pos_Z( attributes.getValueAsFloat( "pos_z", posz ), attributes.getValueAsFloat( "pos_z_rand", posz_rand ) );
	// emitter rect
	Set_Emitter_Rect( static_cast<float>(attributes.getValueAsInteger( "pos_x", static_cast<int>(posx) )), static_cast<float>(attributes.getValueAsInteger( "pos_y", static_cast<int>(posy) )), static_cast<float>(attributes.getValueAsInteger( "size_x", static_cast<int>(start_rect.w) )), static_cast<float>(attributes.getValueAsInteger( "size_y", static_cast<int>(start_rect.h) )) );
	// emitter time to live
	Set_Emitter_Time_to_Live( attributes.getValueAsFloat( "emitter_time_to_live", emitter_time_to_live ) );
	// emitter interval
	Set_Emitter_Iteration_Interval( attributes.getValueAsFloat( "emitter_interval", emitter_iteration_interval ) );
	// quota/count
	Set_Quota( attributes.getValueAsInteger( "quota", emitter_quota ) );
	// time for particle to live
	Set_Time_to_Live( attributes.getValueAsFloat( "time_to_live", time_to_live ), attributes.getValueAsFloat( "time_to_live_rand", time_to_live_rand ) );
	// velocity
	Set_Speed( attributes.getValueAsFloat( "vel", vel ), attributes.getValueAsFloat( "vel_rand", vel_rand ) );
	// start rotation
	Set_Rotation( attributes.getValueAsFloat( "rot_x", start_rotx ), attributes.getValueAsFloat( "rot_y", start_roty ), attributes.getValueAsFloat( "rot_z", start_rotz ), 1 );
	Set_Start_Rot_Z_Uses_Direction( attributes.getValueAsBool( "start_rot_z_uses_direction", start_rot_z_uses_direction ) );
	// constant rotation x
	Set_Const_Rotation_X( attributes.getValueAsFloat( "const_rot_x", const_rotx ), attributes.getValueAsFloat( "const_rot_x_rand", const_rotx_rand ) );
	// constant rotation y
	Set_Const_Rotation_Y( attributes.getValueAsFloat( "const_rot_y", const_roty ), attributes.getValueAsFloat( "const_rot_y_rand", const_roty_rand ) );
	// constant rotation z
	Set_Const_Rotation_Z( attributes.getValueAsFloat( "const_rot_z", const_rotz ), attributes.getValueAsFloat( "const_rot_z_rand", const_rotz_rand ) );
	// angle
	Set_Direction_Range( attributes.getValueAsFloat( "angle_start", angle_start ), attributes.getValueAsFloat( "angle_range", angle_range ) );
	// scale
	Set_Scale( attributes.getValueAsFloat( "size_scale", size_scale ), attributes.getValueAsFloat( "size_scale_rand", size_scale_rand ) );
	// horizontal gravity
	Set_Horizontal_Gravity( attributes.getValueAsFloat( "gravity_x", gravity_x ), attributes.getValueAsFloat( "gravity_x_rand", gravity_x_rand )  );
	// vertical gravity
	Set_Vertical_Gravity( attributes.getValueAsFloat( "gravity_y", gravity_y ), attributes.getValueAsFloat( "gravity_y_rand", gravity_y_rand ) );
}

void cParticle_Emitter :: Save_to_Stream( ofstream &file )
{
	file << "\t<particle_emitter>" << std::endl;

	// filename
	file << "\t\t<Property name=\"file\" value=\"" << filename.c_str() << "\" />" << std::endl;
	// position z
	file << "\t\t<Property name=\"pos_z\" value=\"" << posz << "\" />" << std::endl;
	file << "\t\t<Property name=\"pos_z_rand\" value=\"" << posz_rand << "\" />" << std::endl;
	// emitter rect
	file << "\t\t<Property name=\"pos_x\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"pos_y\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	file << "\t\t<Property name=\"size_x\" value=\"" << static_cast<int>(start_rect.w) << "\" />" << std::endl;
	file << "\t\t<Property name=\"size_y\" value=\"" << static_cast<int>(start_rect.h) << "\" />" << std::endl;
	// emitter time to live
	file << "\t\t<Property name=\"emitter_time_to_live\" value=\"" << emitter_time_to_live << "\" />" << std::endl;
	// emitter interval
	file << "\t\t<Property name=\"emitter_interval\" value=\"" << emitter_iteration_interval << "\" />" << std::endl;
	// quota/count
	file << "\t\t<Property name=\"quota\" value=\"" << emitter_quota << "\" />" << std::endl;
	// time to live
	file << "\t\t<Property name=\"time_to_live\" value=\"" << time_to_live << "\" />" << std::endl;
	file << "\t\t<Property name=\"time_to_live_rand\" value=\"" << time_to_live_rand << "\" />" << std::endl;
	// velocity
	file << "\t\t<Property name=\"vel\" value=\"" << vel << "\" />" << std::endl;
	file << "\t\t<Property name=\"vel_rand\" value=\"" << vel_rand << "\" />" << std::endl;
	// start rotation
	file << "\t\t<Property name=\"rot_x\" value=\"" << start_rotx << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_y\" value=\"" << start_roty << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_z\" value=\"" << start_rotz << "\" />" << std::endl;
	file << "\t\t<Property name=\"start_rot_z_uses_direction\" value=\"" << start_rot_z_uses_direction << "\" />" << std::endl;
	// constant rotation x
	file << "\t\t<Property name=\"const_rot_x\" value=\"" << const_rotx << "\" />" << std::endl;
	file << "\t\t<Property name=\"const_rot_x_rand\" value=\"" << const_rotx_rand << "\" />" << std::endl;
	// constant rotation y
	file << "\t\t<Property name=\"const_rot_y\" value=\"" << const_roty << "\" />" << std::endl;
	file << "\t\t<Property name=\"const_rot_y_rand\" value=\"" << const_roty_rand << "\" />" << std::endl;
	// constant rotation z
	file << "\t\t<Property name=\"const_rot_z\" value=\"" << const_rotz << "\" />" << std::endl;
	file << "\t\t<Property name=\"const_rot_z_rand\" value=\"" << const_rotz_rand << "\" />" << std::endl;
	// angle
	file << "\t\t<Property name=\"angle_start\" value=\"" << angle_start << "\" />" << std::endl;
	file << "\t\t<Property name=\"angle_range\" value=\"" << angle_range << "\" />" << std::endl;
	// scale
	file << "\t\t<Property name=\"size_scale\" value=\"" << size_scale << "\" />" << std::endl;
	file << "\t\t<Property name=\"size_scale_rand\" value=\"" << size_scale_rand << "\" />" << std::endl;
	// horizontal gravity
	file << "\t\t<Property name=\"gravity_x\" value=\"" << gravity_x << "\" />" << std::endl;
	file << "\t\t<Property name=\"gravity_x_rand\" value=\"" << gravity_x_rand << "\" />" << std::endl;
	// vertical gravity
	file << "\t\t<Property name=\"gravity_y\" value=\"" << gravity_y << "\" />" << std::endl;
	file << "\t\t<Property name=\"gravity_y_rand\" value=\"" << gravity_y_rand << "\" />" << std::endl;

	file << "\t</particle_emitter>" << std::endl;
}

void cParticle_Emitter :: Init( void )
{
	animtype = PARTICLE_EXPLOSION;
	editor_posz = 0.111f;
	sprite_array = ARRAY_ANIM;
	type = TYPE_ANIMATION;
	name = "Particle Emitter";

	rect.w = 0;
	rect.h = 0;
	col_rect.w = rect.w;
	col_rect.h = rect.h;
	start_rect.w = rect.w;
	start_rect.h = rect.h;

	// 0 = 1 emit
	emitter_time_to_live = 0;
	emitter_iteration_interval = 0.2f;
	emitter_quota = 1;

	// velocity
	vel = 2;
	vel_rand = 2;
	// rotation
	start_rot_z_uses_direction = 0;
	const_rotx = 0;
	const_roty = 0;
	const_rotz = 0;
	const_rotx_rand = 0;
	const_roty_rand = 0;
	const_rotz_rand = 0;
	// angle
	angle_start = 0;
	angle_range = 360;
	// scale
	size_scale = 1;
	size_scale_rand = 0;
	// gravity
	gravity_x = 0;
	gravity_x_rand = 0;
	gravity_y = 0;
	gravity_y_rand = 0;

	// color
	color_rand = Color( static_cast<Uint8>(0), 0, 0, 0 );
	// default 1 second
	time_to_live = 1;
	// default fading is alpha
	fade_size = 0;
	fade_alpha = 1;
	fade_color = 0;

	blending = BLEND_NONE;

	// animation data
	emit_counter = 0;
	emitter_living_time = 0;
}

cParticle_Emitter *cParticle_Emitter :: Copy( void )
{
	cParticle_Emitter *particle_animation = new cParticle_Emitter();
	particle_animation->Set_Pos( startposx, startposy, 1 );
	particle_animation->Set_Pos_Z( posz, posz_rand );
	particle_animation->Set_Filename( filename.c_str() );
	particle_animation->Set_Emitter_Rect( rect );
	particle_animation->Set_Emitter_Time_to_Live( emitter_time_to_live );
	particle_animation->Set_Emitter_Iteration_Interval( emitter_iteration_interval );
	particle_animation->Set_Quota( emitter_quota );
	particle_animation->Set_Time_to_Live( time_to_live, time_to_live_rand );
	particle_animation->Set_Speed( vel, vel_rand );
	particle_animation->Set_Rotation( start_rotx, start_roty, start_rotz, 1 );
	particle_animation->Set_Start_Rot_Z_Uses_Direction( start_rot_z_uses_direction );
	particle_animation->Set_Const_Rotation_X( const_rotx, const_rotx_rand );
	particle_animation->Set_Const_Rotation_Y( const_roty, const_roty_rand );
	particle_animation->Set_Const_Rotation_Z( const_rotz, const_rotz_rand );
	particle_animation->Set_Direction_Range( angle_start, angle_range );
	particle_animation->Set_Scale( size_scale, size_scale_rand );
	particle_animation->Set_Color( color, color_rand );
	particle_animation->Set_Horizontal_Gravity( gravity_x, gravity_x_rand );
	particle_animation->Set_Vertical_Gravity( gravity_y, gravity_y_rand );
	particle_animation->spawned = spawned;

	return particle_animation;
}

void cParticle_Emitter :: Init_Anim( void )
{
	Emit();
}

void cParticle_Emitter :: Emit( void )
{
	if( !image )
	{
		printf( "Warning : cParticle_Emitter can not emit with no image set\n" );
		return;
	}

	for( unsigned int i = 0; i < emitter_quota; i++ )
	{
		cParticle *particle = new cParticle();

		// X Position
		float x = posx;
		if( rect.w > 0 )
		{
			x += Get_Random_Float( 0, rect.w );
		}
		// Y Position
		float y = posy;
		if( rect.h > 0 )
		{
			y += Get_Random_Float( 0, rect.h );
		}
		// Set Position
		particle->Set_Pos( x, y, 1 );

		// Image
		particle->Set_Image( image, 1, 0 );

		// Z position
		particle->posz = posz;
		if( posz_rand > 0 )
		{
			particle->posz += Get_Random_Float( 0, posz_rand );
		}

		// angle range
		float dir_angle = angle_start;
		// start angle
		if( angle_range > 0 )
		{
			dir_angle += Get_Random_Float( 0, angle_range );
		}

		// Velocity
		float speed = vel;
		if( vel_rand > 0 )
		{
			speed += Get_Random_Float( 0, vel_rand );
		}
		// set Velocity
		particle->Set_Direction( dir_angle, speed, 1 );

		// Start rotation
		particle->rotx = start_rotx;
		particle->roty = start_roty;
		particle->rotz = start_rotz;

		// rotation z uses start direction
		if( start_rot_z_uses_direction )
		{
			particle->rotz += dir_angle;
		}

		// Constant rotation
		particle->const_rotx = const_rotx;
		particle->const_roty = const_roty;
		particle->const_rotz = const_rotz;
		if( const_rotx_rand > 0 )
		{
			particle->const_rotx += Get_Random_Float( 0, const_rotx_rand );
		}
		if( const_roty_rand > 0 )
		{
			particle->const_roty += Get_Random_Float( 0, const_roty_rand );
		}
		if( const_rotz_rand > 0 )
		{
			particle->const_rotz += Get_Random_Float( 0, const_rotz_rand );
		}

		// Scale
		particle->Set_Scale( size_scale );
		if( size_scale_rand > 0 )
		{
			particle->Add_Scale( Get_Random_Float( 0, size_scale_rand ) );
		}
		// Gravity
		float grav_x = gravity_x;
		if( gravity_x_rand > 0 )
		{
			grav_x += Get_Random_Float( 0, gravity_x_rand );
		}
		float grav_y = gravity_y;
		if( gravity_y_rand > 0 )
		{
			grav_y += Get_Random_Float( 0, gravity_y_rand );
		}
		// set Gravity
		particle->Set_Gravity( grav_x, grav_y );

		// Color
		particle->Set_Color( color );
		if( color_rand.red > 0 )
		{
			particle->color.red += rand() % color_rand.red;
		}
		if( color_rand.green > 0 )
		{
			particle->color.green += rand() % color_rand.green;
		}
		if( color_rand.blue > 0 )
		{
			particle->color.blue += rand() % color_rand.blue;
		}
		if( color_rand.alpha > 0 )
		{
			particle->color.alpha += rand() % color_rand.alpha;
		}

		// Time to life
		particle->time_to_live = time_to_live;
		if( time_to_live_rand > 0 )
		{
			particle->time_to_live += Get_Random_Float( 0, time_to_live_rand );
		}

		objects.push_back( particle );
	}
}

void cParticle_Emitter :: Clear( void )
{
	// clear particles
	for( ParticleList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();

	// clear animation data
	emit_counter = 0;
	emitter_living_time = 0;
}

void cParticle_Emitter :: Update( void )
{
	Update_Valid_Update();

	if( !valid_update )
	{
		return;
	}
	
	emitter_living_time += pFramerate->speedfactor * ( static_cast<float>(speedfactor_fps) * 0.001f );

	// update objects
	for( ParticleList::iterator itr = objects.begin(); itr != objects.end(); )
	{
		// get object pointer
		cParticle *obj = (*itr);

		// update
		obj->Update();

		// if finished
		if( obj->fade_pos <= 0 )
		{
			itr = objects.erase( itr );
			delete obj;
		}
		// increment
		else
		{
			++itr;
		}
	}

	// if able to emit or endless emitter
	if( emitter_living_time < emitter_time_to_live || emitter_time_to_live == -1 )
	{
		// emit
		while( emit_counter > emitter_iteration_interval )
		{
			Emit();
			emit_counter -= emitter_iteration_interval;
		}

		emit_counter += pFramerate->speedfactor * ( static_cast<float>(speedfactor_fps) * 0.001f );
	}
	// no particles are active
	else if( !objects.size() )
	{
		Set_Visible( 0 );
	}
}

void cParticle_Emitter :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	if( !editor_enabled )
	{
		for( ParticleList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
		{
			(*itr)->Draw( this );
		}
	}
	else
	{
		if( !spawned )
		{
			// draw color rect
			GL_rect color_rect = GL_rect( col_rect.x - pActive_Camera->x, col_rect.y - pActive_Camera->y, col_rect.w, col_rect.h );

			// minimum visible rect size
			if( color_rect.w < 5 )
			{
				color_rect.w = 5;
			}
			if( color_rect.h < 5 )
			{
				color_rect.h = 5;
			}

			cRectRequest *rect_request = new cRectRequest();
			pVideo->Draw_Rect( &color_rect, editor_posz, &darkgreen, rect_request );
			rect_request->filled = 0;
			pRenderer->Add( rect_request );
		}
	}
}

bool cParticle_Emitter :: Is_Update_Valid( void )
{
	// if not visible
	if( !visible )
	{
		return 0;
	}

	// if not in player range and not visible on the screen
	if( !is_Player_range() && !Is_Visible_on_Screen() )
	{
		return 0;
	}

	return 1;
}

bool cParticle_Emitter :: Is_Draw_Valid( void )
{
	// if not visible
	if( !visible )
	{
		return 0;
	}

	// if not in player range and not visible on the screen
	if( !is_Player_range() && !Is_Visible_on_Screen() )
	{
		return 0;
	}

	return 1;
}

void cParticle_Emitter :: Set_Emitter_Rect( float x, float y, float w /* = 0 */, float h /* = 0 */ )
{
	// don't set x/y to 0 or the next Set_Pos call will overwrite the start position
	if( x == 0 && y == 0 )
	{
		y = -1;
	}

	Set_Pos( x, y, 1 );
	rect.w = w;
	rect.h = h;
	col_rect.w = rect.w;
	col_rect.h = rect.h;
	start_rect.w = rect.w;
	start_rect.h = rect.h;

	// invalid width
	if( rect.w < 0 )
	{
		rect.w = 0;
	}
	// invalid height
	if( rect.h < 0 )
	{
		rect.h = 0;
	}
}

void cParticle_Emitter :: Set_Emitter_Rect( GL_rect r )
{
	Set_Emitter_Rect( r.x, r.y, r.w, r.h );
}

void cParticle_Emitter :: Set_Emitter_Time_to_Live( float time )
{
	emitter_time_to_live = time;
}

void cParticle_Emitter :: Set_Emitter_Iteration_Interval( float time )
{
	if( time < 0.001f )
	{
		time = 0.001f;
	}

	emitter_iteration_interval = time;
}

void cParticle_Emitter :: Set_Quota( unsigned int size )
{
	if( size > 1000 )
	{
		size = 1000;
	}

	emitter_quota = size;
}

void cParticle_Emitter :: Set_Speed( float vel_base, float vel_random /* = 2 */ )
{
	vel = vel_base;
	vel_rand = vel_random;
}

void cParticle_Emitter :: Set_Start_Rot_Z_Uses_Direction( bool enable )
{
	start_rot_z_uses_direction = enable;
}

void cParticle_Emitter :: Set_Const_Rotation_X( float rot, float rot_random /* = 0 */ )
{
	const_rotx = rot;
	const_rotx_rand = rot_random;
}

void cParticle_Emitter :: Set_Const_Rotation_Y( float rot, float rot_random /* = 0 */ )
{
	const_roty = rot;
	const_roty_rand = rot_random;
}

void cParticle_Emitter :: Set_Const_Rotation_Z( float rot, float rot_random /* = 0 */ )
{
	const_rotz = rot;
	const_rotz_rand = rot_random;
}

void cParticle_Emitter :: Set_Direction_Range( float start, float range /* = 0 */ )
{
	angle_start = start;
	angle_range = range;
}

void cParticle_Emitter :: Set_Scale( float nscale, float scale_random /* = 0 */ )
{
	size_scale = nscale;
	size_scale_rand = scale_random;
}

void cParticle_Emitter :: Set_Horizontal_Gravity( float start, float random /* = 0 */ )
{
	gravity_x = start;
	gravity_x_rand = random;
}

void cParticle_Emitter :: Set_Vertical_Gravity( float start, float random /* = 0 */ )
{
	gravity_y = start;
	gravity_y_rand = random;
}

void cParticle_Emitter :: Set_Color( Color col, Color col_rand /* = Color( static_cast<Uint8>(0) ) */ )
{
	color = col;
	color_rand = col_rand;
}

void cParticle_Emitter :: Set_Fading_Size( bool enable )
{
	fade_size = enable;
}

void cParticle_Emitter :: Set_Fading_Alpha( bool enable )
{
	fade_alpha = enable;
}

void cParticle_Emitter :: Set_Fading_Color( bool enable )
{
	fade_color = enable;
}

void cParticle_Emitter :: Set_Blending( BlendingMode mode )
{
	blending = mode;
}

void cParticle_Emitter :: Set_Image( GL_Surface *img )
{
	if( !img )
	{
		return;
	}

	image = img;
}

void cParticle_Emitter :: Set_Filename( string str_filename )
{
	// remember the filename for saving
	filename = str_filename;
	Convert_Path_Separators( filename );
	// set new image
	Set_Image( pVideo->Get_Surface( filename, 0 ) );
}

void cParticle_Emitter :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// position z base
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_pos_z_base" ));
	Editor_Add( UTF8_("Position z"), UTF8_("Position z base"), editbox, 150 );

	editbox->setText( float_to_string( posz ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Pos_Z_Base_Key, this ) );

	// position z rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_pos_z_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Position z rand"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( posz_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Pos_Z_Rand_Key, this ) );
	
	// image filename
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_image_filename" ));
	Editor_Add( UTF8_("Filename"), UTF8_("Image filename"), editbox, 300 );

	editbox->setText( filename.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Filename_Key, this ) );

	// emitter width
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_width" ));
	Editor_Add( UTF8_("Emitter width"), UTF8_("Emitter width in which the particles spawn"), editbox, 150 );

	editbox->setText( float_to_string( rect.w ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Emitter_Width_Key, this ) );

	// emitter height
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_height" ));
	Editor_Add( UTF8_("Height"), UTF8_("Emitter height in which the particles spawn"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( rect.h ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Emitter_Height_Key, this ) );

	// emitter time to live
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_time_to_live" ));
	Editor_Add( UTF8_("Emitter TTL"), UTF8_("Emitter time to live. Set -1 for infinite."), editbox, 150 );

	editbox->setText( float_to_string( emitter_time_to_live ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Emitter_Time_To_Live_Key, this ) );

	// emitter interval
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_interval" ));
	Editor_Add( UTF8_("Emitter interval"), UTF8_("Time between spawning particles based on the quota"), editbox, 150 );

	editbox->setText( float_to_string( emitter_iteration_interval ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Emitter_Interval_Key, this ) );

	// quota
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_quota" ));
	Editor_Add( UTF8_("Quota"), UTF8_("The amount of particles to spawn for an interval"), editbox, 50 );

	editbox->setText( int_to_string( emitter_quota ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Quota_Key, this ) );

	// time to live base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_ttl_base" ));
	Editor_Add( UTF8_("TTL"), UTF8_("Time to live base"), editbox, 150 );

	editbox->setText( float_to_string( time_to_live ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Ttl_Base_Key, this ) );
	
	// time to live rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_ttl_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Time to live random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( time_to_live_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Ttl_Rand_Key, this ) );

	// velocity base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_vel_base" ));
	Editor_Add( UTF8_("Velocity"), UTF8_("Velocity base"), editbox, 150 );

	editbox->setText( float_to_string( vel ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Velocity_Base_Key, this ) );

	// velocity rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_vel_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Velocity random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( vel_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Velocity_Rand_Key, this ) );
	
	// start rotation x base
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_rotation_x_base" ));
	Editor_Add( UTF8_("Rotation x"), UTF8_("Rotation x base"), editbox, 150 );

	editbox->setText( float_to_string( start_rotx ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Rotation_X_Base_Key, this ) );
	*/
	// start rotation y base
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_rotation_y_base" ));
	Editor_Add( UTF8_("Rotation y"), UTF8_("Rotation y base"), editbox, 150 );

	editbox->setText( float_to_string( start_roty ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Rotation_Y_Base_Key, this ) );
	*/
	// start rotation z base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_rotation_z_base" ));
	Editor_Add( UTF8_("Rotation z"), UTF8_("Rotation z base"), editbox, 150 );

	editbox->setText( float_to_string( start_rotz ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Rotation_Z_Base_Key, this ) );
	
	// start rotation z uses start direction
	CEGUI::Checkbox *checkbox = static_cast<CEGUI::Checkbox *>(wmgr.createWindow( "TaharezLook/Checkbox", "emitter_start_rot_z_uses_direction" ));
	Editor_Add( UTF8_("Add direction"), UTF8_("Start rotation z uses start direction"), checkbox, 50, 28, 0 );

	checkbox->setSelected( start_rot_z_uses_direction );
	checkbox->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Start_Rot_Z_Uses_Direction_Changed, this ) );

	// constant rotation x base
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_x_base" ));
	Editor_Add( UTF8_("Const. rotation x"), UTF8_("Constant rotation x base"), editbox, 150 );

	editbox->setText( float_to_string( const_rotx ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_X_Base_Key, this ) );
	*/
	// constant rotation x rand
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_x_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Constant rotation x random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( const_rotx_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_X_Rand_Key, this ) );
	*/
	// constant rotation y base
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_y_base" ));
	Editor_Add( UTF8_("Const. rotation y"), UTF8_("Constant rotation y base"), editbox, 150 );

	editbox->setText( float_to_string( const_roty ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_Y_Base_Key, this ) );
	*/
	// constant rotation y rand
	/*editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_y_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Constant rotation y random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( const_roty_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_Y_Rand_Key, this ) );
	*/
	// constant rotation z base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_z_base" ));
	Editor_Add( UTF8_("Const. rotation z"), UTF8_("Constant rotation z base"), editbox, 150 );

	editbox->setText( float_to_string( const_rotz ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_Z_Base_Key, this ) );
	
	// constant rotation z rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_const_rotation_z_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Constant rotation z random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( const_rotz_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Const_Rotation_Z_Rand_Key, this ) );

	// direction base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_direction_base" ));
	Editor_Add( UTF8_("Direction"), UTF8_("Direction start"), editbox, 150 );

	editbox->setText( float_to_string( angle_start ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Direction_Base_Key, this ) );

	// direction rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_direction_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Direction range"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( angle_range ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Direction_Rand_Key, this ) );

	// scale base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_scale_base" ));
	Editor_Add( UTF8_("Scale"), UTF8_("Scale base"), editbox, 150 );

	editbox->setText( float_to_string( size_scale ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Scale_Base_Key, this ) );

	// scale rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_scale_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Scale random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( size_scale_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Scale_Rand_Key, this ) );
	
	// horizontal gravity base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_hor_gravity_base" ));
	Editor_Add( UTF8_("Hor gravity"), UTF8_("Horizontal gravity base"), editbox, 150 );

	editbox->setText( float_to_string( gravity_x ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Horizontal_Gravity_Base_Key, this ) );

	// horizontal gravity rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_hor_gravity_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Horizontal gravity random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( gravity_x_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Horizontal_Gravity_Rand_Key, this ) );
	
	// vertical gravity base
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_ver_gravity_base" ));
	Editor_Add( UTF8_("Ver gravity"), UTF8_("Vertical gravity base"), editbox, 150 );

	editbox->setText( float_to_string( gravity_y ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Vertical_Gravity_Base_Key, this ) );

	// vertical gravity rand
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "emitter_ver_gravity_rand" ));
	Editor_Add( UTF8_("Random"), UTF8_("Vertical gravity random"), editbox, 150, 28, 0 );

	editbox->setText( float_to_string( gravity_y_rand ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cParticle_Emitter::Editor_Vertical_Gravity_Rand_Key, this ) );

	// init
	Editor_Init();
}

bool cParticle_Emitter :: Editor_Filename_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Filename( str_text );

	return 1;
}

bool cParticle_Emitter :: Editor_Pos_Z_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Pos_Z( string_to_float( str_text ), posz_rand ); 

	return 1;
}

bool cParticle_Emitter :: Editor_Pos_Z_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Pos_Z( posz, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Emitter_Width_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Emitter_Rect( posx, posy, string_to_float( str_text ), rect.h );

	return 1;
}

bool cParticle_Emitter :: Editor_Emitter_Height_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Emitter_Rect( posx, posy, rect.w, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Emitter_Time_To_Live_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Emitter_Time_to_Live( string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Emitter_Interval_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Emitter_Iteration_Interval( string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Quota_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Quota( string_to_int( str_text ) );
	
	return 1;
}

bool cParticle_Emitter :: Editor_Ttl_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Time_to_Live( string_to_float( str_text ), time_to_live_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Ttl_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Time_to_Live( time_to_live, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Velocity_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ), vel_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Velocity_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( vel, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Rotation_X_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Rotation_X( string_to_float( str_text ), 1 );

	return 1;
}

bool cParticle_Emitter :: Editor_Rotation_Y_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Rotation_Y( string_to_float( str_text ), 1 );

	return 1;
}

bool cParticle_Emitter :: Editor_Start_Rot_Z_Uses_Direction_Changed( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	bool enabled = static_cast<CEGUI::Checkbox *>( windowEventArgs.window )->isSelected();

	Set_Start_Rot_Z_Uses_Direction( enabled );

	return 1;
}

bool cParticle_Emitter :: Editor_Rotation_Z_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Rotation_Z( string_to_float( str_text ), 1 );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_X_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_X( string_to_float( str_text ), const_rotz_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_X_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_X( const_rotz, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_Y_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_Y( string_to_float( str_text ), const_rotz_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_Y_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_Y( const_rotz, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_Z_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_Z( string_to_float( str_text ), const_rotz_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Const_Rotation_Z_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Const_Rotation_Z( const_rotz, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Direction_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Direction_Range( string_to_float( str_text ), angle_range );

	return 1;
}

bool cParticle_Emitter :: Editor_Direction_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Direction_Range( angle_start, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Scale_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Scale( string_to_float( str_text ), size_scale_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Scale_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Scale( size_scale, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Horizontal_Gravity_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Horizontal_Gravity( string_to_float( str_text ), gravity_x_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Horizontal_Gravity_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Horizontal_Gravity( gravity_x, string_to_float( str_text ) );

	return 1;
}

bool cParticle_Emitter :: Editor_Vertical_Gravity_Base_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Vertical_Gravity( string_to_float( str_text ), gravity_y_rand );

	return 1;
}

bool cParticle_Emitter :: Editor_Vertical_Gravity_Rand_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Vertical_Gravity( gravity_y, string_to_float( str_text ) );

	return 1;
}

/* *** *** *** *** *** cAnimation_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

cAnimation_Manager :: cAnimation_Manager( void )
: cObject_Manager<cAnimation>()
{
	
}

cAnimation_Manager :: ~cAnimation_Manager( void )
{
	Delete_All();
}

void cAnimation_Manager :: Update( void )
{
	for( AnimationList::iterator itr = objects.begin(); itr != objects.end(); )
	{
		// get object pointer
		cAnimation *obj = (*itr);

		// update
		obj->Update();

		// delete if finished
		if( !obj->visible )
		{
			itr = objects.erase( itr );
			delete obj;
		}
		// increment
		else
		{
			++itr;
		}
	}
}

void cAnimation_Manager :: Draw( void )
{
	for( AnimationList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cAnimation_Manager :: Add( cAnimation *animation )
{
	if( !animation )
	{
		return;
	}

	// Initialize
	animation->Init_Anim();

	// Add
	cObject_Manager<cAnimation>::Add( animation );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cAnimation_Manager *pAnimation_Manager = NULL;
