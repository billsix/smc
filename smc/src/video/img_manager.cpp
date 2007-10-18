/***************************************************************************
			img_manager.cpp  -  Image Handler/Manager
                              -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../video/img_manager.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cSoftware_texture *** *** *** *** *** *** *** *** *** *** *** *** */

cSoftware_texture :: cSoftware_texture( void )
{
	base = NULL;
	pixels = NULL;

	width = 0;
	height = 0;

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
: cObject_Manager<GL_Surface>()
{
	high_texture_id = 0;
}

cImageManager :: ~cImageManager( void )
{
	Delete_All();
}

GL_Surface *cImageManager :: Get_Pointer( string path )
{
	for( SurfaceList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		GL_Surface *obj = (*itr);

		// return first match
		if( obj->filename.compare( path ) == 0 )
		{
			return obj;
		}
	}

	// not found
	return NULL;
}

GL_Surface *cImageManager :: Copy( string path )
{
	for( SurfaceList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get object
		GL_Surface *obj = (*itr);

		// first match
		if( obj->filename.compare( path ) == 0 )
		{
			return obj->Copy();
		}
	}

	// not found
	return NULL;
}

void cImageManager :: Grab_Textures( void )
{
	// save to software memory
	for( SurfaceList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get surface
		GL_Surface *obj = (*itr);

		// get software texture and save it
		software_textures.push_back( obj->Get_Software_Texture() );
		// delete hardware texture
		if( glIsTexture( obj->image ) )
		{
			glDeleteTextures( 1, &obj->image );
		}
		obj->image = 0;
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

void cImageManager :: Delete_Image_Textures( void )
{
	for( SurfaceList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr ) 
	{
		// get object
		GL_Surface *obj = (*itr);

		if( obj->auto_del_img && glIsTexture( obj->image ) )
		{
			glDeleteTextures( 1, &obj->image );
		}
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
	// stops GL_Surface destructor from checking if GL texture id still in use
	Delete_Image_Textures();
	cObject_Manager<GL_Surface>::Delete_All();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cImageManager *pImageManager = NULL;
