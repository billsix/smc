/***************************************************************************
				waypoint.cpp  -  waypoint class for the Overworld
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../overworld/waypoint.h"
#include "../overworld/overworld.h"
#include "../core/game_core.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../user/preferences.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"
#include "../level/level.h"

/* *** *** *** *** *** *** *** *** cWaypoint *** *** *** *** *** *** *** *** *** */

cWaypoint :: cWaypoint( void )
: cSprite()
{
	Init();
}

cWaypoint :: cWaypoint( XMLAttributes &attributes )
: cSprite()
{
	Init();
	Create_from_Stream( attributes );
}


cWaypoint :: ~cWaypoint( void )
{
	//
}

void cWaypoint :: Init( void )
{
	sprite_array = ARRAY_PASSIVE;
	type = TYPE_OW_WAYPOINT;
	massivetype = MASS_PASSIVE;
	posz = 0.08f;
	player_range = 0;
	
	waypoint_type = WAYPOINT_NORMAL;
	name = "Waypoint";

	gcolor = (float)( rand() % 100 );
	glim = 1;
	
	access = 0;
	access_default = 0;

	direction_backward = DIR_UNDEFINED;
	direction_forward = DIR_UNDEFINED;

	Set_Image( pVideo->Get_Surface( "world/waypoint/default_1.png" ) );

	arrow_blue_l = pVideo->Get_Surface( "game/arrow/small/blue/left.png" );
	arrow_blue_r = pVideo->Get_Surface( "game/arrow/small/blue/right.png" );
	arrow_blue_u = pVideo->Get_Surface( "game/arrow/small/blue/up.png" );
	arrow_blue_d = pVideo->Get_Surface( "game/arrow/small/blue/down.png" );

	arrow_white_l = pVideo->Get_Surface( "game/arrow/small/white/left.png" );
	arrow_white_r = pVideo->Get_Surface( "game/arrow/small/white/right.png" );
	arrow_white_u = pVideo->Get_Surface( "game/arrow/small/white/up.png" );
	arrow_white_d = pVideo->Get_Surface( "game/arrow/small/white/down.png" );
}

cWaypoint *cWaypoint :: Copy( void )
{
	cWaypoint *waypoint = new cWaypoint();

	waypoint->Set_Pos( startposx, startposy, 1 );
	waypoint->Set_Image( start_image, 1 );
	waypoint->waypoint_type = waypoint_type;
	waypoint->Set_Destination( destination );
	waypoint->direction_backward = direction_backward;
	waypoint->direction_forward = direction_forward;
	waypoint->Set_Access( access_default, 1 );

	return waypoint;
}

void cWaypoint :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "x" ), (float)attributes.getValueAsInteger( "y" ), 1 );
	// image
	/*if( attributes.exists( "image" ) )
	{
		Set_Image( pVideo->Get_Surface( attributes.getValueAsString( "image" ).c_str() ), 1 ) ;
	}*/
	// type
	waypoint_type = (Waypoint_type)attributes.getValueAsInteger( "type", WAYPOINT_NORMAL );
	// destination
	// pre 0.99.6 : world
	if( attributes.exists( "world" ) )
	{
		Set_Destination( attributes.getValueAsString( "world" ).c_str() );
	}
	// pre 0.99.6 : level
	else if( attributes.exists( "level" ) )
	{
		Set_Destination( attributes.getValueAsString( "level" ).c_str() );
	}
	// default : destination
	else
	{
		Set_Destination( attributes.getValueAsString( "destination" ).c_str() );
	}
	// backward direction
	direction_backward = Get_Direction_id( attributes.getValueAsString( "direction_backward", "left" ).c_str() );
	// forward direction
	direction_forward = Get_Direction_id( attributes.getValueAsString( "direction_forward", "right" ).c_str() );
	// access
	Set_Access( attributes.getValueAsBool( "access", 1 ), 1 );
}

void cWaypoint :: Save_to_Stream( ofstream &file )
{
	// begin waypoint
	file << "\t<waypoint>" << std::endl;

	// position
	file << "\t\t<Property name=\"x\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"y\" value=\"" << (int)startposy << "\" />" << std::endl;
	// image
	/*if( start_image )
	{
		string img_filename = start_image->filename;

		// remove pixmaps directory from string
		if( img_filename.find( PIXMAPS_DIR ) == 0 )
		{
			img_filename.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	}*/
	// type
	file << "\t\t<Property name=\"type\" value=\"" << (int)waypoint_type << "\" />" << std::endl;
	// destination
	file << "\t\t<Property name=\"destination\" value=\"" << destination << "\" />" << std::endl;
	// direction backward
	file << "\t\t<Property name=\"direction_backward\" value=\"" << Get_Direction_name( direction_backward ) << "\" />" << std::endl;
	// direction forward
	file << "\t\t<Property name=\"direction_forward\" value=\"" << Get_Direction_name( direction_forward ) << "\" />" << std::endl;
	// access
	file << "\t\t<Property name=\"access\" value=\"" << access_default << "\" />" << std::endl;

	// end waypoint
	file << "\t</waypoint>" << std::endl;
}

void cWaypoint :: Update( void )
{
	cSprite::Update();

	if( glim )
	{
		gcolor += pFramerate->speedfactor * 3;
	}
	else
	{
		gcolor -= pFramerate->speedfactor * 3;
	}

	if( gcolor > 120 )
	{
		glim = 0;
	}
	else if( gcolor < 7 )
	{
		glim = 1;
	}
}

void cWaypoint :: Draw( cSurfaceRequest *request /* = NULL  */ )
{	
	if( pOverworld_manager->debugmode || editor_world_enabled )
	{
		float x = rect.x - pCamera->x;
		float y = rect.y - pCamera->y;
		
		// direction back
		if( direction_backward == DIR_RIGHT )
		{
			x += image->w;
			y += image->h * 0.2f;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			// blit
			arrow_blue_r->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_backward == DIR_LEFT )
		{
			x -= arrow_blue_l->w;
			y += image->h * 0.2f;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			// blit
			arrow_blue_l->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_backward == DIR_UP )
		{
			y -= arrow_blue_u->h;
			x += 4;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			// blit
			arrow_blue_u->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_backward == DIR_DOWN )
		{
			y += image->h;
			x += 4;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			// blit
			arrow_blue_d->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}

		x = rect.x - pCamera->x;
		y = rect.y - pCamera->y;

		// direction forward
		if( direction_forward == DIR_RIGHT )
		{
			x += image->w;
			y += image->h * 0.2f;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			arrow_white_r->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_forward == DIR_LEFT )
		{
			x -= arrow_white_l->w;
			y += image->h * 0.2f;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			arrow_white_l->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_forward == DIR_UP )
		{
			y -= arrow_white_u->h;
			x += 4;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			arrow_white_u->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
		else if( direction_forward == DIR_DOWN )
		{
			y += image->h;
			x += 4;

			// create request
			cSurfaceRequest *surface_request = new cSurfaceRequest();
			arrow_white_d->Blit( x, y, 0.089f, surface_request );
			surface_request->shadow_pos = 2;
			surface_request->shadow_color = lightgreyalpha64;
			// add request
			pRenderer->Add( surface_request );
		}
	}

	// draw waypoint
	if( ( access && ( waypoint_type == 1 || waypoint_type == 2 ) ) || pOverworld_manager->debugmode || editor_world_enabled )
	{
		bool create_request = 0;

		if( !request )
		{
			create_request = 1;
			// create request
			request = new cSurfaceRequest();
		}

		// draw
		cSprite::Draw( request );

		// default color
		if( !pOverworld_manager->debugmode )
		{
			request->color = Color( (Uint8)255, 100 + (Uint8)gcolor, 10 );
		}
		// change to debug color
		else
		{
			if( access )
			{
				request->color = Color( (Uint8)255, 100 + (Uint8)gcolor, 200 );
			}
			else
			{
				request->color =  Color( (Uint8)20, 100 + (Uint8)gcolor, 10 );
			}
		}

		if( create_request )
		{
			// add request
			pRenderer->Add( request );
		}
	}
}

void cWaypoint :: Set_Access( bool enabled, bool new_start_access /* = 0 */ )
{
	access = enabled;
	
	if( new_start_access )
	{
		access_default = access;
	}
}

void cWaypoint :: Set_Destination( string str )
{
	// normal waypoint
	if( waypoint_type == WAYPOINT_NORMAL )
	{
		// erase file type and directory  if set
		str = Get_filename( str, 0, 0 );
	}
	// world waypoint
	else if( waypoint_type == WAYPOINT_WORLD_LINK && str.find( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) != string::npos )
	{
		str.erase( 0, strlen( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) );
	}

	destination = str;
}

string cWaypoint :: Get_Destination( bool with_dir /* = 0 */, bool with_end /* = 0 */ )
{
	string name = destination;

	if( waypoint_type == WAYPOINT_NORMAL )
	{
		pLevel->Get_path( name );
		name = Get_filename( name, with_dir, with_end );
	}
	else if( waypoint_type == WAYPOINT_WORLD_LINK )
	{
		if( with_dir && name.find( DATA_DIR "/" GAME_OVERWORLD_DIR "/" ) == string::npos )
		{
			name.insert( 0, DATA_DIR "/" GAME_OVERWORLD_DIR "/" );
		}
	}

	return name;
}

void cWaypoint :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// destination
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "waypoint_destination" ));
	editbox->setTooltipText( "Destination" );
	Editor_Add( editbox, 150 );

	editbox->setText( Get_Destination() );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cWaypoint::Editor_Destination_Key, this ) );

	// backward direction
	Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_backward_direction" ));
	combobox->getEditbox()->setTooltipText( "Backward Direction" );
	Editor_Add( combobox, 100, 105 );

	combobox->addItem( new ListboxTextItem( "up" ) );
	combobox->addItem( new ListboxTextItem( "down" ) );
	combobox->addItem( new ListboxTextItem( "right" ) );
	combobox->addItem( new ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_name( direction_backward ) );

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cWaypoint::Editor_Backward_Direction_Select, this ) );

	// forward direction
	combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_forward_direction" ));
	combobox->getEditbox()->setTooltipText( "Forward Direction" );
	Editor_Add( combobox, 100, 105 );

	combobox->addItem( new ListboxTextItem( "up" ) );
	combobox->addItem( new ListboxTextItem( "down" ) );
	combobox->addItem( new ListboxTextItem( "right" ) );
	combobox->addItem( new ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_name( direction_forward ) );

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cWaypoint::Editor_Forward_Direction_Select, this ) );

	// Access
	combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "waypoint_access" ));
	combobox->getEditbox()->setTooltipText( "Default Access" );
	Editor_Add( combobox, 120, 80 );

	combobox->addItem( new ListboxTextItem( "Enabled" ) );
	combobox->addItem( new ListboxTextItem( "Disabled" ) );

	if( access_default == 1 )
	{
		combobox->setText( "Enabled" );
	}
	else
	{
		combobox->setText( "Disabled" );
	}

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cWaypoint::Editor_Access_Select, this ) );

	// set position
	Editor_pos_update();
}

bool cWaypoint :: Editor_Destination_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Destination( str_text );

	return 1;
}

bool cWaypoint :: Editor_Backward_Direction_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	direction_backward = Get_Direction_id( item->getText().c_str() );

	return 1;
}

bool cWaypoint :: Editor_Forward_Direction_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	direction_forward = Get_Direction_id( item->getText().c_str() );

	return 1;
}

bool cWaypoint :: Editor_Access_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( "Enabled" ) == 0 )
	{
		Set_Access( 1 );
	}
	else
	{
		Set_Access( 0 );
	}

	return 1;
}
