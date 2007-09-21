/***************************************************************************
           moving_platform.cpp  -  default moving platforms handler
                             -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
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

/* *** *** *** *** *** *** *** cMoving_Platform *** *** *** *** *** *** *** *** *** *** */

cMoving_Platform :: cMoving_Platform( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cMoving_Platform :: cMoving_Platform( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
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

	Set_Massivetype( MASS_HALFMASSIVE );
	Set_Max_Distance( 150 );
	Set_Speed( 3 );
	Set_Middle_Count( 2 );

	Set_Direction( DIR_RIGHT );

	// create default images
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_left.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_middle.png" ) );
	images.push_back( pVideo->Get_Surface( "slider/grey_1/slider_right.png" ) );
	Set_Image( 1, 1 );

	Update_rect();

	Create_Name();
}

cMoving_Platform *cMoving_Platform :: Copy( void )
{
	cMoving_Platform *moving_platform = new cMoving_Platform( startposx, startposy );
	moving_platform->Set_Massivetype( massivetype );
	moving_platform->Set_Direction( start_direction );
	moving_platform->Set_Max_Distance( max_distance );
	moving_platform->Set_Speed( speed );
	moving_platform->Set_Middle_Count( middle_count );
	moving_platform->Set_image_top_left( images[0] );
	moving_platform->Set_image_top_middle( images[1] );
	moving_platform->Set_image_top_right( images[2] );

	return moving_platform;
}

void cMoving_Platform :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// massive type
	Set_Massivetype( Get_Massivetype_id( attributes.getValueAsString( "massive_type", Get_Massivetype_name( massivetype ) ).c_str() ) );
	// direction
	Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	// max distance
	Set_Max_Distance( attributes.getValueAsInteger( "max_distance", max_distance ) );
	// speed
	Set_Speed( attributes.getValueAsFloat( "speed", speed ) );
	// middle image count
	Set_Middle_Count( attributes.getValueAsInteger( "middle_img_count", middle_count ) );
	// Image Top Left
	Set_image_top_left( pVideo->Get_Surface( attributes.getValueAsString( "image_top_left", images[0]->Get_filename() ).c_str() ) );
	// Image Top Middle
	Set_image_top_middle( pVideo->Get_Surface( attributes.getValueAsString( "image_top_middle", images[1]->Get_filename() ).c_str() ) );
	// Image Top Right
	Set_image_top_right( pVideo->Get_Surface( attributes.getValueAsString( "image_top_right", images[2]->Get_filename() ).c_str() ) );
}

void cMoving_Platform :: Save_to_Stream( ofstream &file )
{
	// begin moving_platform
	file << "\t<moving_platform>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// massive type
	file << "\t\t<Property name=\"massive_type\" value=\"" << Get_Massivetype_name( massivetype ) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	// max distance
	file << "\t\t<Property name=\"max_distance\" value=\"" << max_distance << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;
	// middle image count
	file << "\t\t<Property name=\"middle_img_count\" value=\"" << middle_count << "\" />" << std::endl;
	// image top left
	file << "\t\t<Property name=\"image_top_left\" value=\"" << images[0]->Get_filename( 1 ) << "\" />" << std::endl;
	// image top middle
	file << "\t\t<Property name=\"image_top_middle\" value=\"" << images[1]->Get_filename( 1 ) << "\" />" << std::endl;
	// image top right
	file << "\t\t<Property name=\"image_top_right\" value=\"" << images[2]->Get_filename( 1 ) << "\" />" << std::endl;

	// end moving_platform
	file << "\t</moving_platform>" << std::endl;
}

void cMoving_Platform :: Set_Massivetype( MassiveType mtype )
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

void cMoving_Platform :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( start_direction == dir )
	{
		return;
	}

	cImageObjectSprite::Set_Direction( dir, 1 );
	Create_Name();
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

	if( speed < 0 )
	{
		speed = 0;
	}
}

void cMoving_Platform :: Set_Middle_Count( int val )
{
	middle_count = val;

	if( middle_count < 0 )
	{
		middle_count = 0;
	}
}

void cMoving_Platform :: Set_image_top_left( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[0] = surface;
	Update_rect();
}

void cMoving_Platform :: Set_image_top_middle( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[1] = surface;
	Update_rect();
}

void cMoving_Platform :: Set_image_top_right( GL_Surface *surface )
{
	if( !surface )
	{
		return;
	}

	images[2] = surface;
	Update_rect();
}

void cMoving_Platform :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	if( direction == DIR_UP )
	{
		velx = 0;
		vely = -speed;

		if( posy - startposy < 0 )
		{
			direction = DIR_DOWN;
		}
	}
	else if( direction == DIR_DOWN )
	{
		velx = 0;
		vely = speed;

		if( posy - startposy > max_distance )
		{
			direction = DIR_UP;
		}
	}
	else if( direction == DIR_LEFT )
	{
		velx = -speed;
		vely = 0;

		if( posx - startposx < 0 )
		{
			direction = DIR_RIGHT;
		}
	}
	else if( direction == DIR_RIGHT )
	{
		velx = speed;
		vely = 0;

		if( posx - startposx > max_distance )
		{
			direction = DIR_LEFT;
		}
	}
}

void cMoving_Platform :: Draw( cSurfaceRequest *request /* = NULL */ )
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

	// draw distance rect
	if( editor_level_enabled )
	{
		if( direction == DIR_RIGHT || direction == DIR_LEFT )
	    {
            pVideo->Draw_Rect( startposx - pCamera->x, startposy - pCamera->y, max_distance + col_rect.w, 15, posz - 0.000001f, &whitealpha128 );
	    }
		else if( direction == DIR_DOWN || direction == DIR_UP )
	    {
            pVideo->Draw_Rect( startposx - pCamera->x, startposy - pCamera->y, 15, max_distance + col_rect.h, posz - 0.000001f, &whitealpha128 );
	    }
	}
}

void cMoving_Platform :: Update_rect( void )
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

bool cMoving_Platform :: is_Update_valid( void )
{
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
		if( moving_sprite->is_onTop( this ) && direction == DIR_UP && moving_sprite->vely > vely )
		{
			// halfmassive only if no ground but massive always picks you up
			if( ( massivetype == MASS_HALFMASSIVE && !moving_sprite->ground_object ) ||
				 ( massivetype == MASS_MASSIVE && moving_sprite->ground_object != this ) )
			{
				return 1;
			}
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

void cMoving_Platform :: Handle_Collision_Player( cObjectCollision *collision )
{
	if( collision->direction == DIR_TOP )
	{
		printf( "MP : Send top collision\n" );
		// send collision
		Send_Collision( collision );
	}
}

void cMoving_Platform :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// max distance
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_max_distance" ));

	Editor_Add( editbox, 120 );
	editbox->setTooltipText( "Max Distance" );
	editbox->setText( int_to_string( max_distance ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Max_Distance_Key, this ) );

	// speed
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_speed" ));
	editbox->setTooltipText( "Speed" );
	Editor_Add( editbox, 120 );

	editbox->setText( float_to_string( speed ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Speed_Key, this ) );

	// horizontal middle image count
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_hor_middle_count" ));
	editbox->setTooltipText( "Horizontal Middle Image Count" );
	Editor_Add( editbox, 120 );

	editbox->setText( int_to_string( middle_count ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Hor_Middle_Count_Key, this ) );

	// image top left
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_left" ));
	editbox->setTooltipText( "Image Top Left" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[0]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Left_Key, this ) );

	// image top middle
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_middle" ));
	editbox->setTooltipText( "Image Top Middle" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[1]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Middle_Key, this ) );

	// image top right
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_moving_platform_image_top_right" ));
	editbox->setTooltipText( "Image Top Right" );
	Editor_Add( editbox, 200 );

	editbox->setText( images[2]->Get_filename( 1 ) );
	editbox->subscribeEvent( Editbox::EventTextChanged, Event::Subscriber( &cMoving_Platform::Editor_Image_Top_Right_Key, this ) );


	// set position
	Editor_pos_update();
}

bool cMoving_Platform :: Editor_Max_Distance_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Max_Distance( string_to_int( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Speed_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Speed( string_to_float( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Hor_Middle_Count_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Middle_Count( string_to_int( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Left_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_left( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Middle_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_middle( pVideo->Get_Surface( str_text ) );

	return 1;
}

bool cMoving_Platform :: Editor_Image_Top_Right_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_image_top_right( pVideo->Get_Surface( str_text ) );

	return 1;
}

void cMoving_Platform :: Create_Name( void )
{
	name = "Moving Platform - " + Get_Direction_name( start_direction ) + " - " + Get_Massivetype_name( massivetype );
}
