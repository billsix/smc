/***************************************************************************
			file_parser.cpp  -  Image Settings Handler
                              -------------------
    copyright                 :	(C) 2005 - 2007 by Florian Richter
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
#include "../core/file_parser.h"

cFile_parser :: cFile_parser( void )
{
	error_count = 0;
}

cFile_parser :: ~cFile_parser( void )
{
	//
}

bool cFile_parser :: Parse( string &filename )
{
	error_count = 0;

	ifstream ifs( filename.c_str(), ios::in );
	
	if( !ifs )
	{
		printf( "Could not load data file : %s\n", filename.c_str() );
		return 0;
	}

	data_file = filename;

	char contents[500]; // maximum length of a line
	
	for( unsigned int i = 0; ifs.getline( contents, sizeof( contents ) ); i++ )
	{
		if( !Parse_Line( contents, i ) )
		{
			error_count++;
		}
	}

	return 1;
}

bool cFile_parser :: Parse_Line( string command, int line )
{
	if( command.length() <= 5 || command.find_first_of( '#' ) == 0 )
	{
		return 1; // no error
	}

	while( command.find( '\r' ) != string::npos ) // Linux support
	{
		command.erase( command.find( '\r' ), 1 );
	}

	while( command.find( '\t' ) != string::npos ) // No Tabs
	{
		command.replace( command.find( '\t' ), 1, " " );
	}

	while( command.find_last_of( ' ' ) == command.length() - 1  ) // No Spaces at the end
	{
		command.erase( command.find_last_of( ' ' ), 1 );
	}

	while( command.find_first_of( ' ' ) == 0  ) // No Spaces at the beginning
	{
		command.erase( command.find_first_of( ' ' ), 1 );
	}

	string tempstr = command;
	int count = 1;

	while( tempstr.find( ' ' ) != string::npos  ) // Count Spaces
	{
		tempstr.erase( tempstr.find( ' ' ) , 1 );
		count++;
	}

	tempstr = command;
	
	string *parts = new string[ count + 1 ];
	
	int len;
	int i = 0;

	while( count > 0 )
	{
		len = static_cast<int>(tempstr.find_first_of( ' ' ));
		parts[i] = tempstr.substr( 0, len );
		tempstr.erase( 0, len + 1 );
		i++;
		count--;
	}

	parts[i] = tempstr;

	// Message handler
	bool success = HandleMessage( parts, i, line );

	delete []parts;

	return success;
}

bool cFile_parser :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	// virtual
	return 1;
}
