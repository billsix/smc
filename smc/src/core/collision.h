/***************************************************************************
			collision.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2007 Florian Richter
						   (C) 2005 Amir Taaki ( Circle Collision tests )
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "../core/globals.h"

/* *** *** *** *** *** *** *** collision type *** *** *** *** *** *** *** *** *** *** */

enum ObjectCollisionType
{
	CO_NOTHING = 0,
	CO_MASSIVE = 1,
	CO_ACTIVE = 2,
	CO_ENEMY = 3,
	CO_PLAYER = 4,
	CO_PASSIVE = 5
};

typedef vector<ObjectCollisionType> ObjectCollisionTypeList;

/* *** *** *** *** *** *** *** collision check type *** *** *** *** *** *** *** *** *** *** */

enum ColCheckType
{
	COLLIDE_ONLY_CHECK = 0,
	COLLIDE_ALLOW_INTERNAL = 1,
	COLLIDE_ALLOW_BLOCKING = 2,
	COLLIDE_COMPLETE = 3
};

/* *** *** *** *** *** *** *** collision type class *** *** *** *** *** *** *** *** *** *** */

class cObjectCollisionType
{
public:
	cObjectCollisionType( void );
	~cObjectCollisionType( void );

	// add a Object Collision Type
	void add( ObjectCollisionType type );
	// returns true if the given Object Collision type was found
	bool find( ObjectCollisionType type );
	// returns the found collision size
	unsigned int size( void );

	ObjectCollisionTypeList list;
};

/* *** *** *** *** *** *** *** cObjectCollision *** *** *** *** *** *** *** *** *** *** */

// Object collision data
class cObjectCollision
{
public:
	cObjectCollision( void );
	~cObjectCollision( void );

	/* Function: Set_direction
	 * Set the collision direction
	 * base - the base sprite
	 * col - the colliding sprite
	*/
	void Set_direction( cSprite *base, cSprite *col );
	// Function: Get_ColObject_rect
	// returns the colliding object rect
	GL_rect Get_ColObject_rect( void );


	/* if true this is a receiving collision
	 * and not a normal self detected collision
	*/
	bool received;

	// Int: number
	// colliding object number
	int number;

	// Variable: direction
	// direction
	ObjectDirection direction;
	// Variable: type
	// Colliding Object type
	ObjectCollisionType type;
};

typedef vector<cObjectCollision *> ObjectCollisionList;

/* *** *** *** *** *** *** *** functions *** *** *** *** *** *** *** *** *** *** */

/* Returns the collision direction
 * base - the base sprite
 * col - the colliding sprite
*/
ObjectDirection Get_col_direction( cSprite *base, cSprite *col );

// returns true if a collision is on the top
bool Is_collision_top( GL_rect *base_rect, GL_rect *col_rect );
// returns true if a collision is on the bottom
bool Is_collision_bottom( GL_rect *base_rect, GL_rect *col_rect );
// returns true if a collision is on the left
bool Is_collision_left( GL_rect *base_rect, GL_rect *col_rect );
// returns true if a collision is on the right
bool Is_collision_right( GL_rect *base_rect, GL_rect *col_rect );

/* Returns the Collision Type from the Array Type
 * if unknown returns CO_NOTHING
*/
ObjectCollisionType Get_CollisionType( ArrayType array_type );

/* Function: Col_Box
 * Bounding box collision test
 * Checks if the first rect intersects with the second
*/
bool Col_Box( GL_rect *a, GL_rect *b );
bool Col_Box( SDL_Rect *a, GL_rect *b );

/* Function: Col_Box_full
 * Bounding box collision test (SDL_Rect)
 * Checks if the first rect intersects completely with the second
*/
bool Col_Box_full( SDL_Rect *a, SDL_Rect *b );

/* Function: Col_Circle
 * tests whether 2 circles intersect
 *
 * Parameters:
 * circle1 - center (x1,y1) with radius r1
 * circle2 - center (x2,y2) with radius r2
 * (allow distance between circles of offset)
*/
bool Col_Circle( float x1, float y1, float r1, float x2, float y2, float r2, int offset = 1);
/* Function: Col_Circle
 * a circle intersection detection algorithm that will use
 * the position of the center of the surface as the center of
 * the circle and approximate the radius using the width and height
 * of the surface (for example a rect of 4x6 would have r = 2.5).
*/
bool Col_Circle( GL_Surface *surface1, SDL_Rect *a, GL_Surface *surface2, SDL_Rect *b, int offset = 0 );
bool Col_Circle( GL_Surface *a, float x1, float y1, GL_Surface *b, float x2, float y2, int offset );

// Function: distance_to_line
// returns the distance from a point to the given line
float distance_to_line( float x, float y, GL_line *line );
// Function: get_lineintersection
// returns the intersection point between the lines
GL_point get_lineintersection( GL_line *line1, GL_line *line2 );
// Function: Col_line
// returns true if the lines intersect
bool Col_line( GL_line *line1, GL_line *line2, bool collinear_intersect = 0 );

#endif
