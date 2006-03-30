/***************************************************************************
    collison.cpp  -  internal collision functions
                             -------------------
    copyright            : (C) 2005 - 2006 FluXy
						   (C) 2005 Amir Taaki
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

cObjectCollision :: cObjectCollision( void )
{
	type = CO_NOTHING;
	number = 0;
	direction = DIR_NOTHING;
}

cObjectCollision :: ~cObjectCollision( void )
{
	//
}

void cObjectCollision :: Set_direction( cSprite *base, cSprite *col )
{
	
	// if the position is over the object
	bool col_top = base->col_rect.y + 1 >= col->col_rect.y + col->col_rect.h;
	// if the position is under the object
	bool col_bottom = base->col_rect.y + base->col_rect.h - 1 <= col->col_rect.y;
	// if the position is on the left of the object
	bool col_left = base->col_rect.x + 1 >= col->col_rect.x + col->col_rect.w;
	// if the position is on the right of the object
	bool col_right = base->col_rect.x + base->col_rect.w - 1 <= col->col_rect.x;
	
	if( col_top && base->vely < 0 )
	{
		direction = DIR_UP;
	}
	else if( col_bottom && base->vely > 0 )
	{
		direction = DIR_DOWN;
	}
	else if( col_left && base->velx < 0 )
	{
		direction = DIR_LEFT;
	}
	else if( col_right && base->velx > 0 )
	{
		direction = DIR_RIGHT;
	}
	else // more advanced detection
	{
		GL_point point_base = base->col_rect.Get_pos_middle();
		GL_point point_col = col->col_rect.Get_pos_middle();

		// if col object point left
		if( point_col.x >= base->col_rect.x + base->col_rect.w )
		{
			direction = DIR_RIGHT;
		}
		// if col object point is right
		else if( point_col.x <= base->col_rect.x )
		{
			direction = DIR_LEFT;
		}

		// if col object point is on top
		if( point_col.y >= base->col_rect.y + base->col_rect.h )
		{
			direction = DIR_DOWN;
		}
		// if col object point is below
		else if( point_col.y <= base->col_rect.y )
		{
			direction = DIR_UP;
		}
	}

	// needs better detection for valid velocity directions
	if( base->type == TYPE_PLAYER )
	{
		//DrawRect( &base->col_rect, &white );
		//DrawRect( &col->col_rect, &red );

		//SDL_GL_SwapBuffers();
		//SDL_Delay( 300 );

		//printf( "%d -- %d\n", direction, ( rand() % 1000 ) );
	}
}

GL_rect cObjectCollision :: Get_ColObject_rect( void )
{
	GL_rect colrect;
	colrect.clear();

	if( type == CO_MASSIVE || type == CO_ACTIVE || type == CO_ENEMY )
	{
		colrect = pObjManager->items[number]->col_rect;
	}
	else if( type == CO_PLAYER )
	{
		colrect = pPlayer->col_rect;
	}

	return colrect;
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

	return 1;	// bounding boxes intersect
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

	return 1;	// bounding boxes intersect
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
bool Col_Circle( double x1, double y1, double r1, double x2, double y2, double r2, int offset )
{
	double xdiff = x2 - x1;	// x plane difference
	double ydiff = y2 - y1;	// y plane difference
	
	/* distance between the circles centres squared */
	double dcentre_sq = ( ydiff * ydiff ) + ( xdiff * xdiff );
	
	/* calculate sum of radiuses squared */
	double r_sum_sq = r1 + r2;	// square on seperate line, so
	r_sum_sq *= r_sum_sq;	// dont recompute r1 + r2

	return ( dcentre_sq - r_sum_sq <= ( offset * offset ) );
}

bool Col_Circle( GL_Surface *surface1, SDL_Rect *a, GL_Surface *surface2, SDL_Rect *b, int offset )
{
	return Col_Circle( surface1, a->x, a->y, surface2, b->x, b->y, offset );
}

// from SDL_collide ( Copyright (C) 2005 Amir Taaki )
bool Col_Circle( GL_Surface *a, double x1, double y1, GL_Surface *b, double x2, double y2, int offset )
{
	/* if radius is not specified
	we approximate them using SDL_Surface's
	width and height average and divide by 2*/
	double r1 = ( ( a->w + a->h ) / 4 );	// same as / 2) / 2;
	double r2 = ( ( b->w + b->h ) / 4 );

	x1 += a->w / 2;		// offset x and y
	y1 += a->h / 2;		// co-ordinates into
				// centre of image
	x2 += b->w / 2;
	y2 += b->h / 2;

	return Col_Circle( x1, y1, r1, x2, y2, r2, offset );
}

template<typename T> inline T pow2( T value ) { return value * value; }
template<typename T> inline T cl_min( T a, T b ) { if( a < b ) return a; return b; }

double distance_to_line( double x, double y, GL_line *line )
{
	double L = sqrt( pow2( line->x2 - line->x1 ) + pow2( line->y2 - line->y1 ) );
	double r = ( ( x - line->x1 ) * ( line->x2 - line->x1 ) + ( y - line->y1 ) * ( line->y2 - line->y1 ) ) / pow2( L );
	
	if( r <= 0 || r >= 1 )
	{
		GL_point p( x, y );
		GL_point A( line->x1, line->y1 );
		GL_point B( line->x2, line->y2 );
		
		return cl_min( p.distance( A ), p.distance( B ) );
	}
	
	double s = ( ( line->y1 - y ) * ( line->x2 - line->x1 ) - ( line->x1 - x ) * ( line->y2 - line->y1 ) ) / pow2( L );
	return fabs( s ) * L ;
}

GL_point get_lineintersection( GL_line *line1, GL_line *line2 )
{
	double denominator = ( ( line1->x2 - line1->x1 ) * ( line2->y2 - line2->y1 ) - ( line1->y2 - line1->y1 ) * ( line2->x2 - line2->x1 ) );

	if( denominator == 0 )
	{
		return GL_point( line1->x1, line1->y1 );
	}
	
	double r = ( ( line1->y1 - line2->y1 ) * ( line2->x2 - line2->x1 ) - ( line1->x1 - line2->x1 ) * ( line2->y2 - line2->y1 ) ) / denominator;
	
	GL_point p;
	p.x = line1->x1 + r * ( line1->x2 - line1->x1 );
	p.y = line1->y1 + r * ( line1->y2 - line1->y1 );
	
	return p;
}

bool Col_line( GL_line *line1, GL_line *line2, bool collinear_intersect )
{
	double denominator = ( ( line1->x2 - line1->x1 ) * ( line2->y2 - line2->y1 ) - ( line1->y2 - line1->y1 ) * ( line2->x2 - line2->x1 ) );	
	
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
	
	double r = ( ( line1->y1 - line2->y1 ) * ( line2->x2 - line2->x1 ) - ( line1->x1 - line2->x1 ) * ( line2->y2 - line2->y1 ) ) / denominator;
	double s = ( ( line1->y1 - line2->y1 ) * ( line1->x2 - line1->x1 ) - ( line1->x1 - line2->x1 ) * ( line1->y2 - line1->y1 ) ) / denominator;
	
	if( ( s >= 0.0f && s <= 1.0f ) && ( r >= 0.0f && r <= 1.0f ) )
	{
		return 1;
	}
	
	return 0;
}
