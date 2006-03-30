/***************************************************************************
			file_parser.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "globals.h"

#ifndef __FILE_PARSER_H__
#define __FILE_PARSER_H__

// Class: cFile_parser
//  Base class for parsing files.
class cFile_parser
{
public:
	cFile_parser( void );
	virtual ~cFile_parser( void );

	// Function: Get_filename
	// Returns the current filename with the given options
	string Get_filename( bool with_dir = 1, bool with_end = 1 );

	// Function: Parse
	// Parses the given file
	bool Parse( string filename );

	// Function: Parse_Line
	// Parses a line of a description file into string parts
	bool Parse_Line( string command, int line );

	// Function: HandleMessage
	// Handles the File Line
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );

	string data_file;
	unsigned int error_count;
};

#endif

