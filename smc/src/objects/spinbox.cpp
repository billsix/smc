/***************************************************************************
           spinbox.cpp  -  spinning box
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/spinbox.h"
#include "../core/obj_manager.h"
#include "../core/framerate.h"
#include "../core/game_core.h"

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

cSpinBox :: cSpinBox( float x, float y )
: cBaseBox( x, y )
{
	Init();
}

cSpinBox :: cSpinBox( XMLAttributes &attributes )
: cBaseBox()
{
	Init();
	Create_from_Stream( attributes );
}

cSpinBox :: ~cSpinBox( void )
{

}

void cSpinBox :: Init( void )
{
	type = TYPE_SPINBOX;
	box_type = type;
	player_range = 5000;

	spin_counter = 0;
	spin = 0;

	// default is infinite times activate-able
	Set_UseableCount( -1, 1 );
	// Spinbox Animation
	Set_Animation( "Spin" );

	// no animation
	anim_counter_max = 1;

	// editor image
	item_image = pVideo->Get_Surface( "game/arrow/small/white/up.png" );

	Create_Name();
}

cSpinBox *cSpinBox :: Copy( void )
{
	cSpinBox *spinbox = new cSpinBox( startposx, startposy );
	spinbox->Set_Invisible( box_invisible );
	spinbox->Set_UseableCount( start_useable_count, 1 );

	return spinbox;
}

void cSpinBox :: Create_from_Stream( XMLAttributes &attributes )
{
	cBaseBox::Create_from_Stream( attributes );
}

void cSpinBox :: Save_to_Stream( ofstream &file )
{
	// begin box
	file << "\t<box>" << std::endl;

	cBaseBox::Save_to_Stream( file );

	// end box
	file << "\t</box>" << std::endl;
}

void cSpinBox :: Activate( void )
{
	// already spinning
	if( spin )
	{
		return;
	}

	spin = 1;
	Update_valid_update();
	// passive box for spinning
	massivetype = MASS_PASSIVE;

	counter = static_cast<float>(anim_counter_min);
	spin_counter = 0;
	// set the BaseBox maximum animation image
	anim_counter_max = 6;
}

void cSpinBox :: Stop( void )
{
	// already stopped spinning
	if( !spin )
	{
		return;
	}

	// disabled image
	if( !useable_count )
	{
		Set_Image( 0, 0, 0 );
	}
	// default image
	else
	{
		Set_Image( 1, 0, 0 );
	}
	// reset
	spin = 0;
	Update_valid_update();
	counter = static_cast<float>(anim_counter_min);
	spin_counter = 0;

	// back to a massive box
	massivetype = MASS_MASSIVE;
	// set the BaseBox maximum animation image
	anim_counter_max = 1;
}

void cSpinBox :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	cBaseBox::Update();

	if( spin )
	{
		spin_counter += pFramerate->speedfactor;

		// spinning animation finished
		if( counter == 1 )
		{
			// spinning time finished
			if( spin_counter > DESIRED_FPS * 5 )
			{
				// reset spin counter
				spin_counter = 0;
				// for collision check and if spinning stopped back to a massive box
				massivetype = MASS_MASSIVE;
				// collision data
				cObjectCollisionType col_list = Collision_Check( &col_rect, COLLIDE_ONLY_CHECK );
				
				// check if spinning should continue
				bool spin_again = 0;

				// colliding with player or enemy
				if( col_list.find( TYPE_PLAYER ) || col_list.find( ARRAY_ENEMY ) )
				{
					spin_again = 1;
				}
				// colliding with an active object
				else if( col_list.find( ARRAY_ACTIVE ) )
				{
					cSprite *col_obj = col_list.find( ARRAY_ACTIVE );
					
					// check for items
					if( col_obj->type == TYPE_MUSHROOM_LIVE_1 || col_obj->type == TYPE_MUSHROOM_DEFAULT || 
						col_obj->type == TYPE_MUSHROOM_POISON || col_obj->type == TYPE_MUSHROOM_BLUE || col_obj->type == TYPE_MUSHROOM_GHOST || 
						col_obj->type == TYPE_FIREPLANT || col_obj->type == TYPE_JSTAR || col_obj->type == TYPE_FGOLDPIECE )
					{
						// found blocking active object
						spin_again = 1;
					}
				}

				// continue spinning
				if( spin_again )
				{
					// spin some time again
					spin_counter = DESIRED_FPS * 2;
					// passive for spinning
					massivetype = MASS_PASSIVE;
				}
				// finished spinning
				else
				{
					Stop();
				}
			}
		}
	}
}

bool cSpinBox :: is_Update_valid( void )
{
	if( spin )
	{
		return 1;
	}

	return cBaseBox::is_Update_valid();
}
