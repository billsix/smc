/***************************************************************************
 * enemystopper.cpp  -  enemystopper class
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/enemystopper.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../core/i18n.h"

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

cEnemyStopper :: cEnemyStopper( float x, float y )
: cImageObjectSprite( x, y )
{
	cEnemyStopper::Init();
}

cEnemyStopper :: cEnemyStopper( CEGUI::XMLAttributes &attributes )
: cImageObjectSprite()
{
	cEnemyStopper::Init();
	cEnemyStopper::Create_from_Stream( attributes );
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
	type = TYPE_ENEMY_STOPPER;
	massivetype = MASS_PASSIVE;
	editor_posz = 0.11f;

	name = _("Enemystopper");

	rect.w = 15;
	rect.h = 15;
	col_rect.w = 15;
	col_rect.h = 15;
	start_rect.w = 15;
	start_rect.h = 15;

	editor_color = Color( static_cast<Uint8>(0), 0, 255, 128 );
}

void cEnemyStopper :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
}

void cEnemyStopper :: Save_to_Stream( ofstream &file )
{
	// begin enemystopper
	file << "\t<enemystopper>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;

	// end enemystopper
	file << "\t</enemystopper>" << std::endl;
}

void cEnemyStopper :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	pVideo->Draw_Rect( col_rect.x - pActive_Camera->x, col_rect.y - pActive_Camera->y, col_rect.w, col_rect.h, editor_posz, &editor_color );
}

bool cEnemyStopper :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		return 0;
	}

	// if not visible on the screen
	if( !visible || !Is_Visible_on_Screen() )
	{
		return 0;
	}

	return 1;
}
