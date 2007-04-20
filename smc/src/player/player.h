/***************************************************************************
                player.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
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

#include "../core/globals.h"
#include "../objects/ball.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** *** Maryo states *** *** *** *** *** *** *** *** *** */

enum Maryo_type
{
	// normal
	MARYO_DEAD = 0,
	MARYO_SMALL = 1,
	MARYO_BIG = 2,
	MARYO_FIRE = 3,
	MARYO_ICE = 4,
	MARYO_CAPE = 5,
	MARYO_GHOST = 6,
	MARYO_DOG_SMALL = 21 // todo
};

/* *** *** *** *** *** *** *** *** Maryo image array positions *** *** *** *** *** *** *** *** *** */

enum Maryo_imgpos
{
	MARYO_IMG_STAND = 0,
	MARYO_IMG_WALK = 0, // walking also uses the stay image
	MARYO_IMG_RUN = 8,
	MARYO_IMG_FALL = 12,
	MARYO_IMG_JUMP = 14,
	MARYO_IMG_DEAD = 16,
	MARYO_IMG_DUCK = 18,
	MARYO_IMG_CLIMB = 20,
	MARYO_IMG_THROW = 22,
	MARYO_IMG_FLY = 26,
	MARYO_IMG_SPECIAL_1 = 34
};

/* *** *** *** *** *** *** *** Player class *** *** *** *** *** *** *** *** *** *** */

class cPlayer : public cImageObjectSprite
{
public:
	// constructor
	cPlayer( float x = 120, float y = 0 );
	// destructor
	virtual ~cPlayer( void );

	// copy
	virtual cPlayer *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// lets the Player hold in
	void Hold( void );

	/* Set the direction
	 * if new_start_direction is set also set the start/editor direction
	*/
	virtual void Set_Direction( ObjectDirection dir, bool new_start_direction = 0 );

	// downgrade maryo's state ( if small maryo : dies )
	void DownGrade( bool handle = 0 );

	// moves in the current direction
	void Move_Player( float velocity, float vel_wrongway );

	// Update the walking
	void Update_walking( void );
	// Update the running
	void Update_running( void );
	// Update the staying
	void Update_staying( void );
	// Update the flying
	void Update_flying( void );
	/* Stop flying
	 * parachute : if set fall slow with parachute
	*/
	void Stop_flying( bool nparachute = 1 );
	// Start falling
	void Start_falling( void );
	// Update falling
	void Update_falling();
	// Start ducking
	void Start_ducking( void );
	// Stop ducking
	void Stop_ducking( void );
	// Update ducking
	void Update_ducking( void );
	// Start climbing
	void Start_climbing( void );
	// Update the climbing
	void Update_climbing( void );
	// Set up the upkeytime for better jumping detection
	void Start_jump_keytime( void );
	// Update the upkeytime and start a jump if possible
	void Update_jump_keytime( void );
	// Start a jump with the given values
	void Start_jump( float power = 17, float accel = 4, float deaccel = 0.08f );
	// Update jump
	void Update_jump( void );

	// Update the active item
	void Update_item( void );

	/* Release the Active Item
	 * if set_position is set the active item will be set in front of the player
	 * if no_action is set the item will just be released
	*/
	void Release_item( bool set_position = 1, bool no_action = 0 );

	/* Set the Type
	 * animate : show type change animation
	 * sound : define if the upgrade sound should be played
	*/
	void Set_Type( Maryo_type mtype, bool animation = 1, bool sound = 1 );
	void Set_Type( SpriteType item_type, bool animation = 1, bool sound = 1 );
	// sets the moving state
	void Set_MovingState( Moving_state new_state );

	// default update
	virtual void Update( void );
	/* Updates the Camera
	 * set smooth_x for slow moving into the new position
	*/
	void Update_Camera( bool smooth_x = 0 );

	// Draw the current image
	virtual void Draw( cSurfaceRequest *request = NULL );
	// Draw the player
	void Draw_Player( cSurfaceRequest *request = NULL );
	// Draw an animation using the given new type
	void Draw_Animation( Maryo_type new_mtype );
	// Returns the current image number without the direction modifier
	unsigned int Get_Image( void );

	// Loads the images depending on maryo_type
	void Load_Images( void );

	/* Sets the best position to advance in size
	 * if only_check is set no position changes will be made
	*/
	bool ChangeSize( float x, float y, bool only_check = 0 );

	/* Player gets the given Item
	 * force : if true the player will use the item immediately
	 * and it will not be placed in the itembox
	 * base : must be given on non-item objects ( like shell )
	 */
	void Get_Item( SpriteType item_type, bool force = 0, cMovingSprite *base = NULL );
	
	// returns the current velocity modifier
	float Get_Vel_mod( void );

	// Start a jump
	void Action_Jump( bool enemy_jump = 0 );
	// Interact with items
	void Action_Interact();
	// Shoot if available
	void Action_Shoot();

	/* Adds a Fire/Ice-ball
	 * Returns 1 if successful
	*/
	bool Ball_add( ball_effect effect_type = FIREBALL_DEFAULT );
	// Destroys all Fire/Ice-balls
	void Ball_clear( void );
	
	// Resets the complete game state
	void ResetSave( void );
	/* reset maryo's state
	 * if full is set resets only level states
	*/
	void Reset( bool full = 1 );

	// Exits the level and walks to the next Overworld waypoint
	void Goto_next_Level( void );
	// Enters the given sublevel
	void Goto_SubLevel( string sublevel );
	
	// Add an enemy to the kill multiplier
	void Add_Kill_Multiplier( void );
	// Update the kill multiplier
	void Update_Kill_Multiplier( void );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// handles the collision
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Direction_Select( const EventArgs &event ); // editor direction option selected

	// current Maryo type
	Maryo_type maryo_type;
	//	Maryo type after the temporary powerup
	Maryo_type maryo_type_temp_power;

	// true if player godmode is active	
	bool godmode;

	// time maryo walked
	float walk_time;
	// running particle counter
	float running_particle_counter;
	// time maryo is in ghost mode
	float ghost_time;
	// ghost mode drawing modifier
	float ghost_time_mod;
	// time maryo gets invincible
	float invincible;
	// invincible time if maryo got a star
	float invincible_star;
	// invincible drawing modifier
	float invincible_mod;
	// invincible star counter
	float invincible_starcounter;

	// walking animation counter
	float walk_count;
	// lives left
	int lives;
	// if on the next update the downgrade should be updated
	bool update_downgrade;

	// jumping keytime
	float UpKeytime;
	// enemy jump activator
	bool start_enemyjump;
	// Acceleration if up key is pressed
	float jump_accel_up;
	// vely De-acceleration use 0.05 - 0.08
	float jump_vel_deaccel;
	// next jump power
	float jump_power;

	// throwing animation counter
	float throwing_counter;
	// if maryo is ducked
	bool ducked;
	// if parachute falling
	bool parachute;
	// goldpiece amount collected
	unsigned int goldpieces;
	// collected points
	long points;
	// enemy killed in a row multiplier
	float kill_multiplier;
	// time since the last killed enemy
	float last_kill_counter;
	// time player didn't move
	float no_velx_counter, no_vely_counter;

	// fire/ice-ball count
	unsigned int fireball_count, iceball_count;
	// active/holding object ( f.e. shell )
	cMovingSprite *active_object;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Player
extern cPlayer *pPlayer;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
