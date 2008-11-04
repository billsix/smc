/***************************************************************************
 * star.h  -  header for the corresponding cpp file
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

#ifndef SMC_STAR_H
#define SMC_STAR_H

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
