/***************************************************************************
                global_effect.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __GLOBAL_EFFECT_H__
#define __GLOBAL_EFFECT_H__

#include "../core/globals.h"
#include "../core/math/rect.h"
#include "../video/animation.h"

/* *** *** *** *** *** *** *** Global Effect Type *** *** *** *** *** *** *** *** *** *** */

enum  GlobalEffectType
{
	GL_EFF_NONE = 0,	// no global effect
	GL_EFF_FALLING = 1,	// falling down images ( leafs or rain )
	GL_EFF_FLYING = 2	// moving upwards images ( heat points or dust )
};

/* *** *** *** *** *** *** *** Global effect class *** *** *** *** *** *** *** *** *** *** */

class cGlobal_effect
{
public:
	// default constructor
	cGlobal_effect( void );
	// create from stream
	cGlobal_effect( XMLAttributes &attributes );
	// destructor
	~cGlobal_effect( void );

	// init defaults
	void Init_default( void );
	// initialise
	void Init( void );
	// clear
	void Clear( void );

	// create from stream
	void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	void Save_to_Stream( ofstream &file );

	// update
	void Update( void );
	// update animation
	void Update_anim( void );
	// draw
	void Draw( void );

	// Sets the type of the global effect
	void Set_type( GlobalEffectType ntype );
	void Set_type( string ntype );
	// Set the image
	void Set_image( string nimg_file );
	// Set creation rect
	void Set_rect( GL_rect nrect );
	// Set Z position
	void Set_ZPos( float pos, float pos_random = 0 );
	// Set Time to Live
	void Set_Time_to_Live( float val );
	// Set scale and randomness
	void Set_Scale( float nscale, float scale_random = 0 );
	// Set creation speed
	void Set_Creation_Speed( float nspeed );
	// Set speed and randomness
	void Set_Speed( float nspeed, float speed_random = 0 );
	// Set direction range start and size
	void Set_Direction( float range_start, float range_size );
	// Set constant rotation and randomness
	void Set_ConstRotationZ( float rot, float rot_random = 0 );

	// Returns the name of the current type
	string Get_Typename( void );

	// type
	GlobalEffectType type;

	// image filename
	string image_filename;
	// image
	GL_Surface *image;
	// rect
	GL_rect rect;
	// Z position
	float posz, posz_rand;

	// object time to live
	float time_to_live;
	// object creation speed
	float creation_speed;
	// scale
	float scale, scale_rand;
	// speed
	float speed, speed_rand;
	// direction range
	float dir_range_start, dir_range_size;
	// constant rotation
	float const_rotz, const_rotz_rand;

	// counter
	float counter;
	// valid global effect
	bool valid;

	// animation manager
	cAnimationManager *anim_manager;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
