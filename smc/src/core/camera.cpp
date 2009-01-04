/***************************************************************************
 * camera.cpp  -  class for handling screen camera movement
 *
 * copyright (C) 2006 - 2008 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/camera.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../core/framerate.h"
#include "../input/mouse.h"
#include "../core/obj_manager.h"
#include "../overworld/world_manager.h"
#include "../level/level.h"
#include "../overworld/overworld.h"
#include "../core/sprite_manager.h"

/* *** *** *** *** *** *** *** cCamera *** *** *** *** *** *** *** *** *** *** */

cCamera :: cCamera( void )
{
	x = 0;
	y = 0;

	x_offset = 0;
	y_offset = 0;
	hor_offset_speed = 0.3f;
	ver_offset_speed = 0.2f;

	fixed_hor_vel = 0;

	// default camera limit
	Reset_Limits();
}

cCamera :: ~cCamera( void )
{

}

void cCamera :: Set_Pos( float nx, float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
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

void cCamera :: Set_Pos_X( float nx )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		if( !editor_enabled )
		{
			// camera offset
			nx += x_offset;
			
			// camera limits
			Update_Limit_X( nx );
		}
	}

	x = nx;

	Update_Position();
}

void cCamera :: Set_Pos_Y( float ny )
{
	// level mode
	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		if( !editor_enabled )
		{
			// camera offset
			ny += y_offset;

			// camera limits
			Update_Limit_Y( ny );
		}
	}

	y = ny;

	Update_Position();
}

void cCamera :: Move( float move_x, float move_y )
{
	if( ( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD ) && !editor_enabled )
	{
		Set_Pos( x + move_x - x_offset, y + move_y - y_offset );
	}
	else
	{
		Set_Pos( x + move_x, y + move_y );
	}
}

bool cCamera :: Move_to_Position_Gradually( float px, float py )
{
	// limit to nearest possible position
	float px_final = px + pActive_Camera->x_offset;
	float py_final = py + pActive_Camera->y_offset;
	Update_Limit( px_final, py_final );

	// check distance to new position
	float distance_x = px_final - pActive_Camera->x;
	float distance_y = py_final - pActive_Camera->y;
	// velocity
	float vel_x = ( distance_x * 0.04f ) * pFramerate->speedfactor;
	float vel_y = ( distance_y * 0.04f ) * pFramerate->speedfactor;
	// true if reached position
	bool reached_y = 0, reached_x = 0;

	if( distance_x > 1 )
	{
		if( vel_x < 1 )
		{
			vel_x = 1;
		}

		pActive_Camera->Move( vel_x, 0 );
	}
	else if( distance_x < -1 )
	{
		if( vel_x > -1 )
		{
			vel_x = -1;
		}

		pActive_Camera->Move( vel_x, 0 );
	}
	// reached destination position x
	else
	{
		reached_x = 1;
	}

	if( distance_y > 1 )
	{
		if( vel_y < 1 )
		{
			vel_y = 1;
		}

		pActive_Camera->Move( 0, vel_y );
	}
	else if( distance_y < -1 )
	{
		if( vel_y > -1 )
		{
			vel_y = -1;
		}

		pActive_Camera->Move( 0, vel_y );
	}
	// reached destination position y
	else
	{
		reached_y = 1;
	}

	// reached position
	if( reached_x && reached_y )
	{
		return 0;
	}

	// position not reached
	return 1;
}

void cCamera :: Center( ObjectDirection direction /* = DIR_ALL */ )
{
	// Center camera on a not changing player position
	if( direction == DIR_VERTICAL )
	{
		Set_Pos_X( Get_Center_Pos_X() );
	}
	else if( direction == DIR_HORIZONTAL )
	{
		Set_Pos_Y( Get_Center_Pos_Y() );
	}
	else if( direction == DIR_ALL )
	{
		Set_Pos( Get_Center_Pos_X(), Get_Center_Pos_Y() );
	}
}

float cCamera :: Get_Center_Pos_X( void )
{
	return pActive_Player->col_rect.x + 5 - ( game_res_w / 2 );
}

float cCamera :: Get_Center_Pos_Y( void )
{
	return pActive_Player->col_rect.y + pActive_Player->col_rect.h - 5 - ( game_res_h / 2 );
}

void cCamera :: Reset_Limits( void )
{
	limit_rect = GL_rect( 0, 0, 20000, -4000 );
}

void cCamera :: Set_Limits( GL_rect rect )
{
	limit_rect = rect;

	// minimal size
	if( limit_rect.w < 800 )
	{
		limit_rect.w = 800;
	}
	if( limit_rect.h > 0 )
	{
		limit_rect.h = 0;
	}
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
	Update_Limit_X( nx );
	Update_Limit_Y( ny );
}

void cCamera :: Update_Limit_X( float &nx )
{
	// left
	if( nx < limit_rect.x )
	{
		nx = limit_rect.x;
	}
	// right
	else if( nx + game_res_w > limit_rect.x + limit_rect.w )
	{
		nx = limit_rect.x + limit_rect.w - game_res_w;
	}
}

void cCamera :: Update_Limit_Y( float &ny )
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
	// level
	if( Game_Mode == MODE_LEVEL )
	{
		// no leveleditor mode
		if( !editor_enabled )
		{
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

			if( fixed_hor_vel )
			{
				// todo : remove hackiness
				x += fixed_hor_vel * pFramerate->speedfactor;
				// check limit
				Update_Limit_X( x );
				// center one side
				Center( DIR_HORIZONTAL );

				// scrolls to the right
				if( fixed_hor_vel > 0 )
				{
					// out of camera on the left side
					if( pPlayer->col_rect.x + pPlayer->col_rect.w < x )
					{
						pPlayer->DownGrade_Player( 1, 1 );
					}

				}
				// scrolls to the left
				else
				{
					// out of camera on the right side
					if( pPlayer->col_rect.x - game_res_w > x )
					{
						pPlayer->DownGrade_Player( 1, 1 );
					}
				}
			}
			else
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

				// set position
				Center();
			}
		}
	}
	// world
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// Left
		if( pActive_Player->posx - x < game_res_w * 0.4f )
		{
			Set_Pos_X( pActive_Player->posx - game_res_w * 0.4f );
		}
		// Right
		else if( pActive_Player->posx - x > game_res_w * 0.6f )
		{
			Set_Pos_X( pActive_Player->posx - game_res_w * 0.6f );
		}
		
		// Up
		if( pActive_Player->posy - y < game_res_h * 0.4f )
		{
			Set_Pos_Y( pActive_Player->posy - game_res_h * 0.4f );
		}
		// Down
		else if( pActive_Player->posy - y > game_res_h * 0.6f )
		{
			Set_Pos_Y( pActive_Player->posy - game_res_h * 0.6f );
		}
	}
}

void cCamera :: Update_Position( void )
{
	// mouse
	pMouseCursor->Update_Position();

	if( Game_Mode == MODE_LEVEL || Game_Mode == MODE_OVERWORLD )
	{
		// update player
		pActive_Player->Update_Valid_Draw();
		// update sprite manager
		pActive_Sprite_Manager->Update_Items_Valid_Draw();

		// editor
		if( editor_enabled )
		{
			// update settings activated object position
			if( pMouseCursor->active_object )
			{
				pMouseCursor->active_object->Editor_Position_Update();
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

