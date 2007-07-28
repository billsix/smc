/***************************************************************************
     rect.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GL_RECT_H__
#define __GL_RECT_H__

#include "../../core/globals.h"
#include "../../core/math/point.h"

/* *** *** *** *** *** *** *** GL_rect *** *** *** *** *** *** *** *** *** *** */

class GL_rect
{
public:
	GL_rect( void )
	: x( 0 ), y( 0 ), w( 0 ), h( 0 ) {}

	GL_rect( GL_rect *rect )
	: x( rect->x ), y( rect->y ), w( rect->w ), h( rect->h ) {}

	GL_rect( float x, float y, float w, float h )
	: x( x ), y( y ), w( w ), h( h ) {}

	// returns a SDL_Rect
	SDL_Rect Get_Rect( void )
	{
		SDL_Rect rect;
		rect.x = (Sint16)x;
		rect.y = (Sint16)y;
		rect.w = (Uint16)w;
		rect.h = (Uint16)h;

		return rect;
	}

	// returns a SDL_Rect with the given position
	SDL_Rect Get_Rect_pos( float posx, float posy )
	{
		SDL_Rect rect;
		rect.x = (Sint16)( x + posx );
		rect.y = (Sint16)( y + posy );
		rect.w = (Uint16)w;
		rect.h = (Uint16)h;

		return rect;
	}

	// returns the point in the middle of the rect
	GL_point Get_pos_middle( void )
	{
		return GL_point( x + ( w / 2 ), y + ( h / 2 ) );
	}

	// Set the rect data
	void Set_Rect( GL_rect *rect )
	{
		x = rect->x;
		y = rect->y;
		w = rect->w;
		h = rect->h;
	}

	// clears the data
	void clear( void )
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	float x, y;
	float w, h;
};

#endif
