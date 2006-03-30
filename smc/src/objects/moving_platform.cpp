/***************************************************************************
           moving_platform.cpp  -  default moving platforms handler
                             -------------------
    copyright            :	(C) 2005 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 
#include "../globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cMoving_Platform :: cMoving_Platform( double x, double y, mPlatformType ptype /* = MPLATFORM_VER */, int pmax_area /* = 150 */, double pspeed /* = 3 */ ) : cActiveSprite( x, y )
{
	type = TYPE_MOVING_PLATFORM;
	massivetype = MASS_HALFMASSIVE;

	player_range = 2000;

	platform_type = ptype;
	max_area = pmax_area;
	speed = pspeed;

	if( platform_type == MPLATFORM_HOR )
	{
		direction = DIR_RIGHT;
	}
	else
	{
		direction = DIR_DOWN;
	}

	images.push_back( GetImage( "clouds/default_1/2_small.png" ) );

	SetImage( 0 );
}

cMoving_Platform :: ~cMoving_Platform( void )
{
	//
}

void cMoving_Platform :: Update( void )
{
	if( !visible || !isPlayer_range() )
	{
		return;
	}

	if( direction == DIR_UP )
	{
		velx = 0;
		vely = -speed;

		if( posy - startposy < 0 )
		{
			direction = DIR_DOWN;
		}
	}
	else if( direction == DIR_DOWN )
	{
		velx = 0;
		vely = speed;

		if( posy - startposy > max_area )
		{
			direction = DIR_UP;
		}
	}
	else if( direction == DIR_LEFT )
	{
		velx = -speed;
		vely = 0;
	}
	else if( direction == DIR_RIGHT )
	{
		velx = speed;
		vely = 0;
	}

	Move( velx, vely, 0, 1 );

	Draw();
}
