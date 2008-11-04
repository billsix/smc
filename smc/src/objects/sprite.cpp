/***************************************************************************
 * sprite.cpp  -  basic sprite class
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

#include "../objects/sprite.h"
#include "../objects/movingsprite.h"
#include "../core/game_core.h"
#include "../level/level.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../core/sprite_manager.h"

cEditor_Object_Settings_Item :: cEditor_Object_Settings_Item( void )
{
	window_name = NULL;
	window_setting = NULL;
	advance_row = 1;
}

cEditor_Object_Settings_Item :: ~cEditor_Object_Settings_Item( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	wmgr.destroyWindow( window_name );
	wmgr.destroyWindow( window_setting );
}


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
		collision->Set_Direction( base, col );
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
				delete collision;
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

			cSprite *col_obj = pActive_Sprite_Manager->Get_Pointer( col->number );

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
		Handle_Collision_Player( collision );
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

void cCollidingSprite :: Handle_Collision_Player( cObjectCollision *collision )
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
	cSprite::Init();

	if( new_image )
	{
		Set_Image( new_image, 0, del_img );
	}

	Set_Pos( x, y );
}

cSprite :: cSprite( CEGUI::XMLAttributes &attributes )
{
	cSprite::Init();
	cSprite::Create_from_Stream( attributes );
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

	color = static_cast<Uint8>(255);

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
	can_be_ground = 0;

	// rotation
	rotation_affects_rect = 0;
	start_rotx = 0;
	start_roty = 0;
	start_rotz = 0;
	rotx = 0;
	roty = 0;
	rotz = 0;
	// scale
	scale_affects_rect = 0;
	scale_up = 0;
	scale_down = 1;
	scale_left = 0;
	scale_right = 1;
	start_scalex = 1;
	start_scaley = 1;
	scalex = 1;
	scaley = 1;

	valid_draw = 1;
	valid_update = 1;

	editor_window_name_width = 0;
}

cSprite *cSprite :: Copy( void )
{
	cSprite *basic_sprite = new cSprite( start_image, startposx, startposy );

	basic_sprite->type = type;
	basic_sprite->sprite_array = sprite_array;
	basic_sprite->Set_Massive_Type( massivetype );
	basic_sprite->can_be_ground = can_be_ground;
	basic_sprite->Set_Rotation_affects_Rect( rotation_affects_rect );
	basic_sprite->Set_Scale_affects_Rect( scale_affects_rect );
	basic_sprite->Set_Scale_Directions( scale_up, scale_down, scale_left, scale_right );
	basic_sprite->Set_Ignore_Camera( no_camera );
	basic_sprite->Set_Shadow_Pos( shadow_pos );
	basic_sprite->Set_Shadow_Color( shadow_color );
	basic_sprite->spawned = spawned;

	return basic_sprite;
}

void cSprite :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// image
	Set_Image( pVideo->Get_Surface( attributes.getValueAsString( "image" ).c_str() ), 1 ) ;
	// type
	Set_Sprite_Type( Get_Sprite_Type_Id( attributes.getValueAsString( "type" ).c_str() ) );
}

void cSprite :: Save_to_Stream( ofstream &file )
{
	// begin sprite
	file << "\t<sprite>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
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
		printf( "Warning: cSprite::Save_to_Stream no image from type %d\n", type );
	}

	// remove pixmaps directory from string
	if( img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}
	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	// type
	file << "\t\t<Property name=\"type\" value=\"" << Get_Sprite_Type_String() << "\" />" << std::endl;

	// end sprite
	file << "\t</sprite>" << std::endl;
}

void cSprite :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	// virtual
}

cSave_Level_Object *cSprite :: Save_to_Savegame( void )
{
	return NULL;
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
		// scale affects the rect
		if( scale_affects_rect )
		{
			col_rect.w = image->col_w * scalex;
			col_rect.h = image->col_h * scaley;
			// image data
			rect.w = image->w * scalex;
			rect.h = image->h * scaley;
		}
		// scale does not affect the rect
		else
		{
			col_rect.w = image->col_w;
			col_rect.h = image->col_h;
			// image data
			rect.w = image->w;
			rect.h = image->h;
		}
		// rotation affects the rect
		if( rotation_affects_rect )
		{
			Update_Rect_Rotation();
		}

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
	
	// because col_pos could have changed
	Update_Position_Rect();
}

void cSprite :: Set_Sprite_Type( SpriteType ntype )
{
	// set first because of massivetype z calculation
	type = ntype;

	if( ntype == TYPE_MASSIVE )
	{
		sprite_array = ARRAY_MASSIVE;
		Set_Massive_Type( MASS_MASSIVE );
		can_be_ground = 1;
	}
	else if( ntype == TYPE_PASSIVE )
	{
		sprite_array = ARRAY_PASSIVE;
		Set_Massive_Type( MASS_PASSIVE );
		can_be_ground = 0;
	}
	else if( ntype == TYPE_FRONT_PASSIVE )
	{
		sprite_array = ARRAY_FRONT_PASSIVE;
		Set_Massive_Type( MASS_PASSIVE );
		can_be_ground = 0;
	}
	else if( ntype == TYPE_HALFMASSIVE )
	{
		sprite_array = ARRAY_ACTIVE;
		Set_Massive_Type( MASS_HALFMASSIVE );
		can_be_ground = 1;
	}
	else if( ntype == TYPE_CLIMBABLE )
	{
		sprite_array = ARRAY_ACTIVE;
		Set_Massive_Type( MASS_CLIMBABLE );
		can_be_ground = 0;
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

	Update_Valid_Draw();
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

void cSprite :: Set_Pos_X( float x, bool new_startpos /* = 0 */ )
{
	posx = x;

	if( new_startpos )
	{
		startposx = x;
	}

	Update_Position_Rect();
}

void cSprite :: Set_Pos_Y( float y, bool new_startpos /* = 0 */ )
{
	posy = y;

	if( new_startpos )
	{
		startposy = y;
	}

	Update_Position_Rect();
}

void cSprite :: Set_Visible( bool enabled )
{
	// already set
	if( visible == enabled )
	{
		return;
	}

	visible = enabled;

	Update_Valid_Draw();
	Update_Valid_Update();
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

void cSprite :: Set_Rotation_affects_Rect( bool enable /* = 0 */ )
{
	rotation_affects_rect = enable;
}

void cSprite :: Update_Rect_Rotation( void )
{
	// Z must be first for correct rotation ( see Eato )
	Update_Rect_Rotation_Z();
	Update_Rect_Rotation_X();
	Update_Rect_Rotation_Y();
}

void cSprite :: Update_Rect_Rotation_X( void )
{
	// mirror
	if( rotx >= 180 )
	{
		col_pos.y = rect.h - ( col_rect.h + col_pos.y );
	}
}

void cSprite :: Update_Rect_Rotation_Y( void )
{
	// mirror
	if( roty >= 180 )
	{
		col_pos.x = rect.w - ( col_rect.w + col_pos.x );
	}
}

void cSprite :: Update_Rect_Rotation_Z( void )
{
	// rotate 270°
	if( rotz >= 270 )
	{
		// rotate collision position
		float orig_x = col_pos.x;
		col_pos.x = col_pos.y;
		col_pos.y = orig_x;

		// switch width and height
		float orig_w = rect.w;
		rect.w = rect.h;
		rect.h = orig_w;
		// switch collision width and height
		float orig_col_w = col_rect.w;
		col_rect.w = col_rect.h;
		col_rect.h = orig_col_w;
	}
	// mirror
	else if( rotz >= 180 )
	{
		col_pos.y = rect.h - ( col_rect.h + col_pos.y );
	}
	// rotate 90°
	else if( rotz >= 0.00001f )
	{
		// rotate collision position
		float orig_x = col_pos.x;
		col_pos.x = rect.h - ( col_rect.h + col_pos.y );
		col_pos.y = orig_x;

		// switch width and height
		float orig_w = rect.w;
		rect.w = rect.h;
		rect.h = orig_w;
		// switch collision width and height
		float orig_col_w = col_rect.w;
		col_rect.w = col_rect.h;
		col_rect.h = orig_col_w;
	}
}

void cSprite :: Set_Rotation_X( float rot, bool new_start_rot /* = 0 */ )
{
	rotx = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_rotx = rotx;
	}

	if( rotation_affects_rect )
	{
		Update_Rect_Rotation_X();
	}
}

void cSprite :: Set_Rotation_Y( float rot, bool new_start_rot /* = 0 */ )
{
	roty = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_roty = roty;
	}

	if( rotation_affects_rect )
	{
		Update_Rect_Rotation_Y();
	}
}

void cSprite :: Set_Rotation_Z( float rot, bool new_start_rot /* = 0 */ )
{
	rotz = fmod( rot, 360 );

	if( new_start_rot )
	{
		start_rotz = rotz;
	}

	if( rotation_affects_rect )
	{
		Update_Rect_Rotation_Z();
	}
}

void cSprite :: Set_Rotation( float x, float y, float z, bool new_start_rot /* = 0 */ )
{
	Set_Rotation_X( x, new_start_rot );
	Set_Rotation_Y( y, new_start_rot );
	Set_Rotation_Z( z, new_start_rot );
}

void cSprite :: Add_Rotation_X( float rot )
{
	Set_Rotation_X( rotx + rot );
}

void cSprite :: Add_Rotation_Y( float rot )
{
	Set_Rotation_Y( roty + rot );
}

void cSprite :: Add_Rotation_Z( float rot )
{
	Set_Rotation_Z( rotz + rot );
}

void cSprite :: Add_Rotation( float x, float y, float z )
{
	Set_Rotation_X( rotx + x );
	Set_Rotation_Y( roty + y );
	Set_Rotation_Z( rotz + z );
}

void cSprite :: Set_Scale_affects_Rect( bool enable /* = 0 */ )
{
	scale_affects_rect = enable;
}

void cSprite :: Set_Scale_Directions( bool up /* = 1 */, bool down /* = 0 */, bool left /* = 1 */, bool right /* = 0 */ )
{
	scale_up = up;
	scale_down = down;
	scale_left = left;
	scale_right = right;
}

void cSprite :: Set_Scale_X( const float scale, const bool new_startscale /* = 0 */ )
{
	// invalid value
	if( scale == 0 )
	{
		return;
	}

	// undo previous scale from rect
	if( scale_affects_rect && scalex != 1 )
	{
		col_rect.w /= scalex;
		rect.w /= scalex;
	}

	scalex = scale;

	// set new scale to rect
	if( scale_affects_rect && scalex != 1 )
	{
		col_rect.w *= scalex;
		rect.w *= scalex;
	}

	if( new_startscale )
	{
		start_scalex = scalex;
	}
}

void cSprite :: Set_Scale_Y( const float scale, const bool new_startscale /* = 0 */ )
{
	// invalid value
	if( scale == 0 )
	{
		return;
	}

	// undo previous scale from rect
	if( scale_affects_rect && scaley != 1 )
	{
		col_rect.h /= scaley;
		rect.h /= scaley;
	}

	scaley = scale;

	// set new scale to rect
	if( scale_affects_rect && scaley != 1 )
	{
		col_rect.h *= scaley;
		rect.h *= scaley;
	}

	if( new_startscale )
	{
		start_scaley = scaley;
	}
}

void cSprite :: Set_Scale( const float scale, const bool new_startscale /* = 0 */ )
{
	Set_Scale_X( scale, new_startscale );
	Set_Scale_Y( scale, new_startscale );
}

void cSprite :: Add_Scale_X( const float val )
{
	Set_Scale_X( scalex + val );
}

void cSprite :: Add_Scale_Y( const float val )
{
	Set_Scale_Y( scaley + val );
}

void cSprite :: Add_Scale( const float val )
{
	Set_Scale_X( scalex + val );
	Set_Scale_Y( scaley + val );
}

void cSprite :: Set_on_Top( const cSprite *sprite, bool optimize_hor_pos /* = 1 */ )
{
	// set ground position 0.1f over it
	posy = sprite->col_rect.y - col_pos.y - col_rect.h - 0.1f;

	// optimize the horizontal position if given
	if( optimize_hor_pos && ( posx < sprite->posx || posx > sprite->posx + sprite->col_rect.w ) )
	{
		posx = sprite->posx + sprite->col_rect.w / 3;
	}

	Update_Position_Rect();
}

void cSprite :: Move( float move_x, float move_y, const bool real /* = 0 */ )
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

void cSprite :: Collide_Move( void )
{
	// virtual
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
		// Don't use startposx/startposy because col_rect is not the editor/start rect
		col_rect.x = posx + col_pos.x; // todo : startcol_pos ?
		col_rect.y = posy + col_pos.y;
	}

	Update_Valid_Draw();
}

void cSprite :: Update( void )
{
	// virtual
}

void cSprite :: Update_Valid_Draw( void )
{
	valid_draw = Is_Draw_Valid();
}

void cSprite :: Update_Valid_Update( void )
{
	valid_update = Is_Update_Valid();
}

void cSprite :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	Draw_Image( request );

	// draw debugging collision rects
	if( game_debug )
	{
		// - image rect
		// create request
		cRectRequest *rect_request = new cRectRequest();
		// draw
		pVideo->Draw_Rect( &rect, posz + 0.000002f, &lightgrey, rect_request );
		rect_request->no_camera = no_camera;
		rect_request->filled = 0;
		rect_request->blend_sfactor = GL_SRC_COLOR;
		rect_request->blend_dfactor = GL_DST_ALPHA;
		// scale
		if( !scale_affects_rect )
		{
			// scale position y
			if( scale_up )
			{
				rect_request->rect.y += ( image->int_y * scaley ) - ( ( image->h * 0.5f ) * ( scaley - 1 ) );
			}

			// scale height
			if( scale_down )
			{
				rect_request->rect.h += image->h * ( scaley - 1 );
			}

			// scale position x
			if( scale_left )
			{
				rect_request->rect.x += ( image->int_x * scalex ) - ( ( image->w * 0.5f ) * ( scalex - 1 ) );
			}

			// scale width
			if( scale_right )
			{
				rect_request->rect.w += image->w * ( scalex - 1 );
			}
		}
		// add request
		pRenderer->Add( rect_request );

		// - collision rect
		// create request
		rect_request = new cRectRequest();
		// draw
		Color sprite_color = Get_Sprite_Color( this );
		pVideo->Draw_Rect( &col_rect, posz + 0.000001f, &sprite_color, rect_request );
		rect_request->no_camera = no_camera;
		// blending
		rect_request->blend_sfactor = GL_SRC_COLOR;
		rect_request->blend_dfactor = GL_DST_ALPHA;

		// add request
		pRenderer->Add( rect_request );
	}

	// no editor drawing
	if( !editor_enabled )
	{
		// draw debug rect
		if( game_debug )
		{
			if( type == TYPE_PLAYER )
			{
				// Get Moving Sprite
				cMovingSprite *moving_sprite = static_cast<cMovingSprite *>(this);

				// if on ground
				if( moving_sprite->ground_object )
				{
					// create request
					cRectRequest *rect_request = new cRectRequest();
					// draw
					pVideo->Draw_Rect( &moving_sprite->ground_object->col_rect, posz + 0.000002f, &grey, rect_request );
					rect_request->no_camera = 0;
					// blending
					rect_request->blend_sfactor = GL_SRC_COLOR;
					rect_request->blend_dfactor = GL_DST_ALPHA;

					// add request
					pRenderer->Add( rect_request );
				}
			}
		}
	}
	// show obsolete images in editor
	else if( image && image->obsolete )
	{
		// create request
		cRectRequest *rect_request = new cRectRequest();
		// draw
		pVideo->Draw_Rect( &col_rect, posz + 0.000001f, &red, rect_request );
		rect_request->no_camera = 0;

		// blending
		rect_request->blend_sfactor = GL_SRC_COLOR;
		rect_request->blend_dfactor = GL_DST_ALPHA;

		// add request
		pRenderer->Add( rect_request );
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

	// editor
	if( editor_enabled )
	{
		// blit surface
		start_image->Blit_Data( request );

		// rotation
		request->rotx += start_rotx;
		request->roty += start_roty;
		request->rotz += start_rotz;
		// position x and
		// scalex
		if( start_scalex != 1 )
		{
			// scale to the right and left
			if( scale_right && scale_left )
			{
				request->scale_x = start_scalex;
				request->pos_x = startposx + ( start_image->int_x * start_scalex ) - ( ( start_image->w * 0.5f ) * ( start_scalex - 1 ) );
			}
			// scale to the right only
			else if( scale_right )
			{
				request->scale_x = start_scalex;
				request->pos_x = startposx + ( start_image->int_x * start_scalex );
			}
			// scale to the left only
			else if( scale_left )
			{
				request->scale_x = start_scalex;
				request->pos_x = startposx + ( start_image->int_x * start_scalex ) - ( ( start_image->w ) * ( start_scalex - 1 ) );
			}
			// no scaling
			else
			{
				request->pos_x = startposx + start_image->int_x;
			}
		}
		// no scalex
		else
		{
			request->pos_x = startposx + start_image->int_x;
		}
		// position y and
		// scaley
		if( start_scaley != 1 )
		{
			// scale down and up
			if( scale_down && scale_up )
			{
				request->scale_y = start_scaley;
				request->pos_y = startposy + ( start_image->int_y * start_scaley ) - ( ( start_image->h * 0.5f ) * ( start_scaley - 1 ) );
			}
			// scale down only
			else if( scale_down )
			{
				request->scale_y = start_scaley;
				request->pos_y = startposy + ( start_image->int_y * start_scaley );
			}
			// scale up only
			else if( scale_up )
			{
				request->scale_y = start_scaley;
				request->pos_y = startposy + ( start_image->int_y * start_scaley ) - ( ( start_image->h ) * ( start_scaley - 1 ) );
			}
			// no scaling
			else
			{
				request->pos_y = startposy + start_image->int_y;
			}
		}
		// no scaley
		else
		{
			request->pos_y = startposy + start_image->int_y;
		}
	
		// if editor z position is given
		if( editor_posz > 0 )
		{
			request->pos_z = editor_posz;
		}
		// normal position z
		else
		{
			request->pos_z = posz;
		}
	}
	// no editor
	else
	{
		// blit surface
		image->Blit_Data( request );

		// rotation
		request->rotx += rotx;
		request->roty += roty;
		request->rotz += rotz;
		// position x and
		// scalex
		if( scalex != 1 )
		{
			// scale to the right and left
			if( scale_right && scale_left )
			{
				request->scale_x = scalex;
				request->pos_x = posx + ( image->int_x * scalex ) - ( ( image->w * 0.5f ) * ( scalex - 1 ) );
			}
			// scale to the right only
			else if( scale_right )
			{
				request->scale_x = scalex;
				request->pos_x = posx + ( image->int_x * scalex );
			}
			// scale to the left only
			else if( scale_left )
			{
				request->scale_x = scalex;
				request->pos_x = posx + ( image->int_x * scalex ) - ( ( image->w ) * ( scalex - 1 ) );
			}
			// no scaling
			else
			{
				request->pos_x = posx + image->int_x;
			}
		}
		// no scalex
		else
		{
			request->pos_x = posx + image->int_x;
		}
		// position y and
		// scaley
		if( scaley != 1 )
		{
			// scale down and up
			if( scale_down && scale_up )
			{
				request->scale_y = scaley;
				request->pos_y = posy + ( image->int_y * scaley ) - ( ( image->h * 0.5f ) * ( scaley - 1 ) );
			}
			// scale down only
			else if( scale_down )
			{
				request->scale_y = scaley;
				request->pos_y = posy + ( image->int_y * scaley );
			}
			// scale up only
			else if( scale_up )
			{
				request->scale_y = scaley;
				request->pos_y = posy + ( image->int_y * scaley ) - ( ( image->h ) * ( scaley - 1 ) );
			}
			// no scaling
			else
			{
				request->pos_y = posy + image->int_y;
			}
		}
		// no scaley
		else
		{
			request->pos_y = posy + image->int_y;
		}
	
		// position z
		request->pos_z = posz;
	}

	// no camera setting
	request->no_camera = no_camera;

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

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void cSprite :: Set_Massive_Type( MassiveType mtype )
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
	pActive_Sprite_Manager->Set_Z( this );
}

bool cSprite :: Is_on_Top( const cSprite *obj )
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

bool cSprite :: Is_Visible_on_Screen( void )
{
	// camera position
	float cam_x = 0, cam_y = 0;

	if( !no_camera )
	{
		cam_x = pActive_Camera->x;
		cam_y = pActive_Camera->y;
	}

	// not visible left
	if( rect.x + rect.w < cam_x )
	{
		return 0;
	}
	// not visible right
	else if( rect.x > cam_x + game_res_w )
	{
		return 0;
	}
	// not visible down
	else if( rect.y + rect.h < cam_y )
	{
		return 0;
	}
	// not visible up
	else if( rect.y > cam_y + game_res_h )
	{
		return 0;
	}

	return 1;
}

bool cSprite :: is_Player_range( void )
{
	// no player range set
	if( player_range < 300 )
	{
		return Is_Visible_on_Screen();
	}
	// check if not in range
	else if( posx < pPlayer->posx - player_range || posy < pPlayer->posy - player_range ||
		posx > pPlayer->posx + player_range || posy > pPlayer->posy + player_range )
	{
		return 0;
	}

	// is in range
	return 1;
}

bool cSprite :: Is_Update_Valid( void )
{
	// if destroyed
	if( destroy )
	{
		return 0;
	}

	return 1;
}

bool cSprite :: Is_Draw_Valid( void )
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
	if( !Is_Visible_on_Screen() )
	{
		return 0;
	}

	return 1;
}

void cSprite :: Destroy( void )
{
	destroy = 1;
	visible = 0;
	valid_draw = 0;
	valid_update = 0;
	Set_Image( NULL, 1 );
}

void cSprite :: Editor_Add( const CEGUI::String name, const CEGUI::String tooltip, CEGUI::Window *window_setting, float obj_width, float obj_height /* = 28 */, bool advance_row /* = 1 */ )
{
	if( obj_height < 28 )
	{
		obj_height = 28;
	}

	// get gui sheet
	CEGUI::Window *guisheet = pGuiSystem->getGUISheet();
	// get window manager
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// create name window
	CEGUI::Window *window_name = wmgr.createWindow( "TaharezLook/StaticText", "text_" + window_setting->getName() );
	window_name->setText( name );
	window_name->setTooltipText( tooltip );
	// get text width
	CEGUI::Font *font = CEGUI::FontManager::getSingleton().getFont( "bluebold_medium" );
	float text_width = 12 + font->getTextExtent( name ) * global_downscalex;
	// all names should have the same width
	if( text_width > editor_window_name_width )
	{
		editor_window_name_width = text_width;
	}
	// set size
	window_name->setWidth( CEGUI::UDim( 0, text_width * global_upscalex ) );
	window_name->setHeight( CEGUI::UDim( 0, 28 * global_upscaley ) );

	// create settings window
	cEditor_Object_Settings_Item *settings_item = new cEditor_Object_Settings_Item();

	// set size
	window_setting->setWidth( CEGUI::UDim( 0, obj_width * global_upscalex ) );
	window_setting->setHeight( CEGUI::UDim( 0, obj_height * global_upscaley ) );

	settings_item->window_name = window_name;
	settings_item->window_setting = window_setting;
	settings_item->advance_row = advance_row;

	editor_windows.push_back( settings_item );

	// add to main window
	guisheet->addChildWindow( window_name );
	guisheet->addChildWindow( window_setting );
}

void cSprite :: Editor_Activate( void )
{
	// virtual
}

void cSprite :: Editor_Deactivate( void )
{
	// remove editor controls

	for( Editor_Object_Settings_List::iterator itr = editor_windows.begin(), itr_end = editor_windows.end(); itr != itr_end; ++itr )
	{
		cEditor_Object_Settings_Item *obj = (*itr);

		delete obj;
	}

	editor_windows.clear();
}

void cSprite :: Editor_Position_Update( void )
{
	float obj_posx = 0;
	float obj_posy = 0;
	float row_height = 0;

	// set all positions
	for( Editor_Object_Settings_List::iterator itr = editor_windows.begin(), itr_end = editor_windows.end(); itr != itr_end; ++itr )
	{
		cEditor_Object_Settings_Item *obj = (*itr);
		CEGUI::Window *window_name = obj->window_name;
		CEGUI::Window *window_setting = obj->window_setting;

		// start a new row
		if( obj->advance_row )
		{
			obj_posx = 0;
			obj_posy += row_height;
			row_height = 0;
		}

		// get window text width
		float window_name_width = window_name->getWidth().asAbsolute( static_cast<float>(game_res_w) ) * global_downscalex;
		float window_name_height = window_name->getHeight().asAbsolute( static_cast<float>(game_res_h) ) * global_downscaley;

		// get window setting width
		float window_setting_width;
		float window_setting_height;

		// if combobox get the editbox dimension
		if( window_setting->getType() == "TaharezLook/Combobox" )
		{
			window_setting_width = static_cast<CEGUI::Combobox *>(window_setting)->getEditbox()->getWidth().asAbsolute( static_cast<float>(game_res_w) ) * global_downscalex;
			window_setting_height = static_cast<CEGUI::Combobox *>(window_setting)->getEditbox()->getHeight().asAbsolute( static_cast<float>(game_res_h) ) * global_downscaley;
		}
		// get default dimension
		else
		{
			window_setting_width = window_setting->getWidth().asAbsolute( static_cast<float>(game_res_w) ) * global_downscalex;
			window_setting_height = window_setting->getHeight().asAbsolute( static_cast<float>(game_res_h) ) * global_downscaley;
		}

		// update row height
		if( window_setting_height > row_height )
		{
			row_height = window_setting_height;
		}
		if( window_name_height > row_height )
		{
			row_height = window_name_height;
		}

		// current position
		float object_final_pos_x = startposx + rect.w + 5 + obj_posx - pActive_Camera->x;
		float object_final_pos_y = startposy + 5 + obj_posy - pActive_Camera->y;

		// set name position
		window_name->setXPosition( CEGUI::UDim( 0, object_final_pos_x * global_upscalex ) );
		window_name->setYPosition( CEGUI::UDim( 0, object_final_pos_y * global_upscaley ) );
		// set setting position
		window_setting->setXPosition( CEGUI::UDim( 0, ( window_name_width + object_final_pos_x ) * global_upscalex ) );
		window_setting->setYPosition( CEGUI::UDim( 0, object_final_pos_y * global_upscaley ) );
		
		// set new position x
		obj_posx += window_name_width + window_setting_width + 0.05f;
	}
}

void cSprite :: Editor_Init( void )
{
	// set state
	Editor_State_Update();

	// init
	for( Editor_Object_Settings_List::iterator itr = editor_windows.begin(), itr_end = editor_windows.end(); itr != itr_end; ++itr )
	{
		cEditor_Object_Settings_Item *obj = (*itr);
		CEGUI::Window *window_name = obj->window_name;

		// set first row width
		if( obj->advance_row )
		{
			window_name->setWidth( CEGUI::UDim( 0, editor_window_name_width * global_upscalex ) );
		}
	}

	// set position
	Editor_Position_Update();
}

void cSprite :: Editor_State_Update( void )
{
	// virtual
}