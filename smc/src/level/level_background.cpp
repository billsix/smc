/***************************************************************************
    level_background.cpp  -  class for handling level background images and colors
                             -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../level/level_background.h"
#include "../user/preferences.h"
#include "../core/camera.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** cBackground *** *** *** *** *** *** *** *** *** *** */

cBackground :: cBackground( void )
{
	Init();
}

cBackground :: cBackground( XMLAttributes &attributes )
{
	Init();
	Create_from_Stream( attributes );
}

cBackground :: ~cBackground( void )
{
	//
}

void cBackground :: Init( void )
{
	type = BG_NONE;

	color_1 = (Uint8)0;
	color_2 = (Uint8)0;

	img_file_1.reserve( 120 );
	img_1 = NULL;

	posx = 0;
	posy = 0;
	posz = 0.00011f;

	speedx = 1;
	speedy = 1;
}

void cBackground :: Create_from_Stream( XMLAttributes &attributes )
{
	Set_image( attributes.getValueAsString( "image" ).c_str() );
	Set_type( (BackgroundType)attributes.getValueAsInteger( "type" ) );

	Set_scroll_speed( attributes.getValueAsFloat( "speedx" ), attributes.getValueAsFloat( "speedy" ) );
	Set_PosZ( attributes.getValueAsFloat( "posz" ) );
}

void cBackground :: Save_to_Stream( ofstream &file )
{
	if( type == BG_NONE && img_file_1.length() <= 3 )
	{
		return;
	}

	// begin background
	file << "\t<background>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"" << type << "\" />" << std::endl;

	if( type == BG_GR_HOR || type == BG_GR_VER )
	{
		// background color 1
		file << "\t\t<Property name=\"bg_color_1_red\" value=\"" << (int)color_1.red << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_1_green\" value=\"" << (int)color_1.green << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_1_blue\" value=\"" << (int)color_1.blue << "\" />" << std::endl;
		// background color 2
		file << "\t\t<Property name=\"bg_color_2_red\" value=\"" << (int)color_2.red << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_2_green\" value=\"" << (int)color_2.green << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_2_blue\" value=\"" << (int)color_2.blue << "\" />" << std::endl;
	}
	else if( type == BG_IMG_LEFTRIGHT || type == BG_IMG_TOP || type == BG_IMG_ALL )
	{
		// image filename
		file << "\t\t<Property name=\"image\" value=\"" << img_file_1 << "\" />" << std::endl;
		// speed
		file << "\t\t<Property name=\"speedx\" value=\"" << speedx << "\" />" << std::endl;
		file << "\t\t<Property name=\"speedy\" value=\"" << speedy << "\" />" << std::endl;
		// position z
		file << "\t\t<Property name=\"posz\" value=\"" << posz << "\" />" << std::endl;
	}
	else
	{
		printf( "Error : Unknown Background Type %d\n", type );
	}

	// end background
	file << "\t</background>" << std::endl;
}

void cBackground :: Set_type( BackgroundType ntype )
{
	type = ntype;
}

void cBackground :: Set_type( string ntype ) 
{
	if( ntype.compare( "Disabled" ) == 0 )
	{
		type = BG_NONE;
	}
	else if( ntype.compare( "Left and Right" ) == 0 )
	{
		type = BG_IMG_LEFTRIGHT;
	}
	else
	{
		printf( "Warning : Unknown Background type %s\n", ntype.c_str() );
	}
}

void cBackground :: Set_color_1( const Color &color )
{ 
	color_1 = color; 
}

void cBackground :: Set_color_2( const Color &color )
{
	color_2 = color; 
}

void cBackground :: Set_image( string nimg_file_1 )
{
	img_file_1 = nimg_file_1;

	if( img_file_1.length() > 3 )
	{
		if( img_file_1.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) != string::npos )
		{
			img_file_1.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
		}

		img_1 = pVideo->Get_Surface( img_file_1 );
	}
	else
	{
		img_1 = NULL;
	}
}

void cBackground :: Set_scroll_speed( float x /* = 1 */, float y /* = 1 */ )
{
	speedx = x;
	speedy = y;
}

void cBackground :: Set_PosZ( float val )
{
	posz = val;
}

void cBackground :: Draw( void )
{ 
	// gradient
	if( type == BG_GR_VER || type == BG_GR_HOR )
	{
		DrawGradient();
	}
	// image
	else if( type == BG_IMG_LEFTRIGHT && img_1 ) 
	{
		if( !pPreferences->background_images )
		{
			return;
		}

		// get position
		float xpos = -( ( pCamera->x / 5 ) * speedx );

		while( xpos < -img_1->w )
		{
			xpos += img_1->w;
		}
		
		while( xpos < GAME_RES_W )
		{
			posx = xpos;
			posy = ( GAME_RES_H - img_1->h ) - ( ( pCamera->y / 3 ) * speedy );

			img_1->Blit( posx, posy, posz );

			xpos += img_1->w;
		}
	}
}

void cBackground :: DrawGradient( void )
{
	pVideo->Clear_Screen();

	// no need to draw a gradient if both colors are the same
	if( color_1 == color_2 )
	{
		pVideo->Draw_Rect( NULL, posz, &color_1 );
	}
	else
	{
		Color color, color_start;

		// set color start
		color_start = color_1;

		if( pCamera->y < -1 )
		{
			float power = ( -pCamera->y / 10000 );

			if( power > 1 )
			{
				power = 1;
			}

			color_start.red += (Uint8)( (float)( color_2.red - color_1.red ) * power );
			color_start.green += (Uint8)( (float)( color_2.green - color_1.green ) * power );
			color_start.blue += (Uint8)( (float)( color_2.blue - color_1.blue ) * power );
		}

		if( type == BG_GR_VER )
		{
			pVideo->Draw_Gradient( NULL, posz, &color_start, &color_2, DIR_VERTICAL );
		}
		else if( type == BG_GR_HOR )
		{
			pVideo->Draw_Gradient( NULL, posz, &color_start, &color_2, DIR_HORIZONTAL );
		}
	}
}

string cBackground :: Get_Typename( void )
{
	if( type == BG_NONE )
	{
		return "Disabled";
	}
	else if( type == BG_IMG_LEFTRIGHT )
	{
		return "Left and Right";
	}
	else
	{
		return "Unknown";
	}

	return "";
};
