/***************************************************************************
           sprite.cpp  -  sprite class
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

#include "../objects/sprite.h"
#include "../objects/movingsprite.h"
#include "../core/game_core.h"
#include "../core/obj_manager.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** *** cCollidingSprite *** *** *** *** *** *** *** *** *** */

cCollidingSprite :: cCollidingSprite( void )
{

}

cCollidingSprite :: ~cCollidingSprite( void )
{

}

void cCollidingSprite :: Parse_Collisions( void )
{
	// get collision list
	ObjectCollisionList col_list;
	collisions.swap( col_list );

	// parse the found collisions
	for( ObjectCollisionList::iterator itr = col_list.begin(), itr_end = col_list.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cObjectCollision *col = (*itr);

		// handle
		Handle_Collision( col );
	}
}

void cCollidingSprite :: Handle_Collisions( void )
{
	// parse
	Parse_Collisions();
	// clear
	Collisions_Clear();
}

bool cCollidingSprite :: Collision_Add( cSprite *base, cSprite *col, unsigned int col_id /* = 0 */, ObjectCollisionType col_type /* = CO_NOTHING */, bool add_if_new /* = 0 */ )
{
	// invalid base
	if( !base )
	{
		return 0;
	}

	// create
	cObjectCollision *collision = new cObjectCollision();

	// type
	collision->type = col_type;
	// identifier
	collision->number = col_id;

	// if col object is available
	if( col )
	{
		// direction 
		collision->Set_direction( base, col );
	}

	// add
	return Collision_Add( collision, add_if_new );
}

bool cCollidingSprite :: Collision_Add( cObjectCollision *collision, bool add_if_new /* = 0 */ )
{
	// invalid collision data
	if( !collision )
	{
		return 0;
	}

	// check if collision data is new
	if( add_if_new )
	{
		// check found collisions list
		for( ObjectCollisionList::iterator itr = collisions.begin(), itr_end = collisions.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cObjectCollision *col = (*itr);

			// already in list
			if( col->number == collision->number )
			{
				return 0;
			}
		}
	}

	collisions.push_back( collision );

	return 1;
}

void cCollidingSprite :: Collision_Delete( cObjectCollision *collision )
{
	if( !collision )
	{
		return;
	}

	for( ObjectCollisionList::iterator itr = collisions.begin(), itr_end = collisions.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cObjectCollision *col = (*itr);

		if( col == collision )
		{
			delete collision;
			collisions.erase( itr );
			break;
		}
	}
}

void cCollidingSprite :: Collision_Delete_last( void )
{
	if( collisions.empty() )
	{
		return;
	}
	
	delete collisions[collisions.size() - 1];
	collisions.erase( collisions.begin() + collisions.size() - 1 );
}

int cCollidingSprite :: Collision_Check_Direction( ObjectDirection dir )
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

cObjectCollision *cCollidingSprite :: Collision_Get_first( void )
{
	// no collision available
	if( collisions.empty() )
	{
		return NULL;	
	}

	return collisions[0];
}

cObjectCollision *cCollidingSprite :: Collision_Get_last( bool only_blocking /* = 0 */ )
{
	// no collisions available
	if( collisions.empty() )
	{
		return NULL;
	}

	if( only_blocking )
	{
		for( ObjectCollisionList::reverse_iterator itr = collisions.rbegin(), itr_end = collisions.rend(); itr != itr_end; ++itr )
		{
			// get object pointer
			cObjectCollision *col = (*itr);

			cSprite *col_obj = pObjManager->Get_Pointer( col->number );

			// ignore passive
			if( col_obj->massivetype == MASS_PASSIVE )
			{
				continue;
			}
			// if active check if not climbable
			else if( col->type == CO_ACTIVE )
			{
				// not a valid object
				if( col_obj->massivetype == MASS_CLIMBABLE )
				{
					continue;
				}

			}

			// is blocking
			return col;
		}

		// not found
		return NULL;
	}

	return collisions[collisions.size() - 1];
}

void cCollidingSprite :: Collisions_Clear( void )
{
	for( ObjectCollisionList::iterator itr = collisions.begin(), itr_end = collisions.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	collisions.clear();
}

void cCollidingSprite :: Handle_Collision( cObjectCollision *collision )
{
	// player
	if( collision->type == CO_PLAYER )
	{
		Handle_Collision_Player( collision->direction );
	}
	// enemy
	else if( collision->type == CO_ENEMY )
	{
		Handle_Collision_Enemy( collision );
	}
	// massive
	else if( collision->type == CO_MASSIVE || collision->type == CO_ACTIVE )
	{
		Handle_Collision_Massive( collision );
	}
}

void cCollidingSprite :: Handle_Collision_Player( ObjectDirection direction )
{
	// virtual
}

void cCollidingSprite :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	// virtual
}

void cCollidingSprite :: Handle_Collision_Massive( cObjectCollision *collision )
{
	// virtual
}

void cCollidingSprite :: Handle_Collision_Box( ObjectDirection direction, GL_rect *r2 )
{
	// virtual
}

/* *** *** *** *** *** *** *** cSprite *** *** *** *** *** *** *** *** *** *** */

cSprite :: cSprite( GL_Surface *new_image /* = NULL */, float x /* = 0 */, float y /* = 0 */, bool del_img /* = 0 */ )
{
	Init();

	if( new_image )
	{
		Set_Image( new_image, 0, del_img );
	}

	Set_Pos( x, y );
}

cSprite :: cSprite( XMLAttributes &attributes )
{
	Init();
	Create_from_Stream( attributes );
}

cSprite :: ~cSprite( void )
{
	if( delete_image && image )
	{
		delete image;
	}
}

void cSprite :: Init( void )
{
	// undefined
	type = TYPE_UNDEFINED;
	sprite_array = ARRAY_UNDEFINED;

	// collision data
	col_pos.x = 0;
	col_pos.y = 0;
	col_rect.x = 0;
	col_rect.y = 0;
	col_rect.w = 0;
	col_rect.h = 0;
	// image data
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;
	start_rect.clear();

	startposx = 0;
	startposy = 0;

	start_image = NULL;
	image = NULL;
	destroy = 0;
	delete_image = 0;
	shadow_pos = 0;
	shadow_color = black;
	no_camera = 0;

	color = (Uint8)255;

	combine_type = 0;
	combine_col[0] = 0;
	combine_col[1] = 0;
	combine_col[2] = 0;

	posx = 0;
	posy = 0;
	posz = 0;
	editor_posz = 0;

	massivetype = MASS_PASSIVE;
	visible = 1;
	spawned = 0;
	player_range = 1000;

	// rotation
	start_rotx = 0;
	start_roty = 0;
	start_rotz = 0;
	rotx = 0;
	roty = 0;
	rotz = 0;
	// scale
	start_scalex = 1;
	start_scaley = 1;
	scalex = 1;
	scaley = 1;

	valid_draw = 1;
}

cSprite *cSprite :: Copy( void )
{
	cSprite *basic_sprite = new cSprite( start_image, startposx, startposy );

	basic_sprite->type = type;
	basic_sprite->sprite_array = sprite_array;
	basic_sprite->Set_Massivetype( massivetype );
	basic_sprite->Set_Ignore_Camera( no_camera );
	basic_sprite->Set_Shadow_Pos( shadow_pos );
	basic_sprite->Set_Shadow_Color( shadow_color );

	return basic_sprite;
}

void cSprite :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// image
	Set_Image( pVideo->Get_Surface( attributes.getValueAsString( "image" ).c_str() ), 1 ) ;
	// type
	Set_Sprite_Type( Get_Sprite_Type_id( attributes.getValueAsString( "type" ).c_str() ) );
}

void cSprite :: Save_to_Stream( ofstream &file )
{
	// begin sprite
	file << "\t<sprite>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// image
	string img_filename;

	if( start_image )
	{
		img_filename = start_image->filename;
	}
	else if( image )
	{
		img_filename = image->filename;
	}
	else
	{
		img_filename = "No Image from type : " + int_to_string( type );
	}

	// remove pixmaps directory from string
	if( img_filename.find( PIXMAPS_DIR ) == 0 )
	{
		img_filename.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
	}
	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	// type
	file << "\t\t<Property name=\"type\" value=\"" << Get_Sprite_Type_String() << "\" />" << std::endl;

	// end sprite
	file << "\t</sprite>" << std::endl;
}

void cSprite :: Set_Image( GL_Surface *new_image, bool new_start_image /* = 0 */, bool del_img /* = 0 */ )
{
	if( delete_image )
	{
		if( image )
		{
			// if same image reset start_image
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

	if( image )
	{
		// collision data
		col_pos = image->col_pos;
		col_rect.w = image->col_w;
		col_rect.h = image->col_h;
		// image data
		rect.w = image->w;
		rect.h = image->h;

		delete_image = del_img;

		// if no name is set use the first image name
		if( !name.length() )
		{
			name = image->name;
		}
		// if no editor tags are set use the first image editor tags
		if( !editor_tags.length() )
		{
			editor_tags = image->editor_tags;
		}
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
	
	Update_valid_draw();
}

void cSprite :: Set_Sprite_Type( SpriteType ntype )
{
	// set first because of massivetype z calculation
	type = ntype;

	if( ntype == TYPE_MASSIVE )
	{
		sprite_array = ARRAY_MASSIVE;
		Set_Massivetype( MASS_MASSIVE );
	}
	else if( ntype == TYPE_PASSIVE )
	{
		sprite_array = ARRAY_PASSIVE;
		Set_Massivetype( MASS_PASSIVE );
	}
	else if( ntype == TYPE_FRONT_PASSIVE )
	{
		sprite_array = ARRAY_FRONT_PASSIVE;
		Set_Massivetype( MASS_PASSIVE );
	}
	else if( ntype == TYPE_HALFMASSIVE )
	{
		sprite_array = ARRAY_ACTIVE;
		Set_Massivetype( MASS_HALFMASSIVE );
	}
	else if( ntype == TYPE_CLIMBABLE )
	{
		sprite_array = ARRAY_ACTIVE;
		Set_Massivetype( MASS_CLIMBABLE );
	}
}

string cSprite :: Get_Sprite_Type_String( void )
{
	if( sprite_array == ARRAY_PASSIVE )
	{
		return "passive";
	}
	else if( sprite_array == ARRAY_FRONT_PASSIVE )
	{
		return "front_passive";
	}
	else if( sprite_array == ARRAY_ACTIVE )
	{
		if( type == TYPE_HALFMASSIVE )
		{
			return "halfmassive";
		}
		else if( type == TYPE_CLIMBABLE )
		{
			return "climbable";
		}
		else
		{
			printf( "Warning : Sprite array set as active but unknown type %d\n", type );
		}
	}
	else if( sprite_array == ARRAY_MASSIVE )
	{
		return "massive";
	}
	else
	{
		printf( "Warning : Sprite unknown array %d\n", sprite_array );
	}

	return "";
}

void cSprite :: Set_Ignore_Camera( bool enable /* = 0 */ )
{
	// already set
	if( no_camera == enable )
	{
		return;
	}

	no_camera = enable;

	Update_valid_draw();
}

void cSprite :: Set_Pos( float x, float y, bool new_startpos /* = 0 */ )
{
	posx = x;
	posy = y;

	if( new_startpos || ( startposx == 0 && startposy == 0 ) )
	{
		startposx = x;
		startposy = y;
	}

	Update_Position_Rect();
}

void cSprite :: Set_PosX( float x, bool new_startpos /* = 0 */ )
{
	posx = x;

	if( new_startpos )
	{
		startposx = x;
	}

	Update_Position_Rect();
}

void cSprite :: Set_PosY( float y, bool new_startpos /* = 0 */ )
{
	posy = y;

	if( new_startpos )
	{
		startposx = y;
	}

	Update_Position_Rect();
}

void cSprite :: Set_Visible( bool enabled )
{
	visible = enabled;

	Update_valid_draw();
}

void cSprite :: Set_Shadow( Color shadow, float pos )
{
	Set_Shadow_Pos( pos );
	Set_Shadow_Color( shadow );
}

void cSprite :: Set_Shadow_Pos( float pos )
{
	shadow_pos = pos;
}

void cSprite :: Set_Shadow_Color( Color shadow )
{
	shadow_color = shadow;
}

void cSprite :: Set_Color( Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha /* = 255 */ )
{
	color.red = red;
	color.green = green;
	color.blue = blue;
	color.alpha = alpha;
}

void cSprite :: Set_Color( Color col )
{
	color = col;
}

void cSprite :: Set_Color_Combine( float red, float green, float blue, GLint com_type )
{
	combine_type = com_type;
	combine_col[0] = red;
	combine_col[1] = green;
	combine_col[2] = blue;

	// red limits
	if( combine_col[0] > 1 )
	{
		combine_col[0] = 1;
	}
	else if( combine_col[0] <= 0 )
	{
		combine_col[0] = 0.000001f;
	}
	// green limits
	if( combine_col[1] > 1 )
	{
		combine_col[1] = 1;
	}
	else if( combine_col[1] <= 0 )
	{
		combine_col[1] = 0.000001f;
	}
	// blue limits
	if( combine_col[2] > 1 )
	{
		combine_col[2] = 1;
	}
	else if( combine_col[2] <= 0 )
	{
		combine_col[2] = 0.000001f;
	}
}

void cSprite :: Set_RotationX( float rot, bool new_start_rot /* = 0 */ )
{
	rotx = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_rotx = rotx;
	}
}

void cSprite :: Set_RotationY( float rot, bool new_start_rot /* = 0 */ )
{
	roty = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_roty = roty;
	}
}

void cSprite :: Set_RotationZ( float rot, bool new_start_rot /* = 0 */ )
{
	rotz = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_rotz = rotz;
	}
}

void cSprite :: Set_Rotation( float x, float y, float z, bool new_start_rot /* = 0 */ )
{
	Set_RotationX( x, new_start_rot );
	Set_RotationY( y, new_start_rot );
	Set_RotationZ( z, new_start_rot );
}

void cSprite :: Add_RotationX( float rot )
{
	Set_RotationX( rotx + rot );
}

void cSprite :: Add_RotationY( float rot )
{
	Set_RotationY( roty + rot );
}

void cSprite :: Add_RotationZ( float rot )
{
	Set_RotationZ( rotz + rot );
}

void cSprite :: Add_Rotation( float x, float y, float z )
{
	Set_RotationX( rotx + x );
	Set_RotationY( roty + y );
	Set_RotationZ( rotz + z );
}

void cSprite :: Set_ScaleX( float scale )
{
	scalex = scale;
}

void cSprite :: Set_ScaleY( float scale )
{
	scaley = scale;
}

void cSprite :: Set_Scale( float scale )
{
	scalex = scale;
	scaley = scale;
}

void cSprite :: Add_ScaleX( float val )
{
	Set_ScaleX( scalex + val );
}

void cSprite :: Add_ScaleY( float val )
{
	Set_ScaleY( scaley + val );
}

void cSprite :: Add_Scale( float val )
{
	Set_ScaleX( scalex + val );
	Set_ScaleY( scaley + val );
}

void cSprite :: Set_onTop( cSprite *sprite, bool optimize_hor_pos /* = 1 */ )
{
	// set correct onground position
	posy = sprite->col_rect.y - col_pos.y - col_rect.h - 0.01f;

	// optimize the horizontal position if given
	if( optimize_hor_pos && ( posx < sprite->posx || posx > sprite->posx + sprite->col_rect.w ) )
	{
		posx = sprite->posx + sprite->col_rect.w / 3;
	}

	Update_Position_Rect();
}

void cSprite :: Move( float move_x, float move_y, bool real /* = 0 */ )
{
	if( move_x == 0 && move_y == 0 )
	{
		return;
	}

	if( !real )
	{
		move_x *= pFramerate->speedfactor;
		move_y *= pFramerate->speedfactor;
	}

	posx += move_x;
	posy += move_y;

	Update_Position_Rect();
}

void cSprite :: Update_Position_Rect( void )
{
	// if not editor mode
	if( !editor_enabled )
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
	// editor mode
	else
	{
		rect.x = startposx;
		rect.y = startposy;
		start_rect.x = startposx;
		start_rect.y = startposy;
		// Use not startposx/startposy because col_rect is not the editor/start rect
		col_rect.x = posx + col_pos.x; // todo : startcol_pos ?
		col_rect.y = posy + col_pos.y;
	}

	Update_valid_draw();
}

void cSprite :: Update( void )
{
	// virtual
}

void cSprite :: Update_valid_draw( void )
{
	valid_draw = is_Draw_valid();
}

void cSprite :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	Draw_Image( request );

	// no editor drawing
	if( !editor_enabled )
	{
		// draw debugging collision rects
		if( Game_debug )
		{
			// create request
			cRectRequest *rect_request = new cRectRequest();

			if( type == TYPE_PLAYER )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &lila, rect_request );
			}
			else if( sprite_array == ARRAY_ENEMY )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &red, rect_request );
			}
			else if( sprite_array == ARRAY_ACTIVE )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &blue, rect_request );
			}
			else if( sprite_array == ARRAY_MASSIVE )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &orange, rect_request );
			}
			else if( sprite_array == ARRAY_PASSIVE )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &green, rect_request );
			}
			else if( sprite_array == ARRAY_FRONT_PASSIVE )
			{
				pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz + 0.000001f, &greenyellow, rect_request );
			}
			else
			{
				pVideo->Draw_Rect( &col_rect, posz + 0.000001f, &lightgreyalpha64, rect_request );

				if( !no_camera )
				{
					rect_request->rect.x -= pCamera->x;
					rect_request->rect.y -= pCamera->y;
				}
			}

			rect_request->blend_sfactor = GL_SRC_COLOR;
			rect_request->blend_dfactor = GL_DST_ALPHA;

			// add request
			pRenderer->Add( rect_request );

			if( type == TYPE_PLAYER )
			{
				// Get Moving Sprite
				cMovingSprite *moving_sprite = static_cast<cMovingSprite *>(this);

				// if on ground
				if( moving_sprite->ground_object )
				{
					// create request
					rect_request = new cRectRequest();

					pVideo->Draw_Rect( moving_sprite->ground_object->col_rect.x - pCamera->x, moving_sprite->ground_object->col_rect.y  - pCamera->y, moving_sprite->ground_object->col_rect.w, moving_sprite->ground_object->col_rect.h, posz + 0.000002f, &grey, rect_request );

					rect_request->blend_sfactor = GL_SRC_COLOR;
					rect_request->blend_dfactor = GL_DST_ALPHA;

					// add request
					pRenderer->Add( rect_request );
				}
			}
		}
	}
}

void cSprite :: Draw_Image( cSurfaceRequest *request /* = NULL */ )
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

	// blit surface
	if( editor_enabled )
	{
		start_image->Blit( startposx, startposy, posz, request );
	}
	else
	{
		image->Blit( posx, posy, posz, request );
	}

	// if editor z position is given
	if( editor_enabled && editor_posz > 0 )
	{
		request->pos_z = editor_posz;
	}

	if( !no_camera )
	{
		request->pos_x -= pCamera->x;
		request->pos_y -= pCamera->y;
	}

	// color
	request->color = color;
	// combine color
	if( combine_type )
	{
		request->combine_type = combine_type;
		request->combine_col[0] = combine_col[0];
		request->combine_col[1] = combine_col[1];
		request->combine_col[2] = combine_col[2];
	}

	// shadow
	if( shadow_pos )
	{
		request->shadow_pos = shadow_pos;
		request->shadow_color = shadow_color;
	}


	if( editor_enabled )
	{
		// rotation
		request->rotx += start_rotx;
		request->roty += start_roty;
		request->rotz += start_rotz;
		// scale
		request->scale_x = start_scalex;
		request->scale_y = start_scaley;
	}
	else
	{
		// rotation
		request->rotx += rotx;
		request->roty += roty;
		request->rotz += rotz;
		// scale
		request->scale_x = scalex;
		request->scale_y = scaley;
	}

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cSprite :: Set_Massivetype( MassiveType mtype )
{
	// set massivetype z position
	if( mtype == MASS_MASSIVE )
	{
		posz = 0.08f;
	}
	else if( mtype == MASS_PASSIVE )
	{
		if( sprite_array == ARRAY_FRONT_PASSIVE )
		{
			posz = 0.1f;
		}
		else
		{
			posz = 0.01f;
		}
	}
	else if( mtype == MASS_CLIMBABLE || mtype == MASS_HALFMASSIVE )
	{
		posz = 0.04f;
	}

	massivetype = mtype;

	// set correct Z position
	pObjManager->Set_Z( this );
}

bool cSprite :: is_onTop( cSprite *obj )
{
	// invalid
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

bool cSprite :: is_Visible_onScreen( void )
{
	// camera position
	float cam_x = 0, cam_y = 0;

	if( !no_camera )
	{
		cam_x = pCamera->x;
		cam_y = pCamera->y;
	}

	// not visible left
	if( rect.x + rect.w < cam_x )
	{
		return 0;
	}
	// not visible right
	else if( rect.x > cam_x + GAME_RES_W )
	{
		return 0;
	}
	// not visible down
	else if( rect.y + rect.h < cam_y )
	{
		return 0;
	}
	// not visible up
	else if( rect.y > cam_y + GAME_RES_H )
	{
		return 0;
	}

	return 1;
}

bool cSprite :: is_Player_range( void )
{
	if( player_range > 300 && ( posx < pPlayer->posx - player_range || posy < pPlayer->posy - player_range ||
		posx > pPlayer->posx + player_range || posy > pPlayer->posy + player_range ) )
	{
		return 0;
	}

	return 1;
}

bool cSprite :: is_Draw_valid( void )
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
	// editor enabled
	else
	{
		// no image
		if( !start_image )
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

void cSprite :: Destroy( void )
{
	destroy = 1;
}

void cSprite :: Editor_Add( Window *obj, float obj_width, float obj_height )
{
	// add to main window
	pGuiSystem->getGUISheet()->addChildWindow( obj );

	// needs to be always selectable
	obj->setAlwaysOnTop( 1 );
	// set size
	obj->setWidth( UDim( 0, obj_width ) );
	obj->setHeight( UDim( 0, obj_height ) );
	// mouse hover events
	// if combobox use the editbox
	if( obj->getType() == "TaharezLook/Combobox" )
	{
		((Combobox *)obj)->getEditbox()->subscribeEvent( Window::EventMouseEnters, Event::Subscriber( &cSprite::Editor_Mouse_Enter, this ) );
		((Combobox *)obj)->getEditbox()->subscribeEvent( Window::EventMouseLeaves, Event::Subscriber( &cSprite::Editor_Mouse_Leave, this ) );
	}
	else
	{
		obj->subscribeEvent( Window::EventMouseEnters, Event::Subscriber( &cSprite::Editor_Mouse_Enter, this ) );
		obj->subscribeEvent( Window::EventMouseLeaves, Event::Subscriber( &cSprite::Editor_Mouse_Leave, this ) );
	}


	editor_windows.push_back( obj );
}

void cSprite :: Editor_Activate( void )
{
	// virtual
}

void cSprite :: Editor_Deactivate( void )
{
	// remove editor controls
	WindowManager &wmgr = WindowManager::getSingleton();

	for( unsigned int i = 0; i < editor_windows.size(); i++ )
	{
		wmgr.destroyWindow( editor_windows[i] );
	}

	editor_windows.clear();
}

void cSprite :: Editor_pos_update( void )
{
	float obj_posy = 0;

	// set all positions
	for( unsigned int i = 0; i < editor_windows.size(); i++ )
	{
		editor_windows[i]->setXPosition( UDim( 0, posx + rect.w + 5 - pCamera->x ) );
		editor_windows[i]->setYPosition( UDim( 0, posy + 5 + obj_posy - pCamera->y ) );

		// if combobox get the editbox height
		if( editor_windows[i]->getType() == "TaharezLook/Combobox" )
		{
			obj_posy += ((Combobox *)editor_windows[i])->getEditbox()->getHeight().asAbsolute( GAME_RES_H );
		}
		// get default height
		else
		{
			obj_posy += editor_windows[i]->getHeight().asAbsolute( GAME_RES_H );
		}

		// some space between objects
		obj_posy += 1;
	}
}

bool cSprite :: Editor_Mouse_Enter( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = windowEventArgs.window->getTooltipText().c_str();

	// if no tooltip defined
	if( str_text.empty() )
	{
		// use the CEGUI window name
		debugdisplay->Set_Text( windowEventArgs.window->getName().c_str(), 100000 );
	}
	else
	{
		debugdisplay->Set_Text( str_text, 100000 );
	}

	return 1;
}

bool cSprite :: Editor_Mouse_Leave( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = windowEventArgs.window->getTooltipText().c_str();

	debugdisplay->Set_Text( "" );

	return 1;
}
