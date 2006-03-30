/***************************************************************************
                player.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2004 by Artur Hallmann
							(C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "globals.h"

/* *** *** *** *** *** *** *** *** Maryo states *** *** *** *** *** *** *** *** *** */

typedef enum
{
	// normal
	MARYO_DEAD = 0,
	MARYO_SMALL = 1,
	MARYO_BIG = 2,
	MARYO_FIRE = 3,
	MARYO_FEATHER = 4, // todo
	MARYO_DOG_SMALL = 21 // todo
} Maryo_type;

/* *** *** *** *** *** *** *** *** Maryo image array positions *** *** *** *** *** *** *** *** *** */

typedef enum
{
	MARYO_IMG_STAND = 0,
	MARYO_IMG_WALK = 0, // walking uses the stay image
	MARYO_IMG_FALL = 6,
	MARYO_IMG_JUMP = 8,
	MARYO_IMG_DEAD = 10,
	MARYO_IMG_DUCK = 12,
	MARYO_IMG_CLIMB = 14
} Maryo_imgpos;

/* *** *** *** *** *** *** *** Player class *** *** *** *** *** *** *** *** *** *** */

class cPlayer : public cObjectSprite
{
public:
	cPlayer( double x, double y );
	virtual ~cPlayer( void );

	// lets the Player hold in
	void Hold( void );

	// moves in the current direction
	void Move_Player( double velocity, double vel_wrongway );

	// Updates the walking
	void Update_walking( void );
	// Updates the staying
	void Update_staying( void );
	// Updates the flying
	void Update_flying( void );
	// Starts falling
	void Start_falling( void );
	// Updates the falling
	void Update_falling();
	// Lets the Player duck
	void Start_ducking( void );
	// Stops the ducking
	void Stop_ducking( void );
	// Updates the ducking
	void Update_ducking( void );
	// Starts climbing
	void Start_climbing( void );
	// Updates the climbing
	void Update_climbing( void );
	// Set up the upkeytime for better jumping detection
	void Start_jump_keytime( void );
	// Starts a jump with the given values
	void Start_jump( double power = 17.0, double accel = 4.0, double deaccel = 0.06 );
	// Updates the jump
	void Update_jump( void );

	// sets the moving state
	void Set_MovingState( Moving_state new_state );

	// default update
	void Update( void );

	// handles the collision
	virtual void HandleCollision( cObjectCollision *collision );
	// Updates the Camera
	void Update_Camera( bool init = 0 );

	// Draw the current image
	void Draw( void );
	// Draw the player with the given image number
	void Draw_Player( unsigned int imgnr );
	// Draw an animation based on the given item
	void Draw_Animation( unsigned int item_type );
	// Returns the current image number without the direction modifier
	unsigned int Get_Image( void );

	// Loads the images depending on maryo_type
	void LoadImages( bool preload = 0 );
	
	// downgrade maryo's state ( if small maryo dies )
	void DownGrade( bool handle = 0 );

	/* Sets the best position to advance in size
	 * if only_check is set no position changes will be made
	*/
	bool ChangeSize( double x, double y, bool only_check = 0 );

	/*	player gets the given Item.
	 *	if force is true the player will use the item immediality
	 *	and it will not be placed in the itembox
	 */
	void Get_Item( unsigned int item_type, bool force = 0 );
	
	// Adds a Fireball
	void Fireball_add( FireballEffect firetype = FIREBALL_DEFAULT );
	// Destroys all Fireballs
	void Fireball_clear( void );
	
	// Resets the complete game state
	void ResetSave( void );
	// resets only maryo's state
	void Reset( void );

	// Exits the level and walks to the next Overwold waypoint
	void GotoNextLevel( void );
	// Enters the given sublevel
	void GotoSubLevel( string sublevel );
	
	/*	Maryos current type
	 *  look at the definitions
	 */
	Maryo_type maryo_type;

	// true if player godmode is active	
	bool godmode;

	// time maryo gets invincible
	double invincible;
	// invincible time if maryo got a star
	double invincible_star;
	// invincible drawing modifier
	double invincible_mod;
	// invincible star counter
	double invincible_starcounter;

	// maryo's walking animation
	double walk_count;
	// lives left
	int lives;
	// if on the next update the downgrade should be updated
	bool update_downgrade;

	// jumping keytime
	double UpKeytime;
	// enemy jump activator
	bool start_enemyjump;
	// Acceleration if up key is pressed
	double jump_accel_up;
	// vely De-acceleration use 0.05 - 0.08
	double jump_vel_deaccel;
	// next jump power
	double jump_power;

	// if maryo is ducked
	bool ducked;
	// current goldpieces collected
	unsigned int goldpieces;
	// current points collected
	long points;

	// fireballs
	typedef vector<cFireball *> FireballList;
	FireballList fireballs; 
};

#endif
