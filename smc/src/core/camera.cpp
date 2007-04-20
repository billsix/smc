/***************************************************************************
			camera.cpp  -  class for handling screen camera movement
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../core/camera.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../core/framerate.h"
#include "../input/mouse.h"
#include "../core/obj_manager.h"
#include "../overworld/worlds.h"

/* *** *** *** *** *** *** *** cCamera *** *** *** *** *** *** *** *** *** *** */

cCamera :: cCamera( void )
{
	x = 0;
	y = 0;

	x_offset = 0;
	y_offset = 0;
	hor_offset_speed = 0.3f;
	ver_offset_speed = 0.2f;

	// default camera limit
	Reset_Limits();
}

cCamera :: ~cCamera( void )
{

}

void cCamera :: Set_Pos( float nx, float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL )
	{
		if( !editor_enabled )
		{
			// camera offset
			nx += x_offset;
			ny += y_offset;
			
			// camera limits
			Update_Limit( nx, ny );
		}
	}

	x = nx;
	y = ny;

	Update_Position();
}

void cCamera :: Set_PosX( float nx )
{
	// level mode
	if( Game_Mode == MODE_LEVEL )
	{
		if( !editor_enabled )
		{
			// camera offset
			nx += x_offset;
			
			// camera limits
			Update_LimitX( nx );
		}
	}

	x = nx;

	Update_Position();
}

void cCamera :: Set_PosY( float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL )
	{
		if( !editor_enabled )
		{
			// camera offset
			ny += y_offset;

			// camera limits
			Update_LimitY( ny );
		}
	}

	y = ny;

	Update_Position();
}

void cCamera :: Move( float move_x, float move_y )
{
	if( Game_Mode == MODE_LEVEL && !editor_enabled )
	{
		Set_Pos( x + move_x - x_offset, y + move_y - y_offset );
	}
	else
	{
		Set_Pos( x + move_x, y + move_y );
	}
}

void cCamera :: Center( ObjectDirection direction /* = DIR_ALL */ )
{
	if( direction == DIR_ALL || direction == DIR_HORIZONTAL )
	{
		Set_PosY( pPlayer->posy + pPlayer->col_pos.y + pPlayer->col_rect.h - ( GAME_RES_H / 2 ) );
	}

	if( direction == DIR_ALL || direction == DIR_VERTICAL )
	{
		Set_PosX( pPlayer->posx + pPlayer->col_pos.x - ( GAME_RES_W / 2 ) );
	}
}

void cCamera :: Reset_Limits( void )
{
	limit_rect = GL_rect( 0, 0, 20000, -4000 );
}

void cCamera :: Set_Limits( GL_rect rect )
{
	limit_rect = rect;
}

void cCamera :: Set_Limit_X( float val )
{
	limit_rect.x = val;
}

void cCamera :: Set_Limit_Y( float val )
{
	limit_rect.y = val;
}

void cCamera :: Set_Limit_W( float val )
{
	limit_rect.w = val;
}

void cCamera :: Set_Limit_H( float val )
{
	limit_rect.h = val;
}

void cCamera :: Update_Limit( float &nx, float &ny )
{
	Update_LimitX( nx );
	Update_LimitY( ny );
}

void cCamera :: Update_LimitX( float &nx )
{
	// left
	if( nx < limit_rect.x )
	{
		nx = limit_rect.x;
	}
	// right
	else if( nx + GAME_RES_W > limit_rect.x + limit_rect.w )
	{
		nx = limit_rect.x + limit_rect.w - GAME_RES_W;
	}
}

void cCamera :: Update_LimitY( float &ny )
{
	// down
	if( ny > limit_rect.y )
	{
		ny = limit_rect.x;
	}
	// up
	else if( ny < limit_rect.y + limit_rect.h )
	{
		ny = limit_rect.y + limit_rect.h;
	}
}

void cCamera :: Update( void ) 
{
	// level mode
	if( Game_Mode == MODE_LEVEL )
	{
		// no leveleditor mode
		if( !editor_enabled )
		{
			// player is moving horizontal
			if( pPlayer->velx != 0 && hor_offset_speed > 0 )
			{
				pPlayer->no_velx_counter = 0;

				if( ( x_offset < 200 && pPlayer->velx > 0 ) || ( x_offset > -200 && pPlayer->velx < 0 ) )
				{
					x_offset += ( pPlayer->velx * hor_offset_speed ) * pFramerate->speedfactor;
				}
			}
			// slowly remove offset
			else
			{
				pPlayer->no_velx_counter += pFramerate->speedfactor;

				if( pPlayer->no_velx_counter > 10 && ( x_offset > 40 || x_offset < -40 ) )
				{
					x_offset += -( x_offset / 50 ) * pFramerate->speedfactor;
				}
			}
			
			// player is moving vertical
			if( pPlayer->vely != 0 && ver_offset_speed > 0 )
			{
				pPlayer->no_vely_counter = 0;

				if( ( y_offset < 100 && pPlayer->vely > 0 ) || ( y_offset > -100 && pPlayer->vely < 0 ) )
				{
					y_offset += ( pPlayer->vely * ver_offset_speed ) * pFramerate->speedfactor;
				}
			}
			// slowly remove offset
			else 
			{
				pPlayer->no_vely_counter += pFramerate->speedfactor;

				if( pPlayer->no_vely_counter > 10 && ( y_offset > 20 || y_offset < -20 ) )
				{
					y_offset += -( y_offset / 30 ) * pFramerate->speedfactor;
				}
			}

			// set position
			Center();
		}
	}
}

void cCamera :: Update_Position( void )
{
	// mouse
	pMouseCursor->Update_Position();

	// object manager
	cObjectManager *obj_manager = NULL;

	if( Game_Mode == MODE_LEVEL )
	{
		obj_manager = pObjManager;

		// player
		pPlayer->Update_valid_draw();
	}
	else
	{
		obj_manager = pActive_Overworld->object_manager;

		// player
		pOverworld_Player->Update_valid_draw();
	}

	obj_manager->Update_items_valid_draw();

	// editor
	if( editor_enabled )
	{
		// update settings activated object position
		if( pMouseCursor->active_object )
		{
			pMouseCursor->active_object->Editor_pos_update();
		}
	}
}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cCamera *pCamera = NULL;
