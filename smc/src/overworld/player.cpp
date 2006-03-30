/***************************************************************************
    player.cpp  - player class for the Overworld
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
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

/* *** *** *** *** *** *** *** *** World Maryo *** *** *** *** *** *** *** *** *** */

cWorld_Maryo :: cWorld_Maryo( void ) : cSprite()
{
	maryo_state = MARYO_SMALL;
	current_waypoint = -2; // no waypoint
	current_line = -2;

	anim_counter = 0;
	anim_speed = 0;
	anim_max = 0;

	fixed_walking = 0;

	line_hor.clear();
	line_ver.clear();
}

cWorld_Maryo :: ~cWorld_Maryo( void )
{
	Unload_Images();
}

void cWorld_Maryo :: Load_Images( void )
{
	Unload_Images();

	if( maryo_state == MARYO_SMALL )
	{
		if( direction == DIR_DOWN || ( direction != DIR_DOWN && direction != DIR_UP && direction != DIR_LEFT && direction != DIR_RIGHT ) )
		{
			images.push_back( GetImage( "world/maryo/small/down.png" ) );
			images.push_back( GetImage( "world/maryo/small/down_1.png" ) );
			images.push_back( GetImage( "world/maryo/small/down.png" ) );
			images.push_back( GetImage( "world/maryo/small/down_2.png" ) );
			
			anim_max = 4;
			anim_speed = 2;
		}
		else if( direction == DIR_UP )
		{
			images.push_back( GetImage( "world/maryo/small/up_1.png" ) );
			images.push_back( GetImage( "world/maryo/small/up.png" ) );
			images.push_back( GetImage( "world/maryo/small/up_1.png" ) );
			images.push_back( GetImage( "world/maryo/small/up_2.png" ) );
			
			anim_max = 4;
			anim_speed = 2;
		}
		else if( direction == DIR_LEFT )
		{
			images.push_back( GetImage( "world/maryo/small/left.png" ) );
			images.push_back( GetImage( "world/maryo/small/left_1.png" ) );
			images.push_back( GetImage( "world/maryo/small/left.png" ) );
			images.push_back( GetImage( "world/maryo/small/left_2.png" ) );

			anim_max = 4;
			anim_speed = 1.7;
		}
		else if( direction == DIR_RIGHT )
		{
			images.push_back( GetImage( "world/maryo/small/right.png" ) );
			images.push_back( GetImage( "world/maryo/small/right_1.png" ) );
			images.push_back( GetImage( "world/maryo/small/right.png" ) );
			images.push_back( GetImage( "world/maryo/small/right_2.png" ) );

			anim_max = 4;
			anim_speed = 1.7;
		}
	}
	else
	{
		printf( "Unsupported Maryo state : %d\n", (unsigned int)maryo_state );
		maryo_state = MARYO_SMALL;
		return;
	}
	
	if( images.size() )
	{
		SetImage( images[0] );	
	}
}

void cWorld_Maryo :: Unload_Images( void )
{
	images.clear();

	anim_counter = 0;
	anim_speed = 0;
	anim_max = 0;
}

void cWorld_Maryo :: Draw( void )
{
	anim_counter += Framerate.speedfactor;

	if( maryo_state == MARYO_SMALL )
	{
		for( unsigned int i = 0; i < 5; i++ )
		{
			if( anim_counter < anim_max * ( ( i + 1 ) * anim_speed ) )
			{
				if( anim_max < i + 1 ) // reset
				{
					anim_counter = 0;
					break;
				}
				else
				{
					SetImage( images[i] );
					break;
				}
			}
		}
	}
	else
	{
		printf( "Animation state error : %d\n", (unsigned int)maryo_state );
	}

	cSprite::Draw();
}

void cWorld_Maryo :: Update( void )
{
	if( direction == DIR_NOTHING )
	{
		return;
	}

	Update_Vel();

	if( !fixed_walking ) // default walking
	{
		Update_Walk();
	}
	else // fixed walking
	{
		Update_WaypointWalk();
	}
}

void cWorld_Maryo :: Update_Vel( void )
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

bool cWorld_Maryo :: Start_Walk( ObjectDirection new_direction )
{
	if( new_direction == direction ) // already walking into the given direction
	{
		return 0;
	}

	if( pOverWorld->debugmode )
	{
		printf( "Current Maryo Direction : %d\n", direction );

		if( current_waypoint > 0 )
		{
			printf( "Waypoint Direction Forward : %d Backward : %d\n", pOverWorld->Waypoints[current_waypoint]->direction_forward, pOverWorld->Waypoints[current_waypoint]->direction_back );
		}
	}

	// a start from waypoint
	if( current_waypoint >= 0 && direction == DIR_NOTHING )
	{
		// correct forward direction
		if( pOverWorld->Waypoints[current_waypoint]->direction_forward == new_direction )
		{
			// next waypoint is not accessable
			if( current_waypoint >= pOverWorld->Get_LastValidWaypoint() )
			{
				if( pOverWorld->debugmode )
				{
					printf( "No access to next waypoint\n" );
				}

				return 0;
			}
		}
		// if also not going back : wrong direction
		else if( pOverWorld->Waypoints[current_waypoint]->direction_back != new_direction )
		{
			return 0;
		}
	}

	if( !images.size() )
	{
		return 0;
	}

	// remember current direction
	ObjectDirection direction_old = direction;
	// the opposite direction
	ObjectDirection direction_inv = DIR_NOTHING;

	if( new_direction == DIR_LEFT )
	{
		direction_inv = DIR_RIGHT;
	}
	else if( new_direction == DIR_RIGHT )
	{
		direction_inv = DIR_LEFT;
	}
	else if( new_direction == DIR_UP )
	{
		direction_inv = DIR_DOWN;
	}
	else if( new_direction == DIR_DOWN )
	{
		direction_inv = DIR_UP;
	}
	else
	{
		printf( "Warning : direction is invalid : %d\n", (int)new_direction );
		return 0;
	}

	if( direction != DIR_NOTHING ) // if already walking
	{
		if( direction == direction_inv && !fixed_walking && pOverWorld->Get_Waypoint_Collision( &col_rect ) < 0 )
		{
			direction = new_direction;
			Load_Images();
		}
	}
	else // start walking
	{
		direction = new_direction;
		Load_Images();
		
		// force a small move into the new direction
		Update_Vel();
		Move( velx, vely, 1, 1 );
	}

	// if maryo is not on his current waypoint collide with every waypoint
	if( current_waypoint >= 0 && !Col_Box( &col_rect, &pOverWorld->Waypoints[current_waypoint]->rect ) )
	{
		current_waypoint = -1;
	}

	if( direction_old != direction )
	{
		return 1; // return true if maryo is walking into a new direction
	}

	return 0; // no direction change
}

void cWorld_Maryo :: Update_Walk( void )
{
	GL_rect crect = Get_ColRect();

	Move( velx, vely, 0, 1 );

	Update_PathDiff();

	/* Check if Maryo is too far away from the line in the current direction
	 * or if no line is anymore connected
	 * if true change direction
	*/
	if( ( ( direction == DIR_UP || direction == DIR_DOWN ) && line_hor.line_number >= 0 && ( line_hor.difference > 20 || line_hor.difference < -20 ) ) || 
		( ( direction == DIR_LEFT || direction == DIR_RIGHT ) && line_ver.line_number >= 0 && ( line_ver.difference > 20 || line_ver.difference < -20 ) ) ||
		 ( line_hor.line_number < 0 && line_ver.line_number < 0 ) )
	{
		current_waypoint = -1;

		Change_Direction();
		Load_Images();
	}

	Auto_Pos_correct();

	// Check if a new Waypoint is near Maryo
	for( unsigned int i = 0; i < pOverWorld->Waypoints.size(); i++ )
	{
		if( i == current_waypoint ) // skip the start Waypoint
		{
			continue;
		}

		if( Col_Box( &pOverWorld->Waypoints[i]->rect, &col_rect ) )
		{
			Start_WaypointWalk( i );
			break;
		}
	}
}

void cWorld_Maryo :: Start_WaypointWalk( unsigned int new_waypoint )
{
	fixed_walking = 1;
	current_waypoint = new_waypoint;
	current_line = -2;

	pOverWorld->hud_level->SetImage( RenderText( font_normal, pOverWorld->Waypoints[current_waypoint]->Get_Levelfile( 0, 0 ), lightblue ), 1, 1 );
}

void cWorld_Maryo :: Update_WaypointWalk( void )
{
	unsigned int reached = 0;

	if( pOverWorld->Waypoints[current_waypoint]->rect.x + 11 > (int)( posx + ( col_rect.w / 2 ) ) )
	{
		posx += 3 * Framerate.speedfactor;
	}
	else if( pOverWorld->Waypoints[current_waypoint]->rect.x + 16 < (int)( posx + ( col_rect.w / 2 ) ) )
	{
		posx -= 3 * Framerate.speedfactor;
	}
	else
	{
		reached++;
	}

	if( pOverWorld->Waypoints[current_waypoint]->rect.y + 5 > (int)( posy + ( col_rect.h * 0.9 ) ) )
	{
		posy += 3 * Framerate.speedfactor;
	}
	else if( pOverWorld->Waypoints[current_waypoint]->rect.y + 10 < (int)( posy + ( col_rect.h * 0.9 ) ) )
	{
		posy -= 3 * Framerate.speedfactor;
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
		direction = DIR_NOTHING;

		pAudio->PlaySound( SOUNDS_DIR "/waypoint.ogg" );
	}
}

bool cWorld_Maryo :: Set_Waypoint( unsigned int waypoint )
{
	if( waypoint > pOverWorld->Waypoints.size() ) 
	{
		return 0;
	}

	SetPos( pOverWorld->Waypoints[waypoint]->rect.x - 15, pOverWorld->Waypoints[waypoint]->rect.y - 35 );
	current_waypoint = waypoint;

	pOverWorld->hud_level->SetImage( RenderText( font_normal, pOverWorld->Waypoints[current_waypoint]->Get_Levelfile( 0, 0 ), lightblue ), 1, 1 );

	return 1;
}

void cWorld_Maryo :: Update_PathDiff( unsigned int csize /* = 25 */ )
{
	GL_rect crect = Get_ColRect();

	line_hor = pOverWorld->Layer->Get_diff( crect.x, crect.y, DIR_HORIZONTAL, csize );
 	line_ver = pOverWorld->Layer->Get_diff( crect.x, crect.y, DIR_VERTICAL, csize );
}

void cWorld_Maryo :: Change_Direction( void )
{
	GL_rect crect = Get_ColRect();

	line_hor.clear();
	line_ver.clear();

	/* check with a big collision line the difference to the next layer line behind maryo
	 * if no line is found turn around
	 * if a line is found walk into it's direction
	*/
	if( direction == DIR_RIGHT )
	{
		line_ver = pOverWorld->Layer->Get_diff( crect.x - 10, crect.y, DIR_VERTICAL, 80 );
		if( line_ver.line_number < 0 ) // nothing found - try again with bigger distance
		{
			line_ver = pOverWorld->Layer->Get_diff( crect.x - 20, crect.y, DIR_VERTICAL, 80 );
		}
		direction = DIR_LEFT;

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
	}
	else if( direction == DIR_LEFT )
	{
		line_ver = pOverWorld->Layer->Get_diff( crect.x + 10, crect.y, DIR_VERTICAL, 80 );
		if( line_ver.line_number < 0 ) // nothing found - try again with bigger distance
		{
			line_ver = pOverWorld->Layer->Get_diff( crect.x + 20, crect.y, DIR_VERTICAL, 80 );
		}
		direction = DIR_RIGHT;

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
	}
	else if( direction == DIR_UP )
	{
		line_hor = pOverWorld->Layer->Get_diff( crect.x, crect.y + 10, DIR_HORIZONTAL, 80 );
		if( line_hor.line_number < 0 ) // nothing found - try again with bigger distance
		{
			line_hor = pOverWorld->Layer->Get_diff( crect.x, crect.y + 20, DIR_HORIZONTAL, 80 );
		}
		direction = DIR_DOWN;

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
	}
	else if( direction == DIR_DOWN )
	{
		line_hor = pOverWorld->Layer->Get_diff( crect.x, crect.y - 10, DIR_HORIZONTAL, 80 );
		if( line_hor.line_number < 0 ) // nothing found - try again with bigger distance
		{
			line_hor = pOverWorld->Layer->Get_diff( crect.x, crect.y - 20, DIR_HORIZONTAL, 80 );
		}
		direction = DIR_UP;

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
	}
	else
	{
		printf( "Warning : Unknown Maryo direction %d\n", (int)direction );
		direction = DIR_DOWN;
	}

	Update_Vel();
}

GL_rect cWorld_Maryo :: Get_ColRect( void )
{
	GL_rect crect;
	crect.x = posx + images[0]->w / 2;
	crect.y = posy + images[0]->h / 1.25;
	crect.w = 10;
	crect.h = 10;

	return crect;
}

void cWorld_Maryo :: Auto_Pos_correct( double size /* = 1.7 */, double min_distance /* = 5 */ )
{
	GL_rect rect1 = Get_ColRect();

	if( pOverWorld->Get_Waypoint_Collision( &rect1 ) == -1 )
	{
		Update_PathDiff();

		if( direction == DIR_LEFT || direction == DIR_RIGHT )
		{
			// up and down correction
			if( line_ver.difference > min_distance )
			{
				posy += size * Framerate.speedfactor;
			}
			else if( line_ver.difference < -min_distance )
			{
				posy -= size * Framerate.speedfactor;
			}
		}
		else if( direction == DIR_UP || direction == DIR_DOWN )
		{ 
			// left and right correction
			if( line_hor.difference < -min_distance ) // left
			{
				posx -= size * Framerate.speedfactor;
			}
			else if( line_hor.difference > min_distance ) // right
			{
				posx += size * Framerate.speedfactor;
			}
		}
	}
}
