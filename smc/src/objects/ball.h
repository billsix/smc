/***************************************************************************
                ball.h  -  header for the corresponding cpp file
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

#ifndef __BALL_H__
#define __BALL_H__

#include "../core/globals.h"
#include "../video/video.h"
#include "../core/collision.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** *** *** Ball class *** *** *** *** *** *** *** *** *** *** */

class cBall : public cMovingSprite
{
public:
	cBall( float x, float y, float nvelx, float nvely = 0, cMovingSprite *norigin = NULL, ball_effect btype = FIREBALL_DEFAULT );
	virtual ~cBall( void );

	// delete this ball on the next frame start
	virtual void Destroy( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	// ball origin
	cMovingSprite *origin;
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
