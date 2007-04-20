/***************************************************************************
				ow_player.cpp  - Overworld Player class
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 
#include "../overworld/ow_player.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../overworld/overworld.h"
#include "../level/level.h"
#include "../video/font.h"
#include "../audio/audio.h"
#include "../gui/menu.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** *** cOverworld_Player *** *** *** *** *** *** *** *** *** */

cOverworld_Player :: cOverworld_Player( void )
: cImageObjectSprite()
{
	type = TYPE_PLAYER;
	posz = 0.0999f;
	massivetype = MASS_MASSIVE;
	player_range = 0;
	name = "Maryo";

	maryo_state = MARYO_SMALL;
	current_waypoint = -2; // no waypoint
	line_waypoint = 0;
	current_line = -2;

	anim_counter = 0;
	anim_speed = 0;
	anim_max = 0;

	fixed_walking = 0;

	line_hor.clear();
	line_ver.clear();

	debug_current_line_last = -100;
	debug_lines_last = -100;
	debug_current_waypoint_last = -100;

	debug_lines = new cHudSprite( NULL, 150, GAME_RES_H * 0.97f );
	debug_current_line = new cHudSprite( NULL, 300, GAME_RES_H * 0.97f );
	debug_current_waypoint = new cHudSprite( NULL, 500, GAME_RES_H * 0.97f );

	Load_Images();
}

cOverworld_Player :: ~cOverworld_Player( void )
{
	delete debug_lines;
	delete debug_current_line;
	delete debug_current_waypoint;

	Unload_Images();
}

void cOverworld_Player :: Load_Images( void )
{
	Unload_Images();

	if( maryo_state == MARYO_SMALL )
	{
		if( direction == DIR_DOWN || direction == DIR_UNDEFINED )
		{
			images.push_back( pVideo->Get_Surface( "world/maryo/small/down.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/down_1.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/down.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/down_2.png" ) );
			
			anim_max = 4;
			anim_speed = 2;
		}
		else if( direction == DIR_UP )
		{
			images.push_back( pVideo->Get_Surface( "world/maryo/small/up_1.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/up.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/up_1.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/up_2.png" ) );
			
			anim_max = 4;
			anim_speed = 2;
		}
		else if( direction == DIR_LEFT )
		{
			images.push_back( pVideo->Get_Surface( "world/maryo/small/left.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/left_1.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/left.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/left_2.png" ) );

			anim_max = 4;
			anim_speed = 1.7f;
		}
		else if( direction == DIR_RIGHT )
		{
			images.push_back( pVideo->Get_Surface( "world/maryo/small/right.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/right_1.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/right.png" ) );
			images.push_back( pVideo->Get_Surface( "world/maryo/small/right_2.png" ) );

			anim_max = 4;
			anim_speed = 1.7f;
		}
	}
	else
	{
		printf( "Unsupported Maryo state : %d\n", (unsigned int)maryo_state );
		maryo_state = MARYO_SMALL;
		return;
	}
	
	Set_Image( 0, 1 );	
}

void cOverworld_Player :: Unload_Images( void )
{
	Clear_Images();

	anim_counter = 0;
	anim_speed = 0;
	anim_max = 0;
}

void cOverworld_Player :: Update( void )
{
	cImageObjectSprite::Update();

	anim_counter += pFramerate->speedfactor;

	if( maryo_state == MARYO_SMALL )
	{
		// uhm ... CLEANUP !
		for( unsigned int i = 0; i < 5; i++ )
		{
			if( anim_counter < anim_max * ( ( i + 1 ) * anim_speed ) )
			{
				// reset
				if( anim_max < i + 1 )
				{
					anim_counter = 0;
					break;
				}
				else
				{
					Set_Image( i );
					break;
				}
			}
		}
	}
	else
	{
		printf( "Error : Unknown Overworld Animation state %d\n", (unsigned int)maryo_state );
	}

	if( direction == DIR_UNDEFINED )
	{
		return;
	}

	Update_Vel();

	// default walking
	if( !fixed_walking )
	{
		Update_Walk();
	} 
	// fixed walking
	else
	{
		Update_Waypoint_Walk();
	}
}

void cOverworld_Player :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	// Draw Player
	cImageObjectSprite::Draw( request );
	// Draw Debug text
	Draw_Debugtext();
}

void cOverworld_Player :: Draw_Debugtext( void )
{
	if( !pOverworld_manager->debugmode )
	{
		return;
	}

	// Update Lines Texts
	if( pActive_Overworld->pLayer->lines.size() != debug_lines_last )
	{
		debug_lines->Set_Image( pFont->RenderText( pFont->font_small, "Lines : " + int_to_string( pActive_Overworld->pLayer->lines.size() ), blue ), 1, 1 );
	}
	// Update Current Line Texts
	if( current_line != debug_current_line_last )
	{
		debug_current_line->Set_Image( pFont->RenderText( pFont->font_small, "Curr Line : " + int_to_string( current_line ), green ), 1, 1 );
	}
	// Update Current Waypoint Texts
	if( current_waypoint != debug_current_waypoint_last )
	{
		debug_current_waypoint->Set_Image( pFont->RenderText( pFont->font_small, "Curr Waypoint : " + int_to_string( current_waypoint ), green ), 1, 1 );
	}

	// Draw Line count
	cSurfaceRequest *request = new cSurfaceRequest();
	debug_lines->Draw( request );
	request->shadow_pos = 1;
	request->shadow_color = black;
	// add request
	pRenderer->Add( request );

	// Draw Current Line
	request = new cSurfaceRequest();
	debug_current_line->Draw( request );
	request->shadow_pos = 1;
	request->shadow_color = black;
	// add request
	pRenderer->Add( request );
	
	// Draw Current Waypoint
	request = new cSurfaceRequest();
	debug_current_waypoint->Draw( request );
	request->shadow_pos = 1;
	request->shadow_color = black;
	// add request
	pRenderer->Add( request );
}

void cOverworld_Player :: Update_Camera( void )
{
	// Right
	if( posx - pCamera->x - 1 > ( GAME_RES_W / 4 * 3 ) )
	{
		pCamera->Set_PosX( posx - ( GAME_RES_W / 4 * 3 ) );
	}
	// Left
	else if( pCamera->x && posx - pCamera->x + 1 < ( GAME_RES_W / 8 * 2 ) )
	{
		pCamera->Set_PosX( posx - ( GAME_RES_W / 8 * 2 ) );
	}
	
	// Up
	if( posy - pCamera->y + 1 < ( GAME_RES_H / 6 ) )
	{
		pCamera->Set_PosY( posy - ( GAME_RES_H / 6 ) );
	}
	// Down
	else if( pCamera->y && posy - pCamera->y - 1 > ( GAME_RES_H / 6 * 2.9f ) )
	{
		pCamera->Set_PosY( posy - ( GAME_RES_H / 6 * 2.9f ) );
	}
}

void cOverworld_Player :: Reset( void )
{
	current_waypoint = -2;
	line_waypoint = 0;
	current_line = -2;

	fixed_walking = 0;
	direction = DIR_UNDEFINED;

	Load_Images();
}

void cOverworld_Player :: Activate_Waypoint( void )
{
	// if no waypoint or already walking
	if( current_waypoint < 0 || direction != DIR_UNDEFINED )
	{
		return;
	}

	// get waypoint
	cWaypoint *waypoint =pActive_Overworld->waypoints[current_waypoint];

	// normal level waypoint
	if( waypoint->waypoint_type == WAYPOINT_NORMAL )
	{
		pLevel->Load( waypoint->Get_Destination() );
		
		// valid level
		if( pLevel->is_Loaded() )
		{
			pActive_Overworld->entered = 1;
			DrawEffect( FIXED_COLORBOX_FADEOUT );
		}
		// not a valid level - just walk to the next
		else
		{
			printf( "Error : Overworld level not found %s\n", waypoint->Get_Destination().c_str() );
			pActive_Overworld->Goto_next_Level();
		}
	}
	// world link waypoint
	else if( waypoint->waypoint_type == WAYPOINT_WORLD_LINK )
	{
		// remember Overworld origin
		cOverworld *overworld_origin = pActive_Overworld;

		// world string
		string str_world = waypoint->Get_Destination();

		// Enter Credits Menu ( World End )
		if( str_world.compare( "credits" ) == 0 )
		{
			DrawEffect( HORIZONTAL_VERTICAL_FADE );
			pActive_Overworld->entered = 3;
		}
		// world link
		else if( pOverworld_manager->Set_Active( str_world ) )
		{
			// get world waypoint
			int waypoint_num = pActive_Overworld->Get_Waypoint_num( overworld_origin->description->path );

			// waypoint available
			if( waypoint_num >= 0 )
			{
				// set all levels accessible until the world link
				pActive_Overworld->Set_Progress( waypoint_num, 0 );
				Set_Waypoint( waypoint_num );
			}

			DrawEffect( TILE_PIXELATION );
		}
		else
		{
			printf( "Error : Overworld world not found %s\n", str_world.c_str() );
		}
	}
}

void cOverworld_Player :: Update_Vel( void )
{
	if( direction == DIR_LEFT )
	{
		velx = -3;
		vely = 0;
	}
	else if( direction == DIR_RIGHT )
	{
		velx = 3;
		vely = 0;
	}
	else if( direction == DIR_UP )
	{
		velx = 0;
		vely = -3;
	}
	else if( direction == DIR_DOWN )
	{
		velx = 0;
		vely = 3;
	}
}

bool cOverworld_Player :: Start_Walk( ObjectDirection new_direction )
{
	// already walking into the given direction
	if( new_direction == direction )
	{
		return 0;
	}

	// invalid direction
	if( !( new_direction == DIR_LEFT || new_direction == DIR_RIGHT || new_direction == DIR_UP || new_direction == DIR_DOWN ) )
	{
		printf( "Warning : New direction is invalid : %d\n", new_direction );
		return 0;
	}

	// print debug info
	if( pOverworld_manager->debugmode )
	{
		printf( "Current Maryo Direction : %d\n", direction );

		if( current_waypoint > 0 )
		{
			printf( "Waypoint Direction Forward : %d Backward : %d\n", pActive_Overworld->waypoints[current_waypoint]->direction_forward, pActive_Overworld->waypoints[current_waypoint]->direction_backward );
		}
	}

	// a start from waypoint
	if( current_waypoint >= 0 && direction == DIR_UNDEFINED )
	{
		// forward
		if( pActive_Overworld->waypoints[current_waypoint]->direction_forward == new_direction )
		{
			// next waypoint is not accessible
			if( current_waypoint >= pActive_Overworld->Get_LastValidWaypoint() )
			{
				if( pOverworld_manager->debugmode )
				{
					printf( "no access to next waypoint\n" );
				}

				return 0;
			}

			// set line waypoint to current waypoint
			line_waypoint = current_waypoint;
		}
		// backward
		else if( pActive_Overworld->waypoints[current_waypoint]->direction_backward == new_direction )
		{
			// set line waypoint to previous waypoint
			line_waypoint = current_waypoint - 1;
		}
		// invalid direction
		else
		{
			return 0;
		}
	}

	// remember current direction
	ObjectDirection direction_old = direction;

	// if already walking
	if( direction != DIR_UNDEFINED )
	{
		if( direction == Get_OppositeDirection( new_direction ) && !fixed_walking && pActive_Overworld->Get_Waypoint_Collision( &col_rect ) < 0 )
		{
			direction = new_direction;
			Load_Images();
		}
	}
	// start walking
	else
	{
		direction = new_direction;
		Load_Images();
		
		// force a small move into the new direction
		Update_Vel();
		Move( velx, vely, 1 );
	}

	// if maryo is not on his current waypoint collide with every waypoint
	if( current_waypoint >= 0 && !Col_Box( &col_rect, &pActive_Overworld->waypoints[current_waypoint]->rect ) )
	{
		current_waypoint = -1;
	}

	if( direction_old != direction )
	{
		return 1; // return true if maryo is walking into a new direction
	}

	return 0; // no direction change
}

void cOverworld_Player :: Update_Walk( void )
{
	Move( velx, vely );
	Update_Path_Diff();

	/* Check if Maryo is too far away from the line in the current direction
	 * or if no line is anymore connected
	 * if true change direction
	*/
	if( ( ( direction == DIR_UP || direction == DIR_DOWN ) && line_hor.line_number >= 0 && ( line_hor.difference > 20 || line_hor.difference < -20 ) ) || 
		( ( direction == DIR_LEFT || direction == DIR_RIGHT ) && line_ver.line_number >= 0 && ( line_ver.difference > 20 || line_ver.difference < -20 ) ) ||
		 ( line_hor.line_number < 0 && line_ver.line_number < 0 ) )
	{
		if( pOverworld_manager->debugmode )
		{
			printf( "line connection broken : num %d diff %f\n", line_hor.line_number, line_hor.difference );
		}

		current_waypoint = -1;

		Change_Direction();
		Load_Images();
	}

	Auto_Pos_correct();

	// check if a new waypoint is near maryo
	for( unsigned int i = 0; i < pActive_Overworld->waypoints.size(); i++ )
	{
		// skip the start waypoint
		if( (int)i == current_waypoint )
		{
			continue;
		}

		if( Col_Box( &pActive_Overworld->waypoints[i]->rect, &col_rect ) )
		{
			Start_Waypoint_Walk( i );
			break;
		}
	}
}

void cOverworld_Player :: Start_Waypoint_Walk( unsigned int new_waypoint )
{
	fixed_walking = 1;
	current_waypoint = new_waypoint;
	current_line = -2;

	pActive_Overworld->Update_Waypoint_text();
}

void cOverworld_Player :: Update_Waypoint_Walk( void )
{
	// invalid current waypoint
	if( current_waypoint < 0 )
	{
		return;
	}

	unsigned int reached = 0;
	cWaypoint *waypoint = pActive_Overworld->waypoints[current_waypoint];

	// horizontal position to waypoint
	float x = col_rect.x + ( col_rect.w * 0.5f );

	if( waypoint->rect.x + ( waypoint->rect.w * 0.4f ) > x )
	{
		Move( 3, 0 );
	}
	else if( waypoint->rect.x + ( waypoint->rect.w * 0.7f ) < x )
	{
		Move( -3, 0 );
	}
	else
	{
		reached++;
	}

	// vertical position to waypoint
	float y = col_rect.y + ( col_rect.h * 0.5f );

	if( waypoint->rect.y + ( waypoint->rect.h * 0.4f ) > y )
	{
		Move( 0, 3 );
	}
	else if( waypoint->rect.y + ( waypoint->rect.h * 0.7f ) < y )
	{
		Move( 0, -3 );
	}
	else
	{
		reached++;
	}

	if( reached == 2 )
	{
		fixed_walking = 0;

		direction = DIR_DOWN;
		Load_Images();
		direction = DIR_UNDEFINED;
		Set_Waypoint( current_waypoint );

		pAudio->PlaySound( "waypoint_reached.ogg" );
	}
}

bool cOverworld_Player :: Set_Waypoint( unsigned int waypoint, bool new_startpos /* = 0 */ )
{
	if( waypoint > pActive_Overworld->waypoints.size() ) 
	{
		return 0;
	}

	cWaypoint *wp = pActive_Overworld->waypoints[waypoint];

	Set_Pos( wp->rect.x - col_pos.x + ( ( wp->rect.w - col_rect.w ) * 0.5f ), wp->rect.y - col_pos.y + ( ( wp->rect.h - col_rect.h ) * 0.5f ), new_startpos );
	current_waypoint = waypoint;

	pActive_Overworld->Update_Waypoint_text();

	return 1;
}

void cOverworld_Player :: Update_Path_Diff( unsigned int csize /* = 25 */ )
{
	float x = col_rect.x + ( col_rect.w * 0.5f ) + velx;
	float y = col_rect.y + ( col_rect.h * 0.5f ) + vely;

	// bigger direction check size
	float hor_advance = 0;
	float ver_advance = 0;
	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		ver_advance += 10;
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{
		hor_advance += 10;
	}

	// get nearest lines
	line_hor = pActive_Overworld->pLayer->Get_diff( x, y, DIR_HORIZONTAL, csize + (unsigned int)hor_advance );
 	line_ver = pActive_Overworld->pLayer->Get_diff( x, y, DIR_VERTICAL, csize + (unsigned int)ver_advance );
}

void cOverworld_Player :: Change_Direction( void )
{
	line_hor.clear();
	line_ver.clear();

	float x = col_rect.x + ( col_rect.w * 0.5f );
	float y = col_rect.y + ( col_rect.h * 0.5f );

	/* check with a big collision line the difference to the next layer line behind maryo
	 * if no line is found turn around
	 * if a line is found walk into it's direction
	*/
	if( direction == DIR_RIGHT || direction == DIR_LEFT )
	{
		// search for a line with increasing distance
		for( unsigned int i = 0; i < 30; i++ )
		{
			line_ver = pActive_Overworld->pLayer->Get_diff( x - i, y, DIR_VERTICAL, 80 );

			// found
			if( line_ver.line_number >= 0 )
			{
				break;
			}

			line_ver = pActive_Overworld->pLayer->Get_diff( x + i, y, DIR_VERTICAL, 80 );

			// found
			if( line_ver.line_number >= 0 )
			{
				break;
			}
		}

		// if line is found
		if( line_ver.line_number >= 0 )
		{
			if( line_ver.difference < 0 )
			{
				direction = DIR_UP;
			}
			else if( line_ver.difference > 0 )
			{
				direction = DIR_DOWN;
			}
		}
		// if no line found walk in to the opposite direction
		else
		{
			direction = Get_OppositeDirection( direction );
		}
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{
		// search for a line with increasing distance
		for( unsigned int i = 0; i < 30; i++ )
		{
			line_hor = pActive_Overworld->pLayer->Get_diff( x, y + i, DIR_HORIZONTAL, 80 );

			// found
			if( line_hor.line_number >= 0 )
			{
				break;
			}

			line_hor = pActive_Overworld->pLayer->Get_diff( x, y - i, DIR_HORIZONTAL, 80 );

			// found
			if( line_hor.line_number >= 0 )
			{
				break;
			}
		}

		// if line is found
		if( line_hor.line_number >= 0 )
		{
			if( line_hor.difference < 0 )
			{
				direction = DIR_LEFT;
			}
			else if( line_hor.difference > 0 )
			{
				direction = DIR_RIGHT;
			}
		}
		// if no line found walk in to the opposite direction
		else
		{
			direction = Get_OppositeDirection( direction );
		}
	}
	else
	{
		printf( "Warning : Unknown Maryo direction %d\n", (int)direction );
		direction = DIR_DOWN;
	}

	if( pOverworld_manager->debugmode )
	{
		printf( "Changed direction to %d\n", direction );
	}

	Update_Vel();
}

void cOverworld_Player :: Auto_Pos_correct( float size /* = 1.7f */, float min_distance /* = 5 */ )
{
	// if colliding with an waypoint
	if( pActive_Overworld->Get_Waypoint_Collision( &col_rect ) != -1 )
	{
		return;
	}

	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		// up and down correction
		if( line_ver.difference > min_distance )
		{
			Move( 0, size );
		}
		else if( line_ver.difference < -min_distance )
		{
			Move( 0, -size );
		}

		// Set current line number
		if( current_line != line_ver.line_number )
		{
			current_line = line_ver.line_number;
		}
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{ 
		// left correction
		if( line_hor.difference < -min_distance )
		{
			Move( -size, 0 );
		}
		// right correction
		else if( line_hor.difference > min_distance )
		{
			Move( size, 0 );
		}

		// Set current line number
		if( current_line != line_hor.line_number )
		{
			current_line = line_hor.line_number;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cOverworld_Player *pOverworld_Player = NULL;

