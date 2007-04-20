/***************************************************************************
           movingsprite.cpp  -  moving sprite class
                             -------------------
    copyright            :	(C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../core/globals.h"
#include "../objects/movingsprite.h"
#include "../core/main.h"
#include "../core/framerate.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../core/obj_manager.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../audio/audio.h"
#include "../player/player.h"
#include "../enemies/turtle.h"
#include "../input/keyboard.h"
#include "../objects/box.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** cMovingSprite *** *** *** *** *** *** *** *** *** *** */

cMovingSprite :: cMovingSprite( GL_Surface *new_image /* = NULL */, float x /* = 0 */, float y /* = 0 */, bool del_img /* = 0 */ )
: cSprite( new_image, x, y, del_img )
{
	Init();
}

cMovingSprite :: cMovingSprite( XMLAttributes &attributes )
: cSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cMovingSprite :: ~cMovingSprite( void )
{
	if( delete_image && image )
	{
		delete image;
	}

	Collisions_Clear();
}

void cMovingSprite :: Init( void )
{
	state = STA_STAY;

	velx = 0;
	vely = 0;
	
	direction = DIR_UNDEFINED;
	start_direction = DIR_UNDEFINED;
	ground_object = NULL;

	ice_resistance = 0;
	freeze_counter = 0;
}

cMovingSprite *cMovingSprite :: Copy( void )
{
	cMovingSprite *moving_sprite = new cMovingSprite( start_image, startposx, startposy );

	moving_sprite->type = type;
	moving_sprite->sprite_array = sprite_array;
	moving_sprite->Set_Massivetype( massivetype );
	moving_sprite->Set_Ignore_Camera( no_camera );
	moving_sprite->Set_Shadow_Pos( shadow_pos );
	moving_sprite->Set_Shadow_Color( shadow_color );

	return moving_sprite;
}

void cMovingSprite :: Set_Image( GL_Surface *new_image, bool new_start_image /* = 0 */, bool del_img /* = 0 */ )
{
	// get a possible collision point change
	GL_point col_pos_change = GL_point();

	if( image && new_image )
	{
		col_pos_change = new_image->col_pos - image->col_pos;
	}

	cSprite::Set_Image( new_image, new_start_image, del_img );

	// handle collision point change
	if( col_pos_change.x != 0 || col_pos_change.y != 0 )
	{
		Move( -col_pos_change.x, -col_pos_change.y, 1 );
	}

	// check onground
	Check_onGround();
}

void cMovingSprite :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
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

void cMovingSprite :: Set_Direction( float angle, float speed, bool new_start_direction /* = 0 */ )
{
	velx = cos( angle * 0.01745329f ) * speed;
	vely = sin( angle * 0.01745329f ) * speed;

	Update_Direction();

	if( new_start_direction )
	{
		start_direction = direction;
	}
}

void cMovingSprite :: Auto_SlowDown( float x_speed, float y_speed /* = 0 */ )
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

void cMovingSprite :: Move( float move_x, float move_y, bool real /* = 0 */ )
{
	cSprite::Move( move_x, move_y, real );

	Check_OutofLevel( move_x, move_y, 1 );
}

ObjectDirection cMovingSprite :: Col_Move( float move_x, float move_y, bool real /* = 0 */, bool force /* = 0 */, bool check_onground /* = 1 */ )
{
	// nothing to move
	if( move_x == 0 && move_y == 0 )
	{
		return DIR_UNDEFINED;
	}

	// invalid collision rect
	if( col_rect.w == 0 || col_rect.h == 0 )
	{
		return DIR_UNDEFINED;
	}

	// use speedfactor
	if( !real )
	{
		move_x *= pFramerate->speedfactor;
		move_y *= pFramerate->speedfactor;
	}

	ObjectDirection collision_dir = DIR_UNDEFINED;
 
	// check for collisions
	if( !force )
	{
		float posx_old = posx;
		float posy_old = posy;
		float check_x = move_x;
		float check_y = move_y;

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

		/* Checks in both directions simultaneous
		 * if a collision occurs changes to pixel collision testing
		*/
		while( check_x < -0.01f || check_x > 0.01f || check_y < -0.01f || check_y > 0.01f )
		{
			if( check_x != 0 )
			{
				if( !ColCheckAuto( check_x, 0, 0, 0, ( check_x <= 1 && check_x >= -1 ) ? (COLLIDE_COMPLETE) : (COLLIDE_ALLOW_INTERNAL) ).size() )
				{
					posx += check_x;

					if( ( check_x > 0 && posx_old + move_x <= posx ) || ( check_x < 0 && posx_old + move_x >= posx ) )
					{
						posx = posx_old + move_x;
						check_x = 0;
					}

					// update collision rects
					Update_Position_Rect();
				}
				// collision found
				else
				{
					// final collision
					if( check_x <= 1 && check_x >= -1 )
					{
						if( collision_dir == DIR_UNDEFINED )
						{
							// Collision Left/Right
							collision_dir = DIR_HORIZONTAL;
						}
						else
						{
							// Collision Up/Down/Left/Right
							collision_dir = DIR_ALL;
						}
						
						check_x = 0;
					}
					// set pixel movement
					else
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
				if( !ColCheckAuto( 0, check_y, 0, 0, ( check_y <= 1 && check_y >= -1 ) ? (COLLIDE_COMPLETE) : (COLLIDE_ALLOW_INTERNAL) ).size() )
				{
					posy += check_y;

					if( ( check_y > 0 && posy_old + move_y <= posy ) || ( check_y < 0 && posy_old + move_y >= posy ) )
					{
						posy = posy_old + move_y;
						check_y = 0;
					}

					// update collision rects
					Update_Position_Rect();
				}
				// collision found
				else
				{
					// final collision
					if( check_y <= 1 && check_y >= -1 )
					{
						if( collision_dir == DIR_UNDEFINED ) 
						{
							// Collision Up/Down
							collision_dir = DIR_VERTICAL;
						}
						else
						{
							// Collision Up/Down/Left/Right
							collision_dir = DIR_ALL;
						}

						check_y = 0;
					}
					// set pixel movement
					else
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
	// don't check for collisions
	else
	{
		posx += move_x;
		posy += move_y;
	}

	if( collision_dir == DIR_UNDEFINED )
	{
		// update the position values
		Update_Position_Rect();
	}

	// if check on ground
	if( check_onground )
	{
		Check_onGround();
	}

	// handle move out of level rect
	Check_OutofLevel( move_x, move_y, 1 );

	return collision_dir;
}

void cMovingSprite :: AddVel( float move_x, float move_y, bool real /* = 0 */ )
{
	if( real )
	{
		velx += move_x;
		vely += move_y;
	}
	else
	{
		velx += move_x * pFramerate->speedfactor;
		vely += move_y * pFramerate->speedfactor;
	}
}

void cMovingSprite :: Turn_Around( ObjectDirection col_dir /* = DIR_UNDEFINED */ )
{
	if( col_dir != DIR_UNDEFINED )
	{
		// check if the collision direction is in the front
		if( ( col_dir == DIR_RIGHT && direction != DIR_RIGHT ) || ( col_dir == DIR_LEFT && direction != DIR_LEFT ) ||
			( col_dir == DIR_UP && direction != DIR_UP ) || ( col_dir == DIR_DOWN && direction != DIR_DOWN ) )
		{
			// collision direction is not in the front
			return;
		}
	}

	// reverse velocity
	if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		velx *= -1;
	}
	else if( direction == DIR_UP || direction == DIR_DOWN )
	{
		vely *= -1;
	}

	Update_Direction();
}

void cMovingSprite :: Update( void )
{
	if( freeze_counter > 0 )
	{
		freeze_counter -= pFramerate->speedfactor;

		if( freeze_counter < 0 )
		{
			freeze_counter = 0;
		}
	}
}

void cMovingSprite :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cSurfaceRequest();
	}

	cSprite::Draw( request );

	if( !editor_enabled )
	{
		// frozen
		if( freeze_counter )
		{
			request->combine_type = GL_ADD;

			float counter_add = freeze_counter;

			if( counter_add > 1000 )
			{
				counter_add = 1000;
			}

			request->combine_col[0] = counter_add * 0.003f;
			request->combine_col[1] = counter_add * 0.003f;
			request->combine_col[2] = counter_add * 0.0099f;
		}
	}


	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

cObjectCollisionType cMovingSprite :: ColCheckAuto( float x, float y, float w /* = 0 */, float h /* = 0 */, ColCheckType check_type /* = COLLIDE_COMPLETE */ )
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
		// create request
		cRectRequest *request = new cRectRequest();

		pVideo->Draw_Rect( rectb.x - pCamera->x, rectb.y - pCamera->y, rectb.w, rectb.h, posz + 0.00001f, &green, request );
		request->blend_sfactor = GL_SRC_COLOR;
		request->blend_dfactor = GL_DST_ALPHA;

		// add request
		pRenderer->Add( request );
	}

	return ColCheck( &rectb, check_type );
}

cObjectCollisionType cMovingSprite :: ColCheck( float x /* = 0 */, float y /* = 0 */, float w /* = 0 */, float h /* = 0 */, ColCheckType check_type /* = COLLIDE_COMPLETE */ )
{
	GL_rect rectb;

	if( x != 0 )
	{
		rectb.x = x;
	}
	else
	{
		rectb.x = posx + col_pos.x;
	}

	if( y != 0 )
	{
		rectb.y = y;
	}
	else
	{
		rectb.y = posy + col_pos.y;
	}

	// use width and correct the position if given
	if( w > 0 )
	{
		rectb.x += ( col_rect.w - w ) / 2;
		rectb.w = w;
	}
	else
	{
		rectb.w = col_rect.w;
	}
	// use height and correct the position if given
	if( h > 0 )
	{
		rectb.y += ( col_rect.h - h ) / 2;
		rectb.h = h;
	}
	else
	{
		rectb.h = col_rect.h;
	}

	return ColCheck( &rectb, check_type );
}
	
cObjectCollisionType cMovingSprite :: ColCheck( GL_rect *rect1, ColCheckType check_type /* = COLLIDE_COMPLETE */ )
{
	// blocking collisions list
	cObjectCollisionType col_list = cObjectCollisionType();

	// no width or height is invalid
	if( rect1->w == 0 || rect1->h == 0 )
	{
		return col_list;
	}

	// Check Objects
	for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSprite *level_object = (*itr);

		// if not Active/Massive/Enemy
		if( !( level_object->sprite_array == ARRAY_ACTIVE || level_object->sprite_array == ARRAY_MASSIVE || level_object->sprite_array == ARRAY_ENEMY ) )
		{
			continue;
		}

		// if rects doesn't touch
		if( !Col_Box( rect1, &level_object->col_rect ) )
		{
			continue;
		}

		// if enemy is dead
		if( level_object->sprite_array == ARRAY_ENEMY && static_cast<cEnemy *>(level_object)->dead )
		{
			continue;
		}

		// if the same object
		if( this == level_object )
		{
			continue;
		}

		// if col_valid is 1 it's an internal collision
		unsigned int col_valid = Validate_Collision( level_object );

		// not a valid collision
		if( !col_valid )
		{
			continue;
		}

		// ignore internal collisions
		if( check_type == COLLIDE_ALLOW_BLOCKING && col_valid == 1 )
		{
			continue;
		}

		// get collision type
		ObjectCollisionType col_type = Get_CollisionType( level_object->sprite_array );

		// todo : check if check_type should be used instead for validation
		// add to list if full collision
		if( col_valid == 2 )
		{
			col_list.add( col_type );
		}

		// ignore full collisions
		if( check_type == COLLIDE_ALLOW_INTERNAL && col_valid == 2 )
		{
			continue;
		}

		// add collision
		if( check_type != COLLIDE_ONLY_CHECK )
		{
			Collision_Add( this, level_object, pObjManager->Get_Array_num( level_object ), col_type, ( col_valid == 1 ) ? (1) : (0) );
		}
	}

	// Player
	if( type != TYPE_PLAYER && Col_Box( rect1, &pPlayer->col_rect ) )
	{
		// if col_valid is 1 it's an internal collision
		unsigned int col_valid = Validate_Collision( pPlayer );

		// ignore internal collisions
		if( check_type == COLLIDE_ALLOW_BLOCKING && col_valid == 1 )
		{
			col_valid = 0;
		}

		if( col_valid )
		{
			// full collision
			if( col_valid == 2 )
			{
				col_list.add( CO_PLAYER );
			}

			// ignore full collisions
			if( !( check_type == COLLIDE_ALLOW_INTERNAL && col_valid == 2 ) )
			{
				if( check_type != COLLIDE_ONLY_CHECK )
				{
					Collision_Add( this, pPlayer, 0, CO_PLAYER, ( col_valid == 1 ) ? (1) : (0) );
				}
			}
		}
	}

	return col_list;
}

bool cMovingSprite :: Check_OutofLevel( float move_x, float move_y, bool handle /* = 0 */ )
{
	// left
	if( col_rect.x < pCamera->limit_rect.x && col_rect.x - ( move_x - 0.00001f ) >= pCamera->limit_rect.x  )
	{
		if( handle )
		{
			Handle_OutofLevel( DIR_LEFT );
		}

		return 1;
	}
	// right
	else if( col_rect.x + col_rect.w > pCamera->limit_rect.x + pCamera->limit_rect.w && col_rect.x + col_rect.w - ( move_x + 0.00001f ) <= pCamera->limit_rect.x + pCamera->limit_rect.w )
	{
		if( handle )
		{
			Handle_OutofLevel( DIR_RIGHT );
		}

		return 1;
	}
	// top
	if( col_rect.y < pCamera->limit_rect.y + pCamera->limit_rect.h && col_rect.y - ( move_y - 0.00001f ) >= pCamera->limit_rect.h + pCamera->limit_rect.y )
	{
		if( handle )
		{
			Handle_OutofLevel( DIR_TOP );
		}

		return 1;
	}
	// bottom
	else if( col_rect.y + col_rect.h > pCamera->limit_rect.y + GAME_RES_H && col_rect.y + col_rect.h - ( move_y + 0.00001f ) <= pCamera->limit_rect.y + GAME_RES_H  )
	{
		if( handle )
		{
			Handle_OutofLevel( DIR_BOTTOM );
		}

		return 1;
	}

	return 0;
}

void cMovingSprite :: Set_onGround( cObjectCollision *collision )
{
	// no invalid or player collisions
	if( !collision || collision->type == CO_PLAYER )
	{
		return;
	}

	// check enemies 
	if( collision->type == CO_ENEMY )
	{
		cSprite *enemy = pObjManager->items[collision->number];

		// if enemy is not valid groundobject
		if( !enemy || !( enemy->type == TYPE_THROMP || enemy->type == TYPE_EATO || enemy->type == TYPE_SPIKA || enemy->type == TYPE_ROKKO ) )
		{
			return;
		}
	}

	// set groundobject
	ground_object = pObjManager->Get_Pointer( collision->number );
	Set_onTop( ground_object, 0 );
}

void cMovingSprite :: Check_onGround( void )
{
	if( type != TYPE_PLAYER && sprite_array != ARRAY_ENEMY && sprite_array != ARRAY_ACTIVE )
	{
		return;
	}

	// if on ground
	if( ground_object )
	{
		GL_rect rect2( col_rect.x, col_rect.y + col_rect.h, col_rect.w, 1 );

		// onground
		if( Col_Box( &ground_object->col_rect, &rect2 ) )
		{
			return;
		}
	}

	// don't check if flying
	if( state == STA_FLY )
	{
		return;
	}

	// new onground check
	unsigned int object_count = ColCheckAuto( 0, col_rect.h, 0, 1, COLLIDE_ALLOW_BLOCKING ).size();

	Reset_onGround();

	// possible ground objects
	for( unsigned int i = 0; i < object_count; i++ )
	{
		cObjectCollision *collision = Collision_Get_last();

		// no valid collision
		if( !collision )
		{
			continue;
		}

		// ground collision found
		if( collision->direction == DIR_BOTTOM )
		{
			Set_onGround( collision );
		}
		
		Collision_Delete( collision );
	}
}

void cMovingSprite :: Reset_onGround( void )
{
	ground_object = NULL;
}

void cMovingSprite :: Update_antistuck( void )
{
	// collision count
	unsigned int count = ColCheck( 0, 0, 0, 0, COLLIDE_ALLOW_BLOCKING ).size();

	// check collisions
	for( unsigned int i = 0; i < count; i++ )
	{
		cObjectCollision *collision = Collision_Get_last();

		if( !collision )
		{
			continue;
		}

		cSprite *col_obj = pObjManager->items[collision->number];

		if( collision->type != CO_ENEMY && !( collision->type == CO_ACTIVE && ( col_obj->massivetype == MASS_HALFMASSIVE || col_obj->massivetype == MASS_CLIMBABLE ) ) )
		{
			if( collision->direction == DIR_LEFT ) 
			{
				Col_Move( 2, 0, 0, 1 );
			}
			else if( collision->direction == DIR_RIGHT ) 
			{
				Col_Move( -2, 0, 0, 1 );
			}
			else if( collision->direction == DIR_UP ) 
			{
				Col_Move( 0, 2, 0, 1 );
			}
			else if( collision->direction == DIR_DOWN ) 
			{
				Col_Move( 0, -2, 0, 1 );
			}
		}

		Collision_Delete( collision );
	}
}

void cMovingSprite :: CollideMove( void )
{
	// if the ground object moves also move this sprite
	if( ground_object && ( ground_object->sprite_array == ARRAY_ACTIVE || ground_object->sprite_array == ARRAY_ENEMY ) ) // || ground_object->sprite_array == ARRAY_MASSIVE
	{
		cMovingSprite *moving_ground_object = dynamic_cast<cMovingSprite *>(ground_object);

		// valid cMovingSprite
		if( moving_ground_object )
		{
			if( moving_ground_object->velx != 0 || moving_ground_object->vely != 0 )
			{
				Col_Move( moving_ground_object->velx, moving_ground_object->vely, 0, 0, 0 );
			}
		}
	}

	// move and create collision data
	Col_Move( velx, vely );
}

void cMovingSprite :: Freeze( float freeze_time /* = DESIRED_FPS * 10 */ )
{
	freeze_counter = freeze_time;

	// full resistance
	if( ice_resistance > 0 )
	{
		freeze_counter *= (int)( ( ice_resistance * -1 ) + 1 );
	}
}

void cMovingSprite :: DownGrade( bool force )
{
	// virtual
}

void cMovingSprite :: Update_Direction( void )
{
	if( velx < 0 )
	{
		direction = DIR_LEFT;
	}
	else if( velx > 0 )
	{
		direction = DIR_RIGHT;
	}
	else if( vely < 0 )
	{
		direction = DIR_UP;
	}
	else if( vely > 0 )
	{
		direction = DIR_DOWN;
	}
}

void cMovingSprite :: Update_Rotation_velx( bool start_rotation /* = 0 */ )
{
	// left
	if( velx < 0 )
	{
		roty = 0;

		if( start_rotation )
		{
			start_roty = roty;
		}
	}
	// right
	else if( velx > 0 )
	{
		roty = 180;

		if( start_rotation )
		{
			start_roty = roty;
		}
	}
}

unsigned int cMovingSprite :: Validate_Collision( cSprite *obj )
{
	if( obj->massivetype == MASS_MASSIVE )
	{
		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

int cMovingSprite :: Validate_Collision_Ghost( cSprite *obj )
{
	if( obj->type == TYPE_BONUSBOX || obj->type == TYPE_SPINBOX )
	{
		cBaseBox *box = static_cast<cBaseBox *>(obj);

		// ghost
		if( box->box_invisible == 2 )
		{
			// maryo is not ghost
			if( pPlayer->maryo_type != MARYO_GHOST )
			{
				return 0;
			}
		}
	}

	return -1;
}

void cMovingSprite :: Send_Collision( cObjectCollision *collision )
{
	// empty collision
	if( !collision )
	{
		return;
	}

	// if no target object number is available
	if( collision->number < 0 )
	{
		return;
	}

	/* if collision is received ignore
	 * a received collision can't create a received collision
	 * only a self detected collision can create a received collision
	*/
	if( collision->received )
	{
		return;
	}

	// create a new collision
	cObjectCollision *ncollision = new cObjectCollision();

	// this is a received collision
	ncollision->received = 1;
	// set data
	ncollision->number = pObjManager->Get_Array_num( this );

	// set direction
	if( collision->direction != DIR_UNDEFINED )
	{
		ncollision->direction = Get_OppositeDirection( collision->direction );
	}

	// set type
	if( type != TYPE_PLAYER )
	{
		ncollision->type = Get_CollisionType( sprite_array );
	}
	// from player
	else
	{
		ncollision->type = CO_PLAYER;
	}
	
	// add collision to the list
	if( collision->type == CO_PLAYER )
	{
		pPlayer->Collision_Add( ncollision, 1 );
	}
	else
	{
		pObjManager->Get_Pointer( collision->number )->Collision_Add( ncollision, 1 );
	}
}

void cMovingSprite :: Handle_Collision( cObjectCollision *collision )
{
	// ignore player/enemy if frozen
	if( collision->type == CO_PLAYER || collision->type == CO_ENEMY )
	{
		if( freeze_counter )
		{
			return;
		}
	}

	cSprite::Handle_Collision( collision );
}

void cMovingSprite :: Handle_OutofLevel( ObjectDirection dir )
{
	// virtual
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
