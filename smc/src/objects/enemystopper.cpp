/***************************************************************************
           enemystopper.cpp  -  enemystopper class
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../objects/enemystopper.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../core/camera.h"

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

cEnemyStopper :: cEnemyStopper( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cEnemyStopper :: cEnemyStopper( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cEnemyStopper :: ~cEnemyStopper( void )
{

}

cEnemyStopper *cEnemyStopper :: Copy( void )
{
	cEnemyStopper *enemystopper = new cEnemyStopper( startposx, startposy );

	return enemystopper;
}

void cEnemyStopper :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_ENEMYSTOPPER;
	massivetype = MASS_PASSIVE;
	editor_posz = 0.11f;

	name = "Enemystopper";

	rect.w = 15;
	rect.h = 15;
	col_rect.w = 15;
	col_rect.h = 15;
	start_rect.w = 15;
	start_rect.h = 15;

	editor_color = Color( (Uint8)0, 0, 255, 128 );
}

void cEnemyStopper :: Create_from_Stream( XMLAttributes &attributes )
{
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
}

void cEnemyStopper :: Save_to_Stream( ofstream &file )
{
	// begin enemystopper
	file << "\t<enemystopper>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;

	// end enemystopper
	file << "\t</enemystopper>" << std::endl;
}

void cEnemyStopper :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	if( !editor_level_enabled )
	{
		return;
	}

	pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, editor_posz, &editor_color );
}

bool cEnemyStopper :: is_Draw_valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		// not visible
		if( !visible || !image )
		{
			return 0;
		}
	}

	// not visible on the screen
	if( !is_Visible_onScreen() )
	{
		return 0;
	}

	return 1;
}
