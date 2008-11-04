/***************************************************************************
 * ball.h  -  header for the corresponding cpp file
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

#ifndef SMC_BALL_H
#define SMC_BALL_H

#include "../core/globals.h"
#include "../video/video.h"
#include "../core/collision.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** *** *** Ball class *** *** *** *** *** *** *** *** *** *** */

class cBall : public cMovingSprite
{
public:
	cBall( float x, float y, cSprite *origin_object = NULL, ball_effect btype = FIREBALL_DEFAULT );
	virtual ~cBall( void );

	// delete on the next frame start
	virtual void Destroy( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Generate the default animation Particles
	void Generate_Particles( cParticle_Emitter *anim = NULL );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_out_of_Level( ObjectDirection dir );

	// ball origin
	ArrayType origin_array;
	SpriteType origin_type;
	// ball type
	ball_effect ball_type;

	// rotation animation counter
	float counter;

	// glim animation modifier
	bool glim_mod;
	// glim animation counter
	float glim_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
