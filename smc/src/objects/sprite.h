/***************************************************************************
                sprite.h  -  header for the corresponding cpp file
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

#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "../core/globals.h"
#include "../core/math/rect.h"
#include "../video/video.h"
#include "../core/collision.h"

/* *** *** *** *** *** *** *** cCollidingSprite *** *** *** *** *** *** *** *** *** *** */

class cCollidingSprite
{
public:
	cCollidingSprite( void );
	virtual ~cCollidingSprite( void );

	// Parses the collision data
	void Parse_Collisions( void );
	// Handle collision data
	void Handle_Collisions( void );
	/* Add a collision to the collision list
	 * returns true if successful
	 *
	 * base : base/origin object
	 * col : colliding object ( only needed if the collision direction is needed )
	 * col_id : colliding object identifier
	 * col_type : collision object type
	 * add_if_new : add the collision only if the object doesn't already collide
	*/
	bool Collision_Add( cSprite *base, cSprite *col, unsigned int col_id = 0, ObjectCollisionType col_type = CO_NOTHING, bool add_if_new = 0 );
	/* Add a collision object to the collision list
	 * returns true if successful
	 * add_if_new : add the collision only if the collision object doesn't already collide
	 */
	bool Collision_Add( cObjectCollision *collision, bool add_if_new = 0 );
	// Delete the given collision from the list
	void Collision_Delete( cObjectCollision *collision );
	// Delete the last collision
	void Collision_Delete_last( void );
	/* Check if a collision is in the given direction 
	 * and returns the collision object number else -1
	 */
	int Collision_Check_Direction( ObjectDirection dir );
	// Returns the first added collision
	cObjectCollision *Collision_Get_first( void );
	/* Returns the last added collision
	 * if only_blocking is set only movement blocking collisions are returned
	*/
	cObjectCollision *Collision_Get_last( bool only_blocking = 0 );
	// Clear the Collision list
	void Collisions_Clear( void );

	// default collision handler
	virtual void Handle_Collision( cObjectCollision *collision );
	// collision from player
	virtual void Handle_Collision_Player( ObjectDirection cdirection );
	// collision from an enemy
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );
	// collision with massive
	virtual void Handle_Collision_Massive( cObjectCollision *collision );
	// collision from a box
	virtual void Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 );

	// object collision list
	ObjectCollisionList collisions;
};

/* *** *** *** *** *** *** *** cSprite *** *** *** *** *** *** *** *** *** *** */

class cSprite : public cCollidingSprite
{
public:
	// if del_img is set the given image will be deleted on change or class deletion
	cSprite( GL_Surface *new_image = NULL, float x = 0, float y = 0, bool del_img = 0 );
	// create from stream
	cSprite( XMLAttributes &attributes );
	// destructor
	virtual ~cSprite( void );

	// Init defaults
	void Init( void );
	// copy this sprite
	virtual cSprite *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	/* Sets the image for drawing
	 * if new_start_image is set the default start_image will be set to the given image
	 * if del_img is set the given image will be deleted
	*/
	virtual void Set_Image( GL_Surface *new_image, bool new_start_image = 0, bool del_img = 0 );

	// Set the sprite type
	void Set_Sprite_Type( SpriteType ntype );
	// Returns the sprite type as string
	string Get_Sprite_Type_String( void );

	// Set if the camera should be ignored
	void Set_Ignore_Camera( bool enable = 0 );

	// Sets the Position
	void Set_Pos( float x, float y, bool new_startpos = 0 );
	void Set_PosX( float x, bool new_startpos = 0 );
	void Set_PosY( float y, bool new_startpos = 0 );
	// Set if visible
	void Set_Visible( bool enabled );
	/* Set the shadow
	 * if position is set to 0 the shadow is disabled
	*/
	void Set_Shadow( Color shadow, float pos );
	/* Set the shadow position
	 * if set to 0 the shadow is disabled
	*/
	void Set_Shadow_Pos( float pos );
	// Set the shadow color
	void Set_Shadow_Color( Color shadow );
	// Set image color
	void Set_Color( Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255 );
	void Set_Color( Color col );
	/* Set a Color Combination ( GL_ADD, GL_MODULATE or GL_REPLACE )
	 * Addition ( adds white to color )
	 * 1.0 is the maximum and the given color will be white
	 * 0.0 is the minimum and the color will have the default color
	 * Modulation ( adds black to color )
	 * 1.0 is the maximum and the color will have the default color
	 * 0.0 is the minimum and the given color will be black
	 * Replace ( replaces color value )
	 * 1.0 is the maximum and the given color has maximum value
	 * 0.0 is the minimum and the given color has minimum value
	*/
	void Set_Color_Combine( float red, float green, float blue, GLint com_type );
	// Set the X rotation
	void Set_RotationX( float rot, bool new_start_rot = 0 );
	// Set the Y rotation
	void Set_RotationY( float rot, bool new_start_rot = 0 );
	// Set the Z rotation
	void Set_RotationZ( float rot, bool new_start_rot = 0 );
	// Set the rotation
	void Set_Rotation( float x, float y, float z, bool new_start_rot = 0 );

	// Add X rotation
	void Add_RotationX( float rot );
	// Add Y rotation
	void Add_RotationY( float rot );
	// Add Z rotation
	void Add_RotationZ( float rot );
	// Add rotation
	void Add_Rotation( float x, float y, float z );

	// Set the scale
	void Set_ScaleX( float scale );
	void Set_ScaleY( float scale );
	void Set_Scale( float scale );

	// Add scale
	void Add_ScaleX( float val );
	void Add_ScaleY( float val );
	void Add_Scale( float val );

	// Set this sprite on top of the given one
	void Set_onTop( cSprite *sprite, bool optimize_hor_pos = 1 );

	/* Move this object
	 * real : if set the speedfactor is not used
	*/
	virtual void Move( float move_x, float move_y, bool real = 0 );

	// default collision and movement handling
	virtual void Collide_Move( void );

	// Update the position rect values
	void Update_Position_Rect( void );
	// default update
	virtual void Update( void );
	// update drawing validation
	virtual void Update_valid_draw( void );
	// update updating validation
	virtual void Update_valid_update( void );

	/* Draw
	* if request is NULL automatically creates the request
	*/
	virtual void Draw( cSurfaceRequest *request = NULL );
	/* only draws the image
	 * no position nor debug updates
	*/
	void Draw_Image( cSurfaceRequest *request = NULL );

	/* Set the massive type
	 * should be called after setting the new array
	*/
	virtual void Set_Massivetype( MassiveType mtype );

	// Check if this sprite is on top of the given object
	bool is_onTop( cSprite *obj );

	// if the sprite is visible on the screen
	bool is_Visible_onScreen( void );
	// if the Object is in player range
	bool is_Player_range( void );
	// if update is valid for the current state
	virtual bool is_Update_valid( void );
	// if draw is valid for the current state and position
	virtual bool is_Draw_valid( void );

	// deletes this sprite on the next frame start
	virtual void Destroy( void );

	// editor add window object
	void Editor_Add( Window *obj, float obj_width, float obj_height = 25 );
	// editor activation
	virtual void Editor_Activate( void );
	// editor deactivation
	virtual void Editor_Deactivate( void );
	// editor position update
	virtual void Editor_pos_update( void );
	// editor mouse hover
	bool Editor_Mouse_Enter( const EventArgs &event );
	bool Editor_Mouse_Leave( const EventArgs &event );

	// current image used for drawing
	GL_Surface *image;
	// editor/first image
	GL_Surface *start_image;

	// complete image rect
	GL_rect rect;
	// editor/first image rect
	GL_rect start_rect;
	// collision rect
	GL_rect col_rect;
	// collision start point
	GL_point col_pos;

	// current position
	float posx, posy, posz;
	// start position
	float startposx, startposy;
	/* editor z position
	 * will be only used if not 0
	*/
	float editor_posz;
	// editor/start rotation
	float start_rotx, start_roty, start_rotz;
	// rotation
	float rotx, roty, rotz;
	// editor/start scale
	float start_scalex, start_scaley;
	// scale
	float scalex, scaley;

	// color
	Color color;
	// combine type
	GLint combine_type;
	// combine color
	float combine_col[3];

	// sprite type
	SpriteType type;
	// sprite array type
	ArrayType sprite_array;
	// massive collision type
	MassiveType massivetype;
	// sprite visible name
	string name;
	// sprite editor tags
	string editor_tags;

	// disable the camera
	bool no_camera;
	// is it visible
	bool visible;
	// if spawned it shouldn't be saved
	bool spawned;
	// range to the player to get updates
	unsigned int player_range;
	// can be used as ground object
	bool can_be_ground;

	// delete the given image when it gets unloaded
	bool delete_image;
	// if true delete this sprite on the next frame
	bool destroy;
	// shadow position
	float shadow_pos;
	// shadow color
	Color shadow_color;

	// if drawing is valid
	bool valid_draw;
	// if updating is valid
	bool valid_update;

	// editor active window list
	typedef vector<Window *> WindowList;
	WindowList editor_windows;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef vector<cSprite *> SpriteList;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
