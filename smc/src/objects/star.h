/***************************************************************************
                star.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 by FluXy
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

#include "../globals.h"
#include "powerup.h"

class cjStar : public cPowerUp
{
public:
	cjStar( double x, double y );
	virtual ~cjStar( void );
	
	virtual void Update( void );
	// Activate the star
	void Activate( void );
	// Adds Star Particles
	void Generate_Particles( double x = 0, double y = 0, unsigned int power = 2 );

	// stars animation counter
	double anim_counter;

	// handles the collision
	virtual void HandleCollision( cObjectCollision *collision );
};

#endif