/***************************************************************************
           savegame.cpp  -  Savegame Engine
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../globals.h"

cSavegame_data :: cSavegame_data( void )
{
	Init();
}

cSavegame_data :: ~cSavegame_data( void )
{

}

void cSavegame_data :: Init( void )
{
	levelname = "yuki_1.txt";
	description.clear();
	timestamp.clear();
	version = 0;
	lives = 0;
	points = 0;
	goldpieces = 0;
	posx = 0;
	posy = 0;
	state = 0;
	itembox_item = 0;

	Overworld_save = 0;
	Overworld_nlevel = 0;
	Overworld_slevel = 0;
	Overworld_curr_waypoint = 0;
	Overworld_world = 0;
}

cSavegame :: cSavegame( void ) : cFile_parser()
{
	error_count = 0;
	savegame_temp.Init();
	debug = 0;
}

cSavegame :: ~cSavegame( void )
{
	//
}

cSavegame_data cSavegame :: Load( unsigned int save_slot )
{
	savegame_temp.Init();
	string save_file = SAVE_DIR "/" + int_to_string( save_slot ) + ".sav";

	ifstream ifs( save_file.c_str(), ios::in );

	if( !valid_file( save_file ) )
	{
		printf( "Savegame_Load : No Savegame found at Slot : %s\n", save_file.c_str() );
		return savegame_temp;
	}

	Parse( save_file );
	
	if( debug )
	{
		Debug_Print( savegame_temp );
	}

	if( error_count )
	{
		printf( "Savegame Loading Error found at Slot : %d\n", save_slot );
		savegame_temp.description.insert( 0, "Unsupported : " );
	}

	if( !savegame_temp.description.length() )
	{
		savegame_temp.description = "No Description";
	}

	return savegame_temp;
}

int cSavegame :: Save( unsigned int save_slot, cSavegame_data savegame )
{
	string save_file = SAVE_DIR "/" + int_to_string( save_slot ) + ".sav";

	ifstream ifs( save_file.c_str(), ios::in );

	if( ifs )
	{
		ifs.close();
		ifs.open( save_file.c_str(), ios::trunc ); // Delete existing
	}
	
	ifs.close();
	
	// Finds all the " " in savegame.description and replaces them with "_"
	int pos = -1;
	
	while( savegame.description.find( " " ) >= 0 )
	{
		pos = (int)savegame.description.find( " " );
		
		if( pos >= 0 )
		{
			savegame.description.replace( pos, 1, "_" );
		}
		else
		{
			break;
		}
	}

	ofstream file( save_file.c_str(), ios::out );
	
	if( !file.is_open() )
	{
		printf( "Error : couldn't open savegame file for saving. Is the file read-only ?" );
		return 0;
	}

	// savegame info
	file << "## Savegame File at Slot " << save_slot << " with Savegame V." << SAVEGAME_VERSION << ", Game V." << VERSION << "##\n\n";
	// Time
	file << "Save_Time_Stamp " << savegame.timestamp; // in .Time_Stamp is already a \n
	// Description
	file << "Save_Description " << savegame.description << "\n";
	// Version
	file << "Save_Version " << int_to_string( savegame.version ) << "\n";
	// Level name
	file << "Level_name " << savegame.levelname << "\n";
	// Lives
	file << "Lives " << int_to_string( savegame.lives ) << "\n";
	// Points
	file << "Points " << int_to_string( savegame.points ) << "\n";
	// Goldpieces
	file << "Goldpieces " << int_to_string( savegame.goldpieces ) << "\n";
	// Player position
	file << "Position " << int_to_string( savegame.posx ) << " " << int_to_string( savegame.posy ) << "\n";
	// Player State
	file << "State " << int_to_string( savegame.state ) << "\n";

	file << "Itembox_Item " << int_to_string( savegame.itembox_item ) << "\n";

	// Overworld info
	file << "\n## OverWorld Data ##\n";
	// Overworld save or not
	file << "OWsave " << int_to_string( savegame.Overworld_save ) << "\n";
	// normal levels accessed
	file << "OWNlevel " << int_to_string( savegame.Overworld_nlevel ) << "\n";
	// secret levels accessed
	file << "OWSlevel " << int_to_string( savegame.Overworld_slevel ) << "\n";
	// Current Waypoint
	file << "OWCurr_WP " << int_to_string( savegame.Overworld_curr_waypoint ) << "\n";
	// Current Overworld
	file << "OverWorld " << int_to_string( savegame.Overworld_world ) << "\n";

	file.close();
	
	if( debug )
	{
		printf( "Saved Savegame into Slot %s\n", save_file.c_str() );
	}
	
	return 1;
}

string cSavegame :: Get_Description( unsigned int save_slot, bool only_description /* = 0 */ )
{
	string full_savefile, str_description;

	full_savefile = SAVE_DIR "/.sav";
	full_savefile.insert( strlen( SAVE_DIR ) + 1, int_to_string( save_slot ) );

	if( !valid_file( full_savefile ) )
	{
		str_description = int_to_string( save_slot ) + ". Free Save";
		return str_description;
	}
	
	cSavegame_data Temp_Savegame = Load( save_slot );

	if( Temp_Savegame.lives < 0 )
	{
		str_description = int_to_string( save_slot ) + ". Broken Save";
		return str_description;
	}

	// Finds all the "_" in Temp_Savegame.Description and replaces them with " "
	int pos = -1;
	
	while( Temp_Savegame.description.find( "_" ) >= 0 )
	{
		pos = (int)Temp_Savegame.description.find( "_" );

		if( pos >= 0 )
		{
			Temp_Savegame.description.replace( pos, 1, " " );
		}
		else
		{
			break;
		}
	}

	if( !only_description ) // Full Description
	{
		str_description = int_to_string( save_slot ) + ". " + Temp_Savegame.description;

		if( Temp_Savegame.Overworld_save )
		{
			str_description += " -  Map " + int_to_string( Temp_Savegame.Overworld_nlevel );
		}
		else
		{
			str_description += " -  Level " + Temp_Savegame.levelname;
		}

		str_description += " - Date " + Temp_Savegame.timestamp;
	}
	else // Only the User Description
	{
		return Temp_Savegame.description;
	}

	return str_description;
}

bool cSavegame :: is_valid( unsigned int save_slot )
{
	string save_file = SAVE_DIR "/" + int_to_string( save_slot ) + ".sav";
	return valid_file( save_file );
}

bool cSavegame :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	if( parts[0].compare( "Level_name" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}
		
		string filename = pPreferences->level_dir + "/" + parts[1];

		if( !valid_file( filename ) )
		{
			printf( "Savegame Error : line %d, Level not found : %s\n", line, filename.c_str() );
			return 0;
		}

		savegame_temp.levelname = parts[1];
	}
	else if( parts[0].compare( "Lifes" ) == 0 || parts[0].compare( "Lives" ) == 0  ) // Lifes for older savegame support
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d : %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) || string_to_int( parts[1] ) < 0 )
		{
			printf( "Savegame Error : line %d : Lives is not a valid number\n", line );
			return 0;
		}

		savegame_temp.lives = string_to_int( parts[1] );

	}
	else if( parts[0].compare( "Position" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d : %s needs 3 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : X Position is not a number\n", line );
			return 0;
		}
		
		if( !is_valid_number( parts[2] ) )
		{
			printf( "Savegame Error : line %d : Y Position is not a number\n", line );
			return 0;
		}

		savegame_temp.posx = string_to_int( parts[1] );
		savegame_temp.posy = string_to_int( parts[2] );
	}
	else if( parts[0].compare( "Points" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Line %d, error : %s %s\n", line, parts[0].c_str(), "needs 2 parameters" );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : Points is not a number\n", line );
			return 0;
		}

		savegame_temp.points = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "Goldpieces" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}
		
		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : Goldpieces is not a number\n", line );
			return 0;
		}

		savegame_temp.goldpieces = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "State" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : State is not a number\n", line );
			return 0;
		}

		savegame_temp.state = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "Itembox_Item" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Line %d, error : %s %s\n", line, parts[0].c_str(), "needs 2 parameters" );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : Itembox_item is not a number\n", line );
			return 0;
		}

		savegame_temp.itembox_item = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "Save_Description" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !savegame_temp.description.length() )
		{
			savegame_temp.description = parts[1];
		}

	}
	else if( parts[0].compare( "Save_Version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : Save_Version is not a number\n", line );
			return 0;
		}

		savegame_temp.version = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "Save_Time_Stamp" ) == 0 )
	{
		if( count != 6 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 6 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		string str_date;
		/* Date parts
		 * 1 = Day string
		 * 2 = Month
		 * 3 = Day number
		 * 4 = Day time
		 * 5 = Year
		*/
		str_date = parts[4] + " " + parts[3] + " " + parts[2] + " " + parts[5];

		savegame_temp.timestamp = str_date;
	}
	else if( parts[0].compare( "OWsave" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : OWsave is not a number\n", line );
			return 0;
		}

		savegame_temp.Overworld_save = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "OWNlevel" ) == 0 ) // Overworld Normal Level
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, error : %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : OWNlevel is not a number\n", line );
			return 0;
		}

		savegame_temp.Overworld_nlevel = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "OWSlevel" ) == 0 ) // Overworld Secret Level
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, error : %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : OWSlevel is not a number\n", line );
			return 0;
		}

		savegame_temp.Overworld_slevel = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "OWCurr_WP" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, error : %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : OWCurr_WP is not a number\n", line );
			return 0;
		}

		savegame_temp.Overworld_curr_waypoint = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "OverWorld" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : ", Get_filename( 0 ).c_str() );
			printf( "Savegame Error : line %d, error : %s needs 2 parameters\n", line, parts[0].c_str() );
			return 0;
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "Savegame Error : line %d : OverWorld is not a number\n", line );
			return 0;
		}

		savegame_temp.Overworld_world = string_to_int( parts[1] );
	}
	else
	{
		printf( "Savegame Error : %s : line %d, ", Get_filename( 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	return 1;
}

void cSavegame :: Debug_Print( cSavegame_data savegame )
{
	printf( "\nSavegame Debug Print : \n\nLevel_Name %s\nLifes : %d\nPos x : %d\nPos y : %d\nPoints : %d\nGoldpieces : %d\nState : %d\nOverWorld Save : %d\nOverWorld Normal Level : %d\nOverWorld Secret Level : %d\nOverWorld : %d\n",
		savegame.levelname.c_str(), savegame.lives, savegame.posx, savegame.posy, savegame.points, savegame.goldpieces, savegame.state, savegame.Overworld_save, savegame.Overworld_nlevel, savegame.Overworld_slevel, savegame.Overworld_world );
}
