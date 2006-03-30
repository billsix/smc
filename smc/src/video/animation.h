/***************************************************************************
			animation.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../globals.h"
#include "../objects/sprite.h"

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

/* *** *** *** *** *** *** *** Animation definitions *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	BLINKING_POINTS,
	FIRE_EXPLOSION,
	PARTICLE_EXPLOSION,
} AnimationEffect;


/* *** *** *** *** *** *** *** Particle fading definitions *** *** *** *** *** *** *** *** *** *** */

typedef enum // todo : multi-select
{
	FADE_SIZE,
	FADE_ALPHA,
	FADE_COLOR, // todo : dest-color
} FadingType;

/* *** *** *** *** *** *** *** Particle blending definitions *** *** *** *** *** *** *** *** *** *** */

typedef enum // todo : more
{
	BLEND_NONE,
	BLEND_ADD,
	BLEND_DRIVE
} BlendingMode;

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

class cAnimation : public cObjectSprite
{
public:
	cAnimation( double posx, double posy, double nfading_speed = 1, Color col = (Uint8)255 );
	~cAnimation( void );

	// initialize the animation
	virtual void Init( void );
	// update the aniamtion
	virtual void Update( void );
	// draw the animation
	virtual void Draw( void );
	// sets the speed of fading out ( 0.01 - 100 )
	void SetFadingSpeed( double speed );
	// sets the draw color
	void SetColor( Color col );

	double fading_speed;
	double counter;
	AnimationEffect animtype;
	Color color;
};

/* *** *** *** *** *** *** *** *** Blinking points *** *** *** *** *** *** *** *** *** */

class cAnimation_1 : public cAnimation
{
public:
	cAnimation_1( double posx, double posy, unsigned int height = 40, unsigned int width = 20 );
	~cAnimation_1( void );

	virtual void Update( void );
	virtual void Draw( void );

	typedef vector<cSprite> Animation1PointList;
	Animation1PointList objects;
	
};

/* *** *** *** *** *** *** *** Fire Explosion *** *** *** *** *** *** *** *** *** *** */

struct Animation2item : public cSprite
{
	Animation2item( void ) : cSprite()
	{
		counter = 0;
	}
	~Animation2item( void ) {}

	double counter;
};

class cAnimation_2 : public cAnimation
{
public:
	cAnimation_2( double posx, double posy, unsigned int power = 5 );
	~cAnimation_2( void );

	virtual void Update( void );
	virtual void Draw( void );

	typedef vector<Animation2item> Animation2List;
	Animation2List objects;
};

/* *** *** *** *** *** *** *** Particle Animation *** *** *** *** *** *** *** *** *** *** */

struct Animation3item : public cSprite
{
	Animation3item( void ) : cSprite()
	{
		counter = 0;
	}
	~Animation3item( void ) {}

	double counter;
	// constant rotation
	double const_rotx, const_roty, const_rotz;
};

class cAnimation_3 : public cAnimation
{
public:
	cAnimation_3( double posx, double posy, unsigned int power = 5, GL_Surface *img = NULL );
	~cAnimation_3( void );

	// Initialize settings
	virtual void Init( void );
	// update given settings
	virtual void Update( void );
	// draw everything
	virtual void Draw( void );

	// set speed ( 0 - 100 )
	void SetSpeed( double vel_base, double vel_random = 2 );
	// set x constant rotation
	void Set_ConstRotationX( double rot, double rot_random = 0 );
	// set y constant rotation
	void Set_ConstRotationY( double rot, double rot_random = 0 );
	// set z constant rotation
	void Set_ConstRotationZ( double rot, double rot_random = 0 );
	// set direction range ( 0 - 360 )
	void SetDirectionRange( double start, double range = 20 );
	// set image scale ( 0.01 - 100 )
	void SetScale( double size );
	// set lifetime modifier ( 0.01 - 100 )
	void SetLifetimeMod( double time );
	// set fading type
	void SetFading( FadingType type );
	// set blending mode
	void SetBlending( BlendingMode mode );
	// set image
	virtual void SetImage( GL_Surface *img );

	typedef vector<Animation3item> Animation3List;
	Animation3List objects;

	// velocity
	double vel, vel_rand;
	// constant rotation
	double const_rotx, const_roty, const_rotz;
	// constant rotation random modifier
	double const_rotx_rand, const_roty_rand, const_rotz_rand;
	// direction range
	double angle_start, angle_range;
	// size scaling
	double size_scale;
	// lifetime modifier
	double lifetime;
	// fading type
	FadingType fadetype;
	// fading value
	double fade;
	// blending mode
	BlendingMode blending;
};

/* *** *** *** *** *** *** *** Animation Manager *** *** *** *** *** *** *** *** *** *** */

class cAnimationManager
{
public:
	cAnimationManager( void );
	~cAnimationManager( void );

	// adds an animation object with the given settings
	void Add( cAnimation *animation );
	// deletes an animation object
	void Delete( unsigned int num );

	// updates the objects
	void Update( void );
	// draws the bjects
	void Draw( void );

	// deletes all animation objects
	void DeleteAll( void );

	// returns the current size
	unsigned int Get_Size( void );

	typedef vector<cAnimation*> AnimationList;
	AnimationList objects;
};

#endif
