/***************************************************************************
                fireball.h  -  header for the corresponding cpp file
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

#ifndef __FIREBALL_H__
#define __FIREBALL_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** Fireball Effect types *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	FIREBALL_DEFAULT,
	FIREBALL_EXPLOSION
} FireballEffect;

/* *** *** *** *** *** *** *** Fireball class *** *** *** *** *** *** *** *** *** *** */

class cFireball : public cSprite
{
public:
	cFireball( double x, double y, double velocity_x, double velocity_y = 0 );
	virtual ~cFireball( void );

	// sets the direction angle and speed
	void SetDirection( double angle, double speed );

	virtual void Update( void );
	// handles the collision
	virtual void HandleCollision( cObjectCollision *collision );

	// fireball images
	typedef vector<GL_Surface *> FireballImageList;
	FireballImageList images;

	// rotation animation counter
	double counter;
};

#endif
