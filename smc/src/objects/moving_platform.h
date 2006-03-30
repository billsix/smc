/***************************************************************************
                moving_platform.h  -  header for the corresponding cpp file
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

#ifndef __MOVING_PLATFORM_H__
#define __MOVING_PLATFORM_H__

typedef enum
{
	MPLATFORM_HOR = 1,
	MPLATFORM_VER = 2
} mPlatformType;

#include "../globals.h"

class cMoving_Platform : public cActiveSprite
{
public:
	cMoving_Platform( double x, double y, mPlatformType ptype = MPLATFORM_VER, int pmax_area = 150, double pspeed = 3 );
	~cMoving_Platform( void );
	
	virtual void Update( void );

	// platform type
	mPlatformType platform_type;
	// current direction
	ObjectDirection moving_direction;
	// moving speed
	double speed;
	// max moving area
	int max_area;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
