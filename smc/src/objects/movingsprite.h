/***************************************************************************
                movingsprite.h  -  header for the corresponding cpp file
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

#ifndef __MOVING_SPRITE_H__
#define __MOVING_SPRITE_H__

#include "../objects/sprite.h"

/* *** *** *** *** *** *** *** Movement states *** *** *** *** *** *** *** *** *** *** */

enum Moving_state
{
	STA_STAY	= 0,
	STA_WALK	= 1,
	STA_RUN		= 2,
	STA_FALL	= 3,
	STA_FLY		= 4,
	STA_JUMP	= 5,
	STA_CLIMB	= 6,
	// another object controls this object ( object doesn't update itself )
	STA_OBJ_LINKED = 7
};


/* *** *** *** *** *** *** *** cMovingSprite *** *** *** *** *** *** *** *** *** *** */

class cMovingSprite : public cSprite
{
public:
	// if del_img is set the given image will be deleted on change or class deletion
	cMovingSprite( GL_Surface *new_image = NULL, float x = 0, float y = 0, bool del_img = 0 );
	// create from stream
	cMovingSprite( XMLAttributes &attributes );
	// destructor
	virtual ~cMovingSprite( void );
	
	// Init defaults
	void Init( void );
	// copy this object
	virtual cMovingSprite *Copy( void );

	/* Sets the image for drawing
	 * if new_start_image is set the default start_image will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	virtual void Set_Image( GL_Surface *new_image, bool new_start_image = 0, bool del_img = 0 );

	/* Set the direction
	 * if new_start_direction is set also set the start/editor direction
	*/
	virtual void Set_Direction( ObjectDirection dir, bool new_start_direction = 0 );
	// Sets the direction/velocity from the given angle and speed
	void Set_Direction( float angle, float speed, bool new_start_direction = 0 );

	// automatically slows down until not moving anymore
	void Auto_SlowDown( float x_speed, float y_speed = 0 );

	/* Moves this object
	 * real : if set the speedfactor is not used
	*/
	virtual void Move( float move_x, float move_y, bool real = 0 );
	/* Returns the Collision direction if not forced
	 * if real is set the speedfactor will not be used
	 * if force is set no collision detection will be performed
	 * if check_onground is set after moving onground will be checked
	*/
	virtual ObjectDirection Col_Move( float move_x, float move_y, bool real = 0, bool force = 0, bool check_onground = 1 );
	// Add Velocity
	void AddVel( float move_x, float move_y, bool real = 0 );
	/* Move into the opposite Direction
	 * if col_dir is given only turns around if the collision direction is in front
	 */
	virtual void Turn_Around( ObjectDirection col_dir = DIR_UNDEFINED );

	// sprite Update
	virtual void Update( void );
	/* Draw
	* if request is NULL automatically creates the request
	*/
	virtual void Draw( cSurfaceRequest *request = NULL );

	/* Check if the given Position is valid
	 * uses the image collision rect
	*/
	cObjectCollisionType ColCheckAuto( float x, float y, float w = 0, float h = 0, ColCheckType check_type = COLLIDE_COMPLETE );
	/* Check if the given Position is valid
	 * automatically creates a collision rect for the given width and height
	*/
	cObjectCollisionType ColCheck( float x = 0, float y = 0, float w = 0, float h = 0, ColCheckType check_type = COLLIDE_COMPLETE );
	/* Check if the given Position is valid
	 * uses the given rect as the source collision rect
	*/
	cObjectCollisionType ColCheck( GL_rect *rect1, ColCheckType check_type = COLLIDE_COMPLETE );
	/* Check if the given movement goes out of the level rect
	 * handle : if set handle out of level if valid
	*/
	bool Check_OutofLevel( float move_x, float move_y, bool handle = 0 );
	// Sets the ground object
	void Set_onGround( cObjectCollision *collision );
	// Check if the Object is onground and sets the state to onground
	virtual void Check_onGround( void );
	// object looses onground state
	void Reset_onGround( void );
	// Corrects the position if the object got stuck
	void Update_antistuck( void );

	// default collision and movement handling
	void CollideMove( void );


	/* Freeze for the given time
	*/
	void Freeze( float freeze_time = DESIRED_FPS * 10 );

	/* usually is a death or a state downgrade
	 * force : if given no downgrade if available will be handled
	 * the object will be completely downgraded
	*/
	virtual void DownGrade( bool force = 0 );

	// updates the direction on the current velocity
	void Update_Direction( void );
	/* sets the sprite rotation if walking
	 * if start_rotation is set also set the start/editor rotation
	*/
	void Update_Rotation_velx( bool start_rotation = 0 );

	/* Validate the given collision object
	 * returns 1 if the given object collides with this object
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// returns -1 if nothing found
	virtual int Validate_Collision_Ghost( cSprite *obj );
	// send a collision handler to an object based from the given collision
	virtual void Send_Collision( cObjectCollision *collision );
	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// handle moved out of Level
	virtual void Handle_OutofLevel( ObjectDirection dir );

	// velocity
	float velx, vely;

	// current direction
	ObjectDirection direction;
	// start direction
	ObjectDirection start_direction;

	// colliding ground object
	cSprite *ground_object;

	/* the different states
	 * look at the definitions
	 */
	Moving_state state;

	// ice resistance
	float ice_resistance;
	// time counter if frozen
	float freeze_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
