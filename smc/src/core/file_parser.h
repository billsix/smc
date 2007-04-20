/***************************************************************************
			file_parser.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FILE_PARSER_H__
#define __FILE_PARSER_H__

#include "../core/globals.h"

/* *** *** *** *** *** *** *** *** cFile_parser *** *** *** *** *** *** *** *** *** */

// Base class for parsing text files
class cFile_parser
{
public:
	cFile_parser( void );
	virtual ~cFile_parser( void );

	// Function: Parse
	// Parses the given file
	bool Parse( string &filename );

	// Function: Parse_Line
	// Parses a line of a description file into string parts
	bool Parse_Line( string command, int line );

	// Function: HandleMessage
	// Handles the File Line
	virtual bool HandleMessage( string *parts, unsigned int count, unsigned int line );

	// data filename
	string data_file;
	// data error count
	unsigned int error_count;
};

#endif

