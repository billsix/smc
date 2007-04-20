/***************************************************************************
			img_manager.cpp  -  Image Handler/Manager
                              -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../video/img_manager.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cSoftware_texture *** *** *** *** *** *** *** *** *** *** *** *** */

cSoftware_texture :: cSoftware_texture( void )
{
	base = NULL;
	pixels = NULL;

	width = 0;
	height = 0;
	border = 0;

	min_filter = 0;
	mag_filter = 0;
	wrap_s = 0;
	wrap_t = 0;
}

cSoftware_texture :: ~cSoftware_texture( void )
{
	if( pixels )
	{
		delete[] pixels;
	}
}

/* *** *** *** *** *** *** cImageManager *** *** *** *** *** *** *** *** *** *** *** */

cImageManager :: cImageManager( void )
{
	load_count = 0;
	high_texture_id = 0;
}

cImageManager :: ~cImageManager( void )
{
	Delete_All();
}

GL_Surface *cImageManager :: Get_Pointer( string path )
{
	for( ImageList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		image_item &obj = (*itr);

		// return first match
		if( obj.item->filename.compare( path ) == 0 )
		{
			return obj.item;
		}
	}

	// not found
	return NULL;
}

GL_Surface *cImageManager :: Get_Pointer( unsigned int identifier )
{
	if( identifier >= load_count ) 
	{
		return NULL;
	}

	for( ImageList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		image_item &obj = (*itr);

		if( obj.count_id == identifier ) 
		{
			return obj.item;
		}
	}

	return NULL; // not found
}

GL_Surface *cImageManager :: Get_Pointer_array( unsigned int identifier )
{
	if( identifier >= items.size() )
	{
		return NULL; // not found
	}

	return items[identifier].item;
}

GL_Surface *cImageManager :: Copy( string path )
{
	for( ImageList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get object
		image_item &obj = (*itr);

		if( obj.item->filename.compare( path ) == 0 )	// The first match
		{
			return obj.item->Copy();
		}
	}

	return NULL; // not found
}

string cImageManager :: Get_Path( unsigned int identifier )
{
	if( identifier >= load_count ) 
	{
		return "";
	}

	for( ImageList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get object
		image_item &obj = (*itr);

		if( obj.count_id == identifier ) 
		{
			return obj.item->filename;
		}
	}

	return ""; // not found
}

unsigned int cImageManager :: Get_Size( void )
{
	return (unsigned int)items.size();
}

void cImageManager :: Add( GL_Surface *image )
{
	load_count++;

	image_item nitem;
	nitem.item = image;
	nitem.count_id = load_count;

	items.push_back( nitem );
}

void cImageManager :: Grab_Textures( void )
{
	// save to software memory
	for( ImageList::iterator itr = items.begin(), itr_end = items.end(); itr != itr_end; ++itr )
	{
		// get software texture and save it
		software_textures.push_back( itr->item->Get_Software_Texture() );
		// delete hardware texture
		if( glIsTexture( itr->item->image ) )
		{
			glDeleteTextures( 1, &itr->item->image );
		}
		itr->item->image = 0;
	}
}

void cImageManager :: Restore_Textures( void )
{
	// load back into hardware textures
	for( SoftwareTextureList::iterator itr = software_textures.begin(), itr_end = software_textures.end(); itr != itr_end; ++itr )
	{
		// get saved texture
		cSoftware_texture *soft_tex = (*itr);

		// load it
		soft_tex->base->Load_Software_Texture( soft_tex );
		// delete
		delete soft_tex;
	}

	software_textures.clear();
}

void cImageManager :: Delete_Images( void )
{
	for( ImageList::iterator itr = items.begin(); itr != items.end(); ++itr ) 
	{
		// get object
		image_item &obj = (*itr);

		glDeleteTextures( 1, &obj.item->image );
		delete obj.item;
		obj.item = NULL;
	}
}

void cImageManager :: Delete_Hardware_Textures( void )
{
	// delete all hardware surfaces
	for( unsigned int i = 0; i < high_texture_id; i++ )
	{
		if( glIsTexture( i ) )
		{
			printf( "ImageManager : deleting texture %d\n", i );
			glDeleteTextures( 1, &i );
		}
	}

	high_texture_id = 0;
}

void cImageManager :: Delete_All( void )
{
	Delete_Images();
	items.clear();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cImageManager *pImageManager = NULL;
