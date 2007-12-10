/***************************************************************************
           animation.cpp  -  Animation and Particle classes
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

#include "../video/animation.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

cAnimation :: cAnimation( float x, float y )
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
		cSprite *obj = new cSprite( NULL, static_cast<float>( rand() % ( width ) ), static_cast<float>( rand() % ( height ) ) );
		objects.push_back( obj );
	}
}

cBlinkAnimation :: ~cBlinkAnimation( void )
{
	// clear
	for( BlinkPointList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

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

		objects[i]->Set_Scale( 1.1f - ( time_to_live / 12 ) );
	}
}

void cBlinkAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
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
		image->Blit( obj->posx - ( pCamera->x - posx ), obj->posy - ( pCamera->y - posy ), posz, request );

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

/* *** *** *** *** *** *** *** cFireAnimation *** *** *** *** *** *** *** *** *** *** */

cFireAnimation :: cFireAnimation( float posx, float posy, unsigned int power /* = 5 */ )
: cAnimation( posx, posy )
{
	animtype = FIRE_EXPLOSION;

	// create objects
	for( unsigned int i = 0; i < power; i++ )
	{
		FireAnimation_item *obj = new FireAnimation_item();
		objects.push_back( obj );
	}
}

cFireAnimation :: ~cFireAnimation( void )
{
	// clear
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	objects.clear();
}

void cFireAnimation :: Init( void )
{
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		FireAnimation_item *obj = (*itr);

		// images
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/4.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/3.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/2.png" ) );
		obj->images.push_back( pVideo->Get_Surface( "animation/fire_1/1.png" ) );
		obj->Set_Image( 0 );

		// velocity
		obj->velx = static_cast<float>( rand() % ( 50 ) - 25 ) / 10;
		obj->vely = static_cast<float>( rand() % ( 50 ) - 25 ) / 10;

		// Z position
		obj->posz = posz;
		if( posz_rand > 0.001f )
		{
			obj->posz += static_cast<float>( rand() % static_cast<int>( posz_rand * 1000 ) ) / 1000;
		}

		// lifetime
		obj->counter = static_cast<float>( 8 + rand() % ( 5 ) );
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
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		FireAnimation_item *obj = (*itr);

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

void cFireAnimation :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !visible )
	{
		return;
	}

	// draw objects
	for( FireAnimList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		FireAnimation_item *obj = (*itr);

		// create request
		cSurfaceRequest *request = new cSurfaceRequest();
		obj->image->Blit( obj->posx - ( pCamera->x - posx ), obj->posy - ( pCamera->y - posy ), obj->posz, request );

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
	fade_pos -= ( pFramerate->speedfactor * ( static_cast<float>(DESIRED_FPS) * 0.001f ) ) / time_to_live;

	// finished fading
	if( fade_pos <= 0 )
	{
		fade_pos = 0;
		Set_Visible( 0 );
		return;
	}

	// move
	Move( velx, vely );
	
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

cParticleAnimation :: cParticleAnimation( float x /* = 0 */, float y /* = 0 */, float w /* = 0 */, float h /* = 0 */ )
: cAnimation( x, y )
{
	rect.w = w;
	rect.h = h;

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
	// angle
	angle_start = 0;
	angle_range = 360;
	// scale
	size_scale = 1;
	size_scale_rand = 0;
	// color
	color_rand = Color( static_cast<Uint8>(0), 0, 0, 0 );
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

		// X Position
		float x = posx;
		if( rect.w > 0 )
		{
			x += ( rand() % static_cast<int>(rect.w) );
		}
		// Y Position
		float y = posy;
		if( rect.h > 0 )
		{
			y += ( rand() % static_cast<int>(rect.h) );
		}
		// Set Position
		particle->Set_Pos( x, y, 1 );

		// Image
		particle->Set_Image( image, 1, 0 );

		// Z position
		particle->posz = posz;
		if( posz_rand > 0.001f )
		{
			particle->posz += static_cast<float>( rand() % static_cast<int>( posz_rand * 1000 ) ) / 1000;
		}

		// angle range
		float dir_angle = angle_start;
		// start angle
		if( angle_range > 0.001f )
		{
			dir_angle += static_cast<float>( rand() % static_cast<int>( angle_range * 10 ) ) / 10;
		}

		// Velocity
		float speed = vel;
		if( vel_rand > 0.001f )
		{
			speed += static_cast<float>( rand() % static_cast<int>( vel_rand * 1000 ) ) / 1000;
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
			particle->const_rotx += static_cast<float>( rand() % static_cast<int>( const_rotx_rand * 100 ) ) / 100;
		}
		if( const_roty_rand > 0.001f )
		{
			particle->const_roty += static_cast<float>( rand() % static_cast<int>( const_roty_rand * 100 ) ) / 100;
		}
		if( const_rotz_rand > 0.001f )
		{
			particle->const_rotz += static_cast<float>( rand() % static_cast<int>( const_rotz_rand * 100 ) ) / 100;
		}

		// Scale
		particle->Set_Scale( size_scale );
		if( size_scale_rand > 0.001f )
		{
			particle->Add_Scale( static_cast<float>( rand() % static_cast<int>( size_scale_rand * 100 ) ) / 100 );
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
			particle->time_to_live += static_cast<float>( rand() % static_cast<int>( time_to_live_rand * 1000 ) ) / 1000;
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
	
	emitter_living_time += pFramerate->speedfactor * ( static_cast<float>(DESIRED_FPS) * 0.001f );

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

		emit_counter += pFramerate->speedfactor * ( static_cast<float>(DESIRED_FPS) * 0.001f );
	}
	// no particles are active
	else if( !objects.size() )
	{
		Set_Visible( 0 );
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

void cParticleAnimation :: Set_Emitter_Rect( float x, float y, float w /* = 0 */, float h /* = 0 */ )
{
	Set_Pos( x, y, 1 );
	rect.w = w;
	rect.h = h;

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

void cParticleAnimation :: Set_Emitter_Rect( GL_rect r )
{
	Set_Emitter_Rect( r.x, r.y, r.w, r.h );
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

void cParticleAnimation :: Set_Speed( float vel_base, float vel_random /* = 2 */ )
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

void cParticleAnimation :: Set_DirectionRange( float start, float range /* = 0 */ )
{
	angle_start = start;
	angle_range = range;
}

void cParticleAnimation :: Set_Scale( float nscale, float scale_random /* = 0 */ )
{
	size_scale = nscale;
	size_scale_rand = scale_random;
}

void cParticleAnimation :: Set_Color( Color col, Color col_rand /* = Color( static_cast<Uint8>(0) ) */ )
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
