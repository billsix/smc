/***************************************************************************
				worlds.cpp  -  class for the worlds data
                             -------------------
    copyright            : (C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../overworld/worlds.h"
#include "../core/game_core.h"
// boost filesystem
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

/* *** *** *** *** *** *** *** *** cOverworld_manager *** *** *** *** *** *** *** *** *** */

cOverworld_manager :: cOverworld_manager( void )
: cObject_Manager<cOverworld>()
{
	worlds_filename = DATA_DIR "/" GAME_OVERWORLD_DIR "/worlds.xml";

	debugmode = 0;
	cameramode = 0;

	Load();
}

cOverworld_manager :: ~cOverworld_manager( void )
{
	Delete_All();
}

bool cOverworld_manager :: New( string name )
{
	// no name given
	if( name.empty() )
	{
		return 0;
	}

	// already available
	if( fs::exists( fs::path( user_data_dir + USER_WORLD_DIR + "/" + name, fs::native ) ) )
	{
		return 0;
	}

	cOverworld *overworld = new cOverworld();

	// set path
	overworld->description->path = name;
	// default name is the path
	overworld->description->name = name;

	objects.push_back( overworld );

	pActive_Overworld = overworld;

	pOverworld_Player->Reset();

	return 1;
}

void cOverworld_manager :: Load( void )
{
	// if already loaded
	if( objects.size() )
	{
		Delete_All();
	}

	// Load Worlds
	Load_dir( DATA_DIR "/" GAME_OVERWORLD_DIR );
	Load_dir( user_data_dir + USER_WORLD_DIR );

	// Get Overworld User Comments
	if( valid_file( worlds_filename ) )
	{
		// Parse
		System::getSingleton().getXMLParser()->parseXMLFile( *this, worlds_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Worlds_User_Data.xsd", "" );
	}
	else
	{
		// filename not valid
		printf( "Warning : Couldn't open Worlds description file : %s\n", worlds_filename.c_str() );
	}

	// set default overworld if not set
	if( !pActive_Overworld )
	{
		Set_Active( "World 1" );
	}
}

void cOverworld_manager :: Load_dir( string dir )
{
	// set world directory
	fs::path full_path( dir, fs::native );
	fs::directory_iterator end_iter;

	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			// only directories with an existing description
			if( fs::is_directory( *dir_itr ) && valid_file( dir + "/" + dir_itr->leaf() + "/description.xml" ) )
			{
				cOverworld *overworld = new cOverworld();

				// set path
				overworld->description->path = dir_itr->leaf();
				// default name is the path
				overworld->description->name = dir_itr->leaf();

				objects.push_back( overworld );

				overworld->Load();
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->leaf().c_str(), ex.what() );
		}
	}
}

bool cOverworld_manager :: Set_Active( string name ) 
{
	cOverworld *world = Get_from_Name( name );

	// pre 0.99.6 : try path
	if( !world )
	{
		world = Get( name );
	}

	return Set_Active( world );
}

bool cOverworld_manager :: Set_Active( cOverworld *world )
{
	if( !world )
	{
		return 0;
	}

	pActive_Overworld = world;

	// set player start waypoint
	pOverworld_Player->Reset();
	pOverworld_Player->Set_Waypoint( pActive_Overworld->player_start_waypoint, 1 );

	return 1;
}

void cOverworld_manager :: Reset( void )
{
	// default Overworld
	Set_Active( "World 1" );
	
	// Set Player to first Waypoint
	pOverworld_Player->Set_Waypoint( pActive_Overworld->player_start_waypoint );

	// Reset all Waypoints
	for( OverworldList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Reset_Waypoints();
	}
}

cOverworld *cOverworld_manager :: Get( string path )
{
	for( OverworldList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cOverworld *obj = (*itr);

		if( obj->description->path.compare( path ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

cOverworld *cOverworld_manager :: Get_from_Name( string name )
{
	for( OverworldList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		cOverworld *obj = (*itr);

		if( obj->description->name.compare( name ) == 0 )
		{
			return obj;
		}
	}

	return NULL;
}

int cOverworld_manager :: Get_Array_num( string path )
{
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( objects[i]->description->path.compare( path ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

// XML element start
void cOverworld_manager :: elementStart( const String &element, const XMLAttributes &attributes )
{
	// Property of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
    }
}

// XML element end
void cOverworld_manager :: elementEnd( const String &element )
{
	if( element != "Property" )
	{
		if( element == "World" )
		{
			handle_world( xml_attributes );
		}
		else if( element == "Worlds" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Overworld Description Unknown element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = XMLAttributes();
	}
}

void cOverworld_manager :: handle_world( const XMLAttributes &attributes )
{
	string ow_name = attributes.getValueAsString( "Name" ).c_str();
	string ow_comment = attributes.getValueAsString( "Comment" ).c_str();

	// if available
	cOverworld *overworld = Get_from_Name( ow_name );

	// set comment
	if( overworld )
	{
		overworld->description->comment = ow_comment;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld information handler
cOverworld_manager *pOverworld_manager = NULL;
