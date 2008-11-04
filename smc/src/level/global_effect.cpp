/***************************************************************************
 * global_effect.cpp  -  class for handling level global effects
 *
 * Copyright (C) 2006 - 2008 Florian Richter
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
: cParticle_Emitter()
{
	Clear();
}

cGlobal_effect :: cGlobal_effect( CEGUI::XMLAttributes &attributes )
: cParticle_Emitter()
{
	Clear();
	Create_from_Stream( attributes );
}

cGlobal_effect :: ~cGlobal_effect( void )
{

}

void cGlobal_effect :: Init_Anim( void )
{
	if( ge_type == GL_EFF_NONE )
	{
		return;
	}

	if( ge_type == GL_EFF_FALLING )
	{
		// image doesn't exist
		if( !pVideo->Get_Surface( image_filename ) )
		{
			valid = 0;
		}
		// image exists
		else
		{
			image = pVideo->Get_Surface( image_filename );
			// valid effect
			valid = 1;
		}
	}

	// update ahead
	if( valid )
	{
		float old_speedfactor = pFramerate->speedfactor;
		pFramerate->speedfactor = 1;
		// use time to live as seconds
		for( float i = 0; i < speedfactor_fps * time_to_live; i++ )
		{
			Update();
		}

		pFramerate->speedfactor = old_speedfactor;
	}
}

void cGlobal_effect :: Clear( void )
{
	cParticle_Emitter::Clear();

	ge_type = GL_EFF_NONE;

	image_filename.clear();
	image = NULL;
	Set_Emitter_Rect( 0, 0, static_cast<float>(game_res_w), 0 );
	Set_Emitter_Time_to_Live( -1 );
	Set_Emitter_Iteration_Interval( 0.3f );
	Set_Pos_Z( 0.12f, 0 );
	Set_Time_to_Live( 7, 0 );
	Set_Scale( 0.2f, 0.2f );
	Set_Speed( 2, 8 );
	Set_Direction_Range( 0, 90 );
	Set_Rotation( 0, 0, 0, 1 );
	Set_Const_Rotation_Z( -5, 10 );

	valid = 0;
}

void cGlobal_effect :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// Type
	Set_Type( (GlobalEffectType)attributes.getValueAsInteger( "type", ge_type ) );
	// Image
	Set_Image( attributes.getValueAsString( "image", image_filename ).c_str() );
	// Creation Rect
	Set_Emitter_Rect( static_cast<float>(attributes.getValueAsInteger( "rect_x", static_cast<int>(startposx) )), static_cast<float>(attributes.getValueAsInteger( "rect_y", static_cast<int>(startposy) )), static_cast<float>(attributes.getValueAsInteger( "rect_w", static_cast<int>(rect.w) )), static_cast<float>(attributes.getValueAsInteger( "rect_h", static_cast<int>(rect.h) )) );
	// Z Position
	Set_Pos_Z( attributes.getValueAsFloat( "z", posz ), attributes.getValueAsFloat( "z_rand", posz_rand ) );
	// Time to Live
	if( attributes.exists( "time_to_live" ) )
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "time_to_live", time_to_live ), attributes.getValueAsFloat( "time_to_live_rand", time_to_live_rand ) );
	}
	// if not set uses old Lifetime mod
	else
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "lifetime_mod", 20 ) * 0.3f );
	}
	// Emitter Iteration Interval
	if( attributes.exists( "emitter_iteration_interval" ) )
	{
		Set_Emitter_Iteration_Interval( attributes.getValueAsFloat( "emitter_iteration_interval", emitter_iteration_interval ) );
	}
	// if not set uses old Creation speed ( 0.99.7 and below )
	else
	{
		Set_Emitter_Iteration_Interval( ( 1 / attributes.getValueAsFloat( "creation_speed", 0.3f ) ) * 0.032f );
	}
	// Scale
	Set_Scale( attributes.getValueAsFloat( "scale", size_scale ), attributes.getValueAsFloat( "scale_rand", size_scale_rand ) );
	// Speed
	Set_Speed( attributes.getValueAsFloat( "speed", vel ), attributes.getValueAsFloat( "speed_rand", vel_rand ) );
	// Direction
	Set_Direction_Range( attributes.getValueAsFloat( "dir_range_start", angle_start ), attributes.getValueAsFloat( "dir_range_size", angle_range ) );
	// start rotation
	Set_Rotation( attributes.getValueAsFloat( "rot_x", start_rotx ), attributes.getValueAsFloat( "rot_y", start_roty ), attributes.getValueAsFloat( "rot_z", start_rotz ), 1 );
	// Constant Rotation Z
	Set_Const_Rotation_Z( attributes.getValueAsFloat( "const_rotz", const_rotz ), attributes.getValueAsFloat( "const_rotz_rand", const_rotz_rand ) );
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
	file << "\t\t<Property name=\"time_to_live_rand\" value=\"" << time_to_live_rand << "\" />" << std::endl;
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
	// start rotation
	file << "\t\t<Property name=\"rot_x\" value=\"" << start_rotx << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_y\" value=\"" << start_roty << "\" />" << std::endl;
	file << "\t\t<Property name=\"rot_z\" value=\"" << start_rotz << "\" />" << std::endl;
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
	
	Set_Pos( startposx + pActive_Camera->x, startposy + pActive_Camera->y );
	// update particles
	Update_Particles();
	// update particle animation
	cParticle_Emitter::Update();
}

void cGlobal_effect :: Update_Particles( void )
{
	GL_rect camera_rect = GL_rect( pActive_Camera->x, pActive_Camera->y, static_cast<float>(game_res_w), static_cast<float>(game_res_h) );

	for( ParticleList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get animation particle pointer
		cParticle *obj = static_cast<cParticle *>(*itr);
		
		GL_rect anim_rect = GL_rect( obj->posx, obj->posy, obj->rect.w, obj->rect.h );
		
		// if not on screen
		if( !Col_Box( &camera_rect, &anim_rect ) )
		{
			// out in left
			if( anim_rect.x + anim_rect.w < camera_rect.x )
			{
				// move to right
				obj->Set_Pos_X( camera_rect.x + camera_rect.w - 1 );
			}
			// out in right
			else if( anim_rect.x > camera_rect.x + camera_rect.w )
			{
				// move to left
				obj->Set_Pos_X( camera_rect.x + 1 );
			}
			// out on top
			else if( anim_rect.y + anim_rect.h < camera_rect.y )
			{
				// move to bottom
				obj->Set_Pos_Y( camera_rect.y + camera_rect.h - 1 );
			}
			// out on bottom
			else if( anim_rect.y > camera_rect.y + camera_rect.h )
			{
				// move to top
				obj->Set_Pos_Y( camera_rect.y + 1 );
			}
		}
	}
}

void cGlobal_effect :: Draw( void )
{
	cParticle_Emitter::Draw();
}

void cGlobal_effect :: Set_Type( GlobalEffectType ntype )
{
	ge_type = ntype;
}

void cGlobal_effect :: Set_Type( string ntype )
{
	if( ntype.compare( "Disabled" ) == 0 )
	{
		ge_type = GL_EFF_NONE;
	}
	else if( ntype.compare( "Falling" ) == 0 || ntype.compare( "Default" ) == 0 )
	{
		ge_type = GL_EFF_FALLING;
	}
	else
	{
		printf( "Warning : Unknown Global Effect type %s\n", ntype.c_str() );
	}
}

void cGlobal_effect :: Set_Image( string nimg_file )
{
	image_filename = nimg_file;
}

string cGlobal_effect :: Get_Type_Name( void )
{
	if( ge_type == GL_EFF_NONE )
	{
		return "Disabled";
	}
	else if( ge_type == GL_EFF_FALLING )
	{
		return "Falling";
	}

	return "Unknown";
}
