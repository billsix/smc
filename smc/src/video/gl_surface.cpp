/***************************************************************************
			gl_surface.cpp  -  GL_Surface class
                             -------------------
    copyright            : (C) 2005 - 2007 Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

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
	obsolete = 0;

	// default type is passive
	type = TYPE_PASSIVE;

	destruction_function = NULL;
}

GL_Surface :: ~GL_Surface( void )
{
	if( auto_del_img && glIsTexture( image ) )
	{
		glDeleteTextures( 1, &image );
	}

	if( destruction_function )
	{
		destruction_function( this );
	}
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

	// texture id
	request->texture_id = image;

	// position
	request->pos_x = x + int_x;
	request->pos_y = y + int_y;
	request->pos_z = z;

	// size
	request->w = w;
	request->h = h;
	
	// rotation
	request->rotx += base_rotx;
	request->roty += base_roty;
	request->rotz += base_rotz;

	if( create_request )
	{
		// add request
		pRenderer->Add( request );
	}
}

GL_Surface *GL_Surface :: Copy( void )
{
	GL_Surface *new_surface = NULL;

	// based on image settings
	if( filename.rfind( ".settings" ) != string::npos )
	{
		new_surface = pVideo->Load_Surface( filename, 1 );
	}
	// default image
	else
	{
		new_surface = pVideo->Load_Surface( filename, 0 );
	}

	if( !new_surface )
	{
		printf( "Error : Couldn't load copy Texture\n" );
		return NULL;
	}

	if( !new_surface->image )
	{
		printf( "Error : Couldn't get GL image\n" );
		delete new_surface;
		return NULL;
	}

	new_surface->int_x = int_x;
	new_surface->int_y = int_y;
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

	// settings
	new_surface->editor_tags = editor_tags;
	new_surface->name = name;
	new_surface->type = type;

	return new_surface;
}

cSoftware_texture *GL_Surface :: Get_Software_Texture( void )
{
	cSoftware_texture *soft_tex = new cSoftware_texture();

	// bind the texture
	glBindTexture( GL_TEXTURE_2D, image );

	// texture settings
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &soft_tex->width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &soft_tex->height );

	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &soft_tex->wrap_s );
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &soft_tex->wrap_t );
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &soft_tex->min_filter );
	glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &soft_tex->mag_filter );

	// texture data
	soft_tex->pixels = new uint8[soft_tex->width * soft_tex->height * 4];

	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, soft_tex->pixels );

	// surface pointer
	soft_tex->base = this;

	return soft_tex;
}

void GL_Surface :: Load_Software_Texture( cSoftware_texture *soft_tex )
{
	if( !soft_tex )
	{
		return;
	}
	
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
	pVideo->Create_Texture( soft_tex->width, soft_tex->height, soft_tex->pixels, mipmaps );

	image = tex_id;
}

string GL_Surface :: Get_filename( int with_dir /*= 2*/, bool with_end /*= 1 */ ) 
{
	string name = filename;

	// erase whole directory
	if( with_dir == 0 && name.rfind( "/" ) != string::npos ) 
	{
		name.erase( 0, name.rfind( "/" ) + 1 );
	}
	// erase pixmaps directory
	else if( with_dir == 1 && name.find( PIXMAPS_DIR ) != string::npos ) 
	{
		name.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
	}

	// erase file type
	if( !with_end && name.rfind( "." ) != string::npos ) 
	{
		name.erase( name.rfind( "." ) );
	}

	return name;
}

void GL_Surface :: Set_destruction_function( void ( *nfunction )( GL_Surface * ) )
{
	destruction_function = nfunction;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
