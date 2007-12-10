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
	ground_type = GROUND_NORMAL;
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

	// ## int_x/int_y
	image->int_x = static_cast<float>(int_x);
	image->int_y = static_cast<float>(int_y);

	// ## width
	if( width > 0 )
	{
		image->start_w = static_cast<float>(width);
		image->w = image->start_w;
		image->col_w = image->w;
	}
	// ## height
	if( height > 0 )
	{
		image->start_h = static_cast<float>(height);
		image->h = image->start_h;
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
		image->base_rotx = static_cast<float>(rotation_x);

		//image->col_pos = image->col_pos.rotate3d( image->base_rotx, 1, 0, 0 );
	}
	// ## rotation y
	if( rotation_y != 0 )
	{
		image->base_roty = static_cast<float>(rotation_y);

		//image->col_pos = image->col_pos.rotate3d( image->base_roty, 0, 1, 0 );
		// mirror
		if( image->base_roty == 180 )
		{
			image->col_pos.x = image->w - ( image->col_w + image->col_pos.x );
		}
	}
	// ## rotation z
	if( rotation_z != 0 )
	{
		image->base_rotz = static_cast<float>(rotation_z);

		//image->col_pos = image->col_pos.rotate3d( image->base_rotz, 0, 0, 1 );

		if( image->base_rotz == 90 )
		{
			// rotate position
			GL_point pos( image->int_x, image->int_y );
			pos = pos.rotate( GL_point( image->w * 0.5f, image->h * 0.5f ), image->base_rotz );
			image->int_x = pos.y;
			image->int_y = pos.x - image->h;
			// rotate collision position
			float orig_x = image->col_pos.x;
			image->col_pos.x = image->h - ( image->col_h + image->col_pos.y );
			image->col_pos.y = orig_x;

			// switch width and height
			float orig_w = image->w;
			image->w = image->h;
			image->h = orig_w;
			// switch collision width and height
			float col_w = image->col_w;
			image->col_w = image->col_h;
			image->col_h = col_w;
		}
		// mirror
		else if( image->base_rotz == 180 )
		{
			image->col_pos.y = image->h - ( image->col_h + image->col_pos.y );
		}
		else if( image->base_rotz == 270 )
		{
			// rotate position
			GL_point pos( image->int_x, image->int_y );
			pos = pos.rotate( GL_point( image->w * 0.5f, image->h * 0.5f ), image->base_rotz );
			image->int_x = pos.y - image->w;
			image->int_y = pos.x;
			// rotate collision position
			float orig_x = image->col_pos.x;
			image->col_pos.x = image->col_pos.y;
			image->col_pos.y = orig_x;

			// switch width and height
			float orig_w = image->w;
			image->w = image->h;
			image->h = orig_w;
			// switch collision width and height
			float col_w = image->col_w;
			image->col_w = image->col_h;
			image->col_h = col_w;
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
	if( type > 0 )
	{
		image->type = type;
	}

	// ## ground type
	image->ground_type = ground_type;

	// ## obsolete
	image->obsolete = obsolete;
}

void cImage_settings_data :: Apply_base( cImage_settings_data *base_settings_data )
{
	if( !base_settings_data->base.empty() )
	{
		base = base_settings_data->base;
		base_settings = base_settings_data->base_settings;
	}

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

	// only set if this isn't obsolete
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

				// if settings file exists
				while( !settings_file.empty() )
				{
					// if not already image settings based
					if( settings_file.rfind( ".settings" ) == string::npos )
					{
						settings_file.erase( settings_file.rfind( "." ) + 1 );
						settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );
					}

					// not found
					if( !file_exists( settings_file ) )
					{
						break;
					}

					// create new temporary parser
					cImage_settings *temp_parser = new cImage_settings();
					cImage_settings_data *base_settings = temp_parser->Get( settings_file );
					// finished loading base settings
					delete temp_parser;
					settings_file.clear();

					// handle
					if( base_settings )
					{
						// todo : apply settings in reverse order ( deepest settings should override first )
						settings->Apply_base( base_settings );
						
						// if also based on settings
						if( !base_settings->base.empty() && base_settings->base_settings )
						{
							settings_file = base_settings->base;
						}
						
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
		settings->col_rect = GL_rect( static_cast<float>(string_to_int( parts[1] )), static_cast<float>(string_to_int( parts[2] )), static_cast<float>(string_to_int( parts[3] )), static_cast<float>(string_to_int( parts[4] )) );
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
	else if( parts[0].compare( "ground_type" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		settings->ground_type = Get_Groundtype_id( parts[1] );
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
