/***************************************************************************
           falling_platform.cpp  -  default falling platforms handler
                             -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
							(C) 2006 Tobias Maasland
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../objects/falling_platform.h"
#include "../core/camera.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../gui/hud.h"
#include "../core/framerate.h"
#include "../player/player.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** cFalling_Platform *** *** *** *** *** *** *** *** *** *** */

cFalling_Platform :: cFalling_Platform( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cFalling_Platform :: cFalling_Platform( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cFalling_Platform :: ~cFalling_Platform( void )
{
	//
}

void cFalling_Platform :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_MOVING_PLATFORM;
	posz = 0.085f;

	player_range = 2000;

	Set_Massivetype( MASS_HALFMASSIVE );
	Set_Time_Fall( 1.5f );
	Set_Middle_Count( 2 );
	name = "Falling Platform";

	// Set_Direction( DIR_RIGHT );

	// create default images
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_left.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_middle.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_right.png" ) );

	Set_Image( 1, 1 );
	Update_rect();

	contact_time = 0;
	platform_state = PLATFORM_STAY;
	shake_dir_counter = 0;
	shake_counter = 0;
}

cFalling_Platform *cFalling_Platform :: Copy( void )
{
	cFalling_Platform *falling_platform = new cFalling_Platform( startposx, startposy );
	falling_platform->Set_Time_Fall( time_fall );
	falling_platform->Set_Middle_Count( middle_count );

	falling_platform->Set_image_top_left( images[0] );
	falling_platform->Set_image_top_middle( images[1] );
	falling_platform->Set_image_top_right( images[2] );

	return falling_platform;
}

void cFalling_Platform :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// speed
	Set_Time_Fall( attributes.getValueAsFloat( "time_fall", time_fall ) );
	// middle image count
	Set_Middle_Count( attributes.getValueAsInteger( "middle_img_count", middle_count ) );
	// Image Top Left
	Set_image_top_left( pVideo->Get_Surface( attributes.getValueAsString( "image_top_left", images[0]->Get_filename() ).c_str() ) );
	// Image Top Middle
	Set_image_top_middle( pVideo->Get_Surface( attributes.getValueAsString( "image_top_middle", images[1]->Get_filename() ).c_str() ) );
	// Image Top Right
	Set_image_top_right( pVideo->Get_Surface( attributes.getValueAsString( "image_top_right", images[2]->Get_filename() ).c_str() ) );
}

void cFalling_Platform :: Save_to_Stream( ofstream &file )
{
	// begin falling_platform
	file << "\t<falling_platform>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// fall time
	file << "\t\t<Property name=\"time_fall\" value=\"" << time_fall << "\" />" << std::endl;
	// middle image count
	file << "\t\t<Property name=\"middle_img_count\" value=\"" << middle_count << "\" />" << std::endl;
	// image begin
	file << "\t\t<Property name=\"image_top_left\" value=\"" << images[0]->Get_filename( 1 ) << "\" />" << std::endl;
	// image middle 1
	file << "\t\t<Property name=\"image_top_middle\" value=\"" << images[1]->Get_filename( 1 ) << "\" />" << std::endl;
	// image end
	file << "\t\t<Property name=\"image_top_right\" value=\"" << images[2]->Get_filename( 1 ) << "\" />" << std::endl;

	// end falling_platform
	file << "\t</falling_platform>" << std::endl;
}

void cFalling_Platform :: Set_Massivetype( MassiveType mtype )
{
	massivetype = mtype;
}

void cFalling_Platform :: Set_Middle_Count( int val )
{
	middle_count = val;

	if( middle_count < 0 )
	{
		middle_count = 0;
	}
}

void cFalling_Platform :: Set_Time_Fall( float val )
{
	time_fall = val;

	if( time_fall < 0 )
	{
		time_fall = 0;
	}
}

void cFalling_Platform :: Set_image_top_left( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[0] = surface;
	Update_rect();
}

void cFalling_Platform :: Set_image_top_middle( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[1] = surface;
	Update_rect();
}

void cFalling_Platform :: Set_image_top_right( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[2] = surface;
	Update_rect();
}

void cFalling_Platform :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( platform_state == PLATFORM_TOUCHED )
	{
		contact_time += pFramerate->speedfactor;
		// time = seconds since start
		float time = contact_time / DESIRED_FPS;

		if( time > time_fall )
		{
			platform_state = PLATFORM_SHAKE;
		}
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
				move_x = -(float)(rand() % 5);
			}
			else
			{
				move_x = (float)(rand() % 5);
			}

			// move
			col_pos.x -= move_x;
			Move( move_x, 0, 1 );

			shake_dir_counter = 0;
		}

		shake_counter += pFramerate->speedfactor;

		if( shake_counter > DESIRED_FPS * 0.5f )
		{
			platform_state = PLATFORM_FALL;
			Set_Massivetype( MASS_PASSIVE );
		}
	}
	else if( platform_state == PLATFORM_FALL )
	{
		if( vely < 25 )
		{
			vely += 3.1f * pFramerate->speedfactor;

			// limit falling speed
			if( vely > 25 )
			{
				vely = 25;
			}
		}

		if( posx > startposx )
		{
			Add_RotationZ( 1.5f );
		}
		else
		{
			Add_RotationZ( -1.5f );
		}

		// below screen
		if( col_rect.y + col_rect.h > pCamera->limit_rect.y + GAME_RES_H )
		{
			platform_state = PLATFORM_STAY;
			Set_Visible( 0 );
		}
	}
}

void cFalling_Platform :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// todo : update & optimize
	float x = posx;
	float startx = startposx;

	// Start
	Set_Image( 0, 1, 0 );
	cImageObjectSprite::Draw( request );
	posx += Get_Image( 0 )->w;
	startposx += Get_Image( 0 )->w;
	Update_Position_Rect();

	// Middle
	Set_Image( 1, 1, 0 );
	for( unsigned int i = 0; i < middle_count; i++ )
	{
		cImageObjectSprite::Draw();
		posx += Get_Image( 1 )->w;
		startposx += Get_Image( 1 )->w;
		Update_Position_Rect();
	}

	// End
	Set_Image( 2, 1, 0 );
	cImageObjectSprite::Draw();

	posx = x;
	startposx = startx;

	Update_rect();
	Update_Position_Rect();
}

void cFalling_Platform :: Update_rect( void )
{
	col_rect.w = 0;
	rect.w = 0;
	if( images[0] )
	{
		col_rect.w += images[0]->col_w;
		rect.w += images[0]->w;
	}
	if( images[1] )
	{
		col_rect.w += images[1]->col_w * middle_count;
		rect.w += images[1]->w * middle_count;
	}
	if( images[2] )
	{
		col_rect.w += images[2]->col_w;
		rect.w += images[2]->w;
	}

	start_rect.w = rect.w;
}

bool cFalling_Platform :: is_Update_valid( void )
{
	if( !visible )
	{
		return 0;
	}

	return 1;
}

unsigned int cFalling_Platform :: Validate_Collision( cSprite *obj )
{
	if( obj->type == TYPE_PLAYER )
	{
		// don't set onground if jumping
		if( pPlayer->is_onTop( this ) && pPlayer->state != STA_JUMP )
		{
			if( !pPlayer->ground_object )
			{
				pPlayer->ground_object = this;
			}

			pPlayer->Set_onTop( this, 0 );

            return 1;
		}

		return 0;
	}
	if( obj->type == TYPE_BALL )
	{
		if( obj->is_onTop( this ) )
		{
			obj->Set_onTop( this, 0 );
			return 1;
		}

		return 0;
	}

	return 0;
}

void cFalling_Platform :: Handle_Collision( cObjectCollision *collision )
{
	if( !massivetype || !visible )
	{
		return;
	}

	cImageObjectSprite::Handle_Collision( collision );
}

void cFalling_Platform :: Handle_Collision_Player( ObjectDirection cdirection )
{
	if( cdirection == DIR_UNDEFINED )
	{
		return;
	}

	if( cdirection == DIR_TOP )
	{
		if( platform_state == PLATFORM_STAY )
		{
			platform_state = PLATFORM_TOUCHED;
		}
	}
}

void cFalling_Platform :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// time to fall
	Editbox *editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_falling_platform_time_fall" );
	editbox->setTooltipText( "Time to fall" );
	Editor_Add( editbox, 120 );

	editbox->setText( float_to_string( time_fall ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cFalling_Platform::Editor_Time_Fall_Key, this ) );

	// horizontal middle image count
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_falling_platform_hor_middle_count" );
	editbox->setTooltipText( "Horizontal Middle Image Count" );
	Editor_Add( editbox, 120 );

	editbox->setText( int_to_string( middle_count ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cFalling_Platform::Editor_Hor_Middle_Count_Key, this ) );

	// image top left
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_falling_platform_image_top_left" );
	editbox->setTooltipText( "Image Top Left" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[0]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cFalling_Platform::Editor_Image_Top_Left_Key, this ) );

	// image top middle
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_falling_platform_image_top_middle" );
	editbox->setTooltipText( "Image Top Middle" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[1]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cFalling_Platform::Editor_Image_Top_Middle_Key, this ) );

	// image top right
	editbox = (Editbox *)wmgr.createWindow( "TaharezLook/Editbox", "editor_falling_platform_image_top_right" );
	editbox->setTooltipText( "Image Top Right" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[2]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cFalling_Platform::Editor_Image_Top_Right_Key, this ) );


	// set position
	Editor_pos_update();
}

bool cFalling_Platform :: Editor_Time_Fall_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Time_Fall( string_to_float( str_text ) );

	return 1;
}

bool cFalling_Platform :: Editor_Hor_Middle_Count_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Middle_Count( string_to_int( str_text ) );

	return 1;
}

bool cFalling_Platform :: Editor_Image_Top_Left_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_left( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cFalling_Platform :: Editor_Image_Top_Middle_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_middle( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cFalling_Platform :: Editor_Image_Top_Right_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_right( pVideo->Get_Surface( str_text ) );

	return 1;
}
