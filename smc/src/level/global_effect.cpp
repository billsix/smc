/***************************************************************************
    global_effect.cpp  -  class for handling level global effects
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 
#include "../level/global_effect.h"
#include "../core/framerate.h"
#include "../level/level_editor.h"
#include "../core/camera.h"
#include "../core/game_core.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cGlobal_effect *** *** *** *** *** *** *** *** *** *** *** *** */

cGlobal_effect :: cGlobal_effect( void )
{
	Init_default();
}

cGlobal_effect :: cGlobal_effect( XMLAttributes &attributes )
{
	Init_default();
	Create_from_Stream( attributes );
}

cGlobal_effect :: ~cGlobal_effect( void )
{
	if( anim_manager )
	{
		delete anim_manager;
	}
}

void cGlobal_effect :: Init_default( void )
{
	anim_manager = new cAnimationManager();
	
	Clear();
}

void cGlobal_effect :: Init( void )
{
	if( type == GL_EFF_NONE )
	{
		return;
	}

	if( type == GL_EFF_FALLING || type == GL_EFF_FLYING )
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
	anim_manager->Delete_All();

	type = GL_EFF_NONE;

	image_filename.clear();
	image = NULL;
	rect = GL_rect( 0, 0, GAME_RES_W, 0 );
	posz = 0.12f;
	posz_rand = 0;
	time_to_live = 7;
	scale = 0.2f;
	scale_rand = 0.2f;
	creation_speed = 0.3f;
	speed = 2;
	speed_rand = 8;
	dir_range_start = 0;
	dir_range_size = 90;
	const_rotz = 0.5f;
	const_rotz_rand = 10;

	counter = 0;
	valid = 0;
}

void cGlobal_effect :: Create_from_Stream( XMLAttributes &attributes )
{
	// Type
	Set_type( (GlobalEffectType)attributes.getValueAsInteger( "type", GL_EFF_NONE ) );
	// Image
	Set_image( attributes.getValueAsString( "image" ).c_str() );
	// Creation Rect
	Set_rect( GL_rect( (float)attributes.getValueAsInteger( "rect_x", 0 ), (float)attributes.getValueAsInteger( "rect_y", 0 ), (float)attributes.getValueAsInteger( "rect_w", GAME_RES_W ), (float)attributes.getValueAsInteger( "rect_h", 0 ) ) );
	// Z Position
	Set_ZPos( attributes.getValueAsFloat( "z", 0.12f ), attributes.getValueAsFloat( "z_rand" ) );
	// Time to Live
	if( attributes.exists( "time_to_live" ) )
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "time_to_live", 7 ) );
	}
	// uses old Lifetime mod
	else
	{
		Set_Time_to_Live( attributes.getValueAsFloat( "lifetime_mod", 20 ) * 0.3f );
	}
	// Creation Speed
	Set_Creation_Speed( attributes.getValueAsFloat( "creation_speed", 0.3f ) );
	// Scale
	Set_Scale( attributes.getValueAsFloat( "scale", 0.2f ), attributes.getValueAsFloat( "scale_rand", 0.2f ) );
	// Speed
	Set_Speed( attributes.getValueAsFloat( "speed", 2 ), attributes.getValueAsFloat( "speed_rand", 8 ) );
	// Direction
	Set_Direction( attributes.getValueAsFloat( "dir_range_start", 0 ), attributes.getValueAsFloat( "dir_range_size", 90 ) );
	// Constant Rotation Z
	Set_ConstRotationZ( attributes.getValueAsFloat( "const_rotz", 0.5f ), attributes.getValueAsFloat( "const_rotz_rand", 10 ) );
}

void cGlobal_effect :: Save_to_Stream( ofstream &file )
{
	if( type == GL_EFF_NONE )
	{
		return;
	}

	// begin global effect
	file << "\t<global_effect>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"" << type << "\" />" << std::endl;
	// image
	string img_filename = image_filename;

	if( img_filename.find( PIXMAPS_DIR ) == 0 )
	{
		img_filename.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
	}

	file << "\t\t<Property name=\"image\" value=\"" << img_filename << "\" />" << std::endl;
	// rect
	file << "\t\t<Property name=\"rect_x\" value=\"" << rect.x << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_y\" value=\"" << rect.y << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_w\" value=\"" << rect.w << "\" />" << std::endl;
	file << "\t\t<Property name=\"rect_h\" value=\"" << rect.h << "\" />" << std::endl;
	// Z Position
	file << "\t\t<Property name=\"z\" value=\"" << posz << "\" />" << std::endl;
	file << "\t\t<Property name=\"z_rand\" value=\"" << posz_rand << "\" />" << std::endl;
	// Time to Live
	file << "\t\t<Property name=\"time_to_live\" value=\"" << time_to_live << "\" />" << std::endl;
	// creation speed
	file << "\t\t<Property name=\"creation_speed\" value=\"" << creation_speed << "\" />" << std::endl;
	// scale
	file << "\t\t<Property name=\"scale\" value=\"" << scale << "\" />" << std::endl;
	file << "\t\t<Property name=\"scale_rand\" value=\"" << scale_rand << "\" />" << std::endl;
	// speed
	file << "\t\t<Property name=\"speed\" value=\"" << speed << "\" />" << std::endl;
	file << "\t\t<Property name=\"speed_rand\" value=\"" << speed_rand << "\" />" << std::endl;
	// direction range
	file << "\t\t<Property name=\"dir_range_start\" value=\"" << dir_range_start << "\" />" << std::endl;
	file << "\t\t<Property name=\"dir_range_size\" value=\"" << dir_range_size << "\" />" << std::endl;
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

	// falling images
	if( type == GL_EFF_FALLING )
	{
		counter += pFramerate->speedfactor * creation_speed;
		float pos_x = rect.x + pCamera->x;
		float pos_y = rect.y + pCamera->y - image->h;

		while( counter > 1 )
		{
			// X Position
			float x = pos_x;
			if( rect.w > 0 )
			{
				x += ( rand() % (int)rect.w );
			}
			// Y Position
			float y = pos_y;
			if( rect.h > 0 )
			{
				y += ( rand() % (int)rect.h );
			}

			cParticleAnimation *anim = new cParticleAnimation( x, y );
			anim->Set_Image( image );
			anim->Set_ZPos( posz, posz_rand );
			anim->Set_DirectionRange( dir_range_start, dir_range_size );
			anim->Set_Time_to_Live( time_to_live );
			anim->Set_Fading_Alpha( 1 );
			anim->Set_Speed( speed, speed_rand );
			anim->Set_Scale( scale, scale_rand );
			anim->Set_ConstRotationZ( const_rotz, const_rotz_rand );
			anim_manager->Add( anim );

			counter--;
		}
	}
}

void cGlobal_effect :: Update_anim( void )
{
	// find out of camera objects and move them to the opposite direction

	GL_rect camera_rect = GL_rect( pCamera->x, pCamera->y, GAME_RES_W, GAME_RES_H );

	for( cAnimationManager::AnimationList::iterator itr = anim_manager->objects.begin(), itr_end = anim_manager->objects.end(); itr != itr_end; ++itr )
	{
		// get  animation particle pointer
		cParticleAnimation *animation = static_cast<cParticleAnimation *>(*itr);
		
		GL_rect anim_rect = GL_rect( animation->objects[0]->posx, animation->objects[0]->posy, animation->objects[0]->rect.w, animation->objects[0]->rect.h );
		
		// if not on screen
		if( !Col_Box( &anim_rect, &camera_rect ) )
		{
			// out in left
			if( animation->objects[0]->posx + animation->image->w < pCamera->x )
			{
				// move to right
				animation->objects[0]->Set_PosX( pCamera->x + camera_rect.w );
			}
			// out in right
			else if( animation->objects[0]->posx > pCamera->x + camera_rect.w )
			{
				// move to left
				animation->objects[0]->Set_PosX( pCamera->x - animation->image->w );
			}
			// out on top
			else if( animation->objects[0]->posy < pCamera->y - image->h - 1 )
			{
				animation->objects[0]->Set_PosY( pCamera->y + camera_rect.h );
			}
			// out on bottom
			else if( animation->objects[0]->posy > pCamera->y + camera_rect.h + 1 )
			{
				animation->objects[0]->Set_PosY( pCamera->y - image->h );
			}
		}
	}

	// update particle animation
	anim_manager->Update();
}

void cGlobal_effect :: Draw( void )
{
	anim_manager->Draw();
}

void cGlobal_effect :: Set_type( GlobalEffectType ntype )
{
	type = ntype;
}

void cGlobal_effect :: Set_type( string ntype )
{
	if( ntype.compare( "Disabled" ) == 0 )
	{
		type = GL_EFF_NONE;
	}
	else if( ntype.compare( "Falling" ) == 0 )
	{
		type = GL_EFF_FALLING;
	}
	else if( ntype.compare( "Flying" ) == 0 )
	{
		type = GL_EFF_FLYING;
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

void cGlobal_effect :: Set_rect( GL_rect nrect )
{
	rect = nrect;

	// fix invalid width
	if( rect.w < 0 )
	{
		rect.w = 0;
	}
	// fix invalid height
	if( rect.h < 0 )
	{
		rect.h = 0;
	}
}

void cGlobal_effect :: Set_ZPos( float pos, float pos_random /* = 0 */ )
{
	posz = pos;
	posz_rand = pos_random;
}

void cGlobal_effect :: Set_Time_to_Live( float val )
{
	time_to_live = val;
}

void cGlobal_effect :: Set_Scale( float nscale, float scale_random /* = 0 */ )
{
	scale = nscale;
	scale_rand = scale_random;
}

void cGlobal_effect :: Set_Creation_Speed( float nspeed )
{
	creation_speed = nspeed;
}

void cGlobal_effect :: Set_Speed( float nspeed, float speed_random /* = 0 */ )
{
	speed = nspeed;
	speed_rand = speed_random;
}

void cGlobal_effect :: Set_Direction( float range_start, float range_size )
{
	dir_range_start = range_start;
	dir_range_size = range_size;
}

void cGlobal_effect :: Set_ConstRotationZ( float rot, float rot_random /* = 0 */ )
{
	const_rotz = rot;
	const_rotz_rand = rot_random;
}

string cGlobal_effect :: Get_Typename( void )
{
	if( type == GL_EFF_NONE )
	{
		return "Disabled";
	}
	else if( type == GL_EFF_FALLING )
	{
		return "Falling";
	}
	else if( type == GL_EFF_FLYING )
	{
		return "Flying";
	}
	else
	{
		return "Unknown";
	}

	return "";
}
