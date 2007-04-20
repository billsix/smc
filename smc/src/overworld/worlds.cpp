/***************************************************************************
				worlds.cpp  -  class for the worlds data
                             -------------------
    copyright            : (C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 
#include "../overworld/worlds.h"
#include "../core/game_core.h"
// boost filesystem
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

/* *** *** *** *** *** *** *** *** cOverworld_manager *** *** *** *** *** *** *** *** *** */

cOverworld_manager :: cOverworld_manager( void )
{
	worlds_filename = OVERWORLD_DIR "/worlds.xml";

	debugmode = 0;
	cameramode = 0;

	Load();
}

cOverworld_manager :: ~cOverworld_manager( void )
{
	Unload();
}

void cOverworld_manager :: Load( void )
{
	// if already loaded
	if( worlds.size() )
	{
		Unload();
	}

	// get all world directories
	fs::path full_path( OVERWORLD_DIR );
	fs::directory_iterator end_iter;

	for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr )
	{
		try
		{
			// only directories with an existing description
			if( fs::is_directory( *dir_itr ) && valid_file( OVERWORLD_DIR "/" + dir_itr->leaf() + "/description.xml" ) )
			{
				cOverworld *overworld = new cOverworld();

				// set path
				overworld->description->path = dir_itr->leaf();
				// default name is the path
				overworld->description->name = dir_itr->leaf();

				worlds.push_back( overworld );
			}
		}
		catch( const std::exception &ex )
		{
			printf( "%s %s\n", dir_itr->leaf().c_str(), ex.what() );
		}
	}

	// ## Get Overworld User Comments
	// filename not valid
	if( !valid_file( worlds_filename ) )
	{
		printf( "Warning : Couldn't open Worlds description file : %s\n", worlds_filename.c_str() );
	}
	else
	{
		// Parse Overworlds
		System::getSingleton().getXMLParser()->parseXMLFile( *this, worlds_filename.c_str(), SCHEMA_DIR "/Worlds_User_Data.xsd", "" );
	}

	// Load Overworlds
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		worlds[i]->Load();
	}

	// first overworld is default
	pActive_Overworld = Get( 0 );
}

void cOverworld_manager :: Unload( void )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		delete worlds[i];
	}
	
	worlds.clear();
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

	if( pActive_Overworld->entered == 0 )
	{
		// exit previous overworld
		pActive_Overworld->entered = 1;
	}

	pActive_Overworld = world;
	pOverworld_Player->current_waypoint = -2;

	return 1;
}

void cOverworld_manager :: Reset( void )
{
	// default Overworld
	Set_Active( "world_1" );
	
	// Set Player to first Waypoint
	pOverworld_Player->Set_Waypoint( pActive_Overworld->player_start_waypoint );

	// Reset all Waypoints
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		worlds[i]->Reset_Waypoints();
	}
}

cOverworld *cOverworld_manager :: Get( string path )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		if( worlds[i]->description->path.compare( path ) == 0 )
		{
			return worlds[i];
		}
	}

	return NULL;
}

cOverworld *cOverworld_manager :: Get_from_Name( string name )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		if( worlds[i]->description->name.compare( name ) == 0 )
		{
			return worlds[i];
		}
	}

	return NULL;
}

cOverworld *cOverworld_manager :: Get( unsigned int num )
{
	if( num >= worlds.size() )
	{
		return NULL;
	}

	return worlds[num];
}

int cOverworld_manager :: Get_array_num( string path )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		if( worlds[i]->description->path.compare( path ) == 0 )
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
