/***************************************************************************
           sprite.cpp  -  basic sprite class
                             -------------------
    copyright            :	(C) 2003 - 2006 FluXy
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
#include "../main.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSprite :: cSprite( GL_Surface *new_image /* = NULL */, double x /* = 0 */, double y /* = 0 */, bool del_img /* = 0 */, Color *shadow /* = NULL */, bool no_cam /* = 0 */ )
{
	// undefined
	type = TYPE_UNDEFINED;
	array = ARRAY_UNDEFINED;

	// collision data
	col_pos.x = 0;
	col_pos.y = 0;
	col_rect.x = x;
	col_rect.y = y;
	col_rect.w = 0;
	col_rect.h = 0;
	// image data
	rect.x = x;
	rect.y = y;
	rect.w = 0;
	rect.h = 0;
	start_rect.x = 0;
	start_rect.y = 0;
	start_rect.w = 0;
	start_rect.h = 0;

	startposx = 0;
	startposy = 0;

	start_image = NULL;
	image = NULL;
	destroy = 0;
	delete_image = 0;
	shadow_color = shadow;
	no_camera = no_cam;

	if( new_image )
	{
		SetImage( new_image, 0, del_img );
	}

	SetPos( x, y );

	massivetype = MASS_PASSIVE;
	visible = 1;
	spawned = 0;
	player_range = 0;
	state = STAY;

	velx = 0;
	vely = 0;
	rotz = 0;
	rotx = 0;
	roty = 0;
	scale = 1;
	direction = DIR_NOTHING;
	start_direction = DIR_NOTHING;
	groundObject = NULL;

	ID = (unsigned int)rand() % ( 600000 ); // Unique Identifier
}

cSprite :: ~cSprite( void )
{
	if( delete_image && image )
	{
		delete image;
	}

	ClearCollisions();
}

void cSprite :: SetImage( GL_Surface *new_image, bool new_start_image /* = 0 */, bool del_img /* = 0 */ )
{
	if( delete_image )
	{
		if( image )
		{
			// if same image reset startimage
			if( start_image == image )
			{
				start_image = NULL;
			}
			
			delete image;
			image = NULL;
		}

		delete_image = 0;
	}

	image = new_image;

	if( new_image )
	{
		// collision data
		col_pos = image->col_pos;
		col_rect.w = image->col_w;
		col_rect.h = image->col_h;
		// image data
		rect.w = image->w;
		rect.h = image->h;
		
		delete_image = del_img;
	}
	else
	{
		// clear image data
		col_pos.x = 0;
		col_pos.y = 0;
		col_rect.w = 0;
		col_rect.h = 0;
		rect.w = 0;
		rect.h = 0;
	}

	if( !start_image || new_start_image )
	{
		start_image = new_image;

		if( new_image )
		{
			start_rect.w = new_image->w;
			start_rect.h = new_image->h;
		}
		else
		{
			start_rect.w = 0;
			start_rect.h = 0;
		}
	}
}

void cSprite :: SetPos( double x, double y, bool new_startpos /* = 0 */ )
{
	posx = x;
	posy = y;

	if( new_startpos || ( startposx == 0 && startposy == 0 ) )
	{
		startposx = x;
		startposy = y;
	}
}

void cSprite :: SetShadow( Color *shadow )
{
	shadow_color = shadow;
}

void cSprite :: SetCamera( bool enabled )
{
	no_camera = !enabled;
}

void cSprite :: SetRotationX( double rot )
{
	while( rot > 360 )
	{
		rot -= 360;
	}

	while( rot < -360 )
	{
		rot += 360;
	}

	rotx = rot;
}

void cSprite :: SetRotationY( double rot )
{
	while( rot > 360 )
	{
		rot -= 360;
	}

	while( rot < -360 )
	{
		rot += 360;
	}

	roty = rot;
}

void cSprite :: SetRotationZ( double rot )
{
	while( rot > 360 )
	{
		rot -= 360;
	}

	while( rot < -360 )
	{
		rot += 360;
	}

	rotz = rot;
}

void cSprite :: SetRotation( double x, double y, double z )
{
	SetRotationX( x );
	SetRotationY( y );
	SetRotationZ( z );
}

void cSprite :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	direction = dir;

	// turn velocity if wrong
	if( ( dir == DIR_LEFT && velx > 0 ) || ( dir == DIR_RIGHT && velx < 0 ) )
	{
		velx *= -1;
	}

	if( new_start_direction )
	{
		start_direction = dir;
	}
}

void cSprite :: Set_onTop( cSprite *sprite, bool optimize_hor_pos /* = 1 */ )
{
	// set correct onground position
	posy = sprite->posy - col_pos.y - col_rect.h - 0.01;

	// optimize the horizontal position if given
	if( optimize_hor_pos && ( posx < sprite->posx || posx > sprite->posx + sprite->col_rect.w ) )
	{
		posx = sprite->posx + sprite->col_rect.w / 3;
	}
}

void cSprite :: AddRotationX( double rot )
{
	SetRotationX( rotx + rot );
}

void cSprite :: AddRotationY( double rot )
{
	SetRotationY( roty + rot );
}

void cSprite :: AddRotationZ( double rot )
{
	SetRotationZ( rotz + rot );
}

void cSprite :: AddRotation( double x, double y, double z )
{
	SetRotationX( rotx + x );
	SetRotationY( roty + y );
	SetRotationZ( rotz + z );
}

void cSprite :: Auto_SlowDown( double x_speed, double y_speed /* = 0 */ )
{
	// horizontal slow down
	if( x_speed > 0 )
	{
		if( velx > 0 )
		{
			AddVel( -x_speed, 0 );

			if( velx < 0 )
			{
				velx = 0;
			}
		}
		else if( velx < 0 )
		{
			AddVel( x_speed, 0 );

			if( velx > 0 )
			{
				velx = 0;
			}
		}
	}

	// vertical slow down
	if( y_speed > 0 )
	{
		if( vely > 0 )
		{
			AddVel( 0, -y_speed );

			if( vely < 0 )
			{
				vely = 0;
			}
		}
		else if( vely < 0 )
		{
			AddVel( 0, y_speed );

			if( vely > 0 )
			{
				vely = 0;
			}
		}
	}
}

ObjectDirection cSprite :: Move( double move_x, double move_y, bool real /* = 0 */, bool force /* = 0 */, bool check_onground /* = 1 */ )
{
	if( move_x == 0 && move_y == 0 )
	{
		return DIR_NOTHING;
	}

	if( col_rect.w == 0 || col_rect.h == 0 )
	{
		return DIR_NOTHING;
	}

	if( !real )
	{
		move_x *= Framerate.speedfactor;
		move_y *= Framerate.speedfactor;
	}

	ObjectDirection collision_dir = DIR_NOTHING;

	if( !force ) // check for collisions
	{
		double posx_old = posx;
		double posy_old = posy;
		double check_x = move_x;
		double check_y = move_y;

		// check if object collision rect is smaller as the position check size
		if( move_x > col_rect.w )
		{
			move_x = col_rect.w;
		}
		else if( move_x < -col_rect.w )
		{
			move_x = -col_rect.w;
		}

		if( move_y > col_rect.h )
		{
			move_y = col_rect.h;
		}
		else if( move_y < -col_rect.h )
		{
			move_y = -col_rect.h;
		}

		/* Checks in both directions simultan
		 * if a collision occurs changes to pixel collision testing
		*/
		while( check_x != 0 || check_y != 0 )
		{
			if( check_x != 0 )
			{
				if( ColCheckAuto( check_x, 0, 0, 0, ( check_x <= 1 && check_x >= -1 ) ? (0) : (1) ) == CO_NOTHING )
				{
					posx += check_x;

					if( ( check_x > 0 && posx_old + move_x <= posx ) || ( check_x < 0 && posx_old + move_x >= posx ) )
					{
						posx = posx_old + move_x;
						check_x = 0;
					}
				}
				else // collision found
				{
					if( check_x <= 1 && check_x >= -1 ) // final collision
					{
						if( collision_dir == DIR_NOTHING )
						{
							collision_dir = DIR_HORIZONTAL;	// Collision Left/Right
						}
						else
						{
							collision_dir = DIR_ALL;	// Collision Up/Down/Left/Right
						}
						
						check_x = 0;
					}
					else // set to pixel movement
					{
						if( check_x > 0 )
						{
							check_x = 1;
						}
						else
						{
							check_x = -1;
						}
					}
				}
			}

			if( check_y != 0 )
			{
				if( ColCheckAuto( 0, check_y, 0, 0, ( check_y <= 1 && check_y >= -1 ) ? (0) : (1) ) == CO_NOTHING )
				{
					posy += check_y;

					if( ( check_y > 0 && posy_old + move_y <= posy ) || ( check_y < 0 && posy_old + move_y >= posy ) )
					{
						posy = posy_old + move_y;
						check_y = 0;
					}
				}
				else // collision found
				{
					if( check_y <= 1 && check_y >= -1 ) // final collision
					{
						if( collision_dir == DIR_NOTHING ) 
						{
							collision_dir = DIR_VERTICAL;	// Collision Up/Down
						}
						else
						{
							collision_dir = DIR_ALL;	// Collision Up/Down/Left/Right
						}

						check_y = 0;
					}
					else // set to pixel movement
					{
						if( check_y > 0 )
						{
							check_y = 1;
						}
						else
						{
							check_y = -1;
						}
					}
				}
			}
		}
	}
	else // do not check for collisions
	{
		posx += move_x;
		posy += move_y;
	}

	if( check_onground )
	{
		Check_onGround();
	}

	return collision_dir;
}

void cSprite :: AddVel( double move_x, double move_y, bool real /* = 0 */ )
{
	if( real )
	{
		velx += move_x;
		vely += move_y;
	}
	else
	{
		velx += move_x * Framerate.speedfactor;
		vely += move_y * Framerate.speedfactor;
	}
}

void cSprite :: Turn_Around( void )
{
	velx *= -1;
	Update_Direction();
}

void cSprite :: Draw( void )
{
	if( !visible && !Leveleditor_Mode )
	{
		return;
	}

	if( !Leveleditor_Mode || Game_Mode != MODE_LEVEL )
	{
		rect.x = posx;
		rect.y = posy;
		// editor rect
		start_rect.x = posx;
		start_rect.y = posy;
		// collision rect
		col_rect.x = posx + col_pos.x;
		col_rect.y = posy + col_pos.y;
	}
	else
	{
		rect.x = startposx;
		rect.y = startposy;
		start_rect.x = startposx;
		start_rect.y = startposy;
		col_rect.x = startposx + col_pos.x; // todo : startcol_pos ?
		col_rect.y = startposy + col_pos.y;
	}

	if( !no_camera && !isVisible_onScreen() ) 
	{
		return;
	}

	// draw the collision rect
	/*SDL_Rect rect1 = Get_Col_rect();
	rect1.x -= (Sint16)cameraposx;
	rect1.y -= (Sint16)cameraposy;
	DrawRect( &rect1, &white );*/

	if( image )
	{
		if( !Leveleditor_Mode || Game_Mode != MODE_LEVEL )
		{
			if( !no_camera )
			{
				image->Blit( posx - cameraposx, posy - cameraposy, shadow_color, rotx, roty, rotz, scale );

				if( Game_debug )
				{
					glBlendFunc( GL_SRC_COLOR, GL_DST_ALPHA );
					if( type == TYPE_PLAYER )
					{
						DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &lila );
					}
					else if( array == ARRAY_ENEMY )
					{
						DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &red );
					}
					else if( array == ARRAY_ACTIVE )
					{
						DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &blue );
					}
					else if( array == ARRAY_MASSIVE )
					{
						DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &orange );
					}
					else if( array == ARRAY_PASSIVE )
					{
						DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &green );
					}

					if( groundObject && type == TYPE_PLAYER )
					{
						DrawRect( groundObject->col_rect.x - cameraposx, groundObject->col_rect.y  - cameraposy, groundObject->col_rect.w, groundObject->col_rect.h, &grey );
					}
					
					glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				}
			}
			else
			{
				image->Blit( posx, posy, shadow_color, rotx, roty, rotz, scale );
			}
		}
		else if( Leveleditor_Mode && start_image )
		{
			if( !no_camera )
			{
				start_image->Blit( startposx - cameraposx, startposy - cameraposy, shadow_color, rotx, roty, rotz, scale );
			}
			else
			{
				start_image->Blit( startposx, startposy, shadow_color, rotx, roty, rotz, scale );
			}
		}
	}
}

bool cSprite :: isVisible_onScreen( void )
{
	if( rect.x + rect.w < cameraposx ) // not visible (left)
	{
		return 0;
	}
	else if( rect.x > cameraposx + GAME_RES_W ) // not visible (right)
	{
		return 0;
	}
	else if( rect.y + rect.h < cameraposy ) // not visible (down)
	{
		return 0;
	}
	else if( rect.y > cameraposy + GAME_RES_H ) // not visible (up)
	{
		return 0;
	}

	return 1;
}

bool cSprite :: is_onTop( cSprite *obj )
{
	if( !obj )
	{
		return 0;
	}

	// always collide upwards because of the image size collision checking
	if( col_rect.x + col_rect.w > obj->col_rect.x && col_rect.x < obj->col_rect.x + obj->col_rect.w &&
		col_rect.y + col_rect.h < obj->col_rect.y )
	{
		return 1;
	}

	return 0;
}

bool cSprite :: isPlayer_range( void )
{
	if( player_range && ( posx < pPlayer->posx - player_range || posy < pPlayer->posy - player_range ||
		posx > pPlayer->posx + player_range || posy > pPlayer->posy + player_range ) )
	{
		return 0;
	}

	return 1;
}

ObjectCollisionType cSprite :: ColCheckAuto( double x, double y, double w /* = 0 */, double h /* = 0 */, bool only_check /* = 0 */ )
{
	GL_rect rectb;
	// use position with collision position and add the given position
	rectb.x = posx + col_pos.x + x;
	rectb.y = posy + col_pos.y + y;

	// use width if given
	if( w > 0 )
	{
		rectb.w = w;
	}
	else
	{
		rectb.w = col_rect.w;
	}
	// use height if given
	if( h > 0 )
	{
		rectb.h = h;
	}
	else
	{
		rectb.h = col_rect.h;
	}

	if( Game_debug )
	{
		glBlendFunc( GL_SRC_COLOR, GL_DST_ALPHA );
		DrawRect( rectb.x - cameraposx, rectb.y - cameraposy, rectb.w, rectb.h, &green );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	return ColCheck( &rectb, only_check );
}

ObjectCollisionType cSprite :: ColCheck( double x, double y, double w /* = 0 */, double h /* = 0 */, bool only_check /* = 0 */ )
{
	GL_rect rectb;
	rectb.x = x;
	rectb.y = y;

	// use width and correct the position if given
	if( w > 0 )
	{
		rectb.x += ( rect.w - w ) / 2;
		rectb.w = w;
	}
	else
	{
		rectb.w = col_rect.w;
	}
	// use height and correct the position if given
	if( h > 0 )
	{
		rectb.y += ( rect.h - h ) / 2;
		rectb.h = h;
	}
	else
	{
		rectb.h = col_rect.h;
	}

	return ColCheck( &rectb, only_check );
}
	
ObjectCollisionType cSprite :: ColCheck( GL_rect *rect1, bool only_check /* = 0 */ )
{
	if( rect1->w == 0 || rect1->h == 0 )
	{
		return CO_NOTHING;
	}

	unsigned int i;
	
	// Active Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		/*	There's no collision if :
		 *	1. This is an Cloud, Banzaii_Bill,  
		 *	2. This is an Fireball, Mushroom, moving Turtle shell and the colliding Object is an Enemystopper
		 *  3. This is an Enemy and the colliding Object is a Powerup
		 */
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ACTIVE || ID == pObjManager->items[i]->ID || type == TYPE_CLOUD || type == TYPE_MOVING_PLATFORM || ( ( type == TYPE_BANZAI_BILL || type == TYPE_JPIRANHA ) && pObjManager->items[i]->type != TYPE_FIREBALL ) ||
			( ( type == TYPE_FIREBALL || type == TYPE_MUSHROOM_DEFAULT || type == TYPE_MUSHROOM_LIVE_1 ||
			( type == TYPE_TURTLE && state == RUN ) ) && pObjManager->items[i]->type == TYPE_ENEMYSTOPPER ) || 
			( array == ARRAY_ACTIVE && ( pObjManager->items[i]->type == TYPE_GOLDPIECE || pObjManager->items[i]->type == TYPE_BOUNCINGGOLDPIECE ) ) || 
			( array == ARRAY_ENEMY && ( pObjManager->items[i]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[i]->type == TYPE_MUSHROOM_LIVE_1 || pObjManager->items[i]->type == TYPE_MOON ) ) )
		{
			continue;
		}

		if( type == TYPE_PLAYER ) // if player
		{
			if( pObjManager->items[i]->massivetype == MASS_PASSIVE ) 
			{
				continue;
			}
		}
		else // if no player
		{
			if( !pObjManager->items[i]->massivetype && pObjManager->items[i]->type != TYPE_ENEMYSTOPPER && 
				pObjManager->items[i]->type != TYPE_MUSHROOM_DEFAULT && pObjManager->items[i]->type != TYPE_MUSHROOM_LIVE_1 ) 
			{
				continue;
			}
			else if( pObjManager->items[i]->type == TYPE_CLOUD || type == TYPE_MOVING_PLATFORM || pObjManager->items[i]->type == TYPE_FIREPLANT || pObjManager->items[i]->massivetype == MASS_CLIMBABLE ) 
			{
				continue;
			}
			else if( ( pObjManager->items[i]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[i]->type == TYPE_MUSHROOM_LIVE_1 ) && type == TYPE_FIREBALL )
			{
				continue;
			}
		}

		if( pObjManager->items[i]->massivetype == MASS_HALFMASSIVE ) // halfmassive
		{
			// if moving downwards and object is on top
			if( vely >= 0 && is_onTop( pObjManager->items[i] ) )
			{
				// valid
			}
			else
			{
				continue;
			}
		}

		if( Col_Box( rect1, &pObjManager->items[i]->col_rect ) )
		{
			if( !only_check )
			{
				Add_Collision( this, pObjManager->items[i], i, CO_ACTIVE );
			}

			if( pObjManager->items[i]->massivetype == MASS_CLIMBABLE )
			{
				if( type == TYPE_PLAYER )
				{
					GL_rect rect3 = pPlayer->col_rect;

					// small rect for collision checking
					rect3.h -= rect3.h * 0.6;
					rect3.w -= rect3.w * 0.8;
					rect3.x += rect3.w / 2;
					rect3.y += rect3.h / 2;

					if( Col_Box( &rect3, &pObjManager->items[i]->col_rect ) )
					{
						if( ( keys[pPreferences->Key_up] || pJoystick->up ) || ( keys[pPreferences->Key_down] || pJoystick->down ) )
						{
							pPlayer->Start_climbing();
						}
					}
				}

				continue;
			}

			return CO_ACTIVE;
		}
	}
	
	// Massive Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_MASSIVE || ID == pObjManager->items[i]->ID || 
			pObjManager->items[i]->massivetype != MASS_MASSIVE || type == TYPE_BANZAI_BILL || type == TYPE_JPIRANHA )
		{
			continue;
		}

		if( Col_Box( rect1, &pObjManager->items[i]->col_rect ) )
		{
			if( !only_check )
			{
				Add_Collision( this, pObjManager->items[i], i, CO_MASSIVE );
			}

			return CO_MASSIVE;
		}
	}

	// Enemy Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( !pObjManager->items[i] || pObjManager->items[i]->array != ARRAY_ENEMY || ID == pObjManager->items[i]->ID || massivetype == MASS_PASSIVE || pObjManager->items[i]->massivetype == MASS_PASSIVE || // only valid enemies
			type == TYPE_MUSHROOM_DEFAULT || type == TYPE_MUSHROOM_LIVE_1 || type == TYPE_FIREPLANT || // items should not collide with enemies
			type == TYPE_BANZAI_BILL || // banzai bill should not collide with enemies
			( !( type == TYPE_PLAYER && ( pPlayer->invincible <= 0 || pPlayer->invincible_star <= 0 ) ) && pObjManager->items[i]->type == TYPE_BANZAI_BILL ) || // only default player should collide with banzai bill
			( array == ARRAY_ENEMY && !( type == TYPE_TURTLE && state == RUN ) && pObjManager->items[i]->type == TYPE_JPIRANHA ) || // no enemy except turtle shells should collide with jumping piranhas
			( type == TYPE_JPIRANHA && pObjManager->items[i]->array == ARRAY_ENEMY ) ) // jumping piranha should not collide with enemies
		{
			continue;
		}

		GL_rect rect2 = pObjManager->items[i]->col_rect;

		if( pObjManager->items[i]->type != TYPE_BANZAI_BILL )
		{
			if( type == TYPE_PLAYER )
			{
				rect2.x += ( rect2.w - ( rect2.w * 0.6 ) ) / 2;
				rect2.w = rect2.w * 0.6;

				rect2.y += ( rect2.h - ( rect2.h * 0.5 ) ) / 2;
				rect2.h = rect2.h * 0.5;
			}
			else if( array == ARRAY_ENEMY )
			{
				rect2.x += ( rect2.w - ( rect2.w * 0.8 ) ) / 2;
				rect2.w = rect2.w * 0.8;

				rect2.y += (rect2.h - ( rect2.h * 0.8 ) ) / 2;
				rect2.h = rect2.h * 0.8;
			}
		}

		if( Col_Box( rect1, &rect2 ) )
		{
			 // do not collide with enemies if invincible from star
			if( type == TYPE_PLAYER && pPlayer->invincible_star > 0 )
			{
				// if colliding force complete downgrade
				if( Col_Box( &pPlayer->col_rect, &pObjManager->items[i]->col_rect ) )
				{
					pAudio->PlaySound( SOUNDS_DIR "/fireball.ogg" );
					pObjManager->items[i]->DownGrade( 1 );
				}
				
				continue;
			}

			if( !only_check ) 
			{
				Add_Collision( this, pObjManager->items[i], i, CO_ENEMY );

				cObjectCollision *col = Get_lastCollision();

				if( ( pObjManager->items[i]->velx >= velx && velx > 0 && col->direction == DIR_LEFT ) || 
					( pObjManager->items[i]->velx <= velx && velx < 0 && col->direction == DIR_RIGHT ) ||
					( pObjManager->items[i]->velx < 0 && velx > 0 ) || ( pObjManager->items[i]->velx > 0 && velx < 0 ) ) 
				{
					pObjManager->items[i]->EnemyCollision( col->direction );
				}
			}

			return CO_ENEMY;
		}
	}

	// Player
	if( massivetype == MASS_MASSIVE && type != TYPE_PLAYER && type != TYPE_FIREBALL && type != TYPE_MUSHROOM_DEFAULT && type != TYPE_MUSHROOM_LIVE_1 &&
		!( type == TYPE_TURTLE && state == RUN && ( ( (cTurtle *)this ) )->playercounter ) && !( type == TYPE_BANZAI_BILL && pPlayer->invincible > 0 ) )
	{
		GL_rect rect2 = pPlayer->col_rect;

		if( type != TYPE_BANZAI_BILL )
		{
			if( array == ARRAY_ENEMY )
			{
				rect2.x += ( rect2.w - rect2.w * 0.6 ) / 2;
				rect2.w = rect2.w * 0.6;

				rect2.y += ( rect2.h - rect2.h * 0.6 ) / 2;
				rect2.h = rect2.h * 0.6;
			}
		}
		
		if( Col_Box( rect1, &rect2 ) )
		{
			if( !only_check )
			{
				Add_Collision( this, pPlayer, 0, CO_PLAYER );
			}

			return CO_PLAYER;
		}
	}

	return CO_NOTHING;
}

void cSprite :: Set_onGround( cObjectCollision *collision )
{
	if( !collision || collision->type == CO_PLAYER || collision->type == CO_ENEMY )
	{
		return;
	}

	groundObject = pObjManager->Get_Pointer( collision->number );
	Set_onTop( groundObject, 0 );
}

void cSprite :: Check_onGround( void )
{
	if( type != TYPE_PLAYER && array != ARRAY_ENEMY && array != ARRAY_ACTIVE )
	{
		return;
	}

	// if on ground
	if( groundObject )
	{
		GL_rect rect2( col_rect.x, col_rect.y + col_rect.h, col_rect.w, 1 );

		if( Col_Box( &groundObject->col_rect, &rect2 ) )
		{
			return;
		}
		else
		{
			Reset_onGround();
		}
	}

	// if no collision
	if( ColCheckAuto( 0, col_rect.h, 0, 1 ) == CO_NOTHING )
	{
		Reset_onGround();
	}
	else // collision found
	{
		cObjectCollision *collision = Get_lastCollision();

		// if no valid collision
		if( !collision )
		{
			Reset_onGround();
		}
		else
		{
			if( collision->direction == DIR_NOTHING )
			{
				Reset_onGround();
			}
			else
			{
				Set_onGround( collision );
			}
			
			Delete_lastCollision();
		}
	}
}

void cSprite :: Reset_onGround( void )
{
	groundObject = NULL;
}

void cSprite :: Update_antistuck( void )
{
	if( ColCheck( posx, posy ) == DIR_NOTHING )
	{
		return;
	}

	cObjectCollision *collision = Get_lastCollision();

	if( !collision )
	{
		return;
	}

	if( collision->type != CO_ENEMY && !( collision->type == CO_ACTIVE && ( pObjManager->items[collision->number]->massivetype == MASS_HALFMASSIVE || pObjManager->items[collision->number]->massivetype == MASS_CLIMBABLE ) ) )
	{
		if( ColCheck( posx + col_rect.w / 2, posy, 0, 0, 1 ) == DIR_NOTHING ) 
		{
			Move( 2, 0 );
		}
		else if( ColCheck( posx - col_rect.w / 2, posy, 0, 0, 1 ) == DIR_NOTHING ) 
		{
			Move( -2, 0 );
		}
		else if( ColCheck( posx, posy + col_rect.h / 2, 0, 0, 1 ) == DIR_NOTHING ) 
		{
			Move( 0, 2 );
		}
		else if( ColCheck( posx, posy - col_rect.h / 2, 0, 0, 1 ) == DIR_NOTHING ) 
		{
			Move( 0, -2 );
		}
	}

	Delete_lastCollision();
}

void cSprite :: CollideMove( void )
{
	// if the ground object moves also move this sprite
	if( groundObject && ( groundObject->array == ARRAY_ACTIVE || groundObject->array == ARRAY_MASSIVE ) )
	{
		if( groundObject->velx != 0 || groundObject->vely != 0 )
		{
			Move( groundObject->velx, groundObject->vely );
		}
	}

	ClearCollisions();

	// move and creates collision data
	Move( velx, vely );

	// don't move out of screen
	if( posx < 0 && type != TYPE_BANZAI_BILL )
	{
		posx = 0;

		// turn around if out of screen
		if( array == ARRAY_ENEMY || array == ARRAY_ACTIVE )
		{
			Turn_Around();
		}
	}

	// handle the found collisions
	for( unsigned int i = 0; i < collisions.size(); i++ )
	{
		HandleCollision( collisions[i] );
	}
}

void cSprite :: Add_Collision( cSprite *base, cSprite *col, unsigned int col_num /* = 0 */, ObjectCollisionType col_type /* = CO_NOTHING */ )
{
	cObjectCollision *collision = new cObjectCollision();

	collision->number = col_num;
	collision->type = col_type;
	if( base && col )
	{
		collision->Set_direction( base, col );
	}

	collisions.push_back( collision );
}

void cSprite :: Delete_lastCollision( void )
{
	if( collisions.size() )
	{
		collisions.erase( collisions.begin() + collisions.size() - 1 );
	}
}

int cSprite :: Check_CollisionDir( ObjectDirection dir )
{
	for( unsigned int i = 0; i < collisions.size(); i++ )
	{
		if( collisions[i]->direction == dir )
		{
			return i;
		}
	}

	return -1;
}

cObjectCollision *cSprite :: Get_firstCollision( void )
{
	if( collisions.size() )
	{
		return collisions[0];
	}

	return NULL;
}

cObjectCollision *cSprite :: Get_lastCollision( void )
{
	if( collisions.size() > 0 )
	{
		return collisions[collisions.size() - 1];
	}

	return NULL;
}

void cSprite :: ClearCollisions( void )
{
	for( unsigned int i = 0; i < collisions.size(); i++ )
	{
		delete collisions[i];
	}

	collisions.clear();
}

void cSprite :: DownGrade( bool force )
{
	// virtual
}

void cSprite :: Destroy( void )
{
	destroy = 1;
}

void cSprite :: Update( void )
{
	Draw();
}

void cSprite :: Update_Direction( void )
{
	if( velx < 0 )
	{
		direction = DIR_LEFT;
	}
	else if( velx > 0 )
	{
		direction = DIR_RIGHT;
	}
}

void cSprite :: Update_Rotation_velx( void )
{
	if( velx < 0 ) // left
	{
		roty = 0;
	}
	else if( velx > 0 ) // right
	{
		roty = 180;
	}
}

void cSprite :: HandleCollision( cObjectCollision *collision )
{
	// virtual
}

void cSprite :: PlayerCollision( ObjectDirection direction )
{
	// virtual
}

void cSprite :: EnemyCollision( ObjectDirection direction )
{
	// virtual
}

void cSprite :: BoxCollision( ObjectDirection direction, GL_rect *r2 )
{
	// virtual
}

void cSprite :: Editor_Activate( void )
{
	// virtual
}

void cSprite :: Editor_Deactivate( void )
{
	// virtual
}

void cSprite :: Editor_pos_update( void )
{
	// virtual
}

void cSprite :: Editor_val_update( void )
{
	// virtual
}

/* *** *** *** *** *** *** *** cObjectSprite *** *** *** *** *** *** *** *** *** *** */

cObjectSprite :: cObjectSprite( double x, double y ) : cSprite( NULL, x, y )
{
	curr_img = -1;
}

cObjectSprite :: ~cObjectSprite( void )
{
	images.clear();
}

void cObjectSprite :: SetImage( int num, bool new_Startimage /* = 0 */ )
{
	if( curr_img == num )
	{
		return;
	}

	curr_img = num;

	if( curr_img < 0 )
	{
		cSprite::SetImage( NULL, new_Startimage );
	}
	else if( (unsigned int)curr_img < images.size() )
	{
		cSprite::SetImage( images[curr_img], new_Startimage );
	}
	else if( Game_debug )
	{
		printf( "Warning : Object image number (%d) bigger as the array size (%d)\n", curr_img, images.size() );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
