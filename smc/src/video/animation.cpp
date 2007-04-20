/***************************************************************************
           animation.cpp  -  Animation and Particle classes
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../video/animation.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

cAnimation :: cAnimation( float posx, float posy )
: cImageObjectSprite( posx, posy )
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

void cAnimation :: Init( void )
{
	// virtual
}

void cAnimation :: Update( void )
{
	// virtual
}

void cAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	// virtual
}


void cAnimation :: Set_Time_to_Live( float time, float time_rand /* = 0 */ )
{
	time_to_live = time;
	time_to_live_rand = time_rand;
}

void cAnimation :: Set_FadingSpeed( float speed )
{
	fading_speed = speed;

	if( fading_speed <= 0 )
	{
		fading_speed = 0.1f;
	}
}

void cAnimation :: Set_ZPos( float pos, float pos_rand /* = 0 */ )
{
	posz = pos;
	posz_rand = pos_rand;
}

/* *** *** *** *** *** *** *** cBlinkAnimation *** *** *** *** *** *** *** *** *** *** */

cBlinkAnimation :: cBlinkAnimation( float posx, float posy, unsigned int height /* = 40 */, unsigned int width /* = 20 */ )
: cAnimation( posx, posy )
{
	animtype = BLINKING_POINTS;

	images.push_back( pVideo->Get_Surface( "animation/light_1/1.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/light_1/2.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/light_1/3.png" ) );

	for( unsigned int i = 0; i < 4; i++ )
	{
		cSprite object( NULL, (float)( rand() % ( width ) ), (float)( rand() % ( height ) ) );
		objects.push_back( object );
	}
}

cBlinkAnimation :: ~cBlinkAnimation( void )
{
	objects.clear();
}

void cBlinkAnimation :: Update( void )
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

		objects[i].Set_Scale( 1.1f - ( time_to_live / 12 ) );
	}
}

void cBlinkAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	// draw the fixed points
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( image )
		{
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			image->Blit( objects[i].posx - ( pCamera->x - posx ), objects[i].posy - ( pCamera->y - posy ), posz, request );

			// scale
			request->scale_x = objects[i].scalex;
			request->scale_y = objects[i].scaley;

			// color
			request->color = color;

			// add request
			pRenderer->Add( request );
		}

		if( time_to_live > 11 || time_to_live < 0 )
		{
			visible = 0;
		}
	}
}

/* *** *** *** *** *** *** *** cFireAnimation *** *** *** *** *** *** *** *** *** *** */

cFireAnimation :: cFireAnimation( float posx, float posy, unsigned int power /* = 5 */ )
: cAnimation( posx, posy )
{
	animtype = FIRE_EXPLOSION;

	images.push_back( pVideo->Get_Surface( "animation/fire_1/4.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/fire_1/3.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/fire_1/2.png" ) );
	images.push_back( pVideo->Get_Surface( "animation/fire_1/1.png" ) );

	// fill the objects list
	for( unsigned int i = 0; i < power; i++ )
	{
		FireAnimation_item temp;
		objects.push_back( temp );
	}
}

cFireAnimation :: ~cFireAnimation( void )
{
	objects.clear();
}

void cFireAnimation :: Init( void )
{
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		// position
		objects[i].velx = ( (float)( rand() % ( 50 ) - 25 ) ) / 10;
		objects[i].vely = ( (float)( rand() % ( 50 ) - 25 ) ) / 10;

		// Z position
		objects[i].posz = posz;
		if( posz_rand > 0.001f )
		{
			objects[i].posz += (float)( rand() % (int)( posz_rand * 1000 ) ) / 1000;
		}

		// lifetime
		objects[i].counter = (float)( 8 + rand() % ( 5 ) );
	}
}

void cFireAnimation :: Update( void )
{
	if( !visible )
	{
		return;
	}

	time_to_live += pFramerate->speedfactor * fading_speed;

	// update objects
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( objects[i].counter > 8 )
		{
			Set_Image( 0 );
		}
		else if( objects[i].counter > 5 )
		{
			Set_Image( 1 );
		}
		else if( objects[i].counter > 3 )
		{
			Set_Image( 2 );
		}
		else if( objects[i].counter > 0 )
		{
			Set_Image( 3 );
		}

		objects[i].counter -= pFramerate->speedfactor * fading_speed;

		objects[i].Set_Scale( objects[i].counter / 10 );
		objects[i].Move( objects[i].velx, objects[i].vely );
	}

	if( time_to_live > 12 || time_to_live < 0 )
	{
		visible = 0;
	}
}

void cFireAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	// draw objects
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( image )
		{
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			image->Blit( objects[i].posx - ( pCamera->x - posx ), objects[i].posy - ( pCamera->y - posy ), objects[i].posz, request );

			// scale
			request->scale_x = objects[i].scalex;
			request->scale_y = objects[i].scaley;

			// color
			request->color = color;

			// add request
			pRenderer->Add( request );
		}
	}
}

/* *** *** *** *** *** *** *** cParticle *** *** *** *** *** *** *** *** *** *** */

cParticle :: cParticle( void )
: cMovingSprite()
{
	time_to_live = 0;
	const_rotx = 0;
	const_roty = 0;
	const_rotz = 0;

	fade_pos = 1;
}

cParticle :: ~cParticle( void )
{

}

void cParticle :: Update( void )
{
	// update fade modifier
	fade_pos -= ( pFramerate->speedfactor * ( (float)DESIRED_FPS * 0.001f ) ) / time_to_live;

	// finished fading
	if( fade_pos <= 0 )
	{
		fade_pos = 0;
		visible = 0;
		return;
	}

	// position
	posx += velx * pFramerate->speedfactor;
	posy += vely * pFramerate->speedfactor;
	
	// constant rotation
	if( const_rotx > 0.0001f )
	{
		Set_RotationX( rotx + ( const_rotx * pFramerate->speedfactor ) );
	}
	if( const_roty > 0.0001f )
	{
		Set_RotationY( roty + ( const_roty * pFramerate->speedfactor ) );
	}
	if( const_rotz > 0.0001f )
	{
		Set_RotationZ( rotz + ( const_rotz * pFramerate->speedfactor ) );
	}
}

void cParticle :: Draw( cParticleAnimation *origin )
{
	// create request
	cSurfaceRequest *request = new cSurfaceRequest();

	image->Blit( posx - pCamera->x, posy - pCamera->y, posz, request );

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
		request->color.red *= fade_pos;
		request->color.green *= fade_pos;
		request->color.blue *= fade_pos;
	}
	// alpha fading
	if( origin->fade_alpha )
	{
		request->color.alpha *= fade_pos;
	}

	// add request
	pRenderer->Add( request );
}

/* *** *** *** *** *** *** *** cParticleAnimation *** *** *** *** *** *** *** *** *** *** */

cParticleAnimation :: cParticleAnimation( float posx, float posy )
: cAnimation( posx, posy )
{
	animtype = PARTICLE_EXPLOSION;

	// 0 = 1 emit
	emitter_time_to_live = 0;
	emitter_iteration_interval = 0.2f;
	emitter_quota = 1;

	// velocity
	vel = 2;
	vel_rand = 2;
	// rotation
	const_rotx = 0;
	const_roty = 0;
	const_rotz = 0;
	const_rotx_rand = 0;
	const_roty_rand = 0;
	const_rotz_rand = 0;
	angle_start = 0;
	angle_range = 360;
	size_scale = 1;
	size_scale_rand = 0;
	color_rand = Color( (Uint8)0, 0, 0, 0 );
	// default 1 second
	time_to_live = 1;
	// default fading is alpha
	fade_size = 0;
	fade_alpha = 1;
	fade_color = 0;

	blending = BLEND_NONE;

	// default image
	image = pVideo->Get_Surface( "animation/particles/smoke.png" );

	// animation data
	emit_counter = 0;
	emitter_living_time = 0;
}

cParticleAnimation :: ~cParticleAnimation( void )
{
	Clear();
}

void cParticleAnimation :: Init( void )
{
	Emit();
}

void cParticleAnimation :: Emit( void )
{
	for( unsigned int i = 0; i < emitter_quota; i++ )
	{
		cParticle *particle = new cParticle();

		// Position
		particle->Set_Pos( posx, posy, 1 );

		// Image
		particle->Set_Image( image, 1, 0 );

		// Z position
		particle->posz = posz;
		if( posz_rand > 0.001f )
		{
			particle->posz += (float)( rand() % (int)( posz_rand * 1000 ) ) / 1000;
		}

		// angle range
		float dir_angle = 0;
		// start angle
		if( angle_start > 0.001f )
		{
			dir_angle += angle_start;
		}
		if( angle_range > 0.001f )
		{
			dir_angle += (float)( rand() % (int)( angle_range * 100 ) ) / 100;
		}

		// Velocity
		float speed = vel;
		if( vel_rand > 0.001f )
		{
			speed += (float)( rand() % (int)( vel_rand * 1000 ) ) / 1000;
		}
		// set Velocity
		particle->Set_Direction( dir_angle, speed, 1 );

		// Basic rotation
		particle->rotx = rotx;
		particle->roty = roty;
		particle->rotz = rotz;

		// Constant rotation
		particle->const_rotx = const_rotx;
		particle->const_roty = const_roty;
		particle->const_rotz = const_rotz;
		if( const_rotx_rand > 0.001f )
		{
			particle->const_rotx += (float)( rand() % (int)( const_rotx_rand * 100 ) ) / 100;
		}
		if( const_roty_rand > 0.001f )
		{
			particle->const_roty += (float)( rand() % (int)( const_roty_rand * 100 ) ) / 100;
		}
		if( const_rotz_rand > 0.001f )
		{
			particle->const_rotz += (float)( rand() % (int)( const_rotz_rand * 100 ) ) / 100;
		}

		// Scale
		particle->Set_Scale( size_scale );
		if( size_scale_rand > 0.001f )
		{
			particle->Add_Scale( (float)( rand() % (int)( size_scale_rand * 100 ) ) / 100 );
		}

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
		if( time_to_live_rand > 0.001f )
		{
			particle->time_to_live += (float)( rand() % (int)( time_to_live_rand * 1000 ) ) / 1000;
		}

		objects.push_back( particle );
	}
}

void cParticleAnimation :: Clear( void )
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

void cParticleAnimation :: Update( void )
{
	if( !visible )
	{
		return;
	}
	
	emitter_living_time += pFramerate->speedfactor * ( (float)DESIRED_FPS * 0.001f );

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

	// if able to emit
	if( emitter_living_time < emitter_time_to_live )
	{
		// emit
		while( emit_counter > emitter_iteration_interval )
		{
			Emit();
			emit_counter -= emitter_iteration_interval;
		}

		emit_counter += pFramerate->speedfactor * ( (float)DESIRED_FPS * 0.001f );
	}
	// no particles are active
	else if( !objects.size() )
	{
		visible = 0;
	}
}

void cParticleAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	for( ParticleList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw( this );
	}
}

void cParticleAnimation :: Set_Emitter_Time_to_Live( float time )
{
	emitter_time_to_live = time;
}

void cParticleAnimation :: Set_Emitter_Iteration_Interval( float time )
{
	emitter_iteration_interval = time;
}

void cParticleAnimation :: Set_Quota( unsigned int size )
{
	emitter_quota = size;
}

void cParticleAnimation :: Set_Speed( float vel_base, float vel_random /* = 1 */ )
{
	vel = vel_base;
	vel_rand = vel_random;
}

void cParticleAnimation :: Set_ConstRotationX( float rot, float rot_random /* = 0 */ )
{
	const_rotx = rot;
	const_rotx_rand = rot_random;
}

void cParticleAnimation :: Set_ConstRotationY( float rot, float rot_random /* = 0 */ )
{
	const_roty = rot;
	const_roty_rand = rot_random;
}

void cParticleAnimation :: Set_ConstRotationZ( float rot, float rot_random /* = 0 */ )
{
	const_rotz = rot;
	const_rotz_rand = rot_random;
}

void cParticleAnimation :: Set_DirectionRange( float start, float range /* = 20 */ )
{
	angle_start = start;
	angle_range = range;
}

void cParticleAnimation :: Set_Scale( float nscale, float scale_random /* = 0 */ )
{
	size_scale = nscale;
	size_scale_rand = scale_random;
}

void cParticleAnimation :: Set_Color( Color col, Color col_rand )
{
	color = col;
	color_rand = col_rand;
}

void cParticleAnimation :: Set_Fading_Size( bool enable )
{
	fade_size = enable;
}

void cParticleAnimation :: Set_Fading_Alpha( bool enable )
{
	fade_alpha = enable;
}

void cParticleAnimation :: Set_Fading_Color( bool enable )
{
	fade_color = enable;
}

void cParticleAnimation :: Set_Blending( BlendingMode mode )
{
	blending = mode;
}

void cParticleAnimation :: Set_Image( GL_Surface *img )
{
	if( !img )
	{
		return;
	}

	image = img;
}

/* *** *** *** *** *** cAnimationManager *** *** *** *** *** *** *** *** *** *** *** *** */

cAnimationManager :: cAnimationManager( void )
{
	
}

cAnimationManager :: ~cAnimationManager( void )
{
	Delete_All();
}

void cAnimationManager :: Update( void )
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

void cAnimationManager :: Draw( void )
{
	for( AnimationList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Draw();
	}
}

void cAnimationManager :: Add( cAnimation *animation )
{
	if( !animation )
	{
		return;
	}

	// Initialize
	animation->Init();

	// Add
	objects.push_back( animation );
}

void cAnimationManager :: Delete( unsigned int num )
{
	// out of array
	if( num > objects.size() )
	{
		return;
	}

	delete objects[num];
	objects.erase( objects.begin() + num );
}

void cAnimationManager :: Delete_All( void )
{
	if( !objects.size() )
	{
		return;
	}

	for( AnimationList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();
}

unsigned int cAnimationManager :: size( void )
{
	return objects.size();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cAnimationManager *pAnimationManager = NULL;
