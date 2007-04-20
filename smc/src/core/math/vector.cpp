/***************************************************************************
           vector.cpp  -  vector class
                             -------------------
    copyright            :	(C) 2006 - 2007 Florian Richter
							(C) Clanlib Team
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../../core/globals.h"
#include "../../core/math/vector.h"

/* *** *** *** *** *** *** *** GL_Vector *** *** *** *** *** *** *** *** *** *** */

GL_Vector :: GL_Vector( float pos_x /* = 0 */, float pos_y /* = 0 */, float pos_z /* = 0 */, float pos_w /* = 1 */ )
{
	x = pos_x;
	y = pos_y;
	z = pos_z;
	w = pos_w;
}

GL_Vector :: GL_Vector( const GL_Vector &v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}

float GL_Vector :: norm( void ) const
{
	return (float)sqrt( x * x + y * y + z * z );
}

void GL_Vector :: normalize( void )
{
	float val = norm();

	if( val != 0 )
	{
		x /= val;
		y /= val;
		z /= val;
	}
}

float GL_Vector :: dot( const GL_Vector& v ) const
{
	return x * v.x + y * v.y + z * v.z;  
}

float GL_Vector :: angle( const GL_Vector& v ) const
{
	return acos( dot( v ) / ( norm() * v.norm() ) );  
}

GL_Vector GL_Vector :: cross( const GL_Vector& v ) const
{
	GL_Vector tmp = GL_Vector( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	return tmp;  
}

// quick hack, same as glRotatef(angle, a);
GL_Vector GL_Vector :: rotate( float angle, const GL_Vector& a ) const
{
	GL_Vector tmp = GL_Vector();

	float s = (float)sin( angle );
	float c = (float)cos( angle );

	tmp.x = x*(a.x*a.x*(1-c)+c)     + y*(a.x*a.y*(1-c)-a.z*s) + z*(a.x*a.z*(1-c)+a.y*s);
	tmp.y = x*(a.y*a.x*(1-c)+a.z*s) + y*(a.y*a.y*(1-c)+c)     + z*(a.y*a.z*(1-c)-a.x*s);
	tmp.z = x*(a.x*a.z*(1-c)-a.y*s) + y*(a.y*a.z*(1-c)+a.x*s) + z*(a.z*a.z*(1-c)+c);

	return tmp;  
}

void GL_Vector :: round( void )
{
	x = (float)int( x + 0.5f );
	y = (float)int( y + 0.5f );
	z = (float)int( z + 0.5f );
	w = (float)int( w + 0.5f );
}

GL_Vector GL_Vector :: operator * ( float s ) const
{
	return GL_Vector( s * x, s * y, s * z, s * w );
}

GL_Vector operator * ( float s, const GL_Vector& v )
{
	return GL_Vector( s * v.x, s * v.y, s * v.z, s * v.w );
}

void GL_Vector :: operator += ( const GL_Vector& v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.z;
}

void GL_Vector :: operator -= ( const GL_Vector& v )
{
  	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

void GL_Vector :: operator *= ( float s )
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}

GL_Vector GL_Vector::operator + ( const GL_Vector& v ) const
{
	return GL_Vector( x + v.x, y + v.y, z + v.z, w + v.w );
}

GL_Vector GL_Vector::operator - ( const GL_Vector& v ) const
{
	return GL_Vector(x - v.x, y - v.y, z - v.z, w - v.z);
}

GL_Vector GL_Vector::operator - () const
{
	return GL_Vector( -x, -y, -z, -w);
}

GL_Vector& GL_Vector::operator = ( const GL_Vector& v )
{ 
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return *this;
}
 
bool GL_Vector::operator == ( const GL_Vector& v ) const
{
	return ( (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w) );
}

bool GL_Vector::operator != ( const GL_Vector& v ) const
{
	return !(operator == (v));
}
