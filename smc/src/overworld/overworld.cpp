/***************************************************************************
 * overworld.cpp  -  Overworld class
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

#include "../core/globals.h"
#include "../core/main.h"
#include "../overworld/overworld.h"
#include "../audio/audio.h"
#include "../core/game_core.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../video/font.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../input/keyboard.h"
#include "../level/level.h"
#include "../core/sprite_manager.h"
#include "../core/i18n.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;


/* *** *** *** *** *** *** *** *** cOverworld_description *** *** *** *** *** *** *** *** *** */

cOverworld_description :: cOverworld_description( void )
{
	path = "world_1";
	name = _("Unnamed");
	visible = 1;
	user = 0;

	comment = _("Empty");
}

cOverworld_description :: ~cOverworld_description( void )
{
	//
}

void cOverworld_description :: Load( void )
{
	string filename = Get_Full_Path() + "/description.xml";

	// filename not valid
	if( !File_Exists( filename ) )
	{
		printf( "Error : Couldn't open World description file : %s\n", filename.c_str() );
		return;
	}

	// Load Description
	CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Description.xsd", "" );
}

void cOverworld_description :: Save( void )
{
	string save_dir = user_data_dir + USER_WORLD_DIR + "/" + path;
	string filename = save_dir + "/description.xml";

	ofstream file( filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		debugdisplay->Set_Text( _("Couldn't save world description ") + filename );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin Description
	file << "<Description>" << std::endl;

	// begin World
	file << "\t<World>" << std::endl;
		// name
		file << "\t\t<Property Name=\"name\" Value=\"" << string_to_xml_string( name ) << "\" />" << std::endl;
		// visible
		file << "\t\t<Property Name=\"visible\" Value=\"" << visible << "\" />" << std::endl;
	// end World
	file << "\t</World>" << std::endl;


	// end Description
	file << "</Description>" << std::endl;
	file.close();
}

string cOverworld_description :: Get_Full_Path( void )
{
	// return user world if available
	if( fs::exists( fs::path( user_data_dir + USER_WORLD_DIR + "/" + path + "/description.xml", fs::native ) ) )
	{
		return user_data_dir + USER_WORLD_DIR + "/" + path;
	}

	// return game world
	return DATA_DIR "/" GAME_OVERWORLD_DIR "/" + path;
}

// XML element start
void cOverworld_description :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
    }
}

// XML element end
void cOverworld_description :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "World" )
		{
			handle_world( xml_attributes );
		}
		else if( element == "Description" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : World Description Unknown element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

void cOverworld_description :: handle_world( const CEGUI::XMLAttributes &attributes )
{
	name = attributes.getValueAsString( "name", name.c_str() ).c_str();
	visible = attributes.getValueAsBool( "visible", 1 );
}

/* *** *** *** *** *** *** *** *** cOverworld *** *** *** *** *** *** *** *** *** */

cOverworld :: cOverworld( void )
{
	sprite_manager = new cSprite_Manager( 500 );

	description = new cOverworld_description();
	pLayer = new cLayer();

	background_color = Color();
	musicfile = "overworld/land_1.ogg";
	hud_world_name = new cHudSprite( NULL, 10, static_cast<float>(game_res_h) - 30, 0 );
	hud_world_name->Set_Shadow( black, 1.5f );
	hud_level_name = new cHudSprite( NULL, 350, 2, 0 );
	hud_level_name->Set_Shadow( black, 1.5f );

	next_level = 0;

	player_start_waypoint = 0;
	player_moving_state = STA_STAY;
}

cOverworld :: ~cOverworld( void )
{
	Unload();

	delete sprite_manager;
	delete description;
	delete pLayer;
	delete hud_level_name;
	delete hud_world_name;
}

void cOverworld :: Enter( bool delayed /* = 0 */ )
{
	if( game_exit )
	{
		return;
	}

	// enter on the next update
	if( delayed )
	{
		Game_Action = GA_ENTER_WORLD;
		return;
	}

	// unload level if possible
	if( pActive_Level->delayed_unload )
	{
		pActive_Level->Unload();
	}

	// if not loaded
	if( !sprite_manager->size() )
	{
		return;
	}

	// change game mode
	Change_Game_Mode( MODE_OVERWORLD );

	// if player start waypoint not set
	if( pOverworld_Player->current_waypoint < 0 )
	{
		pOverworld_Player->Reset();
		pOverworld_Player->Set_Waypoint( player_start_waypoint, 1 );
	}

	// if goto next level
	if( next_level )
	{
		Goto_Next_Level();
	}

	// if on start waypoint
	if( pOverworld_Player->current_waypoint == static_cast<int>(player_start_waypoint) )
	{
		// if player state is walk
		if( player_moving_state == STA_WALK )
		{
			// walk to the next Waypoint
			pOverworld_Player->Start_Walk( waypoints[pOverworld_Player->current_waypoint]->direction_forward );
		}
	}
}

bool cOverworld :: Key_Down( SDLKey key )
{
	if( key == SDLK_LEFT )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_LEFT );
		}
		return 0;
	}
	else if( key == SDLK_RIGHT )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_RIGHT );
		}
		return 0;
	}
	else if( key == SDLK_UP )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_UP );
		}
		return 0;
	}
	else if( key == SDLK_DOWN )
	{
		if( !pOverworld_Manager->cameramode && !editor_world_enabled )
		{
			pOverworld_Player->Action_Interact( INP_DOWN );
		}
		return 0;
	}
	else if( key == SDLK_c && !editor_world_enabled )
	{
		pOverworld_Manager->cameramode = !pOverworld_Manager->cameramode;
	}
	else if( key == SDLK_F8 )
	{
		pWorld_Editor->Toggle();
	}
	else if( key == SDLK_d && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		pOverworld_Manager->debugmode = !pOverworld_Manager->debugmode;
		game_debug = pOverworld_Manager->debugmode;
	}
	else if( key == SDLK_l && pOverworld_Manager->debugmode )
	{
		// toggle layer drawing
		pOverworld_Manager->draw_layer = !pOverworld_Manager->draw_layer;
	}
	else if( pKeyboard->keys[SDLK_g] && pKeyboard->keys[SDLK_o] && pKeyboard->keys[SDLK_d] )
	{
		// all waypoint access
		pActive_Overworld->Set_Progress( pActive_Overworld->waypoints.size(), 1 );
	}
	else if( key == SDLK_F3 && pOverworld_Manager->debugmode )
	{
		pActive_Overworld->Goto_Next_Level();
	}
	// Exit
	else if( key == SDLK_ESCAPE || key == SDLK_BACKSPACE )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	// ## editor
	else if( pWorld_Editor->Key_Down( key ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Key_Up( SDLKey key )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Mouse_Down( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Down( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cOverworld :: Mouse_Up( Uint8 button )
{
	// ## editor
	if( pWorld_Editor->Mouse_Up( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cOverworld :: Joy_Button_Down( Uint8 button )
{
	// Exit
	if( button == pPreferences->joy_button_exit )
	{
		pOverworld_Player->Action_Interact( INP_EXIT );
	}
	// Action
	else if( button == pPreferences->joy_button_action )
	{
		pOverworld_Player->Action_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Joy_Button_Up( Uint8 button )
{
	// nothing yet
	if( 0 )
	{
		//
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cOverworld :: Load( void )
{
	Unload();

	// set active for loading
	pActive_Overworld = this;

	// description
	description->Load();

	// layer
	string layer_filename = description->Get_Full_Path() + "/layer.xml";

	if( !File_Exists( layer_filename ) )
	{
		printf( "Couldn't find World Layer file : %s from %s\n", layer_filename.c_str(), description->path.c_str() );
		return 0;
	}

	pLayer->Load( layer_filename );

	// world
	string world_filename = description->Get_Full_Path() + "/world.xml";

	if( !File_Exists( world_filename ) )
	{
		printf( "Couldn't find World file : %s from %s\n", world_filename.c_str(), description->path.c_str() );
		return 0;
	}

	try
	{
		// parse overworld
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, world_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/World.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Loading World %s CEGUI Exception %s\n", world_filename.c_str(), ex.getMessage().c_str() );
		debugdisplay->Set_Text( _("World Loading failed : ") + (string)ex.getMessage().c_str() );
	}

	hud_world_name->Set_Image( pFont->Render_Text( pFont->font_normal, description->name, yellow ), 1, 1 );

	return 1;
}

void cOverworld :: Unload( void )
{
	// set active for unloading
	pActive_Overworld = this;

	// Waypoints
	waypoints.clear();
	// Layer
	pLayer->Delete_All();
	// Objects
	sprite_manager->Delete_All();

	pActive_Overworld = NULL;
}

void cOverworld :: Save( void )
{
	pAudio->Play_Sound( "editor/save.ogg" );

	string save_dir = user_data_dir + USER_WORLD_DIR + "/" + description->path;
	// Create directory if new world
	if( !fs::exists( fs::path( save_dir, fs::native ) ) )
	{
		fs::create_directory( fs::path( save_dir, fs::native ) );
	}

	string filename = save_dir + "/world.xml";

	ofstream file( filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		debugdisplay->Set_Text( _("Couldn't save world ") + filename );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin overworld
	file << "<overworld>" << std::endl;

	// begin info
	file << "\t<information>" << std::endl;
		// game version
		file << "\t\t<Property name=\"game_version\" value=\"" << smc_version << "\" />" << std::endl;
		// engine version
		file << "\t\t<Property name=\"engine_version\" value=\"" << 1 << "\" />" << std::endl;
		// time ( seconds since 1970 )
		file << "\t\t<Property name=\"save_time\" value=\"" << time( NULL ) << "\" />" << std::endl;
	// end info
	file << "\t</information>" << std::endl;

	// begin settings
	file << "\t<settings>" << std::endl;
		// music
		file << "\t\t<Property name=\"music\" value=\"" << string_to_xml_string( musicfile ) << "\" />" << std::endl;
	// end settings
	file << "\t</settings>" << std::endl;

	// begin background
	file << "\t<background>" << std::endl;
		// color
		file << "\t\t<Property name=\"color_red\" value=\"" << static_cast<int>(background_color.red) << "\" />" << std::endl;
		file << "\t\t<Property name=\"color_green\" value=\"" << static_cast<int>(background_color.green) << "\" />" << std::endl;
		file << "\t\t<Property name=\"color_blue\" value=\"" << static_cast<int>(background_color.blue) << "\" />" << std::endl;
	// end background
	file << "\t</background>" << std::endl;

	// begin player
	file << "\t<player>" << std::endl;
		// start waypoint
		file << "\t\t<Property name=\"waypoint\" value=\"" << player_start_waypoint << "\" />" << std::endl;
		// moving state
		file << "\t\t<Property name=\"moving_state\" value=\"" << static_cast<int>(player_moving_state) << "\" />" << std::endl;
	// end player
	file << "\t</player>" << std::endl;
	// objects
	for( SpriteList::iterator itr = sprite_manager->objects.begin(), itr_end = sprite_manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// skip spawned and destroyed objects
		if( obj->spawned || obj->destroy )
		{
			continue;
		}

		// save to file stream
		obj->Save_to_Stream( file );
	}

	// end overworld
	file << "</overworld>" << std::endl;
	file.close();

	// save Layer
	if( pLayer->Save( save_dir + "/layer.xml" ) )
	{
		debugdisplay->Set_Text( _("World ") + description->name + _(" saved") );
	}

	// save description
	description->Save();
}

void cOverworld :: Set_Progress( unsigned int normal_level, bool force /* = 1 */ )
{
	unsigned int level_num = 0;

	for( WaypointList::iterator itr = waypoints.begin(), itr_end = waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( normal_level >= level_num )
		{
			obj->Set_Access( 1 );
		}
		// force unset
		else if( force )
		{
			obj->Set_Access( 0 );
		}

		level_num++;
	}
}

cWaypoint *cOverworld :: Get_Waypoint( string name )
{
	for( WaypointList::iterator itr = waypoints.begin(), itr_end = waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		// accessible
		if( obj->destination.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

cWaypoint *cOverworld :: Get_Waypoint( unsigned int num )
{
	if( num >= waypoints.size() )
	{
		// out of bounds
		return NULL;
	}

	// available
	return waypoints[num];
}

int cOverworld :: Get_Level_Waypoint_Num( string level_name )
{
	// erase file type if set
	if( level_name.rfind( ".txt" ) != string::npos || level_name.rfind( ".smclvl" ) != string::npos )
	{
		level_name.erase( level_name.rfind( "." ) );
	}

	return Get_Waypoint_Num( level_name );
}

int cOverworld :: Get_Waypoint_Num( string name )
{
	// search waypoint
	for( unsigned int i = 0; i < waypoints.size(); i++ )
	{
		if( waypoints[i]->destination.compare( name ) == 0 )
		{
			// found
			return i;
		}
	}

	// not found
	return -1;
}

int cOverworld :: Get_Waypoint_Collision( GL_rect *rect_2 )
{
	for( unsigned int i = 0; i < waypoints.size(); i++ )
	{
		if( Col_Box( rect_2, &waypoints[i]->rect ) )
		{
			return i;
		}
	}

	return -1;
}

int cOverworld :: Get_Last_Valid_Waypoint( void )
{
	// no waypoints
	if( !waypoints.size() )
	{
		return -1;
	}

	for( unsigned int i = waypoints.size() - 1; i > 0; i-- )
	{
		if( waypoints[i]->access )
		{
			return i;
		}
	}

	return -1;
}

void cOverworld :: Update_Waypoint_text( void )
{
	// get waypoint
	cWaypoint *waypoint = waypoints[pOverworld_Player->current_waypoint];

	// set color
	Color color = static_cast<Uint8>(0);

	if( waypoint->waypoint_type == WAYPOINT_NORMAL )
	{
		color = lightblue;
	}
	else if( waypoint->waypoint_type == WAYPOINT_WORLD_LINK )
	{
		color = green;
	}
	
	hud_level_name->Set_Image( pFont->Render_Text( pFont->font_normal, waypoint->Get_Destination(), color ), 1, 1 );
}

bool cOverworld :: Goto_Next_Level( void )
{
	// if not in overworld only goto next level on overworld enter
	if( Game_Mode != MODE_OVERWORLD )
	{
		next_level = 1;
		return 0;
	}

	next_level = 0;

	cWaypoint *current_waypoint = pOverworld_Player->Get_Waypoint();

	// no Waypoint
	if( !current_waypoint )
	{
		return 0;
	}

	// Waypoint forward direction is invalid/unset
	if( current_waypoint->direction_forward == DIR_UNDEFINED )
	{
		return 0;
	}

	// Get Layer Line in front
	cLayer_Line *front_line = pOverworld_Player->Get_Front_Line( current_waypoint->direction_forward );

	if( !front_line )
	{
		return 0;
	}

	// Get forward Waypoint
	cWaypoint *next_waypoint = front_line->Get_End_Waypoint();

	// if no next waypoint available
	if( !next_waypoint )
	{
		return 0;
	}

	// if next waypoint is new
	if( !next_waypoint->access )
	{
		next_waypoint->Set_Access( 1 );

		// animation
		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Pos( next_waypoint->rect.x + ( next_waypoint->rect.w * 0.4f ), next_waypoint->rect.y + ( next_waypoint->rect.h * 0.4f ) );
		anim->Set_Emitter_Time_to_Live( 1.5f );
		anim->Set_Emitter_Iteration_Interval( 0.1f );
		anim->Set_Quota( 2 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		anim->Set_Pos_Z( 0.081f );
		anim->Set_Time_to_Live( 1.3f );
		anim->Set_Speed( 1, 1 );
		anim->Set_Scale( 0.5f, 0.4f );
		anim->Set_Const_Rotation_Z( -6, 12 );

		// World Waypoint
		if( next_waypoint->waypoint_type == WAYPOINT_WORLD_LINK )
		{
			anim->Set_Color( lightgreen, Color( static_cast<Uint8>(60), 0, 60 ) );
		}
		else
		{
			anim->Set_Color( orange, Color( static_cast<Uint8>(6), 60, 20 ) );
		}

		// add animation
		pAnimation_Manager->Add( anim );
	}

	pOverworld_Player->Start_Walk( current_waypoint->direction_forward );

	return  1;
}

void cOverworld :: Reset_Waypoints( void )
{
	for( WaypointList::iterator itr = waypoints.begin(), itr_end = waypoints.end(); itr != itr_end; ++itr )
	{
		cWaypoint *obj = (*itr);

		obj->Set_Access( obj->access_default );
	}
}

// XML element start
void cOverworld :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
    }
}

// XML element end
void cOverworld :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "information" )
		{
			//engine_version = xml_attributes.getValueAsInteger( "engine_version" );
			//last_saved = xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "settings" )
		{
			// Author
			//author = xml_attributes.getValueAsString( "author" ).c_str();
			// Version
			//version = xml_attributes.getValueAsString( "version" ).c_str();
			// Music
			musicfile = xml_attributes.getValueAsString( "music" ).c_str();
			// Camera Limits
			//pOverworld_Manager->camera->Set_Limits( GL_rect( static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_x" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_y" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_w" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_h" )) ) );
		}
		else if( element == "player" )
		{
			// Start Waypoint
			player_start_waypoint = xml_attributes.getValueAsInteger( "waypoint" );
			// Moving State
			player_moving_state = static_cast<Moving_state>(xml_attributes.getValueAsInteger( "moving_state" ));
		}
		else if( element == "background" )
		{
			background_color = Color( static_cast<Uint8>(xml_attributes.getValueAsInteger( "color_red" )), xml_attributes.getValueAsInteger( "color_green" ), xml_attributes.getValueAsInteger( "color_blue" ) );
		}
		else
		{
			// get World object
			cSprite *object = Get_World_Object( element, xml_attributes );
			
			// valid
			if( object )
			{
				Add_Map_Object( object );
			}
			else if( element == "overworld" )
			{
				// ignore
			}
			else if( element.length() )
			{
				printf( "Warning : Overworld Unknown element : %s\n", element.c_str() );
			}
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

void cOverworld :: Draw( void )
{
	// Background
	pVideo->Clear_Screen();
	pVideo->Draw_Rect( NULL, 0.00001f, &background_color );

	// Map
	sprite_manager->Draw_Items();
	// Animations
	pAnimation_Manager->Draw();
	// Player
	pOverworld_Player->Draw();
	// Hud
	Draw_HUD();

	// Line Layer
	pLayer->Draw();
	// Editor
	pWorld_Editor->Draw();
}

void cOverworld :: Draw_HUD( void )
{
	// if not editor mode
	if( !editor_world_enabled )
	{
		// Background
		Color color = Color( static_cast<Uint8>(230), 170, 0, 128 );
		pVideo->Draw_Rect( 0, 0, static_cast<float>(game_res_w), 30, 0.12f, &color );
		// Line
		color = Color( static_cast<Uint8>(200), 150, 0, 128 );
		pVideo->Draw_Rect( 0, 30, static_cast<float>(game_res_w), 5, 0.121f, &color );

		// Overworld name and level
		hud_world_name->Draw();
		hud_level_name->Draw();
	}

	// hud
	pHud_Manager->Draw();
}

void cOverworld :: Update( void )
{
	// editor
	pWorld_Editor->Process_Input();

	if( !editor_world_enabled )
	{
		// Camera
		Update_Camera();
		// Map
		sprite_manager->Update_Items();
		// Player
		pOverworld_Player->Update();
		// Animations
		pAnimation_Manager->Update();
	}

	// hud
	pHud_Manager->Update();
	// Editor
	pWorld_Editor->Update();
}

void cOverworld :: Update_Camera( void )
{
	if( editor_world_enabled )
	{
		return;
	}

	// todo : move to a Process_Input function
	if( pOverworld_Manager->cameramode )
	{
		if( pKeyboard->keys[pPreferences->key_right] || ( pJoystick->right && pPreferences->joy_enabled ) )
		{
			pActive_Camera->Move( pFramerate->speedfactor * 15, 0 );
		}
		else if( pKeyboard->keys[pPreferences->key_left] || ( pJoystick->left && pPreferences->joy_enabled ) )
		{
			pActive_Camera->Move( pFramerate->speedfactor * -15, 0 );
		}
		if( pKeyboard->keys[pPreferences->key_up] || ( pJoystick->up && pPreferences->joy_enabled ) )
		{
			pActive_Camera->Move( 0, pFramerate->speedfactor * -15 );
		}
		else if( pKeyboard->keys[pPreferences->key_down] || ( pJoystick->down && pPreferences->joy_enabled ) )
		{
			pActive_Camera->Move( 0, pFramerate->speedfactor * 15 );
		}
	}
	// default player camera
	else
	{
		pActive_Camera->Update();
	}
}

void cOverworld :: Add_Map_Object( cSprite *object )
{
	// invalid
	if( !object )
	{
		return;
	}

	// no player range
	object->player_range = 0;

	// add
	sprite_manager->Add( object );

	// Add to Waypoints array
	if( object->type == TYPE_OW_WAYPOINT )
	{
		waypoints.push_back( static_cast<cWaypoint *>(object) );
	}
}

cSprite *Get_World_Object( const CEGUI::String &element, CEGUI::XMLAttributes &attributes )
{
	if( element == "sprite" )
	{
		// old position name
		if( attributes.exists( "filename" ) )
		{
			attributes.add( "image", attributes.getValueAsString( "filename" ) );
			attributes.add( "posx", attributes.getValueAsString( "pos_x" ) );
			attributes.add( "posy", attributes.getValueAsString( "pos_y" ) );
		}

		// create sprite
		cSprite *object = new cSprite( attributes );
		// set sprite type
		object->Set_Sprite_Type( TYPE_PASSIVE );

		return object;
	}
	else if( element == "waypoint" )
	{
		return new cWaypoint( attributes );
	}
	else if( element == "sound" )
	{
		return new cRandom_Sound( attributes );
	}

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cOverworld *pActive_Overworld = NULL;
