/***************************************************************************
 * enemy.h  -  headers for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2009 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_ENEMY_H
#define SMC_ENEMY_H

#include "../objects/objectsprite.h"
#include "../core/framerate.h"
#include "../audio/audio.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemy : public cImageObjectSprite
{
public:
	// constructor
	cEnemy( float x = 0, float y = 0 );
	// destructor
	virtual ~cEnemy( void );

	// load from savegame
	virtual void Load_From_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_To_Savegame( void );

	// Set Dead
	virtual void Set_Dead( bool enable = 1 );

	// death animation step
	virtual void DieStep( void );
	// handle basic enemy updates
	virtual void Update( void );
	/* late update
	 * use if it is needed that other objects are already updated
	*/
	virtual void Update_Late( void );
	// update gravity velocity
	virtual void Update_Gravity( void );
	
	// Generates the default Hit Animation Particles
	void Generate_Hit_Animation( cParticle_Emitter *anim = NULL );

	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// handle moved out of Level event
	virtual void Handle_out_of_Level( ObjectDirection dir );

	// if dead
	bool dead;

	// counter for the walking animation
	float walk_count;
	// default counter for animations
	float counter;

	// sound filename if got killed
	std::string kill_sound;
	// points if enemy got killed
	unsigned int kill_points;

	// is fire able to kill this enemy
	bool fire_resistant;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
