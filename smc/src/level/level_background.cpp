/***************************************************************************
 * level_background.cpp  -  level background image and color handling class
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

#include "../level/level_background.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"
#include "../core/math/utilities.h"

/* *** *** *** *** *** *** *** cBackground *** *** *** *** *** *** *** *** *** *** */

cBackground :: cBackground( void )
{
	Init();
}

cBackground :: cBackground( CEGUI::XMLAttributes &attributes )
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

	posx = 0;
	posy = 0;
	posz = 0.00011f;

	color_1 = static_cast<Uint8>(0);
	color_2 = static_cast<Uint8>(0);

	img_file_1.reserve( 120 );
	img_1 = NULL;

	speedx = 1;
	speedy = 1;
	const_velx = 0;
	const_vely = 0;
}

void cBackground :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	Set_Type( static_cast<BackgroundType>(attributes.getValueAsInteger( "type" )) );

	if( type == BG_GR_HOR || type == BG_GR_VER )
	{
		Set_Color_1( Color( static_cast<Uint8>(attributes.getValueAsInteger( "bg_color_1_red" )), attributes.getValueAsInteger( "bg_color_1_green" ), attributes.getValueAsInteger( "bg_color_1_blue" ) ) );
		Set_Color_2( Color( static_cast<Uint8>(attributes.getValueAsInteger( "bg_color_2_red" )), attributes.getValueAsInteger( "bg_color_2_green" ), attributes.getValueAsInteger( "bg_color_2_blue" ) ) );
	}
	else if( type == BG_IMG_BOTTOM || type == BG_IMG_TOP || type == BG_IMG_ALL )
	{
		Set_Pos( attributes.getValueAsFloat( "posx" ), attributes.getValueAsFloat( "posy" ) );
		Set_Pos_Z( attributes.getValueAsFloat( "posz" ) );

		Set_Image( attributes.getValueAsString( "image" ).c_str() );
		Set_Scroll_Speed( attributes.getValueAsFloat( "speedx" ), attributes.getValueAsFloat( "speedy" ) );
		Set_Const_Velocity_X( attributes.getValueAsFloat( "const_velx" ) );
		Set_Const_Velocity_Y( attributes.getValueAsFloat( "const_vely" ) );
	}
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

	// gradient
	if( type == BG_GR_HOR || type == BG_GR_VER )
	{
		// background color 1
		file << "\t\t<Property name=\"bg_color_1_red\" value=\"" << static_cast<int>(color_1.red) << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_1_green\" value=\"" << static_cast<int>(color_1.green) << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_1_blue\" value=\"" << static_cast<int>(color_1.blue) << "\" />" << std::endl;
		// background color 2
		file << "\t\t<Property name=\"bg_color_2_red\" value=\"" << static_cast<int>(color_2.red) << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_2_green\" value=\"" << static_cast<int>(color_2.green) << "\" />" << std::endl;
		file << "\t\t<Property name=\"bg_color_2_blue\" value=\"" << static_cast<int>(color_2.blue) << "\" />" << std::endl;
	}
	// image
	else if( type == BG_IMG_BOTTOM || type == BG_IMG_TOP || type == BG_IMG_ALL )
	{
		// position
		file << "\t\t<Property name=\"posx\" value=\"" << posx << "\" />" << std::endl;
		file << "\t\t<Property name=\"posy\" value=\"" << posy << "\" />" << std::endl;
		file << "\t\t<Property name=\"posz\" value=\"" << posz << "\" />" << std::endl;

		// image filename
		file << "\t\t<Property name=\"image\" value=\"" << img_file_1 << "\" />" << std::endl;
		// speed
		file << "\t\t<Property name=\"speedx\" value=\"" << speedx << "\" />" << std::endl;
		file << "\t\t<Property name=\"speedy\" value=\"" << speedy << "\" />" << std::endl;
		// constant velocity
		file << "\t\t<Property name=\"const_velx\" value=\"" << const_velx << "\" />" << std::endl;
		file << "\t\t<Property name=\"const_vely\" value=\"" << const_vely << "\" />" << std::endl;
	}
	else
	{
		printf( "Error : Unknown Background Type %d\n", type );
	}

	// end background
	file << "\t</background>" << std::endl;
}

void cBackground :: Set_Type( BackgroundType ntype )
{
	type = ntype;
}

void cBackground :: Set_Type( string ntype ) 
{
	if( ntype.compare( "Disabled" ) == 0 )
	{
		type = BG_NONE;
	}
	else if( ntype.compare( "Bottom" ) == 0 )
	{
		type = BG_IMG_BOTTOM;
	}
	else if( ntype.compare( "All" ) == 0 )
	{
		type = BG_IMG_ALL;
	}
	else
	{
		printf( "Warning : Unknown Background type %s\n", ntype.c_str() );
	}
}

void cBackground :: Set_Color_1( const Color &color )
{ 
	color_1 = color; 
}

void cBackground :: Set_Color_2( const Color &color )
{
	color_2 = color; 
}

void cBackground :: Set_Image( string nimg_file_1 )
{
	img_file_1 = nimg_file_1;

	// empty
	if( img_file_1.empty() )
	{
		img_1 = NULL;
		return;
	}

	if( img_file_1.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) != string::npos )
	{
		img_file_1.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	img_1 = pVideo->Get_Surface( img_file_1 );
}

void cBackground :: Set_Scroll_Speed( float x /* = 1 */, float y /* = 1 */ )
{
	speedx = x;
	speedy = y;
}

void cBackground :: Set_Pos( float x, float y )
{
	posx = x;
	posy = y;
}

void cBackground :: Set_Pos_Z( float val )
{
	posz = val;
}

void cBackground :: Set_Const_Velocity_X( float vel )
{
	const_velx = vel;
}

void cBackground :: Set_Const_Velocity_Y( float vel )
{
	const_vely = vel;
}

void cBackground :: Update( void )
{
	if( const_velx )
	{
		posx += const_velx;
	}

	if( const_vely )
	{
		posy += const_vely;
	}
}

void cBackground :: Draw( void )
{ 
	// gradient
	if( type == BG_GR_VER || type == BG_GR_HOR )
	{
		Draw_Gradient();
	}
	// image
	else if( type == BG_IMG_BOTTOM || type == BG_IMG_ALL ) 
	{
		// if background images are disabled or no image
		if( !pPreferences->level_background_images || !img_1 )
		{
			return;
		}

		// get position
		float posx_final = posx - ( ( pActive_Camera->x * 0.2f ) * speedx );
		float posy_final = posy - ( ( pActive_Camera->y * 0.3f ) * speedy );

		if( type == BG_IMG_BOTTOM || type == BG_IMG_ALL )
		{
			posy_final += game_res_h - img_1->h;
		}

		// align start position x
		// to right
		while( posx_final < -img_1->w )
		{
			posx_final += img_1->w;
		}
		// to left
		while( posx_final > 0 )
		{
			posx_final -= img_1->w;
		}
		// align start position y
		if( type == BG_IMG_ALL )
		{
			// to top
			while( posy_final > 0 )
			{
				posy_final -= img_1->h;
			}
			// to bottom
			while( posy_final < game_res_h - img_1->h )
			{
				posy_final += img_1->h;
			}
		}

		// draw
		while( posx_final < game_res_w )
		{
			// draw horizontal
			img_1->Blit( posx_final, posy_final, posz );

			// draw vertical
			if( type == BG_IMG_ALL )
			{
				float posy_temp = posy_final;

				while( posy_temp > -game_res_h )
				{
					img_1->Blit( posx_final, posy_temp, posz );

					posy_temp -= img_1->h;
				}
			}

			posx_final += img_1->w;
		}
	}
}

void cBackground :: Draw_Gradient( void )
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

		if( pActive_Camera->y < -1 )
		{
			float power = ( -pActive_Camera->y / 10000 );

			if( power > 1 )
			{
				power = 1;
			}

			color_start.red += static_cast<Uint8>( static_cast<float>( color_2.red - color_1.red ) * power );
			color_start.green += static_cast<Uint8>( static_cast<float>( color_2.green - color_1.green ) * power );
			color_start.blue += static_cast<Uint8>( static_cast<float>( color_2.blue - color_1.blue ) * power );
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

string cBackground :: Get_Type_Name( void )
{
	if( type == BG_NONE )
	{
		return "Disabled";
	}
	else if( type == BG_IMG_BOTTOM )
	{
		return "Bottom";
	}
	else if( type == BG_IMG_ALL )
	{
		return "All";
	}

	return "Unknown";
};

/* *** *** *** *** *** *** cBackground_Manager *** *** *** *** *** *** *** *** *** *** *** */

cBackground_Manager :: cBackground_Manager( void )
: cObject_Manager<cBackground>()
{
	//
}

cBackground_Manager :: ~cBackground_Manager( void )
{
	Delete_All();
}
