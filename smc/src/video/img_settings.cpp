/***************************************************************************
			img_settings.cpp  -  Image Settings Handler
                              -------------------
    copyright            :	(C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../video/img_settings.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** cImage_settings_data *** *** *** *** *** *** *** *** *** *** *** */

cImage_settings_data :: cImage_settings_data( void )
{
	base_settings = 0;

	int_x = 0;
	int_y = 0;
	col_rect.clear();
	width = 0;
	height = 0;
	rotation_x = 0;
	rotation_y = 0;
	rotation_z = 0;
	mipmap = 0;


	type = -1;
	obsolete = 0;
}

cImage_settings_data :: ~cImage_settings_data( void )
{

}

void cImage_settings_data :: Apply( GL_Surface *image )
{
	// empty image
	if( !image )
	{
		printf( "Error : surface for base %s does not exist\n", base.c_str() );
		return;
	}

	if( !base.empty() )
	{
		if( image->image )
		{
			printf( "Warning : surface for base %s already assigned the image %d which will be replaced\n", base.c_str(), image->image );
			glDeleteTextures( 1, &image->image );
		}

		// load the base image
		string filename = Get_filename( base, 0, 1 );
		filename.insert( 0, PIXMAPS_DIR "/" );

		GL_Surface *surface_base = pVideo->Get_Surface( filename, 0 );

		// if not a valid file
		if( !valid_file( filename ) )
		{
			// try origin directory
			if( !surface_base )
			{
				filename = base;
				surface_base = pVideo->Get_Surface( filename, 0 );

				// if not valid or no image settings base
				if( !valid_file( filename ) )
				{
					// nothing found
					if( !surface_base )
					{
						printf( "Error : %s is not a valid base image file\n", filename.c_str() );
						return;
					}
				}
			}
		}

		// Couldn't load the image
		if( !surface_base )
		{
			printf( "Error : Couldn't load base image file : %s\n", base.c_str() );
			return;
		}
		// based on image settings
		else if( !valid_file( filename ) )
		{
			// todo : search for "."
			surface_base->filename.replace( surface_base->filename.length() - 4, 4, ".settings" );
		}

		// creates a new surface
		GL_Surface *surface_final = surface_base->Copy();

		if( !surface_final )
		{
			printf( "Couldn't create image copy from : %s\n", surface_base->filename.c_str() );
			return;
		}

		image->image = surface_final->image;
		image->tex_w = surface_final->tex_w;
		image->tex_h = surface_final->tex_h;

		if( base_settings )
		{
			image->int_x = surface_final->int_x;
			image->int_y = surface_final->int_y;
			image->w = surface_final->w;
			image->h = surface_final->h;
			image->base_rotx = surface_final->base_rotx;
			image->base_roty = surface_final->base_roty;
			image->base_rotz = surface_final->base_rotz;
			image->col_pos = surface_final->col_pos;
			image->col_w = surface_final->col_w;
			image->col_h = surface_final->col_h;

			// settings
			image->editor_tags = surface_final->editor_tags;
			image->name = surface_final->name;
			image->type = surface_final->type;
		}
		else
		{
			image->int_x = 0;
			image->int_y = 0;
			image->w = (float)surface_final->tex_w;
			image->h = (float)surface_final->tex_h;
			image->base_rotx = 0;
			image->base_roty = 0;
			image->base_rotz = 0;
			image->col_pos.x = 0;
			image->col_pos.y = 0;
			image->col_w = image->w;
			image->col_h = image->h;
		}

		surface_final->auto_del_img = 0;
		delete surface_final;
		surface_final = NULL;
	}


	// ## int_x/int_y
	image->int_x = (float)int_x;
	image->int_y = (float)int_y;

	// ## width
	if( width > 0 )
	{
		image->w = (float)width;
		image->col_w = image->w;
	}
	// ## height
	if( height > 0 )
	{
		image->h = (float)height;
		image->col_h = image->h;
	}

	// ## col_rect
	if( col_rect.w > 0 && col_rect.h > 0 )
	{
		// position
		image->col_pos.x = col_rect.x;
		image->col_pos.y = col_rect.y;
		// dimension
		image->col_w = col_rect.w;
		image->col_h = col_rect.h;
	}

	// ## rotation x
	if( rotation_x != 0 )
	{
		image->base_rotx = (float)rotation_x;
	}
	// ## rotation y
	if( rotation_y != 0 )
	{
		image->base_roty = (float)rotation_y;

		// mirror
		if( image->base_roty == 180 )
		{
			image->col_pos.x = image->w - ( image->col_w + image->col_pos.x );
		}
	}
	// ## rotation z
	if( rotation_z != 0 )
	{
		image->base_rotz = (float)rotation_z;

		if( image->base_rotz == 90 )
		{
			float col_w = image->col_w;
			image->col_w = image->col_h;
			image->col_h = col_w;


			image->col_pos = image->col_pos.rotate( GL_point( image->w * 0.5f, image->h * 0.5f ), image->base_rotz );
			image->col_pos.x -= image->col_w;
		}
		// mirror
		else if( image->base_rotz == 180 )
		{
			image->col_pos.y = image->h - ( image->col_h + image->col_pos.y );
		}
		else if( image->base_rotz == 270 )
		{
			float col_w = image->col_w;
			image->col_w = image->col_h;
			image->col_h = col_w;

			image->col_pos = image->col_pos.rotate( GL_point( image->w * 0.5f, image->h * 0.5f ), image->base_rotz );
			image->col_pos.y -= image->col_h;
		}
	}

	// ## editor_tags
	if( !editor_tags.empty() )
	{
		image->editor_tags = editor_tags;
	}

	// ## name
	if( !name.empty() )
	{
		image->name = name;

		// finds all "_" and replaces them with " "
		for( string::iterator itr = image->name.begin(); itr != image->name.end(); ++itr )
		{
			// change
			if( *itr == '_' )
			{
				*itr = ' ';
			}
		}
	}

	// ## type
	if( type != 0 )
	{
		image->type = type;
	}

	// ## obsolete
	if( obsolete )
	{
		image->obsolete = 1;
	}
}

void cImage_settings_data :: Apply_base( cImage_settings_data *base_settings_data )
{
	int_x = base_settings_data->int_x;
	int_y = base_settings_data->int_y;
	col_rect = base_settings_data->col_rect;
	width = base_settings_data->width;
	height = base_settings_data->height;
	rotation_x = base_settings_data->rotation_x;
	rotation_y = base_settings_data->rotation_y;
	rotation_z = base_settings_data->rotation_z;
	mipmap = base_settings_data->mipmap;
	editor_tags = base_settings_data->editor_tags;
	name = base_settings_data->name;
	type = base_settings_data->type;

	if( !obsolete && base_settings_data->obsolete )
	{
		obsolete = 1;
	}
}

/* *** *** *** *** *** *** cImage_settings *** *** *** *** *** *** *** *** *** *** *** */

cImage_settings :: cImage_settings( void )
: cFile_parser()
{
	settings = NULL;
	load_base = 1;
}

cImage_settings :: ~cImage_settings( void )
{
	//
}

cImage_settings_data *cImage_settings :: Get( string &filename, bool load_base_settings /* = 1 */ )
{
	load_base = load_base_settings;
	settings = new cImage_settings_data();

	Parse( filename );
	return settings;
}

bool cImage_settings :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	if( parts[0].compare( "base" ) == 0 )
	{
		if( count < 2 || count > 3 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2-3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		settings->base = data_file.substr( 0, data_file.rfind( "/" ) + 1 ) + parts[1];

		// with settings option
		if( count == 3 && string_to_int( parts[2] ) )
		{
			settings->base_settings = 1;

			if( load_base )
			{
				string settings_file = settings->base;

				// if not already image settings based
				if( settings_file.rfind( ".settings" ) == string::npos )
				{
					settings_file.erase( settings_file.rfind( "." ) + 1 );
					settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );
				}

				// if a settings file exists
				if( valid_file( settings_file ) )
				{
					cImage_settings *temp_parser = new cImage_settings();
					cImage_settings_data *base_settings = temp_parser->Get( settings_file );
					delete temp_parser;

					if( base_settings )
					{
						settings->Apply_base( base_settings );
						delete base_settings;
					}
				}
			}
		}
	}
	else if( parts[0].compare( "int_x" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		settings->int_x = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "int_y" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		settings->int_y = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "col_rect" ) == 0 )
	{
		if( count != 5 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		for( unsigned int i = 1; i < 5; i++ )
		{
			if( !is_valid_number( parts[i] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
		}

		// position and dimension
		settings->col_rect = GL_rect( (float)string_to_int( parts[1] ), (float)string_to_int( parts[2] ), (float)string_to_int( parts[3] ), (float)string_to_int( parts[4] ) );
	}
	else if( parts[0].compare( "width" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		settings->width = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "height" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		settings->height = string_to_int( parts[1] );
	}
	else if( parts[0].compare( "rotation" ) == 0 )
	{
		if( count < 2 || count > 5 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		// x
		settings->rotation_x = string_to_int( parts[1] );

		// y
		if( count > 2 )
		{
			if( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			settings->rotation_y = string_to_int( parts[2] );
		}
		// z
		if( count > 3 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			settings->rotation_z = string_to_int( parts[3] );
		}
	}
	else if( parts[0].compare( "mipmap" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		// if mipmaps enabled
		if( string_to_int( parts[1] ) )
		{
			settings->mipmap = 1;
		}
	}
	else if( parts[0].compare( "editor_tags" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		settings->editor_tags = parts[1];
	}
	else if( parts[0].compare( "name" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		settings->name = parts[1];
	}
	else if( parts[0].compare( "type" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		settings->type = Get_Sprite_Type_id( parts[1] );
	}
	else if( parts[0].compare( "obsolete" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		// if tagged obsolete
		if( string_to_int( parts[1] ) )
		{
			settings->obsolete = 1;
		}
	}
	else
	{
		printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cImage_settings *pSettingsParser = NULL;
