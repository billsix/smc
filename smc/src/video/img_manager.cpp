/***************************************************************************
			img_manager.cpp  -  Image Handler/Manager
                              -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
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

cImageManager :: cImageManager( void )
{
	load_count = 0;
}

cImageManager :: ~cImageManager( void )
{
	DeleteAll();
}

GL_Surface *cImageManager :: Get_Pointer( string path )
{
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i].item->filename.compare( path ) == 0 )
		{
			return items[i].item;	// the first found
		}
	}

	return NULL; // not found
}

GL_Surface *cImageManager :: Get_Pointer( unsigned int identifier )
{
	if( load_count > identifier ) 
	{
		for( unsigned int i = 0; i < items.size(); i++ )
		{
			if( items[i].count_id == identifier ) 
			{
				return items[i].item;
			}
		}
	}

	return NULL; // not found
}

GL_Surface *cImageManager :: Get_Pointer_array( unsigned int identifier )
{
	if( Get_Size() > identifier && items[identifier].item )
	{
		return items[identifier].item;
	}

	return NULL; // not found
}

GL_Surface *cImageManager :: Copy( string path )
{
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i].item->filename.compare( path ) == 0 )	// The first match
		{
			return items[i].item->Copy();
		}
	}

	return NULL; // not found
}

string cImageManager :: Get_Path( unsigned int identifier )
{
	if( load_count >= identifier ) 
	{
		for( unsigned int i = 0; i < items.size(); i++ )
		{
			if( items[i].count_id == identifier ) 
			{
				return items[i].item->filename;
			}
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

	ImageItem nitem;
	nitem.item = image;
	nitem.count_id = load_count;

	items.push_back( nitem );
}

/*void cImageManager :: ReloadImages( unsigned int step )
{
	if( step == 1 ) // only delete
	{
		DeleteImages();
		return;
	}

	if( step == 2 ) // Loads the images
	{
		for( unsigned int i = 0; i < items.size(); i++ )
		{
			if( !items[i].item && items[i].path.length() > 4 ) 
			{
				items[i].item = LoadImage( items[i].path );

				if( !items[i].item  )
				{
					printf( "Error loading file : %s\n", items[i].path.c_str() );
				}
			}
		}

		return;
	}

	if( !step ) // Complete Reload
	{
		ReloadImages( 1 );
		ReloadImages( 2 );
	}
}*/

void cImageManager :: DeleteImages( void )
{
	for( unsigned int i = 0; i < items.size(); i++ )
	{
		if( items[i].item ) 
		{
			glDeleteTextures( 1, &items[i].item->image );
			delete items[i].item;
			items[i].item = NULL;
		}
	}
}

void cImageManager :: DeleteAll( void )
{
	DeleteImages();
	items.clear();
}
