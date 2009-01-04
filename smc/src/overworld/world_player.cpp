/***************************************************************************
 * ow_player.cpp  - Overworld Player class
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
 
#include "../overworld/world_player.h"
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

	line_hor = cLine_collision();
	line_ver = cLine_collision();

	debug_current_line_last = -100;
	debug_lines_last = -100;
	debug_current_waypoint_last = -100;

	debug_lines = new cHudSprite( NULL, 150, game_res_h * 0.97f );
	debug_current_line = new cHudSprite( NULL, 300, game_res_h * 0.97f );
	debug_current_waypoint = new cHudSprite( NULL, 500, game_res_h * 0.97f );

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
		// todo : load all direction images at once so we can set the correct direction down start/editor image
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
		printf( "Unsupported Maryo state : %d\n", static_cast<unsigned int>(maryo_state) );
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
		printf( "Error : Unknown Overworld Animation state %d\n", static_cast<unsigned int>(maryo_state) );
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
	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cSurfaceRequest();
	}

	// Draw player
	cImageObjectSprite::Draw( request );
	// alpha in debug mode
	if( pOverworld_Manager->debugmode )
	{
		request->color.alpha = 64;
	}

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}

	// Draw debug text
	Draw_Debug_Text();
}

void cOverworld_Player :: Draw_Debug_Text( void )
{
	if( !pOverworld_Manager->debugmode )
	{
		return;
	}

	// Update Lines Texts
	if( static_cast<int>(pActive_Overworld->pLayer->size()) != debug_lines_last )
	{
		debug_lines->Set_Image( pFont->Render_Text( pFont->font_small, "Lines : " + int_to_string( pActive_Overworld->pLayer->size() ), blue ), 1, 1 );
	}
	// Update Current Line Texts
	if( current_line != debug_current_line_last )
	{
		debug_current_line->Set_Image( pFont->Render_Text( pFont->font_small, "Curr Line : " + int_to_string( current_line ), green ), 1, 1 );
	}
	// Update Current Waypoint Texts
	if( current_waypoint != debug_current_waypoint_last )
	{
		debug_current_waypoint->Set_Image( pFont->Render_Text( pFont->font_small, "Curr Waypoint : " + int_to_string( current_waypoint ), green ), 1, 1 );
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

void cOverworld_Player :: Reset( void )
{
	current_waypoint = -2;
	line_waypoint = 0;
	current_line = -2;

	fixed_walking = 0;
	direction = DIR_UNDEFINED;

	Load_Images();
}

void cOverworld_Player :: Action_Interact( input_identifier key_type )
{
	// Left
	if( key_type == INP_LEFT )
	{
		pOverworld_Player->Start_Walk( DIR_LEFT );
	}
	// Right
	else if( key_type == INP_RIGHT )
	{
		pOverworld_Player->Start_Walk( DIR_RIGHT );
	}
	// Up
	else if( key_type == INP_UP )
	{
		pOverworld_Player->Start_Walk( DIR_UP );
	}
	// Down
	else if( key_type == INP_DOWN )
	{
		pOverworld_Player->Start_Walk( DIR_DOWN );
	}
	// Action
	else if( key_type == INP_ACTION )
	{
		// enter
		pOverworld_Player->Activate_Waypoint();
		Clear_Input_Events();
	}
	// Exit
	else if( key_type == INP_EXIT )
	{
		Game_Action = GA_ENTER_MENU;
	}
}

void cOverworld_Player :: Action_Stop_Interact( input_identifier key_type )
{
	// nothing yet
}

void cOverworld_Player :: Activate_Waypoint( void )
{
	// if no waypoint or already walking
	if( current_waypoint < 0 || direction != DIR_UNDEFINED )
	{
		return;
	}

	// get waypoint
	cWaypoint *waypoint = Get_Waypoint();

	// normal level waypoint
	if( waypoint->waypoint_type == WAYPOINT_NORMAL )
	{
		pActive_Level->Load( waypoint->Get_Destination() );
		
		// valid level
		if( pActive_Level->Is_Loaded() )
		{
			// Enter Level
			Game_Action = GA_ENTER_LEVEL;
		}
		// not a valid level - just walk to the next
		else
		{
			printf( "Error : Overworld level not found %s\n", waypoint->Get_Destination().c_str() );
			pActive_Overworld->Goto_Next_Level();
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
			Draw_Effect_Out( EFFECT_OUT_HORIZONTAL_VERTICAL );
			Game_Action = GA_ENTER_MENU_CREDITS;
		}
		// world link
		else
		{
			cOverworld *new_world = pOverworld_Manager->Get( str_world );

			if( new_world )
			{
				Game_Action = GA_ENTER_WORLD;
				Game_Action_Data.add( "world", str_world.c_str() );
				Game_Action_Data.add( "player_waypoint", overworld_origin->description->path.c_str() );
			}
			else
			{
				printf( "Warning : Overworld not found %s\n", str_world.c_str() );
			}
		}
	}
}

void cOverworld_Player :: Update_Vel( void )
{
	if( direction == DIR_UP )
	{
		velx = 0;
		vely = -3;
	}
	else if( direction == DIR_DOWN )
	{
		velx = 0;
		vely = 3;
	}
	else if( direction == DIR_RIGHT )
	{
		velx = 3;
		vely = 0;
	}
	else if( direction == DIR_LEFT )
	{
		velx = -3;
		vely = 0;
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
	if( pOverworld_Manager->debugmode )
	{
		printf( "Current Maryo Direction : %d\n", direction );

		if( current_waypoint > 0 )
		{
			printf( "Waypoint Direction Forward : %d Backward : %d\n", Get_Waypoint()->direction_forward, Get_Waypoint()->direction_backward );
		}
	}

	// a start from waypoint
	if( current_waypoint >= 0 && direction == DIR_UNDEFINED )
	{
		// Get Layer Line in front
		cLayer_Line_Point_Start *front_line = Get_Front_Line( new_direction );

		if( !front_line )
		{
			if( pOverworld_Manager->debugmode )
			{
				printf( "Waypoint Front line not detected\n" );
			}

			return 0;
		}

		if( pOverworld_Manager->debugmode )
		{
			printf( "Waypoint Front line id %d, origin id %d\n", pActive_Overworld->pLayer->Get_Array_Num( front_line ), front_line->origin );
		}

		// forward
		if( Get_Waypoint()->direction_forward == new_direction )
		{
			cWaypoint *next_waypoint = front_line->Get_End_Waypoint();

			if( !next_waypoint )
			{
				printf( "Next waypoint not detected\n" );
				return 0;
			}

			// next waypoint is not accessible
			if( !next_waypoint->access )
			{
				if( pOverworld_Manager->debugmode )
				{
					printf( "No access to next waypoint\n" );
				}

				return 0;
			}

			if( pOverworld_Manager->debugmode )
			{
				printf( "Next waypoint id : %d\n", current_waypoint );
			}

			// set line waypoint
			line_waypoint = front_line->origin;
		}
		// backward
		else if( Get_Waypoint()->direction_backward == new_direction )
		{
			// set line waypoint
			line_waypoint = front_line->origin;
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
		if( direction == Get_Opposite_Direction( new_direction ) && !fixed_walking && pActive_Overworld->Get_Waypoint_Collision( &col_rect ) < 0 )
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
	if( current_waypoint >= 0 && !Col_Box( &col_rect, &Get_Waypoint()->rect ) )
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
		if( pOverworld_Manager->debugmode )
		{
			if( direction == DIR_UP || direction == DIR_DOWN )
			{
				printf( "horizontal line connection broken : num %d diff %f\n", line_hor.line_number, line_hor.difference );
			}
			else if( direction == DIR_LEFT || direction == DIR_RIGHT )
			{
				printf( "vertical line connection broken : num %d diff %f\n", line_ver.line_number, line_ver.difference );
			}
		}

		current_waypoint = -1;

		Change_Direction();
		Load_Images();
	}

	Auto_Pos_Correction();

	// check if a new waypoint is near maryo
	for( unsigned int i = 0; i < pActive_Overworld->waypoints.size(); i++ )
	{
		// skip the start waypoint
		if( static_cast<int>(i) == current_waypoint )
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
	cWaypoint *waypoint = Get_Waypoint();

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

		pAudio->Play_Sound( "waypoint_reached.ogg" );
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

	// Update Camera
	pActive_Camera->Update();
	// Update Waypoint text
	pActive_Overworld->Update_Waypoint_text();

	return 1;
}

cWaypoint *cOverworld_Player :: Get_Waypoint( void )
{
	if( current_waypoint < 0 )
	{
		return NULL;
	}

	return pActive_Overworld->Get_Waypoint( current_waypoint );
}

cLayer_Line_Point_Start *cOverworld_Player :: Get_Front_Line( ObjectDirection dir )
{
	return pActive_Overworld->pLayer->Get_Line_Collision_Direction( col_rect.x + ( col_rect.w * 0.5f ), col_rect.y + ( col_rect.h * 0.5f ), dir ).line;
}

void cOverworld_Player :: Update_Path_Diff( unsigned int csize /* = 25 */ )
{
	float x = col_rect.x + ( col_rect.w * 0.5f ) + velx;
	float y = col_rect.y + ( col_rect.h * 0.5f ) + vely;

	// bigger direction check size
	unsigned int hor_advance = 0;
	unsigned int ver_advance = 0;
	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		ver_advance += 10;
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{
		hor_advance += 10;
	}

	// get nearest lines
	line_hor = pActive_Overworld->pLayer->Get_Nearest( x, y, DIR_HORIZONTAL, csize + hor_advance, line_waypoint );
 	line_ver = pActive_Overworld->pLayer->Get_Nearest( x, y, DIR_VERTICAL, csize + ver_advance, line_waypoint );
}

void cOverworld_Player :: Change_Direction( void )
{
	line_hor = cLine_collision();
	line_ver = cLine_collision();

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
			line_ver = pActive_Overworld->pLayer->Get_Nearest( x - i, y, DIR_VERTICAL, 80, line_waypoint );

			// found
			if( line_ver.line_number >= 0 )
			{
				break;
			}

			line_ver = pActive_Overworld->pLayer->Get_Nearest( x + i, y, DIR_VERTICAL, 80, line_waypoint );

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
			direction = Get_Opposite_Direction( direction );
		}
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{
		// search for a line with increasing distance
		for( unsigned int i = 0; i < 30; i++ )
		{
			line_hor = pActive_Overworld->pLayer->Get_Nearest( x, y + i, DIR_HORIZONTAL, 80, line_waypoint );

			// found
			if( line_hor.line_number >= 0 )
			{
				break;
			}

			line_hor = pActive_Overworld->pLayer->Get_Nearest( x, y - i, DIR_HORIZONTAL, 80, line_waypoint );

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
			direction = Get_Opposite_Direction( direction );
		}
	}
	else
	{
		printf( "Warning : Unknown Maryo direction %d\n", static_cast<int>(direction) );
		direction = DIR_DOWN;
	}

	if( pOverworld_Manager->debugmode )
	{
		printf( "Changed direction to %s\n", Get_Direction_Name( direction ).c_str() );
	}

	Update_Vel();
}

void cOverworld_Player :: Auto_Pos_Correction( float size /* = 1.7f */, float min_distance /* = 5 */ )
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

