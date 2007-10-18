/***************************************************************************
           savegame.cpp  -  Savegame Engine
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

#include "../core/globals.h"
#include "../user/savegame.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../core/obj_manager.h"
#include "../level/level.h"
#include "../overworld/worlds.h"
#include "../objects/bonusbox.h"
#include "../objects/goldpiece.h"

/* *** *** *** *** *** *** *** cSave_Overworld_Waypoint *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld_Waypoint :: cSave_Overworld_Waypoint( void )
{
	access = 0;
}

cSave_Overworld_Waypoint :: ~cSave_Overworld_Waypoint( void )
{
	
}

/* *** *** *** *** *** *** *** cSave_Overworld *** *** *** *** *** *** *** *** *** *** */

cSave_Overworld :: cSave_Overworld( void )
{
	
}

cSave_Overworld :: ~cSave_Overworld( void )
{
	for( unsigned int i = 0; i < Waypoints.size(); i++ )
	{
		delete Waypoints[i];
	}

	Waypoints.clear();
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object_Property :: cSave_Level_Object_Property( string nName /* = "" */, string nValue /* = "" */ )
{
	Name = nName;
	Value = nValue;
}

/* *** *** *** *** *** *** *** cSave_Level_Object *** *** *** *** *** *** *** *** *** *** */

cSave_Level_Object :: cSave_Level_Object( void )
{
	
}

cSave_Level_Object :: ~cSave_Level_Object( void )
{
	properties.clear();
}

string cSave_Level_Object :: Get_Value( string val_name )
{
	for( unsigned int i = 0; i < properties.size(); i++ )
	{
		if( properties[i].Name.compare( val_name ) == 0 )
		{
			// found
			return properties[i].Value;
		}
	}

	// not found
	return "";
}

/* *** *** *** *** *** *** *** cSave *** *** *** *** *** *** *** *** *** *** */

cSave :: cSave( void )
{
	Clear();
}

cSave :: ~cSave( void )
{
	Clear();
}

void cSave :: Clear( void )
{
	// save
	description.clear();
	save_time = 0;
	version = 0;

	// player
	lives = 0;
	points = 0;
	goldpieces = 0;
	player_state = 0;
	itembox_item = 0;
	
	// level
	level_name.clear();
	level_posx = 0;
	level_posy = 0;

	for( unsigned int i = 0; i < level_objects.size(); i++ )
	{
		delete level_objects[i];
	}

	level_objects.clear();

	// overworld
	overworld_active.clear();
	overworld_current_waypoint = 0;

	for( unsigned int i = 0; i < Overworlds.size(); i++ )
	{
		delete Overworlds[i];
	}

	Overworlds.clear();
}

cSavegame :: cSavegame( void )
{
	error_count = 0;
	save_temp = NULL;
	debug = 0;

	savegame_dir = user_data_dir + USER_SAVEGAME_DIR;
}

cSavegame :: ~cSavegame( void )
{
	//
}

int cSavegame :: Load_Game( unsigned int save_slot )
{
	cSave *savegame = Load( save_slot );
	
	// check if unsupported save
	if( savegame->version <= SAVEGAME_VERSION_UNSUPPORTED )
	{
		printf( "Warning : Savegame %d : Versions %d and below are unsupported\n", save_slot, SAVEGAME_VERSION_UNSUPPORTED );
	}

	// invalid lives value
	if( savegame->lives < 0 )
	{
		printf( "Warning : Savegame %d : Lives was below 0\n", save_slot );
		savegame->lives = 0;
	}

	// level available
	if( savegame->level_name.length() )
	{
		string level_name = savegame->level_name;

		// level not found
		if( !pLevel->Get_path( level_name ) )
		{
			printf( "Warning : Savegame %d : Level not found : %s\n", save_slot, level_name.c_str() );
		}
	}

	// unload level
	pLevel->Unload();
	// reset player
	pPlayer->ResetSave();

	// #### Overworld ####

	// if no overworld is set
	if( savegame->Overworlds.size() == 0 )
	{
		// Use first Overworld
		cSave_Overworld *save_overworld = new cSave_Overworld();
		save_overworld->name = pOverworld_manager->Get_Pointer( 0 )->description->path;

		savegame->Overworlds.push_back( save_overworld );
	}
	// set overworld progress
	else
	{
		for( Save_OverworldList::iterator itr = savegame->Overworlds.begin(), itr_end = savegame->Overworlds.end(); itr != itr_end; ++itr )
		{
			// get savegame overworld pointer
			cSave_Overworld *save_overworld = (*itr);

			// get overworld
			cOverworld *overworld = pOverworld_manager->Get_from_Name( save_overworld->name );

			if( !overworld )
			{
				printf( "Warning : Savegame %d : Overworld %s not found\n", save_slot, save_overworld->name.c_str() );
				continue;
			}

			for( Save_Overworld_WaypointList::iterator wp_itr = save_overworld->Waypoints.begin(), wp_itr_end = save_overworld->Waypoints.end(); wp_itr != wp_itr_end; ++wp_itr )
			{
				// get savegame waypoint pointer
				cSave_Overworld_Waypoint *save_waypoint = (*wp_itr);

				// get overworld waypoint
				cWaypoint *waypoint = overworld->Get_Waypoint( overworld->Get_Waypoint_num( save_waypoint->destination ) );

				// not found
				if( !waypoint )
				{
					printf( "Warning : Savegame %d : Overworld %s Waypoint %s not found\n", save_slot, save_overworld->name.c_str(), save_waypoint->destination.c_str() );
					continue;
				}

				// set access
				waypoint->Set_Access( save_waypoint->access );
			}
		}
	}

	// Set Active Overworld
	if( !pOverworld_manager->Set_Active( pOverworld_manager->Get_from_Name( savegame->overworld_active ) ) )
	{
		printf( "Warning : Savegame %d : Couldn't set Overworld active %s\n", save_slot, savegame->overworld_active.c_str() );
	}

	// Current Waypoint
	if( !pOverworld_Player->Set_Waypoint( savegame->overworld_current_waypoint ) )
	{
		printf( "Warning : Savegame %d : Overworld Current Waypoint %d is invalid\n", save_slot, savegame->overworld_current_waypoint );
	}


	// #### Player ####

	pPlayer->Set_Type( (Maryo_type)savegame->player_state, 0, 0 );

	// in a level
	if( !savegame->level_name.empty() )
	{
		// load level
		if( pLevel->Load( savegame->level_name ) )
		{
			// position
			pPlayer->Set_Pos( savegame->level_posx, savegame->level_posy - (float)GAME_RES_H );

			// Level Objects
			for( Save_Level_ObjectList::iterator itr = savegame->level_objects.begin(), itr_end = savegame->level_objects.end(); itr != itr_end; ++itr )
			{
				// get object pointer
				cSave_Level_Object *save_object = (*itr);

				// get position
				int posx = string_to_int( save_object->Get_Value( "posx" ) );
				int posy = string_to_int( save_object->Get_Value( "posy" ) );

				// get level object
				cSprite *level_object = pSprite_Manager->Get_from_Position( posx, posy, save_object->type );

				// if not anymore available
				if( !level_object )
				{
					printf( "Warning : Savegame object type %d on x %d, y %d not available\n", save_object->type, posx, posy );
					continue;
				}

				if( level_object->type == TYPE_BONUSBOX )
				{
					cBonusBox *bonusbox = static_cast<cBonusBox *>(level_object);

					// useable count
					int useable_count = string_to_int( save_object->Get_Value( "useable_count" ) );
					bonusbox->Set_UseableCount( useable_count );
				}
				else if( level_object->type == TYPE_GOLDPIECE )
				{
					cGoldpiece *goldpiece = static_cast<cGoldpiece *>(level_object);

					// visible
					bool visible = string_to_int( save_object->Get_Value( "visible" ) ) > 0;
					goldpiece->Set_Visible( visible );
				}
				else if( level_object->type == TYPE_MOON )
				{
					cMoon *moon = static_cast<cMoon *>(level_object);

					// visible
					bool visible = string_to_int( save_object->Get_Value( "visible" ) ) > 0;
					moon->Set_Visible( visible );
				}
			}

			// invincible for a second
			pPlayer->invincible = DESIRED_FPS;
		}
		else
		{
			printf( "Error : Couldn't load Savegame Level %s\n", savegame->level_name.c_str() );
		}
	}
	
	pointsdisplay->Set_Points( savegame->points );
	golddisplay->SetGold( savegame->goldpieces );
	livedisplay->SetLives( savegame->lives );
	Itembox->Set_Item( (SpriteType)savegame->itembox_item, 0 );

	pCamera->Center();

	pHudManager->Update();

	debugdisplay->Set_Text( "Savegame " + int_to_string( save_slot ) + " loaded" );

	// default is level save
	int retval = 1;

	// if Overworld Save
	if( savegame->level_name.empty() )
	{
		retval = 2;
	}

	delete savegame;
	return retval;
}

bool cSavegame :: Save_Game( unsigned int save_slot, string description )
{
	if( pPlayer->maryo_type == MARYO_DEAD || pPlayer->lives < 0 )
	{
		printf( "Error : Couldn't save savegame %s because of invalid game state\n", description.c_str() );
		return 0;
	}

	cSave *savegame = new cSave();

	// Description
	savegame->description = description;
	// Goldpieces
	savegame->goldpieces = pPlayer->goldpieces;

	// Level
	if( pLevel->is_Loaded() )
	{
		// name
		savegame->level_name = Get_filename( pLevel->data_file, 0, 0 );

		// position
		savegame->level_posx = pPlayer->posx;
		savegame->level_posy = pPlayer->posy + GAME_RES_H - 5;

		// Level Objects
		for( SpriteList::iterator itr = pSprite_Manager->objects.begin(), itr_end = pSprite_Manager->objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSprite *object = (*itr);
			// savegame object;
			cSave_Level_Object *save_object = NULL;

			// Bonusbox
			if( object->type == TYPE_BONUSBOX )
			{
				cBonusBox *bonusbox = static_cast<cBonusBox *>(object);

				// only save if needed
				if( bonusbox->useable_count == bonusbox->start_useable_count )
				{
					continue;
				}

				save_object = new cSave_Level_Object();

				// Bonusbox Useable Count
				save_object->properties.push_back( cSave_Level_Object_Property( "useable_count", int_to_string( bonusbox->useable_count ) ) );
			}
			// Goldpiece
			else if( object->type == TYPE_GOLDPIECE )
			{
				cGoldpiece *goldpiece = static_cast<cGoldpiece *>(object);

				// only save if needed
				if( goldpiece->visible == 1 )
				{
					continue;
				}

				save_object = new cSave_Level_Object();

				// Goldpiece visible
				save_object->properties.push_back( cSave_Level_Object_Property( "visible", int_to_string( goldpiece->visible ) ) );
			}
			// Moon
			else if( object->type == TYPE_MOON )
			{
				cMoon *moon = static_cast<cMoon *>(object);

				// only save if needed
				if( moon->visible == 1 )
				{
					continue;
				}

				save_object = new cSave_Level_Object();

				// Moon visible
				save_object->properties.push_back( cSave_Level_Object_Property( "visible", int_to_string( moon->visible ) ) );
			}
			// other object
			else
			{
				continue;
			}

			// save default values
			save_object->type = object->type;
			save_object->properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( (int)object->startposx ) ) );
			save_object->properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( (int)object->startposy ) ) );

			// add
			savegame->level_objects.push_back( save_object );
		}
	}

	// Lives
	savegame->lives = pPlayer->lives;
	// Points
	savegame->points = pPlayer->points;

	// Player State
	savegame->player_state = pPlayer->maryo_type;
	// Itembox Item
	savegame->itembox_item = Itembox->item_id;

	// save overworld progress
	for( OverworldList::iterator itr = pOverworld_manager->objects.begin(), itr_end = pOverworld_manager->objects.end(); itr != itr_end; ++itr )
	{
		// Get Overworld
		cOverworld *overworld = (*itr);

		// create Overworld
		cSave_Overworld *save_overworld = new cSave_Overworld();
		// name
		save_overworld->name = overworld->description->name;
		
		// Waypoints
		for( SpriteList::iterator wp_itr = overworld->sprite_manager->objects.begin(), wp_itr_end = overworld->sprite_manager->objects.end(); wp_itr != wp_itr_end; ++wp_itr )
		{
			// get waypoint
			cSprite *obj = static_cast<cSprite *>(*wp_itr);

			if( obj->type != TYPE_OW_WAYPOINT )
			{
				continue;
			}

			// get waypoint
			cWaypoint *waypoint = static_cast<cWaypoint *>(obj);

			// create savegame waypoint
			cSave_Overworld_Waypoint *save_waypoint = new cSave_Overworld_Waypoint();
			
			// destination
			save_waypoint->destination = waypoint->Get_Destination();
			// set access
			save_waypoint->access = waypoint->access;
			// save
			save_overworld->Waypoints.push_back( save_waypoint );
		}

		// save
		savegame->Overworlds.push_back( save_overworld );
	}

	// if an overworld active
	if( pActive_Overworld )
	{
		// which OverWorld
		savegame->overworld_active = pActive_Overworld->description->name;

		// if valid waypoint
		if( pOverworld_Player->current_waypoint >= 0 )
		{
			// Overworld current Waypoint
			savegame->overworld_current_waypoint = pOverworld_Player->current_waypoint;
		}
	}

	// Time ( seconds since 1970 )
	savegame->save_time = time( NULL );
	// Version
	savegame->version = SAVEGAME_VERSION;

	// Save it
	Save( save_slot, savegame );

	// Print
	if( debugdisplay )
	{
		debugdisplay->Set_Text( "Saved to Slot " + int_to_string( save_slot ) );
	}

	// Clear
	delete savegame;

	return 1;
}

cSave *cSavegame :: Load( unsigned int save_slot )
{
	save_temp = new cSave();

	string filename = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !file_exists( filename ) )
	{
		printf( "Error : Savegame Loading : No Savegame found at Slot : %s\n", filename.c_str() );
		return save_temp;
	}

	try
	{
		System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Savegame.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( Exception &ex )
	{
		printf( "Savegame Loading CEGUI Exception %s\n", ex.getMessage().c_str() );
		debugdisplay->Set_Text( "Savegame Loading failed : " + (string)ex.getMessage().c_str() );
	}

	// unknown savegame data found
	if( error_count )
	{
		printf( "Warning : Savegame Loading : Errors found at Slot : %d\n", save_slot );
		save_temp->description.insert( 0, "Unsupported : " );
	}

	// if no description is available
	if( !save_temp->description.length() )
	{
		save_temp->description = "No Description";
	}

	cSave *savegame =  save_temp;
	save_temp = NULL;
	return savegame;
}

int cSavegame :: Save( unsigned int save_slot, cSave *savegame )
{
	string filename = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( file_exists( filename ) )
	{
		ifstream ifs( filename.c_str(), ios::trunc ); // Delete existing
		ifs.close();
	}

	// empty overworld active
	if( savegame->overworld_active.empty() )
	{
		printf( "Warning : Savegame %s saving : Empty Overworld Active\n", savegame->description.c_str() );
	}

	ofstream file( filename.c_str(), ios::out );
	
	if( !file.is_open() )
	{
		printf( "Error : Couldn't open savegame file for saving. Is the file read-only ?" );
		return 0;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin Savegame
	file << "<Savegame>" << std::endl;

	// begin Information
	file << "\t<Information>" << std::endl;
		// Description
		file << "\t\t<Property Name=\"description\" Value=\"" << savegame->description << "\" />" << std::endl;
		// Version
		file << "\t\t<Property Name=\"version\" Value=\"" << savegame->version << "\" />" << std::endl;
		// Time
		file << "\t\t<Property Name=\"save_time\" Value=\"" << savegame->save_time << "\" />" << std::endl;
	// end Information
	file << "\t</Information>" << std::endl;

	if( !savegame->level_name.empty() )
	{
		// begin Level
		file << "\t<Level>" << std::endl;

		// Level name
		file << "\t\t<Property Name=\"level_name\" Value=\"" << savegame->level_name << "\" />" << std::endl;
		// Level position
		file << "\t\t<Property Name=\"player_posx\" Value=\"" << savegame->level_posx << "\" />" << std::endl;
		file << "\t\t<Property Name=\"player_posy\" Value=\"" << savegame->level_posy << "\" />" << std::endl;

		// Level Objects
		for( Save_Level_ObjectList::iterator itr = savegame->level_objects.begin(), itr_end = savegame->level_objects.end(); itr != itr_end; ++itr )
		{
			// get object pointer
			cSave_Level_Object *object = (*itr);

			// begin Level Object
			file << "\t\t<Level_Object>" << std::endl;

			// Object type
			file << "\t\t\t<Property Name=\"type\" Value=\"" << object->type << "\" />" << std::endl;

			// Properties
			for( Save_Level_Object_ProprtyList::iterator prop_itr = object->properties.begin(), prop_itr_end = object->properties.end(); prop_itr != prop_itr_end; ++prop_itr )
			{
				// get properties pointer
				cSave_Level_Object_Property Property = (*prop_itr);

				// property
				file << "\t\t\t<Property Name=\"" << Property.Name <<"\" Value=\"" << Property.Value << "\" />" << std::endl;
			}

			// end Level Object
			file << "\t\t</Level_Object>" << std::endl;
		}
		// end Level
		file << "\t</Level>" << std::endl;
	}

	// begin Player
	file << "\t<Player>" << std::endl;
		// Lives
		file << "\t\t<Property Name=\"lives\" Value=\"" << savegame->lives << "\" />" << std::endl;
		// Points
		file << "\t\t<Property Name=\"points\" Value=\"" << savegame->points << "\" />" << std::endl;
		// Goldpieces
		file << "\t\t<Property Name=\"goldpieces\" Value=\"" << savegame->goldpieces << "\" />" << std::endl;
		// state
		file << "\t\t<Property Name=\"state\" Value=\"" << savegame->player_state << "\" />" << std::endl;
		// Itembox item
		file << "\t\t<Property Name=\"itembox_item\" Value=\"" << savegame->itembox_item << "\" />" << std::endl;
	// end Player
	file << "\t</Player>" << std::endl;

	// begin Overworld_Data
	file << "\t<Overworld_Data>" << std::endl;
		// active Overworld
		file << "\t\t<Property Name=\"active\" Value=\"" << savegame->overworld_active << "\" />" << std::endl;
		// current Overworld Waypoint
		file << "\t\t<Property Name=\"current_waypoint\" Value=\"" << savegame->overworld_current_waypoint << "\" />" << std::endl;
	// end Overworld_Data
	file << "\t</Overworld_Data>" << std::endl;

	// Overworlds
	for( Save_OverworldList::iterator itr = savegame->Overworlds.begin(), itr_end = savegame->Overworlds.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cSave_Overworld *overworld = (*itr);

		// begin Overworld
		file << "\t<Overworld>" << std::endl;

		// current Overworld
		file << "\t\t<Property Name=\"name\" Value=\"" << overworld->name << "\" />" << std::endl;

		for( Save_Overworld_WaypointList::iterator wp_itr = overworld->Waypoints.begin(), wp_itr_end = overworld->Waypoints.end(); wp_itr != wp_itr_end; ++wp_itr )
		{
			// get object pointer
			cSave_Overworld_Waypoint *overworld_waypoint = (*wp_itr);

			// skip empty waypoints
			if( overworld_waypoint->destination.empty() )
			{
				continue;
			}

			// begin Overworld Level
			file << "\t\t<Overworld_Level>" << std::endl;

			// destination
			file << "\t\t\t<Property Name=\"destination\" Value=\"" << overworld_waypoint->destination << "\" />" << std::endl;
			// access
			file << "\t\t\t<Property Name=\"access\" Value=\"" << overworld_waypoint->access << "\" />" << std::endl;

			// end Overworld Level
			file << "\t\t</Overworld_Level>" << std::endl;
		}

		// end Overworld
		file << "\t</Overworld>" << std::endl;
	}

	// end Savegame
	file << "</Savegame>" << std::endl;

	file.close();
	
	if( debug )
	{
		printf( "Saved Savegame %s to slot %d\n", filename.c_str(), save_slot );
	}
	
	return 1;
}

string cSavegame :: Get_Description( unsigned int save_slot, bool only_description /* = 0 */ )
{
	string savefile, str_description;

	savefile = savegame_dir + "/" + int_to_string( save_slot ) + ".save";

	if( !file_exists( savefile ) )
	{
		str_description = int_to_string( save_slot ) + ". Free Save";
		return str_description;
	}
	
	cSave *temp_savegame = Load( save_slot );

	if( temp_savegame->lives < 0 )
	{
		str_description = int_to_string( save_slot ) + ". Broken Save";
		return str_description;
	}

	// complete description
	if( !only_description )
	{
		str_description = int_to_string( save_slot ) + ". " + temp_savegame->description;

		if( temp_savegame->level_name.empty() )
		{
			str_description += " -  Map " + temp_savegame->overworld_active;
		}
		else
		{
			str_description += " -  Level " + temp_savegame->level_name;
		}

		str_description += " - Date " + Time_to_String( temp_savegame->save_time, "%Y-%m-%d  %H:%M:%S" );
	}
	// only the user description
	else
	{
		return temp_savegame->description;
	}

	delete temp_savegame;
	return str_description;
}

bool cSavegame :: is_valid( unsigned int save_slot )
{
	return file_exists( savegame_dir + "/" + int_to_string( save_slot ) + ".save" );
}

// XML element start
void cSavegame :: elementStart( const String &element, const XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "Name" ), attributes.getValueAsString( "Value" ) );
    }
}

// XML element end
void cSavegame :: elementEnd( const String &element )
{
	if( element != "Property" )
	{
		if( element == "Information" )
		{
			save_temp->description = xml_attributes.getValueAsString( "description" ).c_str();
			save_temp->version = xml_attributes.getValueAsInteger( "version" );
			save_temp->save_time = xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "Level" )
		{
			Handle_Level( xml_attributes );
		}
		else if( element == "Level_Object" )
		{
			Handle_Level_Object( xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Player" )
		{
			Handle_Player( xml_attributes );
		}
		else if( element == "Overworld_Data" )
		{
			Handle_Overworld_Data( xml_attributes );
		}
		else if( element == "Overworld" )
		{
			Handle_Overworld( xml_attributes );
		}
		else if( element == "Overworld_Level" )
		{
			Handle_Overworld_Level( xml_attributes );
			// don't clear attributes
			return;
		}
		else if( element == "Savegame" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Savegame Unknown Element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = XMLAttributes();
	}
}

void cSavegame :: Handle_Level( const XMLAttributes &attributes )
{
	save_temp->level_name = xml_attributes.getValueAsString( "level_name" ).c_str();
	save_temp->level_posx = xml_attributes.getValueAsFloat( "player_posx" );
	save_temp->level_posy = xml_attributes.getValueAsFloat( "player_posy" );

	// set level objects
	save_temp->level_objects.swap( level_objects );
	level_objects.clear();
}

void cSavegame :: Handle_Level_Object( const XMLAttributes &attributes )
{
	int type = xml_attributes.getValueAsInteger( "type" );

	if( type <= 0 )
	{
		printf( "Warning : Unknown Savegame Level Object type %d\n", type );
		return;
	}

	cSave_Level_Object *object = new cSave_Level_Object();

	// type
	object->type = (SpriteType)type;

	// position
	object->properties.push_back( cSave_Level_Object_Property( "posx", xml_attributes.getValueAsString( "posx" ).c_str() ) );
	object->properties.push_back( cSave_Level_Object_Property( "posy", xml_attributes.getValueAsString( "posy" ).c_str() ) );

	if( type == TYPE_BONUSBOX )
	{
		// useable count
		object->properties.push_back( cSave_Level_Object_Property( "useable_count", xml_attributes.getValueAsString( "useable_count" ).c_str() ) );

		// clear
		xml_attributes.remove( "useable_count" );
	}
	else if( type == TYPE_GOLDPIECE || type == TYPE_MOON )
	{
		// visible
		object->properties.push_back( cSave_Level_Object_Property( "visible", xml_attributes.getValueAsString( "visible" ).c_str() ) );

		// clear
		xml_attributes.remove( "visible" );
	}

	// add object
	level_objects.push_back( object );

	// clear
	xml_attributes.remove( "type" );
	xml_attributes.remove( "posx" );
	xml_attributes.remove( "posy" );
}

void cSavegame :: Handle_Player( const XMLAttributes &attributes )
{
	save_temp->lives = xml_attributes.getValueAsInteger( "lives" );
	save_temp->points = xml_attributes.getValueAsInteger( "points" );
	save_temp->goldpieces = xml_attributes.getValueAsInteger( "goldpieces" );
	save_temp->player_state = xml_attributes.getValueAsInteger( "state" );
	save_temp->itembox_item = xml_attributes.getValueAsInteger( "itembox_item" );
}

void cSavegame :: Handle_Overworld_Data( const XMLAttributes &attributes )
{
	save_temp->overworld_active = xml_attributes.getValueAsString( "active" ).c_str();
	save_temp->overworld_current_waypoint = xml_attributes.getValueAsInteger( "current_waypoint" );
}

void cSavegame :: Handle_Overworld( const XMLAttributes &attributes )
{
	string name = xml_attributes.getValueAsString( "name" ).c_str();

	// Search if Overworld is available
	cOverworld *overworld = pOverworld_manager->Get_from_Name( name );

	if( !overworld )
	{
		printf( "Warning : Savegame %s Overworld %s not found\n", save_temp->description.c_str(), name.c_str() );
	}

	// Create Savegame Overworld
	cSave_Overworld *save_overworld = new cSave_Overworld();
	// set name
	save_overworld->name = name;
	// set waypoints
	save_overworld->Waypoints.swap( active_waypoints );
	active_waypoints.clear();
	// save
	save_temp->Overworlds.push_back( save_overworld );
}

void cSavegame :: Handle_Overworld_Level( const XMLAttributes &attributes )
{
	bool access = xml_attributes.getValueAsBool( "access" );

	cSave_Overworld_Waypoint *waypoint = new cSave_Overworld_Waypoint();

	// destination ( level_name and world_name is pre 0.99.6 )
	if( xml_attributes.exists( "world_name" ) )
	{
		waypoint->destination = xml_attributes.getValueAsString( "world_name" ).c_str();
	}
	else if( xml_attributes.exists( "level_name" ) )
	{
		waypoint->destination = xml_attributes.getValueAsString( "level_name" ).c_str();
	}
	// default
	else
	{
		waypoint->destination = xml_attributes.getValueAsString( "destination" ).c_str();
	}

	waypoint->access = access;

	active_waypoints.push_back( waypoint );

	// clear
	xml_attributes.remove( "level_name" );
	xml_attributes.remove( "world_name" );
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSavegame *pSavegame = NULL;
