/***************************************************************************
                sprite.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "../globals.h"
#include "../collision.h"

/* *** *** *** *** *** *** *** ## Movement states ## *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	STAY	= 0,
	WALK	= 1,
	RUN		= 2,
	FALL	= 3,
	FLY		= 4,
	JUMP	= 5,
	CLIMB	= 6
} Moving_state;

/* *** *** *** *** *** *** *** ## Sprite Types ## *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	TYPE_UNDEFINED = 0,
	// global
	TYPE_SPRITE = 1,
	TYPE_ENEMY = 2,
	TYPE_PLAYER = 3,
	TYPE_ACTIVESPRITE = 4,
	TYPE_HALFMASSIVE = 5,
	// level
	TYPE_LEVELEXIT = 18,
	TYPE_ENEMYSTOPPER = 20,
	TYPE_BONUSBOX_MUSHROOM_FIRE = 26,
	TYPE_BONUSBOX_LIVE = 31,
	TYPE_BONUSBOX_JSTAR = 40,
	TYPE_GOLDBOX = 21,
	TYPE_SPINBOX = 27,
	// special
	TYPE_FIREBALL = 28,
	TYPE_CLOUD = 7,
	TYPE_MOVING_PLATFORM = 38,
	TYPE_MOUSECURSOR = 100,
	// enemy
	TYPE_GOOMBA = 10,
	TYPE_TURTLE = 19,
	TYPE_JPIRANHA = 29,
	TYPE_BANZAI_BILL = 30,
	TYPE_REX = 36,
	// items
	TYPE_POWERUP = 23,
	TYPE_MUSHROOM_DEFAULT = 25,
	TYPE_MUSHROOM_LIVE_1 = 35,
	TYPE_FIREPLANT = 24,
	TYPE_BOUNCINGGOLDPIECE = 22,
	TYPE_GOLDPIECE = 8,
	TYPE_MOON = 37,
	TYPE_JSTAR = 39,
	// HUD
	TYPE_STATUSTEXT = 9,
	TYPE_POINTDISPLAY = 12,
	TYPE_GAMETIMEDISPLAY = 13,
	TYPE_DEBUGDISPLAY = 14,
	TYPE_LIFEDISPLAY = 15,
	TYPE_GOLDDISPLAY = 16,
	TYPE_MENUBG = 17,
	TYPE_ITEMBOXDISPLAY = 32,
	// dialog
	TYPE_DIALOG_ALL = 200,
	TYPE_DIALOG_ONLY_LETTERS = 201,
	TYPE_DIALOG_ONLY_NUMBERS = 202
} SpriteType;

/* *** *** *** *** *** *** *** ## Array Types ## *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	ARRAY_UNDEFINED = 0,
	// level default
	ARRAY_MASSIVE = 1,
	ARRAY_PASSIVE = 2,
	ARRAY_ENEMY = 3,
	ARRAY_ACTIVE = 4,
	// hud
	ARRAY_HUD = 5,
	// animation
	ARRAY_ANIM = 6,
	// dialog
	ARRAY_DIALOG = 7
} ArrayType;

/* *** *** *** *** *** *** *** Massive Types *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	MASS_PASSIVE = 0,
	MASS_MASSIVE = 1,
	MASS_HALFMASSIVE = 2,
	MASS_CLIMBABLE = 3
} MassiveType;


/* *** *** *** *** *** *** *** Sprite *** *** *** *** *** *** *** *** *** *** */

class cSprite
{
public:
	// if del_img is set the given image will be deleted
	cSprite( GL_Surface *new_image = NULL, double x = 0, double y = 0, bool del_img = 0, Color *shadow = NULL, bool no_cam = 0 );
	virtual ~cSprite( void );

	/* Sets the image for drawing
	 * if new_Startimage is set the default Startimage will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	void SetImage( GL_Surface *new_image, bool new_start_image = 0, bool del_img = 0 );
	// Sets the Position
	void SetPos( double x, double y, bool new_startpos = 0 );
	// Set the shadow
	void SetShadow( Color *shadow );
	// Set camera
	void SetCamera( bool enabled );
	// Sets the X rotation
	void SetRotationX( double rot );
	// Sets the Y rotation
	void SetRotationY( double rot );
	// Sets the Z rotation
	void SetRotationZ( double rot );
	// Sets the rotation
	void SetRotation( double x, double y, double z );
	// Set the direction
	void Set_Direction( ObjectDirection dir, bool new_start_direction = 0 );
	// Sets this sprite on top of the given one
	void Set_onTop( cSprite *sprite, bool optimize_hor_pos = 1 );

	// Adds X rotation
	void AddRotationX( double rot );
	// Adds Y rotation
	void AddRotationY( double rot );
	// Adds Z rotation
	void AddRotationZ( double rot );
	// Adds rotation
	void AddRotation( double x, double y, double z );

	// automatically slows down until not moving anymore
	void Auto_SlowDown( double x_speed, double y_speed = 0 );

	// Returns the Collision direction if not forced
	ObjectDirection Move( double move_x, double move_y, bool real = 0, bool force = 0, bool check_onground = 1 );
	// Adds Velocity
	void AddVel( double move_x, double move_y, bool real = 0 );
	// Walk into the opposite X Direction
	void Turn_Around( void );

	// Draw the Sprite
	virtual void Draw( void );

	// Checks if the Sprite is visible on the screen
	bool isVisible_onScreen( void ); 
	// Checks if this sprite is on top of the given object
	bool is_onTop( cSprite *obj );
	// Checks if the Object is in player range
	bool isPlayer_range( void );

	/* checks if the given Position is valid
	 * uses the image collision rect
	*/
	ObjectCollisionType ColCheckAuto( double x, double y, double w = 0, double h = 0, bool only_check = 0 );
	/* checks if the given Position is valid
	 * automatically creates a collision rect for the given width and height
	*/
	ObjectCollisionType ColCheck( double x, double y, double w = 0, double h = 0, bool only_check = 0 );
	/* checks if the given Position is valid
	 * uses the given rect as the source collision rect
	*/
	ObjectCollisionType ColCheck( GL_rect *rect1, bool only_check = 0 );
	// Sets the ground object
	void Set_onGround( cObjectCollision *collision );
	// Checks if the Object is onground and sets the state to onground
	void Check_onGround( void );
	// object looses onground state
	void Reset_onGround( void );
	// Corrects the position if the object got stuck
	void Update_antistuck( void );

	// default collision and movement handling
	void CollideMove( void );
	/* Adds a new collision to the collision list
	 * base and col pointers are only needed if the collision direction is needed
	*/
	void Add_Collision( cSprite *base, cSprite *col, unsigned int col_num = 0, ObjectCollisionType col_type = CO_NOTHING );
	// Deletes the last collision
	void Delete_lastCollision( void );
	/* checks if a collision is in the given direction 
	 * and returns the collisionobject number else -1
	 */
	int Check_CollisionDir( ObjectDirection dir );
	// returns the first found collision
	cObjectCollision *Get_firstCollision( void );
	// returns the last found collision
	cObjectCollision *Get_lastCollision( void );
	// Clears the Collision informations
	void ClearCollisions( void );

	// deletes this sprite on the next frame start
	void Destroy( void );

	/* usually is a death or a state downgrade
	 * force : if given no downgrade if available will be handled
	 * the object will be completely downgraded
	*/
	virtual void DownGrade( bool force = 0 );

	// default update
	virtual void Update( void );

	// updates the direction on the current velocity
	void Update_Direction( void );
	// sets the sprite rotation if walking
	void Update_Rotation_velx( void );

	// default collision handler
	virtual void HandleCollision( cObjectCollision *collision );
	// collision from player
	virtual void PlayerCollision( ObjectDirection cdirection );
	// collision from an enemy
	virtual void EnemyCollision( ObjectDirection cdirection );
	// collision from a box
	virtual void BoxCollision( ObjectDirection cdirection, GL_rect *r2 );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor deactivation
	virtual void Editor_Deactivate( void );
	// leveleditor position update
	virtual void Editor_pos_update( void );
	// leveleditor value update
	virtual void Editor_val_update( void );

	GL_Surface *image; // current image
	GL_Surface *start_image; // editor / first image

	GL_rect rect; // complete image rect
	GL_rect start_rect; // editor / first image rect
	GL_rect col_rect; // collision rect
	GL_point col_pos; // collision start point

	// current position
	double posx, posy;
	// start position
	double startposx, startposy;
	// velocity
	double velx, vely;
	// rotation
	double rotz, rotx, roty;
	// scale
	double scale;

	// sprite type
	SpriteType type;
	// sprite Array type
	ArrayType array;

	// current direction
	ObjectDirection direction;
	// start direction
	ObjectDirection start_direction;

	// colliding ground object
	cSprite *groundObject;

	// massive collision type
	MassiveType massivetype;

	// object collision list
	ObjectCollisionList collisions;

	// is it visible
	bool visible;
	// if this Object got spawned it shouldn't be saved
	bool spawned;
	// range to the player to get updates
	unsigned int player_range;

	/* the different states
	 * look at the definitions
	 */
	Moving_state state;

	// the unique random ID
	unsigned int ID;

	// if true delete this sprite on the next frame
	bool destroy;
	// delete the given image
	bool delete_image;
	// disable the camera
	bool no_camera;
	// shadow color
	Color *shadow_color;
};

/* *** *** *** *** *** *** *** Objects Sprite *** *** *** *** *** *** *** *** *** *** */

class cObjectSprite : public cSprite
{
public:
	cObjectSprite( double x, double y );
	virtual ~cObjectSprite( void );
	
	void SetImage( int num, bool new_Startimage = 0 );

	// Current image array number for drawing
	int curr_img;
	// Image list
	typedef vector<GL_Surface *> ObjectImageList;
	ObjectImageList images;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
