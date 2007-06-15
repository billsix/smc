/***************************************************************************
                star.h  -  header for the corresponding cpp file
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

#ifndef __STAR_H__
#define __STAR_H__

#include "../core/globals.h"
#include "../objects/powerup.h"

/* *** *** *** *** *** Jumping Star *** *** *** *** *** *** *** *** *** *** *** *** */

class cjStar : public cPowerUp
{
public:
	// constructor
	cjStar( float x, float y );
	// destructor
	virtual ~cjStar( void );
	
	// copy
	virtual cjStar *Copy( void );

	// Activate the star
	void Activate( void );

	// update the Star
	virtual void Update( void );
	// draw the Star
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Adds Star Particles
	void Generate_Particles( float x = 0, float y = 0, unsigned int quota = 2 );

	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );

	// small stars animation counter
	float anim_counter;

	// glim animation modifier
	bool glim_mod;
	// glim animation counter
	float glim_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
