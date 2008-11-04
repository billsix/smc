/***************************************************************************
 * gl_surface.cpp  -  GL_Surface class
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../video/gl_surface.h"
#include "../video/video.h"
#include "../video/renderer.h"
#include "../video/img_manager.h"
#include "../objects/sprite.h"

/* *** *** *** *** *** *** *** *** GL_Surface *** *** *** *** *** *** *** *** *** */

GL_Surface :: GL_Surface( void )
{
	image = 0;

	int_x = 0;
	int_y = 0;
	start_w = 0;
	start_h = 0;
	w = 0;
	h = 0;
	tex_w = 0;
	tex_h = 0;

	// internal rotation data
	base_rotx = 0;
	base_roty = 0;
	base_rotz = 0;
	
	// collision data
	col_pos.x = 0;
	col_pos.y = 0;
	col_w = 0;
	col_h = 0;

	auto_del_img = 1;
	managed = 0;
	obsolete = 0;

	// default type is passive
	type = TYPE_PASSIVE;

	ground_type = GROUND_NORMAL;

	destruction_function = NULL;
}

GL_Surface :: ~GL_Surface( void )
{
	// don't delete a managed OpenGL image if still in use by another managed GL_Surface
	if( auto_del_img && glIsTexture( image ) && ( !managed || !Is_Texture_Use_Multiple() ) )
	{
		glDeleteTextures( 1, &image );
	}

	if( destruction_function )
	{
		destruction_function( this );
	}
}

GL_Surface *GL_Surface :: Copy( void )
{
	// create copy image
	GL_Surface *new_surface = new GL_Surface();

	// data
	new_surface->image = image;
	new_surface->int_x = int_x;
	new_surface->int_y = int_y;
	new_surface->start_w = start_w;
	new_surface->start_h = start_h;
	new_surface->w = w;
	new_surface->h = h;
	new_surface->tex_h = tex_h;
	new_surface->tex_w = tex_w;
	new_surface->base_rotx = base_rotx;
	new_surface->base_roty = base_roty;
	new_surface->base_rotz = base_rotz;
	new_surface->col_pos = col_pos;
	new_surface->col_w = col_w;
	new_surface->col_h = col_h;
	new_surface->filename = filename;

	// settings
	new_surface->obsolete = obsolete;
	new_surface->editor_tags = editor_tags;
	new_surface->name = name;
	new_surface->type = type;
	new_surface->Set_Ground_Type( ground_type );

	return new_surface;
}

void GL_Surface :: Blit( float x, float y, float z, cSurfaceRequest *request /* = NULL */ )
{
	bool create_request = 0;

	if( !request )
	{
		create_request = 1;
		// create request
		request = new cSurfaceRequest();
	}

	Blit_Data( request );

	// position
	request->pos_x += x;
	request->pos_y += y;
	request->pos_z = z;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

void GL_Surface :: Blit_Data( cSurfaceRequest *request )
{
	// texture id
	request->texture_id = image;

	// position
	request->pos_x += int_x;
	request->pos_y += int_y;

	// size
	request->w = start_w;
	request->h = start_h;
	
	// rotation
	request->rotx += base_rotx;
	request->roty += base_roty;
	request->rotz += base_rotz;
}

void GL_Surface :: Save( string filename )
{
	if( !image )
	{
		printf( "Couldn't save GL_Surface : No Image Texture ID set\n" );
		return;
	}

	// bind the texture
	glBindTexture( GL_TEXTURE_2D, image );

	// create image data
	GLubyte *data = new GLubyte[tex_w * tex_h * 4];
	// read texture
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLvoid *>(data) );
	// save
	pVideo->Save_Surface( filename, data, tex_w, tex_h );
	// clear data
	delete[] data;
}

void GL_Surface :: Set_Ground_Type( GroundType gtype )
{
	ground_type = gtype;
}

bool GL_Surface :: Is_Texture_Use_Multiple( void )
{
	for( GL_Surface_List::iterator itr = pImage_Manager->objects.begin(), itr_end = pImage_Manager->objects.end(); itr != itr_end; ++itr )
	{
		GL_Surface *obj = (*itr);

		if( obj == this )
		{
			continue;
		}
		
		if( obj->image == image )
		{
			return 1;
		}
	}

	return 0;
}

cSaved_Texture *GL_Surface :: Get_Software_Texture( bool only_filename /* = 0 */ )
{
	cSaved_Texture *soft_tex = new cSaved_Texture();

	// hardware texture to software texture
	if( !only_filename )
	{
		// bind the texture
		glBindTexture( GL_TEXTURE_2D, image );

		// texture settings
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &soft_tex->width );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &soft_tex->height );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &soft_tex->format );

		glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &soft_tex->wrap_s );
		glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &soft_tex->wrap_t );
		glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &soft_tex->min_filter );
		glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &soft_tex->mag_filter );

		unsigned int bpp;

		if( soft_tex->format == GL_RGBA )
		{
			bpp = 4;
		}
		else if( soft_tex->format == GL_RGB )
		{
			bpp = 3;
		}
		else
		{
			bpp = 4;
			printf( "Warning: GL_Surface :: Get_Software_Texture : Unknown format\n" );
		}

		// texture data
		soft_tex->pixels = new GLubyte[soft_tex->width * soft_tex->height * bpp];

		glGetTexImage( GL_TEXTURE_2D, 0, soft_tex->format, GL_UNSIGNED_BYTE, soft_tex->pixels );
	}

	// surface pointer
	soft_tex->base = this;

	return soft_tex;
}

void GL_Surface :: Load_Software_Texture( cSaved_Texture *soft_tex )
{
	if( !soft_tex )
	{
		return;
	}
	
	// software texture
	if( soft_tex->pixels )
	{
		GLuint tex_id;
		glGenTextures( 1, &tex_id );

		glBindTexture( GL_TEXTURE_2D, tex_id );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, soft_tex->wrap_s );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, soft_tex->wrap_t );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, soft_tex->min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, soft_tex->mag_filter );

		// check if mipmaps are enabled
		bool mipmaps = 0;

		// if mipmaps are enabled
		if( soft_tex->min_filter == GL_LINEAR_MIPMAP_LINEAR )
		{
			mipmaps = 1;
		}

		// Create Hardware Texture
		pVideo->Create_GL_Texture( soft_tex->width, soft_tex->height, soft_tex->pixels, mipmaps );

		image = tex_id;
	}
	// load from file
	else
	{
		GL_Surface *surface_copy = pVideo->Load_GL_Surface( filename );

		if( !surface_copy )
		{
			printf( "Warning: GL_Surface :: Load_Software_Texture %s loading failed\n", filename.c_str() );
			return;
		}

		// get image
		image = surface_copy->image;
		tex_w = surface_copy->tex_w;
		tex_h = surface_copy->tex_h;
		// keep hardware texture
		surface_copy->auto_del_img = 0;
		// delete copy
		delete surface_copy;
	}
}

string GL_Surface :: Get_Filename( int with_dir /* = 2 */, bool with_end /* = 1 */ ) 
{
	string name = filename;

	// erase whole directory
	if( with_dir == 0 && name.rfind( "/" ) != string::npos ) 
	{
		name.erase( 0, name.rfind( "/" ) + 1 );
	}
	// erase pixmaps directory
	else if( with_dir == 1 && name.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) != string::npos ) 
	{
		name.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	// erase file type
	if( !with_end && name.rfind( "." ) != string::npos ) 
	{
		name.erase( name.rfind( "." ) );
	}

	return name;
}

void GL_Surface :: Set_Destruction_Function( void ( *nfunction )( GL_Surface * ) )
{
	destruction_function = nfunction;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
