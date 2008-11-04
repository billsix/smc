/***************************************************************************
 * moving_platform.cpp  -  default moving platforms handler
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/moving_platform.h"
#include "../core/camera.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"
#include "../core/framerate.h"
#include "../video/renderer.h"
#include "../user/savegame.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** *** cMoving_Platform *** *** *** *** *** *** *** *** *** *** */

cMoving_Platform :: cMoving_Platform( float x, float y )
: cImageObjectSprite( x, y )
{
	cMoving_Platform::Init();
}

cMoving_Platform :: cMoving_Platform( CEGUI::XMLAttributes &attributes )
: cImageObjectSprite()
{
	cMoving_Platform::Init();
	cMoving_Platform::Create_from_Stream( attributes );
}

cMoving_Platform :: ~cMoving_Platform( void )
{
	//
}

void cMoving_Platform :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_MOVING_PLATFORM;
	posz = 0.085f;

	player_range = 2000;
	can_be_ground = 1;

	platform_state = PLATFORM_STAY;
	touch_counter = 0;
	shake_dir_counter = 0;
	shake_counter = 0;

	Set_Massive_Type( MASS_HALFMASSIVE );
	Set_Max_Distance( 150 );
	Set_Speed( 3 );
	Set_Touch_Time( 0 );
	Set_Shake_Time( 16 );
	Set_Touch_Move_Time( 0 );

	Set_Direction( DIR_RIGHT, 1 );

	Set_Middle_Count( 2 );
	// create default images
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_left.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_middle.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_right.png" ) );
	Set_Image( 1, 1 );

	Update_Rect();

	Create_Name();
}

cMoving_Platform *cMoving_Platform :: Copy( void )
{
	cMoving_Platform *moving_platform = new cMoving_Platform( startposx, startposy );
	moving_platform->Set_Massive_Type( massivetype );
	moving_platform->Set_Direction( start_direction, 1 );
	moving_platform->Set_Max_Distance( max_distance );
	moving_platform->Set_Speed( speed );
	moving_platform->Set_Touch_Time( touch_time );
	moving_platform->Set_Shake_Time( shake_time );
	moving_platform->Set_Touch_Move_Time( touch_move_time );
	moving_platform->Set_Middle_Count( middle_count );
	moving_platform->Set_Image_Top_Left( images[0] );
	moving_platform->Set_Image_Top_Middle( images[1] );
	moving_platform->Set_Image_Top_Right( images[2] );

	return moving_platform;
}

void cMoving_Platform :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// massive type
	Set_Massive_Type( Get_Massive_Type_Id( attributes.getValueAsString( "massive_type", Get_Massive_Type_Name( massivetype ) ).c_str() ) );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ), 1 );
	// max distance
	Set_Max_Distance( attributes.getValueAsInteger( "max_distance", max_distance ) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", speed ) );
	// touch_time
	Set_Touch_Time( attributes.getValueAsFloat( "touch_time", touch_time ) );
	// shake time
	Set_Shake_Time( attributes.getValueAsFloat( "shake_time", shake_time ) );
	// touch move time
	Set_Touch_Move_Time( attributes.getValueAsFloat( "touch_move_time", touch_move_time ) );
	// middle image count
	Set_Middle_Count( attributes.getValueAsInteger( "middle_img_count", middle_count ) );
	// image top left
	Set_Image_Top_Left( pVideo->Get_Surface( attributes.getValueAsString( "image_top_left", images[0]->Get_Filename() ).c_str() ) );
	// image top middle
	Set_Image_Top_Middle( pVideo->Get_Surface( attributes.getValueAsString( "image_top_middle", images[1]->Get_Filename() ).c_str() ) );
	// image top right
	Set_Image_Top_Right( pVideo->Get_Surface( attributes.getValueAsString( "image_top_right", images[2]->Get_Filename() ).c_str() ) );
}

void cMoving_Platform :: Save_to_Stream( ofstream &file )
{
	// begin moving_platform
	file << "\t<moving_platform>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// massive type
	file << "\t\t<Property name=\"massive_type\" value=\"" << Get_Massive_Type_Name( massivetype ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << max_distance << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;
	// touch time
	file << "\t\t<Property name=\"touch_time\" value=\"" << touch_time << "\" />" << std::endl;
	// shake time
	file << "\t\t<Property name=\"shake_time\" value=\"" << shake_time << "\" />" << std::endl;
	// touch move time
	file << "\t\t<Property name=\"touch_move_time\" value=\"" << touch_move_time << "\" />" << std::endl;
	// middle image count
	file << "\t\t<Property name=\"middle_img_count\" value=\"" << middle_count << "\" />" << std::endl;
	// image top left
	file << "\t\t<Property name=\"image_top_left\" value=\"" << images[0]->Get_Filename( 1 ) << "\" />" << std::endl;
	// image top middle
	file << "\t\t<Property name=\"image_top_middle\" value=\"" << images[1]->Get_Filename( 1 ) << "\" />" << std::endl;
	// image top right
	file << "\t\t<Property name=\"image_top_right\" value=\"" << images[2]->Get_Filename( 1 ) << "\" />" << std::endl;

	// end moving_platform
	file << "\t</moving_platform>" << std::endl;
}

void cMoving_Platform :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	// direction
	if( save_object->exists( "direction" ) )
	{
		direction = static_cast<ObjectDirection>(string_to_int( save_object->Get_Value( "direction" ) ));
	}

	// platform state
	if( save_object->exists( "platform_state" ) )
	{
		platform_state = static_cast<Moving_Platform_State>(string_to_int( save_object->Get_Value( "platform_state" ) ));
	}

	// new position x
	if( save_object->exists( "new_posx" ) )
	{
		Set_Pos_X( string_to_float( save_object->Get_Value( "new_posx" ) ) );
	}

	// new position y
	if( save_object->exists( "new_posy" ) )
	{
		Set_Pos_Y( string_to_float( save_object->Get_Value( "new_posy" ) ) );
	}

	// velocity x
	if( save_object->exists( "velx" ) )
	{
		velx = string_to_float( save_object->Get_Value( "velx" ) );
	}

	// velocity y
	if( save_object->exists( "vely" ) )
	{
		vely = string_to_float( save_object->Get_Value( "vely" ) );
	}

	// visible
	if( save_object->exists( "visible" ) )
	{
		Set_Visible( string_to_int( save_object->Get_Value( "visible" ) ) > 0 );
	}
}

cSave_Level_Object *cMoving_Platform :: Save_to_Savegame( void )
{
	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->type = type;
	save_object->properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(startposx) ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(startposy) ) ) );

	// direction
	save_object->properties.push_back( cSave_Level_Object_Property( "direction", int_to_string( direction ) ) );

	// platform state
	save_object->properties.push_back( cSave_Level_Object_Property( "platform_state", int_to_string( platform_state ) ) );

	// new position ( only save if needed )
	if( startposx != posx || startposy != posy )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "new_posx", int_to_string( static_cast<int>(posx) ) ) );
		save_object->properties.push_back( cSave_Level_Object_Property( "new_posy", int_to_string( static_cast<int>(posy) ) ) );
	}

	// velocity
	save_object->properties.push_back( cSave_Level_Object_Property( "velx", float_to_string( velx ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "vely", float_to_string( vely ) ) );

	// visible ( only save if needed )
	if( !visible )
	{
		save_object->properties.push_back( cSave_Level_Object_Property( "visible", int_to_string( visible ) ) );
	}

	return save_object;
}

void cMoving_Platform :: Set_Massive_Type( MassiveType mtype )
{
	massivetype = mtype;

	if( massivetype == MASS_MASSIVE )
	{
		can_be_ground = 1;
	}
	else if( massivetype == MASS_PASSIVE )
	{
		can_be_ground = 0;
	}
	else if( massivetype == MASS_HALFMASSIVE )
	{
		can_be_ground = 1;
	}
	else if( massivetype == MASS_CLIMBABLE )
	{
		can_be_ground = 0;
	}

	Create_Name();
}

void cMoving_Platform :: Set_Direction( ObjectDirection dir, bool new_start_direction /* = 0 */ )
{
	cImageObjectSprite::Set_Direction( dir, new_start_direction );

	if( new_start_direction )
	{
		// changed direction
		if( dir != start_direction )
		{
			// set back start position
			Set_Pos( startposx, startposy );
			// reset velocity
			Set_Velocity( 0, 0 );
		}

		// update name
		Create_Name();
	}

	// Set new velocity
	Update_Velocity();
}

void cMoving_Platform :: Set_Max_Distance( int nmax_distance )
{
	max_distance = nmax_distance;

	if( max_distance < 0 )
	{
		max_distance = 0;
	}
}

void cMoving_Platform :: Set_Speed( float val )
{
	speed = val;

	// set velocity
	Update_Velocity();

	Create_Name();
}

void cMoving_Platform :: Set_Touch_Time( float val )
{
	touch_time = val;

	if( touch_time < 0 )
	{
		touch_time = 0;
	}

	Create_Name();
}

void cMoving_Platform :: Set_Shake_Time( float val )
{
	shake_time = val;

	if( shake_time < 0 )
	{
		shake_time = 0;
	}
}

void cMoving_Platform :: Set_Touch_Move_Time( float val )
{
	touch_move_time = val;

	if( touch_move_time < 0 )
	{
		touch_move_time = 0;
	}

	// set back start position
	Set_Pos( startposx, startposy );
	// reset velocity
	Set_Velocity( 0, 0 );

	Create_Name();
}

void cMoving_Platform :: Set_Middle_Count( int val )
{
	middle_count = val;

	if( middle_count < 0 )
	{
		middle_count = 0;
	}
}

void cMoving_Platform :: Set_Image_Top_Left( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[0] = surface;
	Update_Rect();
}

void cMoving_Platform :: Set_Image_Top_Middle( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[1] = surface;

	// middle image sets main image
	curr_img = -1;
	Set_Image( 1, 1 );
	Update_Rect();
}

void cMoving_Platform :: Set_Image_Top_Right( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[2] = surface;
	Update_Rect();
}

void cMoving_Platform :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// move time should be smaller than touch time because it should
	// start moving (if it is moving) before shaking starts.
	// if larger than touch time then the platform state will change 
	// and touch_counter will never be more than move_time
	if( touch_time || touch_move_time )
	{
		if( platform_state == PLATFORM_TOUCHED )
		{
			touch_counter += pFramerate->speedfactor;
		}
	}

	// if able to fall
	if( touch_time )
	{
		if( platform_state == PLATFORM_TOUCHED && touch_counter > touch_time )
		{
			platform_state = PLATFORM_SHAKE;
		}
		else if( platform_state == PLATFORM_SHAKE )
		{
			shake_dir_counter += pFramerate->speedfactor;

			// move into opposite direction
			if( shake_dir_counter > 0.8f )
			{
				float move_x = 0;
				if( posx > startposx )
				{
					move_x = -3;
				}
				else
				{
					move_x = 3;
				}

				// move
				col_pos.x -= move_x;
				Move( move_x, 0, 1 );

				shake_dir_counter = 0;
			}

			shake_counter += pFramerate->speedfactor;

			if( shake_counter > shake_time )
			{
				platform_state = PLATFORM_FALL;
			}
		}
		else if( platform_state == PLATFORM_FALL )
		{
			if( vely < 25 )
			{
				vely += 2.1f * pFramerate->speedfactor;

				// limit falling velocity
				if( vely > 25 )
				{
					vely = 25;
				}
			}

			// drop objects when falling fast
			if( can_be_ground && vely > 15 )
			{
				can_be_ground = 0;
			}

			if( posx > startposx )
			{
				Add_Rotation_Z( 1.5f );
			}
			else
			{
				Add_Rotation_Z( -1.5f );
			}

			// below screen
			if( col_rect.y + col_rect.h > pActive_Camera->limit_rect.y + game_res_h )
			{
				platform_state = PLATFORM_STAY;
				Set_Visible( 0 );
			}
		}
	}

	// if moving
	if( speed && platform_state != PLATFORM_FALL && ( !touch_move_time || touch_counter > touch_move_time ) )
	{
		// distance to final position
		float dist_to_final_pos = 0;

		if( direction == DIR_LEFT )
		{
			dist_to_final_pos = 0 - ( startposx - posx );
		}
		else if( direction == DIR_RIGHT )
		{
			dist_to_final_pos = max_distance + ( startposx - posx );
		}
		else if( direction == DIR_UP )
		{
			dist_to_final_pos = 0 - ( startposy - posy );
		}
		else if( direction == DIR_DOWN )
		{
			dist_to_final_pos = max_distance + ( startposy - posy );
		}

		// reached final position
		if( dist_to_final_pos <= 0 )
		{
			// unset velocity
			Set_Velocity( 0, 0 );
			// set new direction
			Set_Direction( Get_Opposite_Direction( direction ) );
		}
		// slow down near final position
		else if( dist_to_final_pos < max_distance * 0.2f )
		{
			if( direction == DIR_DOWN )
			{
				if( vely > 0 )
				{
					// slow down
					if( vely > speed * 0.3f )
					{
						vely *= 1 - ( 0.05f * pFramerate->speedfactor );

						// reached lowest value
						if( vely <= speed * 0.3f )
						{
							vely = speed * 0.3f;
						}
					}
				}
				else
				{
					// set lowest value
					vely = speed * 0.3f;
				}
			}
			else if( direction == DIR_UP )
			{
				if( vely < 0 )
				{
					// slow down
					if( vely < -speed * 0.3f )
					{
						vely *= 1 - ( 0.05f * pFramerate->speedfactor );

						// reached lowest value
						if( vely >= -speed * 0.3f )
						{
							vely = -speed * 0.3f;
						}
					}
				}
				else
				{
					// set lowest value
					vely = -speed * 0.3f;
				}
			}
			else if( direction == DIR_LEFT )
			{
				if( velx < 0 )
				{
					// slow down
					if( velx < -speed * 0.3f )
					{
						velx *= 1 - ( 0.05f * pFramerate->speedfactor );

						// reached lowest value
						if( velx >= -speed * 0.3f )
						{
							velx = -speed * 0.3f;
						}
					}
				}
				else
				{
					// set lowest value
					velx = -speed * 0.3f;
				}
			}
			else if( direction == DIR_RIGHT )
			{
				if( velx > 0 )
				{
					// slow down
					if( velx > speed * 0.3f )
					{
						velx *= 1 - ( 0.05f * pFramerate->speedfactor );

						// reached lowest value
						if( velx <= speed * 0.3f )
						{
							velx = speed * 0.3f;
						}
					}
				}
				else
				{
					// set lowest value
					velx = speed * 0.3f;
				}
			}
		}
		// not near final position
		else
		{
			Update_Velocity();
		}
	}
}

void cMoving_Platform :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	cSurfaceRequest *surface_request;
	float x = 0;

	// top left
	if( images[0] )
	{
		// Start
		image = images[0];
		start_image = images[0];
		// create request

		surface_request = new cSurfaceRequest();
		// draw only first image complete
		cImageObjectSprite::Draw( surface_request );
		surface_request->pos_x += x;
		x += images[0]->w;
		// add request
		pRenderer->Add( surface_request );
	}

	// top middle
	if( images[1] )
	{
		// Middle
		image = images[1];
		start_image = images[1];
		for( unsigned int i = 0; i < middle_count; i++ )
		{
			// create request
			surface_request = new cSurfaceRequest();
			cImageObjectSprite::Draw_Image( surface_request);
			surface_request->pos_x += x;
			x += images[1]->w;
			// add request
			pRenderer->Add( surface_request );
		}
	}

	// top right
	if( images[2] )
	{
		// End
		image = images[2];
		start_image = images[2];
		// create request
		surface_request = new cSurfaceRequest();
		cImageObjectSprite::Draw_Image( surface_request );
		surface_request->pos_x += x;
		//x += images[2]->w;
		// add request
		pRenderer->Add( surface_request );
	}

	Update_Rect();
	Update_Position_Rect();

	// draw distance rect
	if( editor_level_enabled && speed )
	{
		if( start_direction == DIR_RIGHT || start_direction == DIR_LEFT )
		{
			pVideo->Draw_Rect( startposx - pActive_Camera->x, startposy - pActive_Camera->y, max_distance + col_rect.w, 15, posz - 0.000001f, &whitealpha128 );
		}
		else if( start_direction == DIR_DOWN || start_direction == DIR_UP )
		{
			pVideo->Draw_Rect( startposx - pActive_Camera->x, startposy - pActive_Camera->y, 15, max_distance + col_rect.h, posz - 0.000001f, &whitealpha128 );
		}
	}
}

void cMoving_Platform :: Update_Rect( void )
{
	// clear
	col_rect.w = 0;
	rect.w = 0;

	// get width
	if( images[0] )
	{
		rect.w += images[0]->w;
	}
	if( images[1] )
	{
		rect.w += images[1]->w * middle_count;
	}
	if( images[2] )
	{
		rect.w += images[2]->w;
	}

	// set width
	col_rect.w = rect.w;
	start_rect.w = rect.w;
}

void cMoving_Platform :: Update_Velocity( void )
{
	// set velocity
	if( speed > 0 && ( !touch_move_time || touch_counter > touch_move_time ) )
	{
		if( direction == DIR_UP )
		{
			if( vely > -speed )
			{
				vely += -speed * 0.1f * pFramerate->speedfactor;
			}
		}
		else if( direction == DIR_DOWN )
		{
			if( vely < speed )
			{
				vely += speed * 0.1f * pFramerate->speedfactor;
			}
		}
		else if( direction == DIR_LEFT )
		{
			if( velx > -speed )
			{
				velx += -speed * 0.1f * pFramerate->speedfactor;
			}
		}
		else if( direction == DIR_RIGHT )
		{
			if( velx < speed )
			{
				velx += speed * 0.1f * pFramerate->speedfactor;
			}
		}
	}
	else
	{
		// unset velocity
		Set_Velocity( 0, 0 );
	}
}

bool cMoving_Platform :: Is_Update_Valid( void )
{
	// if not visible
	if( !visible )
	{
		return 0;
	}

	return 1;
}

unsigned int cMoving_Platform :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		cMovingSprite *moving_sprite = static_cast<cMovingSprite *>(obj);

		// don't handle if not moving upwards or slower
		if( moving_sprite->Is_on_Top( this ) && direction == DIR_UP && moving_sprite->vely > vely )
		{
			// halfmassive
			if( massivetype == MASS_HALFMASSIVE )
			{
				// only if no ground
				if( !moving_sprite->ground_object )
				{
					return 2;
				}
			}
			// massive
			else if( massivetype == MASS_MASSIVE )
			{
				// always pick up
				if( moving_sprite->ground_object != this )
				{
					moving_sprite->ground_object = this;
					return 0;
				}
			}
		}

		return 0;
	}
	if( obj->type == TYPE_BALL )
	{
		if( obj->Is_on_Top( this ) )
		{
			obj->Set_on_Top( this, 0 );

			return 1;
		}

		return 0;
	}

	return 0;
}

void cMoving_Platform :: Handle_Collision_Player( cObjectCollision *collision )
{
	if( collision->direction == DIR_TOP )
	{
		// send collision
		Send_Collision( collision );
	}

	if( collision->direction == DIR_TOP )
	{
		if( ( touch_time || touch_move_time ) && platform_state == PLATFORM_STAY )
		{
			platform_state = PLATFORM_TOUCHED;
		}

	}
}

void cMoving_Platform :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// max distance
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_max_distance" ));
	Editor_Add( UTF8_("Distance"), UTF8_("Movable distance into its direction"), editbox, 120 );

	editbox->setText( int_to_string( max_distance ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_speed" ));
	Editor_Add( UTF8_("Speed"), UTF8_("Maximum speed"), editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Speed_Key, this ) );

	// touch time
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_touch_time" ));
	Editor_Add( UTF8_("Touch time"), UTF8_("Time when touched until shaking starts"), editbox, 120 );

	editbox->setText( float_to_string( touch_time ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Touch_Time_Key, this ) );

	// shake time
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_shake_time" ));
	Editor_Add( UTF8_("Shake time"), UTF8_("Time it's shaking until falling"), editbox, 120 );

	editbox->setText( float_to_string( shake_time ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Shake_Time_Key, this ) );

	// touch move time
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_move_time" ));
	Editor_Add( UTF8_("Touch move time"), UTF8_("If set does not move until this time has elapsed after touched"), editbox, 120 );

	editbox->setText( float_to_string( touch_move_time ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Touch_Move_Time_Key, this ) );

	// horizontal middle image count
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_hor_middle_count" ));
	Editor_Add( UTF8_("Hor image count"), UTF8_("Horizontal middle image count"), editbox, 120 );

	editbox->setText( int_to_string( middle_count ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Hor_Middle_Count_Key, this ) );

	// image top left
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_left" ));
	Editor_Add( UTF8_("Image top left"), UTF8_("Image top left"), editbox, 200 );

	editbox->setText( images[0]->Get_Filename( 1 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Left_Key, this ) );

	// image top middle
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_middle" ));
	Editor_Add( UTF8_("Image top middle"), UTF8_("Image top middle"), editbox, 200 );

	editbox->setText( images[1]->Get_Filename( 1 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Middle_Key, this ) );

	// image top right
	editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_right" ));
	Editor_Add( UTF8_("Image top right"), UTF8_("Image top right"), editbox, 200 );

	editbox->setText( images[2]->Get_Filename( 1 ) );
	editbox->subscribeEvent( CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Right_Key, this ) );

	// init
	Editor_Init();
}

bool cMoving_Platform :: Editor_Max_Distance_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( string_to_int( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Speed_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Touch_Time_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Touch_Time( string_to_float( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Shake_Time_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Shake_Time( string_to_float( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Touch_Move_Time_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Touch_Move_Time( string_to_float( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Hor_Middle_Count_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Middle_Count( string_to_int( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Left_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Top_Left( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Middle_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Top_Middle( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Right_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Image_Top_Right( pVideo->Get_Surface( str_text ) );

	return 1;
}

void cMoving_Platform :: Create_Name( void )
{
	name.clear();

	if( touch_time )
	{
		name = _("Falling ");
	}
	if( touch_move_time )
	{
		name += _("Delayed ");
	}
	if( speed )
	{
		name += _("Moving ");
	}

	name += _("Platform - ") + Get_Direction_Name( start_direction ) + " - " + Get_Massive_Type_Name( massivetype );
}
