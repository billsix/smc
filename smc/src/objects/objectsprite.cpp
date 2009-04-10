/***************************************************************************
 * objectsprite.cpp  - multi image object sprite class
 *
 * Copyright (C) 2005 - 2009 Florian Richter
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

namespace SMC
{

/* *** *** *** *** *** *** *** cImageObjectSprite *** *** *** *** *** *** *** *** *** *** */

cImageObjectSprite :: cImageObjectSprite( float x /* = 0 */, float y /* = 0 */ )
: cMovingSprite( NULL, x, y )
{
	m_curr_img = -1;
}

cImageObjectSprite :: ~cImageObjectSprite( void )
{
	m_images.clear();
}

void cImageObjectSprite :: Set_Image_Num( const int num, const bool new_startimage /* = 0 */, const bool del_img /* = 0 */ )
{
	if( m_curr_img == num )
	{
		return;
	}

	m_curr_img = num;

	if( m_curr_img < 0 )
	{
		cMovingSprite::Set_Image( NULL, new_startimage, del_img );
	}
	else if( m_curr_img < static_cast<int>(m_images.size()) )
	{
		cMovingSprite::Set_Image( m_images[m_curr_img], new_startimage, del_img );
	}
	else
	{
		printf( "Warning : Object image number %d bigger as the array size %u, sprite type %d, name %s\n", m_curr_img, static_cast<unsigned int>(m_images.size()), m_type, m_name.c_str() );
	}
}

cGL_Surface *cImageObjectSprite :: Get_Image( const unsigned int num ) 
{
	if( num >= m_images.size() )
	{
		return NULL;
	}

	return m_images[num];
}

void cImageObjectSprite :: Clear_Images( void )
{
	m_curr_img = -1;
	m_images.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
