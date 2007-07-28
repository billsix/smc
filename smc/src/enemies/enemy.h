/***************************************************************************
                enemy.h  -  headers for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "../objects/objectsprite.h"
#include "../core/framerate.h"
#include "../audio/audio.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemy : public cImageObjectSprite
{
public:
	cEnemy( float x = 0, float y = 0 );
	virtual ~cEnemy( void );

	// Set Dead
	virtual void Set_Dead( bool enable = 1 );

	// death animation step
	virtual void DieStep( void );
	// handle basic enemy updates
	virtual void Update( void );
	// update gravity velocity
	virtual void Update_Gravity( void );
	
	// Generates the default Hit Animation Particles
	void Generate_Hit_Animation( cParticleAnimation *anim = NULL );

	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	// if dead
	bool dead;

	// counter for the walking animation
	float walk_count;
	// default counter for animations
	float counter;

	// sound filename if got killed
	string kill_sound;
	// points if enemy got killed
	unsigned int kill_points;

    // is fire able to kill enemy
    bool fire_resistant;
};

#endif
