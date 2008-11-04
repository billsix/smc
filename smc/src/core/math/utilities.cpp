/***************************************************************************
 * utilities.cpp  -  General math functions
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../core/math/utilities.h"


/* non digit class
 * returns true if not a number was found
 * used by the valid_number function
*/
class nondigit
{
public:
	bool operator() (char c) { return !isdigit( c ); }
};

/* *** *** *** *** *** *** *** *** Math utility functions *** *** *** *** *** *** *** *** *** */

float Get_Random_Float( float min, float max )
{
	 return min + (max - min) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

unsigned int Get_Power_of_2( unsigned int size )
{
	unsigned int value = 1;

	while( value < size )
	{
		value <<= 1;
	}

	return value;
}

bool is_valid_number( string num, bool accept_floating_point /* = 1 */ )
{
	// accept negative numbers
	if( num.find( '-' ) == 0 )
	{
		num.erase( 0, 1 );
	}

	// accept numbers with a point if given
	if( accept_floating_point )
	{
		std::string::size_type pos = num.find( '.' );

		if( pos != string::npos )
		{
			num.erase( pos, 1 );
		}
	}

	if( std::find_if( num.begin(), num.end(), nondigit() ) == num.end() )
	{
		return 1;
	}

	return 0;
}
