/***************************************************************************
           enemy.cpp  -  base class for all enemies
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
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

cEnemy :: cEnemy( double x, double y ) : cObjectSprite( x, y )
{
	array = ARRAY_ENEMY;
	type = TYPE_ENEMY;

	player_range = 2500;

	massivetype = MASS_MASSIVE;
	state = FALL;
	dead = 0;

	counter = 0;
	walk_count = 0;
}

cEnemy :: ~cEnemy( void )
{

}

void cEnemy :: DieStep( void )
{
	// virtual
}