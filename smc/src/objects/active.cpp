/***************************************************************************
           active.cpp  -  active sprite class
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

cActiveSprite :: cActiveSprite( double x, double y ) : cObjectSprite( x, y )
{
	type = TYPE_ACTIVESPRITE;
	array = ARRAY_ACTIVE;
	massivetype = MASS_MASSIVE;
}

cActiveSprite :: ~cActiveSprite( void )
{
	//
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEnemyStopper :: cEnemyStopper( double x, double y ) : cActiveSprite( x, y )
{
	type = TYPE_ENEMYSTOPPER;
	massivetype = MASS_PASSIVE;

	rect.w = 15;
	rect.h = 15;
	col_rect.w = 15;
	col_rect.h = 15;
	start_rect.w = 15;
	start_rect.h = 15;

	color = Get_Color( 0, 0, 255, 128 );
}

cEnemyStopper :: ~cEnemyStopper( void )
{
	//
}

void cEnemyStopper :: Draw( void )
{
	if( !Leveleditor_Mode )
	{
		rect.x = startposx;
		rect.y = startposy;
		col_rect.x = startposx + col_pos.x;
		col_rect.y = startposy + col_pos.y;
		start_rect.x = startposx + col_pos.x;
		start_rect.y = startposy + col_pos.y;
		
		return;
	}
	
	rect.x = startposx;
	rect.y = startposy;
	col_rect.x = startposx + col_pos.x;
	col_rect.y = startposy + col_pos.y;
	start_rect.x = startposx + col_pos.x;
	start_rect.y = startposy + col_pos.y;

	if( !isVisible_onScreen() ) 
	{
		return;
	}

	DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &color ); 
}

