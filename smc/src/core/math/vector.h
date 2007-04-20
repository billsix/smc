/***************************************************************************
     vector.h  -  header for the corresponding cpp file
                             -------------------
    copyright				: (C) 2006 - 2007 by Florian Richter
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

#ifndef __GL_VECTOR_H__
#define __GL_VECTOR_H__

#include "../../core/globals.h"

/* *** *** *** *** *** *** *** GL_Vector *** *** *** *** *** *** *** *** *** *** */

/*
 * parts from Clanlib
*/
class GL_Vector
{
public:
	GL_Vector( float pos_x = 0, float pos_y = 0, float pos_z = 0, float pos_w = 1 );
	// copy from another vector
	GL_Vector( const GL_Vector &v );

	// Returns the (euclid) norm of the vector (in R^3)
	// This function does not use the w coordinate of the vector
	// uses only x,y,z coordinates
	float norm( void ) const;

	// Normalizes the vector ( not using the w ordinate )
	void normalize( void );

	// Dot products this vector with an other vector
	float dot( const GL_Vector& vector ) const;
	
	// Calculate the angle between this vector and an other vector
	float angle( const GL_Vector& vector ) const;

	// Calculate the cross product between this vector and an other vector
	GL_Vector cross( const GL_Vector& vector ) const;
	
	// Rotate vector around an axis
	// angle: Angle to rotate
	// axis: Rotation axis
	GL_Vector rotate( float angle, const GL_Vector& axis ) const;

	// Rounds all components
	void round( void );

	//: Scalar product (vector * scalar)
	GL_Vector operator * (float scalar) const;  

	// Scalar product (scalar * vector)
	friend GL_Vector operator *  (float scalar, const GL_Vector& vector);

	// += operator
	void operator += (const GL_Vector& v);

	// -= operator
	void operator -= (const GL_Vector& v);  

	// *= operator (scalar multiplication)
	void operator *= (float s);  

	// + operator
	GL_Vector operator + (const GL_Vector& v) const;
	
	// - operator
	GL_Vector operator - (const GL_Vector& v) const;
	
	// unary - operator
	GL_Vector operator - () const;

	// assignment operator
	GL_Vector& operator = (const GL_Vector& v);

	// Returns true if current vector equals v
	// v : other vector
	bool operator == (const GL_Vector& v) const;
    	
	// Returns false if current vector equals v
	// v : other vector
	bool operator != (const GL_Vector& v) const;

	// coordinates
	float x;
	float y;
	float z;
	float w;
};

#endif
