/***************************************************************************
    global_effect.cpp  -  class for handling level global effects
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "../level/global_effect.h"
#include "../core/framerate.h"
#include "../level/level_editor.h"
#include "../core/camera.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cGlobal_effect *** *** *** *** *** *** *** *** *** *** *** *** */

cGlobal_effect :: cGlobal_effect( void )
: cParticleAnimation()
{
	Init_default();
}

cGlobal_effect :: cGlobal_effect( XMLAttributes &attributes )
: cParticleAnimation()
{
	Init_default();
	Create_from_Stream( attributes );
}

cGlobal_effect :: ~cGlobal_effect( void )
{

}

void cGlobal_effect :: Init_default( void )
{
	Clear();
}

void cGlobal_effect :: Init( void )
{
	if( ge_type == GL_EFF_NONE )
	{
		return;
	}

	if( ge_type == GL_EFF_FALLING || ge_type == GL_EFF_FLYING )
	{
		// if the image doesn't exists
		if( !pVideo->Get_Surface( image_filename ) )
		{
			valid = 0;
		}
		// image exists valid effect
		else
		{
			image = pVideo->Get_Surface( image_filename );
			valid = 1;
		}
	}

	// update ahead
	if( valid )
	{
		float old_speedfactor = pFramerate->speedfactor;
		pFramerate->speedfactor = 1;
		// use time to live as seconds
		for( float i = 0; i < DESIRED_FPS * time_to_live; i++ )
		{
			Update();
			Update_anim();
		}

		pFramerate->speedfactor = old_speedfactor;
	}
}

void cGlobal_effect :: Clear( void )
{
	cParticleAnimation::Clear();

	ge_type = GL_EFF_NONE;

	image_filename.clear();
	image = NULL;
	Set_Emitter_Rect( GL_rect( 0, 0, GAME_RES_W, 0 ) );
	Set_Emitter_Time_to_Live( -1 );
	Set_Emitter_Iteration_Interval( 1 * 0.3f );
	Set_ZPos( 0.12f, 0 );
	Set_Time_to_Live( 7, 0 );
	Set_Scale( 0.2f, 0.2f );
	Set_Speed( 2, 8 );
	Set_DirectionRange( 0, 90 );
	Set_ConstRotationZ( 0.5f, 10 );

	valid = 0;
}

void cGlobal_effect :: Create_from_Stream( XMLAttributes &attributes )
{
	// Type
	Set_type( (GlobalEffectType)attributes.getValueAsInteger( "type", GL_EFF_NONE ) );
	// Image
	Set_image( attributes.getValueAsString( "image" ).c_str() );
	// Creation Rect
	Set_Emitter_Rect( static_cast<float>(attributes.getValueAsInteger( "rect_x", 0 )), static_cast<float>(attributes.getValueAsInteger( "rect_y", 0 )), static_cast<float>(attributes.getValueAsInteger( "rect_w", GAME_RES_W )), static_cast<float>(attributes.getValueAsInteger( "rect_h", 0 )) );
	// if start position is 0 change it
	if( startposx == 0 && startposy == 0 )
	{
		Set_Emitter_Rect( startposx, -1, rect.w, rect.h );
	}
	// Z Position
	Set_ZPos( attributes.getValueAsFloat( "z", 0.12f ), attributes.getValueAsFloat( "z_rand" ) );
	// Time to Live
	if( attributes.exists( "time_to_live" ) )
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "time_to_live", 7 ) );
	}
	// if not set uses old Lifetime mod
	else
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "lifetime_mod", 20 ) * 0.3f );
	}
	// Emitter Iteration Interval
	if( attributes.exists( "emitter_iteration_interval" ) )
	{
		Set_Emitter_Iteration_Interval( attributes.getValueAsFloat( "emitter_iteration_interval", 1.3f ) );
	}
	// if not set uses old Creation speed ( 0.99.7 and below )
	else
	{
		Set_Emitter_Iteration_Interval( ( 1 / attributes.getValueAsFloat( "creation_speed", 0.3f ) ) * 0.032f );
	}
	// Scale
	Set_Scale( attributes.getValueAsFloat( "scale", 0.2f ), attributes.getValueAsFloat( "scale_rand", 0.2f ) );
	// Speed
	Set_Speed( attributes.getValueAsFloat( "speed", 2 ), attributes.getValueAsFloat( "speed_rand", 8 ) );
	// Direction
	Set_DirectionRange( attributes.getValueAsFloat( "dir_range_start", 0 ), attributes.getValueAsFloat( "dir_range_size", 90 ) );
	// Constant Rotation Z
	Set_ConstRotationZ( attributes.getValueAsFloat( "const_rotz", 0.5f ), attributes.getValueAsFloat( "const_rotz_rand", 10 ) );
}

void cGlobal_effect :: Save_to_Stream( ofstream &file )
{
	if( ge_type == GL_EFF_NONE )
	{
		return;
	}

	// begin global effect
	file << "\t<global_effect>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"" << ge_type << "\" />" << std::endl;
	// image
	string img_filename = image_filename;

	if( img_filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
	{
		img_filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
	}

	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	// rect
	file << "\t\t<Property name=\"rect_x\" value=\"" << startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_y\" value=\"" << startposy << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_w\" value=\"" << rect.w << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_h\" value=\"" << rect.h << "\" />" << std::endl;
	// Z Position
	file << "\t\t<Property name=\"z\" value=\"" << posz << "\" />" << std::endl;
	file << "\t\t<Property name=\"z_rand\" value=\"" << posz_rand << "\" />" << std::endl;
	// Time to Live
	file << "\t\t<Property name=\"time_to_live\" value=\"" << time_to_live << "\" />" << std::endl;
	// Emitter Iteration Interval
	file << "\t\t<Property name=\"emitter_iteration_interval\" value=\"" << emitter_iteration_interval << "\" />" << std::endl;
	// scale
	file << "\t\t<Property name=\"scale\" value=\"" << size_scale << "\" />" << std::endl;
	file << "\t\t<Property name=\"scale_rand\" value=\"" << size_scale_rand << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << vel << "\" />" << std::endl;
	file << "\t\t<Property name=\"speed_rand\" value=\"" << vel_rand << "\" />" << std::endl;
	// direction range
	file << "\t\t<Property name=\"dir_range_start\" value=\"" << angle_start << "\" />" << std::endl;
	file << "\t\t<Property name=\"dir_range_size\" value=\"" << angle_range << "\" />" << std::endl;
	// constant rotation Z
	file << "\t\t<Property name=\"const_rotz\" value=\"" << const_rotz << "\" />" << std::endl;
	file << "\t\t<Property name=\"const_rotz_rand\" value=\"" << const_rotz_rand << "\" />" << std::endl;

	// end global effect
	file << "\t</global_effect>" << std::endl;
}

void cGlobal_effect :: Update( void )
{
	if( editor_level_enabled || !valid )
	{
		return;
	}

	// if disabled
	if( ge_type == GL_EFF_NONE )
	{
		return;
	}
	
	Set_Pos( pCamera->x, pCamera->y );
	Update_anim();
}

void cGlobal_effect :: Update_anim( void )
{
	// find out of camera objects and move them to the opposite direction

	GL_rect camera_rect = GL_rect( pCamera->x, pCamera->y, GAME_RES_W, GAME_RES_H );

	for( ParticleList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get  animation particle pointer
		cParticle *obj = static_cast<cParticle *>(*itr);
		
		GL_rect anim_rect = GL_rect( obj->posx, obj->posy, obj->rect.w, obj->rect.h );
		
		// if not on screen
		if( !Col_Box( &anim_rect, &camera_rect ) )
		{
			// out in left
			if( obj->posx + image->w < pCamera->x )
			{
				// move to right
				obj->Set_PosX( pCamera->x + camera_rect.w );
			}
			// out in right
			else if( obj->posx > pCamera->x + camera_rect.w )
			{
				// move to left
				obj->Set_PosX( pCamera->x - image->w );
			}
			// out on top
			else if( obj->posy < pCamera->y - image->h - 1 )
			{
				obj->Set_PosY( pCamera->y + camera_rect.h );
			}
			// out on bottom
			else if( obj->posy > pCamera->y + camera_rect.h + 1 )
			{
				obj->Set_PosY( pCamera->y - image->h );
			}
		}
	}

	// update particle animation
	cParticleAnimation::Update();
}

void cGlobal_effect :: Draw( void )
{
	cParticleAnimation::Draw();
}

void cGlobal_effect :: Set_type( GlobalEffectType ntype )
{
	ge_type = ntype;
}

void cGlobal_effect :: Set_type( string ntype )
{
	if( ntype.compare( "Disabled" ) == 0 )
	{
		ge_type = GL_EFF_NONE;
	}
	else if( ntype.compare( "Falling" ) == 0 )
	{
		ge_type = GL_EFF_FALLING;
	}
	else if( ntype.compare( "Flying" ) == 0 )
	{
		ge_type = GL_EFF_FLYING;
	}
	else
	{
		printf( "Warning : Unknown Global Effect type %s\n", ntype.c_str() );
	}
}

void cGlobal_effect :: Set_image( string nimg_file )
{
	image_filename = nimg_file;
}

string cGlobal_effect :: Get_Typename( void )
{
	if( ge_type == GL_EFF_NONE )
	{
		return "Disabled";
	}
	else if( ge_type == GL_EFF_FALLING )
	{
		return "Falling";
	}
	else if( ge_type == GL_EFF_FLYING )
	{
		return "Flying";
	}
	else
	{
		return "Unknown";
	}

	return "";
}
