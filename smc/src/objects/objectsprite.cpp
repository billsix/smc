/***************************************************************************
           objectsprite.cpp  - multi image object sprite class
                             -------------------
    copyright            :	(C) 2005 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/objectsprite.h"
#include "../core/game_core.h"

/* *** *** *** *** *** *** *** cImageObjectSprite *** *** *** *** *** *** *** *** *** *** */

cImageObjectSprite :: cImageObjectSprite( float x /* = 0 */, float y /* = 0 */ )
: cMovingSprite( NULL, x, y )
{
	curr_img = -1;
}

cImageObjectSprite :: ~cImageObjectSprite( void )
{
	images.clear();
}

void cImageObjectSprite :: Set_Image( int num, bool new_startimage /* = 0 */, bool del_img /* = 0 */ )
{
	if( curr_img == num )
	{
		return;
	}

	curr_img = num;

	if( curr_img < 0 )
	{
		cMovingSprite::Set_Image( NULL, new_startimage, del_img );
	}
	else if( curr_img < (int)images.size() )
	{
		cMovingSprite::Set_Image( images[curr_img], new_startimage, del_img );
	}
	else
	{
		printf( "Warning : Object image number %d bigger as the array size %d, sprite type %d, name %s\n", curr_img, images.size(), type, name.c_str() );
	}
}

GL_Surface *cImageObjectSprite :: Get_Image( unsigned int num ) 
{
	if( num >= images.size() )
	{
		return NULL;
	}

	return images[num];
}

void cImageObjectSprite :: Clear_Images( void )
{
	curr_img = -1;
	images.clear();
}
