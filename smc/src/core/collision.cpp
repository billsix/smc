/***************************************************************************
    collison.cpp  -  internal collision functions
                             -------------------
    copyright            : (C) 2005 - 2007 Florian Richter
						   (C) 2005 Amir Taaki
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/globals.h"
#include "../core/collision.h"
#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"
#include "../player/player.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** collision type class *** *** *** *** *** *** *** *** *** *** */

cObjectCollisionType :: cObjectCollisionType( void )
{

}

cObjectCollisionType :: ~cObjectCollisionType( void )
{
	list.clear();
}

void cObjectCollisionType :: add( cSprite *obj )
{
	if( !obj )
	{
		return;
	}

	list.push_back( obj );
}

bool cObjectCollisionType :: find( cSprite *obj )
{
	for( SpriteList::iterator itr = list.begin(), itr_end = list.end(); itr != itr_end; ++itr )
	{
		// matching
		if( (*itr) == obj )
		{
			return 1;
		}
	}

	return 0;
}

cSprite *cObjectCollisionType :: find( ArrayType type )
{
	for( SpriteList::iterator itr = list.begin(), itr_end = list.end(); itr != itr_end; ++itr )
	{
		// matching
		if( (*itr)->sprite_array == type )
		{
			return (*itr);
		}
	}

	return 0;
}

cSprite *cObjectCollisionType :: find( SpriteType type )
{
	for( SpriteList::iterator itr = list.begin(), itr_end = list.end(); itr != itr_end; ++itr )
	{
		// matching
		if( (*itr)->type == type )
		{
			return (*itr);
		}
	}

	return 0;
}

unsigned int cObjectCollisionType :: size( void )
{
	return list.size();
}

/* *** *** *** *** *** *** *** collision description *** *** *** *** *** *** *** *** *** *** */

cObjectCollision :: cObjectCollision( void )
{
	received = 0;
	type = CO_NOTHING;
	number = 0;
	direction = DIR_UNDEFINED;
}

cObjectCollision :: ~cObjectCollision( void )
{
	//
}

void cObjectCollision :: Set_direction( cSprite *base, cSprite *col )
{
	direction = Get_col_direction( base, col );
}

GL_rect cObjectCollision :: Get_ColObject_rect( void )
{
	if( type == CO_MASSIVE || type == CO_ACTIVE || type == CO_ENEMY )
	{
		return pObjManager->items[number]->col_rect;
	}
	else if( type == CO_PLAYER )
	{
		return pPlayer->col_rect;
	}

	// empty
	return GL_rect();
}

/* *** *** *** *** *** *** *** functions *** *** *** *** *** *** *** *** *** *** */

ObjectDirection Get_col_direction( cSprite *base, cSprite *col )
{
	// if valid moving sprite
	if( base->sprite_array == ARRAY_ENEMY || base->sprite_array == ARRAY_ACTIVE )
	{
		cMovingSprite *moving_base = static_cast<cMovingSprite *>(base);

		// ## velocity based detection
		if( moving_base->velx != 0 || moving_base->vely != 0 )
		{
			// top
			if( moving_base->vely < 0 && Is_collision_top( &base->col_rect, &col->col_rect ) )
			{
				return DIR_TOP;
			}
			// bottom
			else if( moving_base->vely > 0 && Is_collision_bottom( &base->col_rect, &col->col_rect ) )
			{
				return DIR_BOTTOM;
			}
			// left
			else if( moving_base->velx < 0 && Is_collision_left( &base->col_rect, &col->col_rect ) )
			{
				return DIR_LEFT;
			}
			// right
			else if( moving_base->velx > 0 && Is_collision_right( &base->col_rect, &col->col_rect ) )
			{
				return DIR_RIGHT;
			}
		}
	}


	// ## detection without velocity

	// top
	if( Is_collision_top( &base->col_rect, &col->col_rect ) )
	{
		return DIR_TOP;
	}
	// bottom
	else if( Is_collision_bottom( &base->col_rect, &col->col_rect ) )
	{
		return DIR_BOTTOM;
	}
	// left
	else if( Is_collision_left( &base->col_rect, &col->col_rect ) )
	{
		return DIR_LEFT;
	}
	// right
	else if( Is_collision_right( &base->col_rect, &col->col_rect ) )
	{
		return DIR_RIGHT;
	}

	// ## advanced detection ( if base object is partly inside col object )

	// get the middle point
	GL_point point_base = base->col_rect.Get_pos_middle();
	GL_point point_col = col->col_rect.Get_pos_middle();

	// difference between points
	float diff_x = point_col.x - point_base.x;
	float diff_y = point_col.y - point_base.y;

	// more horizontal
	if( ( diff_x > 0 && ( diff_x > diff_y ) ) || ( diff_x < 0 && ( diff_x < diff_y ) ) )
	{
		// if col object point left
		if( diff_x > 0 )
		{
			return DIR_RIGHT;
		}
		// if col object point is right
		else
		{
			return DIR_LEFT;
		}
	}
	// more vertical
	else
	{
		// if col object point is on top
		if( diff_y > 0 )
		{
			return DIR_DOWN;
		}
		// if col object point is below
		else
		{
			return DIR_UP;
		}
	}
}

bool Is_collision_top( GL_rect *base_rect, GL_rect *col_rect )
{
	if( base_rect->y + 1 >= col_rect->y + col_rect->h && base_rect->x < col_rect->x + col_rect->w && base_rect->x + base_rect->w > col_rect->x )
	{
		return 1;
	}

	return 0;
}

bool Is_collision_bottom( GL_rect *base_rect, GL_rect *col_rect )
{
	if( base_rect->y + base_rect->h - 1 <= col_rect->y && base_rect->x < col_rect->x + col_rect->w && base_rect->x + base_rect->w > col_rect->x )
	{
		return 1;
	}

	return 0;
}

bool Is_collision_left( GL_rect *base_rect, GL_rect *col_rect )
{
	if( base_rect->x + 1 >= col_rect->x + col_rect->w && base_rect->y < col_rect->y + col_rect->h && base_rect->y + base_rect->h > col_rect->y )
	{
		return 1;
	}

	return 0;
}

bool Is_collision_right( GL_rect *base_rect, GL_rect *col_rect )
{
	if( base_rect->x + base_rect->w - 1 <= col_rect->x && base_rect->y < col_rect->y + col_rect->h && base_rect->y + base_rect->h > col_rect->y )
	{
		return 1;
	}

	return 0;
}

ObjectCollisionType Get_CollisionType( ArrayType array_type )
{
	if( array_type == ARRAY_FRONT_PASSIVE || array_type == ARRAY_PASSIVE )
	{
		return CO_PASSIVE;
	}
	else if( array_type == ARRAY_MASSIVE )
	{
		return CO_MASSIVE;
	}
	else if( array_type == ARRAY_ACTIVE )
	{
		return CO_ACTIVE;
	}
	else if( array_type == ARRAY_ENEMY )
	{
		return CO_ENEMY;
	}

	return CO_NOTHING;
}

bool Col_Box( GL_rect *a, GL_rect *b )
{
	// check if their bounding boxes even touch
	if( b->x + b->w < a->x )
	{
		return 0;
	}
	if( b->x > a->x + a->w )
	{
		return 0;
	}

	if( b->y + b->h < a->y )
	{
		return 0;
	}
	if( b->y > a->y + a->h )
	{
		return 0;
	}

	// bounding boxes intersect
	return 1;
}

bool Col_Box( SDL_Rect *a, GL_rect *b )
{
	// check if their bounding boxes even touch
	if( b->x + b->w < a->x )
	{
		return 0;
	}
	if( b->x > a->x + a->w )
	{
		return 0;
	}

	if( b->y + b->h < a->y )
	{
		return 0;
	}
	if( b->y > a->y + a->h )
	{
		return 0;
	}

	// bounding boxes intersect
	return 1;
}

bool Col_Box_full( SDL_Rect *a, SDL_Rect *b )
{
	if( a->x <= b->x )
	{
		return 0;
	}
	if( a->x + a->w >= b->x + b->w )
	{
		return 0;
	}
	if( a->y <= b->y )
	{
		return 0;
	}
	if( a->y + a->h >= b->y + b->h )
	{
		return 0;
	}

	return 1;
}

// from SDL_collide ( Copyright (C) 2005 Amir Taaki )
bool Col_Circle( float x1, float y1, float r1, float x2, float y2, float r2, int offset )
{
	float xdiff = x2 - x1;	// x plane difference
	float ydiff = y2 - y1;	// y plane difference
	
	/* distance between the circles centres squared */
	float dcentre_sq = ( ydiff * ydiff ) + ( xdiff * xdiff );
	
	/* calculate sum of radiuses squared */
	float r_sum_sq = r1 + r2;	// square on seperate line, so
	r_sum_sq *= r_sum_sq;	// dont recompute r1 + r2

	return ( dcentre_sq - r_sum_sq <= ( offset * offset ) );
}

bool Col_Circle( GL_Surface *surface1, SDL_Rect *a, GL_Surface *surface2, SDL_Rect *b, int offset )
{
	return Col_Circle( surface1, a->x, a->y, surface2, b->x, b->y, offset );
}

// from SDL_collide ( Copyright (C) 2005 Amir Taaki )
bool Col_Circle( GL_Surface *a, float x1, float y1, GL_Surface *b, float x2, float y2, int offset )
{
	/* if radius is not specified
	we approximate them using SDL_Surface's
	width and height average and divide by 2*/
	float r1 = ( ( a->w + a->h ) / 4 );	// same as / 2) / 2;
	float r2 = ( ( b->w + b->h ) / 4 );

	x1 += a->w / 2;		// offset x and y
	y1 += a->h / 2;		// co-ordinates into
				// centre of image
	x2 += b->w / 2;
	y2 += b->h / 2;

	return Col_Circle( x1, y1, r1, x2, y2, r2, offset );
}

template<typename T> inline T pow2( T value ) { return value * value; }
template<typename T> inline T cl_min( T a, T b ) { if( a < b ) return a; return b; }

float distance_to_line( float x, float y, GL_line *line )
{
	float L = sqrt( pow2( line->x2 - line->x1 ) + pow2( line->y2 - line->y1 ) );
	float r = ( ( x - line->x1 ) * ( line->x2 - line->x1 ) + ( y - line->y1 ) * ( line->y2 - line->y1 ) ) / pow2( L );
	
	if( r <= 0 || r >= 1 )
	{
		GL_point p( x, y );
		GL_point A( line->x1, line->y1 );
		GL_point B( line->x2, line->y2 );
		
		return cl_min( p.distance( A ), p.distance( B ) );
	}
	
	float s = ( ( line->y1 - y ) * ( line->x2 - line->x1 ) - ( line->x1 - x ) * ( line->y2 - line->y1 ) ) / pow2( L );
	return fabs( s ) * L ;
}

GL_point get_lineintersection( GL_line *line1, GL_line *line2 )
{
	float denominator = ( ( line1->x2 - line1->x1 ) * ( line2->y2 - line2->y1 ) - ( line1->y2 - line1->y1 ) * ( line2->x2 - line2->x1 ) );

	if( denominator == 0 )
	{
		return GL_point( line1->x1, line1->y1 );
	}
	
	float r = ( ( line1->y1 - line2->y1 ) * ( line2->x2 - line2->x1 ) - ( line1->x1 - line2->x1 ) * ( line2->y2 - line2->y1 ) ) / denominator;
	
	GL_point p;
	p.x = line1->x1 + r * ( line1->x2 - line1->x1 );
	p.y = line1->y1 + r * ( line1->y2 - line1->y1 );
	
	return p;
}

bool Col_line( GL_line *line1, GL_line *line2, bool collinear_intersect )
{
	float denominator = ( ( line1->x2 - line1->x1 ) * ( line2->y2 - line2->y1 ) - ( line1->y2 - line1->y1 ) * ( line2->x2 - line2->x1 ) );	
	
	if( denominator == 0 ) // parallell
	{
		if( ( line1->y1 - line2->y1 ) * ( line2->x2 - line2->x1 ) - ( line1->x1 - line2->x1 ) * ( line2->y2 - line2->y1 ) == 0 ) // collinear
		{
			if( collinear_intersect )
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		return 0;
	}
	
	float r = ( ( line1->y1 - line2->y1 ) * ( line2->x2 - line2->x1 ) - ( line1->x1 - line2->x1 ) * ( line2->y2 - line2->y1 ) ) / denominator;
	float s = ( ( line1->y1 - line2->y1 ) * ( line1->x2 - line1->x1 ) - ( line1->x1 - line2->x1 ) * ( line1->y2 - line1->y1 ) ) / denominator;
	
	if( ( s >= 0.0f && s <= 1.0f ) && ( r >= 0.0f && r <= 1.0f ) )
	{
		return 1;
	}
	
	return 0;
}
