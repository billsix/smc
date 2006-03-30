/***************************************************************************
			collision.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2006 FluXy
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

#include "globals.h"
#include "objects/sprite.h"

#ifndef _COLLISION_H_
#define _COLLISION_H_

/* *** *** *** *** *** *** *** collision type *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	CO_NOTHING = 0,
	CO_MASSIVE = 1,
	CO_ACTIVE = 2,
	CO_ENEMY = 3,
	CO_PLAYER = 4,
	CO_PASSIVE = 5,
	CO_EDITOR_ITEM = 6,
	CO_EDITOR_MENU = 7,
	CO_EDITOR_SCROLL = 8,
	CO_DIALOG = 10
} ObjectCollisionType;

/* *** *** *** *** *** *** *** collision description *** *** *** *** *** *** *** *** *** *** */

class cSprite;

// Class: cObjectCollision
//  Object Collision.
class cObjectCollision
{
public:
	cObjectCollision( void );
	~cObjectCollision( void );

	/* Function: Set_direction
	 * sets the collision direction
	 *
	 * Parameters:
	 * base - the base sprite
	 * col - the colliding sprite
	*/
	void Set_direction( cSprite *base, cSprite *col );
	// Function: Get_ColObject_rect
	// returns the colliding object rect
	GL_rect Get_ColObject_rect( void );

	// Int: number
	// volliding object number	
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
bool Col_Circle( double x1, double y1, double r1, double x2, double y2, double r2, int offset = 1);
/* Function: Col_Circle
 * a circle intersection detection algorithm that will use
 * the position of the centre of the surface as the centre of
 * the circle and approximate the radius using the width and height
 * of the surface (for example a rect of 4x6 would have r = 2.5).
*/
bool Col_Circle( GL_Surface *surface1, SDL_Rect *a, GL_Surface *surface2, SDL_Rect *b, int offset = 0 );
bool Col_Circle( GL_Surface *a, double x1, double y1, GL_Surface *b, double x2, double y2, int offset );

// Function: distance_to_line
// returns the distance from a point to the given line
double distance_to_line( double x, double y, GL_line *line );
// Function: get_lineintersection
// returns the intersection point between the lines
GL_point get_lineintersection( GL_line *line1, GL_line *line2 );
// Function: Col_line
// returns true if the lines intersect
bool Col_line( GL_line *line1, GL_line *line2, bool collinear_intersect = 0 );

#endif
