/***************************************************************************
     point.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __POINT_H__
#define __POINT_H__

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
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
	// hotspot: The point around which to rotate.
	// angle: The amount of degrees to rotate by, clockwise.
	GL_point rotate( const GL_point &hotspot, float angle) const
	{
		//Move the hotspot to 0,0
		GL_point r(x - hotspot.x, y - hotspot.y);
		
		const float c = static_cast<float>(sqrt(r.x*r.x + r.y*r.y));
		const float nw = static_cast<float>(atan2(r.y, r.x) + ((angle + 180) * M_PI / 180));
		r.x = static_cast<float>((sin(1.5 * M_PI - nw) * c) /*+ 0.5*/) + hotspot.x;
		r.y = -static_cast<float>((sin(nw) * c) /*+ 0.5*/) + hotspot.y;

		return r;
	}

	// Return a point rotated around the given axis in 3D.
	// angle : Rotation amount
	// a_: Axis to rotate around.
	GL_point rotate3d( float angle, const float &a_x, const float &a_y, const float &a_z )
	{
		GL_point tmp;

		float s = static_cast<float>(sin(angle));
		float c = static_cast<float>(cos(angle));

		tmp.x = x*(a_x*a_x*(1-c)+c)     + y*(a_x*a_y*(1-c)-a_z*s) + /*z**/(a_x*a_z*(1-c)+a_y*s);
		tmp.y = x*(a_y*a_x*(1-c)+a_z*s) + y*(a_y*a_y*(1-c)+c)     + /*z**/(a_y*a_z*(1-c)-a_x*s);
		//tmp.z = x*(a_x*a_z*(1-c)-a_y*s) + y*(a_y*a_z*(1-c)+a_x*s) + z*(a_z*a_z*(1-c)+c);

		return tmp;
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
