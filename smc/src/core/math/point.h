/***************************************************************************
     point.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __POINT_H__
#define __POINT_H__

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

/* *** *** *** *** *** *** *** GL_point *** *** *** *** *** *** *** *** *** *** */

/*
 * parts from Clanlib
*/
class GL_point
{
public:
	GL_point( void )
	: x( 0 ), y( 0 ) {}
	
	GL_point( float x, float y )
	: x( x ), y( y ) {}
	
	GL_point( const GL_point &p )
	: x( p.x ), y( p.y ) {}

	// Return a rotated version of this point.
	//param hotspot: The point around which to rotate.
	//param angle: The amount of degrees to rotate by, clockwise.
	GL_point rotate( const GL_point &hotspot, float angle) const
	{
		//Move the hotspot to 0,0
		GL_point r(x - hotspot.x, y - hotspot.y);
		
		const float c = (float) sqrt((float)r.x*(float)r.x + (float)r.y*(float)r.y);
		const float nw = (float)(atan2((float)r.y, (float)r.x) + ((angle + 180) * M_PI / 180));
		r.x = (int)((sin(1.5 * M_PI - nw) * c) + 0.5) + hotspot.x;
		r.y = -(int)((sin(nw) * c) + 0.5) + hotspot.y;

		return r;
	}

	// Returns the distance to another point
	float distance( const GL_point &p ) const
	{
    	return sqrt( ( x - p.x ) * ( x- p.x ) + ( y - p.y ) * ( y - p.y ) );
	}

	// Translate point
	GL_point &operator += ( const GL_point &p )
	{
		x += p.x; y += p.y; 
		return *this; 
	}
	
	// Translate point negatively
	GL_point &operator -= ( const GL_point &p )
	{
		x -= p.x; y -= p.y;
		return *this; 
	}
	
	// Point + Point operator
	GL_point operator + ( const GL_point &p ) const
	{
		return GL_point( x + p.x, y + p.y );
	}

	// Point - Point operator
	GL_point operator - ( const GL_point &p ) const
	{
		return GL_point( x - p.x, y - p.y ); 
	}

	// Point == Point operator
	bool operator == ( const GL_point &p ) const
	{
		return ( x == p.x ) && ( y == p.y ); 
	}

	// Point != Point operator
	bool operator != ( const GL_point &p ) const
	{
		return ( x != p.x ) || ( y != p.y ); 
	}

	float x, y;
};

#endif
