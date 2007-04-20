/***************************************************************************
			animation.h  -  header for the corresponding cpp file
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

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "../objects/movingsprite.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** Animation definitions *** *** *** *** *** *** *** *** *** *** */

enum AnimationEffect
{
	ANIM_UNDEFINED,
	BLINKING_POINTS,
	FIRE_EXPLOSION,
	PARTICLE_EXPLOSION,
};

/* *** *** *** *** *** *** *** Particle blending definitions *** *** *** *** *** *** *** *** *** *** */

enum BlendingMode // todo : more
{
	BLEND_NONE,
	BLEND_ADD,
	BLEND_DRIVE
};

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

class cAnimation : public cImageObjectSprite
{
public:
	cAnimation( float posx, float posy );
	virtual ~cAnimation( void );

	// initialize animation
	virtual void Init( void );
	// update animation
	virtual void Update( void );
	// draw animation
	virtual void Draw( cSurfaceRequest *request = NULL );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// Set time to live for Objects in seconds
	void Set_Time_to_Live( float time, float time_rand = 0 );
	/* Set speed of fading out ( 0.01 - 100 )
	* the lower the longer it takes
	* Note : only useful for non particle animation Objects
	*/
	void Set_FadingSpeed( float speed );
	// set z position
	void Set_ZPos( float pos, float pos_rand = 0 );

	// Z random position
	float posz_rand;
	// fading out speed
	float fading_speed;
	// object time to live
	float time_to_live, time_to_live_rand;
	// animation type
	AnimationEffect animtype;
};

/* *** *** *** *** *** *** *** *** Blinking points *** *** *** *** *** *** *** *** *** */

class cBlinkAnimation : public cAnimation
{
public:
	cBlinkAnimation( float posx, float posy, unsigned int height = 40, unsigned int width = 20 );
	virtual ~cBlinkAnimation( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	typedef vector<cSprite> BlinkPointList;
	BlinkPointList objects;
	
};

/* *** *** *** *** *** *** *** Fire Explosion *** *** *** *** *** *** *** *** *** *** */

struct FireAnimation_item : public cMovingSprite
{
	FireAnimation_item( void )
	: cMovingSprite()
	{
		counter = 0;
	}

	~FireAnimation_item( void ) {}

	// lifetime
	float counter;
};

class cFireAnimation : public cAnimation
{
public:
	cFireAnimation( float posx, float posy, unsigned int power = 5 );
	virtual ~cFireAnimation( void );

	// init
	virtual void Init( void );
	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	typedef vector<FireAnimation_item> FireAnimList;
	FireAnimList objects;
};

/* *** *** *** *** *** *** *** Particle Animation *** *** *** *** *** *** *** *** *** *** */

class cParticleAnimation;

// Particle Item
class cParticle : public cMovingSprite
{
public:
	cParticle( void );
	virtual ~cParticle( void );

	// update
	void Update( void );
	// draw
	void Draw( cParticleAnimation *origin );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// time to live
	float time_to_live;
	// constant rotation
	float const_rotx, const_roty, const_rotz;

	// fading position value
	float fade_pos;
};

// Particle Animation Manager
class cParticleAnimation : public cAnimation
{
public:
	cParticleAnimation( float posx, float posy );
	virtual ~cParticleAnimation( void );

	// init
	virtual void Init( void );
	// Emit Particles
	virtual void Emit( void );
	// Clear Particles and Animation data
	void Clear( void );

	// Update given settings
	virtual void Update( void );
	// Draw everything
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Set time to live for the Emitter in seconds
	void Set_Emitter_Time_to_Live( float time );
	// Set time between Iterations
	void Set_Emitter_Iteration_Interval( float time );
	// Set Particle Count/Quota
	void Set_Quota( unsigned int size );
	// Set speed ( 0 - 100 )
	void Set_Speed( float vel_base, float vel_random = 2 );
	// Set x constant rotation
	void Set_ConstRotationX( float rot, float rot_random = 0 );
	// Set y constant rotation
	void Set_ConstRotationY( float rot, float rot_random = 0 );
	// Set z constant rotation
	void Set_ConstRotationZ( float rot, float rot_random = 0 );
	// Set direction range ( 0 - 360 )
	void Set_DirectionRange( float start, float range = 0 );
	// Set image scale ( 0.01 - 100 )
	void Set_Scale( float nscale, float scale_random = 0 );
	// Set the Color
	void Set_Color( Color col, Color col_rand = Color( (Uint8)0, 0, 0, 0 ) );
	// Set fading type
	void Set_Fading_Size( bool enable );
	void Set_Fading_Alpha( bool enable );
	void Set_Fading_Color( bool enable );
	// Set blending mode
	void Set_Blending( BlendingMode mode );
	// Set image
	virtual void Set_Image( GL_Surface *img );

	// Particle items
	typedef vector<cParticle *> ParticleList;
	ParticleList objects;

	// emitter time to live
	float emitter_time_to_live;
	// emitter iteration interval
	float emitter_iteration_interval;
	// emitter object quota
	unsigned int emitter_quota;

	// velocity
	float vel, vel_rand;
	// constant rotation
	float const_rotx, const_roty, const_rotz;
	// constant rotation random modifier
	float const_rotx_rand, const_roty_rand, const_rotz_rand;
	// direction range
	float angle_start, angle_range;
	// size scaling
	float size_scale, size_scale_rand;
	// color random
	Color color_rand;
	// fading types todo : dest-color
	bool fade_size, fade_alpha, fade_color;
	// blending mode
	BlendingMode blending;

private:
	// time alive
	float emitter_living_time;
	// emit counter
	float emit_counter;
};

/* *** *** *** *** *** *** *** Animation Manager *** *** *** *** *** *** *** *** *** *** */

class cAnimationManager
{
public:
	cAnimationManager( void );
	~cAnimationManager( void );

	// Add an animation object with the given settings
	void Add( cAnimation *animation );
	// Delete an animation object
	void Delete( unsigned int num );

	// Update the objects
	void Update( void );
	// Draw the objects
	void Draw( void );

	// Delete all animation objects
	void Delete_All( void );

	// Return the current size
	unsigned int size( void );

	typedef vector<cAnimation *> AnimationList;
	AnimationList objects;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Animation Manager
extern cAnimationManager *pAnimationManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
