/***************************************************************************
			img_settings.cpp  -  Image Settings Handler
                              -------------------
    copyright            :	(C) 2005 - 2006 by FluXy
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

cImage_settings :: cImage_settings( void ) : cFile_parser()
{
	surface = NULL;
}

cImage_settings :: ~cImage_settings( void )
{
	//
}

void cImage_settings :: ApplyTextureSettings( GL_Surface *image, string filename )
{
	surface = image;

	Parse( filename );
}

bool cImage_settings :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	if( parts[0].compare( "base" ) == 0 )
	{
		if( count < 2 || count > 3 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2-3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !surface )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s surface does not exist\n", parts[0].c_str() );
			return 0; // error
		}

		bool orig_settings = 0;

		if( count == 3 && ( string_to_int( parts[2] ) == 1 ) ) // with settings option
		{
			orig_settings = 1;
		}

		// load the base image
		string filename = parts[1];
		filename.insert( 0, "/" );
		filename.insert( 0, PIXMAPS_DIR );

		if( !valid_file( filename ) )
		{
			string origin_dir = Get_filename( 1, 0 );
			origin_dir.erase( origin_dir.rfind( "/" ) + 1, origin_dir.length() - origin_dir.rfind( "/" ) + 1 );
			origin_dir.insert( origin_dir.length(), parts[1] );

			if( !valid_file( origin_dir ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid image file\n", parts[1].c_str() );
				return 0; // error
			}

			filename = origin_dir;
		}
		else
		{
			filename.insert( 0, "/" );
			filename.insert( 0, PIXMAPS_DIR );
		}

		GL_Surface *surface_base = GetImage( filename );

		if( !surface_base )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "base image file : %s\n", parts[1].c_str() );
			return 0; // error
		}

		// creates a new surface
		GL_Surface *surface_final = surface_base->Copy();

		if( !surface_final )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "Couldn't create image copy from : %s\n", surface_base->filename.c_str() );
			return 0; // error
		}

		surface->image = surface_final->image;
		surface->tex_w = surface_final->tex_w;
		surface->tex_h = surface_final->tex_h;

		if( orig_settings )
		{
			surface->int_x = surface_final->int_x;
			surface->int_y = surface_final->int_y;
			surface->w = surface_final->w;
			surface->h = surface_final->h;
			surface->base_rotx = surface_final->base_rotx;
			surface->base_roty = surface_final->base_roty;
			surface->base_rotz = surface_final->base_rotz;
			surface->col_pos = surface_final->col_pos;
			surface->col_w = surface_final->col_w;
			surface->col_h = surface_final->col_h;
		}
		else
		{
			surface->int_x = 0;
			surface->int_y = 0;
			surface->w = (double)surface_final->tex_w;
			surface->h = (double)surface_final->tex_h;
			surface->base_rotx = 0;
			surface->base_roty = 0;
			surface->base_rotz = 0;
			surface->col_pos.x = 0;
			surface->col_pos.y = 0;
			surface->col_w = surface->w;
			surface->col_h = surface->h;
		}

		surface_final->auto_del_img = 0;
		delete surface_final;
		surface_final = NULL;
	}
	else if( parts[0].compare( "int_x" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( surface )
		{
			surface->int_x = string_to_int( parts[1] );
		}
	}
	else if( parts[0].compare( "int_y" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( surface )
		{
			surface->int_y = string_to_int( parts[1] );
		}
	}
	else if( parts[0].compare( "col_rect" ) == 0 )
	{
		if( count != 5 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		for( unsigned int i = 1; i < 5; i++ )
		{
			if( !is_valid_number( parts[i] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
		}


		if( surface )
		{
			// collision start
			surface->col_pos.x = string_to_int( parts[1] );
			surface->col_pos.y = string_to_int( parts[2] );
			// collision width
			surface->col_w = string_to_int( parts[3] );
			surface->col_h = string_to_int( parts[4] );
		}
	}
	else if( parts[0].compare( "width" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( surface )
		{
			surface->w = string_to_int( parts[1] );
			surface->col_w = surface->w;
		}
		
	}
	else if( parts[0].compare( "height" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( surface )
		{
			surface->h = string_to_int( parts[2] );
			surface->col_h = surface->h;
		}
	}
	else if( parts[0].compare( "rotation" ) == 0 )
	{
		if( count < 2 || count > 5 )
		{
			printf( "%s : line %d Error :\n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( surface )
		{
			surface->base_rotx = string_to_int( parts[1] );

			if( count > 2 ) // y
			{
				if( !is_valid_number( parts[2] ) )
				{
					printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[2].c_str() );
					return 0; // error
				}

				surface->base_roty = string_to_int( parts[2] );
			}
			if( count > 3 ) // z
			{
				if( !is_valid_number( parts[3] ) )
				{
					printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[3].c_str() );
					return 0; // error
				}

				surface->base_rotz = string_to_int( parts[3] );
			}
		}
	}
	else
	{
		printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	return 1;
}
