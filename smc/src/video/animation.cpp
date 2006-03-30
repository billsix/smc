/***************************************************************************
           animation.cpp  -  Animation and Particle classes
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
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

/* *** *** *** *** *** *** *** Base Animation class *** *** *** *** *** *** *** *** *** *** */

cAnimation :: cAnimation( double posx, double posy, double nfading_speed, Color col /* = (Uint8)255 */ ) : cObjectSprite( posx, posy )
{
	array = ARRAY_ANIM;
	type = TYPE_ACTIVESPRITE;
	massivetype = MASS_PASSIVE;
	spawned = 1;

	counter = 0;

	fading_speed = nfading_speed;
	color = col;
}

cAnimation :: ~cAnimation( void )
{
	//
}

void cAnimation :: Init( void )
{
	// virtual

}

void cAnimation :: Update( void )
{
	// virtual
}

void cAnimation :: Draw( void )
{
	// virtual
}

void cAnimation :: SetFadingSpeed( double speed )
{
	fading_speed = speed;
}

void cAnimation :: SetColor( Color col )
{
	color = col;
}

/* *** *** *** *** *** *** *** Fire Explosion *** *** *** *** *** *** *** *** *** *** */

cAnimation_1 :: cAnimation_1( double posx, double posy, unsigned int height /* = 40 */, unsigned int width /* = 20 */ ) : cAnimation( posx, posy )
{
	animtype = BLINKING_POINTS;

	images.push_back( GetImage( "animation/light_1/1.png" ) );
	images.push_back( GetImage( "animation/light_1/2.png" ) );
	images.push_back( GetImage( "animation/light_1/3.png" ) );

	for( unsigned int i = 0; i < 4; i++ )
	{
		cSprite object( NULL, (double)( rand() % ( width ) ), (double)( rand() % ( height ) ) );
		objects.push_back( object );
	}
}

cAnimation_1 :: ~cAnimation_1( void )
{
	objects.clear();
}

void cAnimation_1 :: Update( void )
{
	counter += Framerate.speedfactor * fading_speed;
}

void cAnimation_1 :: Draw( void )
{
	if( !visible )
	{
		return;
	}

	// draw the fixed points
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		switch( i ) 
		{
		case 0:
		{
			if( counter < 3 )
			{
				SetImage( 0 );
			}
			else if( counter < 6 )
			{
				SetImage( 0 );
			}
			else if( counter < 9 )
			{
				SetImage( 1 );
			}
			else if( counter < 12 )
			{
				SetImage( 0 );
			}
			break;
		}
		case 1:
		{
			if( counter < 3 )
			{
				SetImage( 0 );
			}
			else if( counter < 6 )
			{
				SetImage( 1 );
			}
			else if( counter < 9 )
			{
				SetImage( 2 );
			}
			else if( counter < 12 )
			{
				SetImage( 1 );
			}
			break;
		}
		case 2:
		{
			if( counter < 3 )
			{
				SetImage( 1 );
			}
			else if( counter < 6 )
			{
				SetImage( 2 );
			}
			else if( counter < 9 )
			{
				SetImage( 0 );
			}
			else if( counter < 12 )
			{
				SetImage( -1 );
			}
			break;			
		}
		case 3:
		{
			if( counter < 3 )
			{
				SetImage( 0 );
			}
			else if( counter < 6 )
			{
				SetImage( 1 );
			}
			else if( counter < 9 )
			{
				SetImage( 0 );
			}
			else if( counter < 12 )
			{
				SetImage( 0 );
			}
			break;
		}
		default:
		{
			break;
		}
		}

		objects[i].scale = 1.1 - ( counter / 12 );

		if( image )
		{
			glColor4ub( color.red, color.green, color.blue, color.alpha );
			image->Blit( objects[i].posx - ( cameraposx - posx ), objects[i].posy - ( cameraposy - posy ), NULL, 0, 0, 0, objects[i].scale );
			glColor4f( 1, 1, 1, 1 );
		}

		if( counter > 11 || counter < 0 )
		{
			visible = 0;
		}
	}
}

/* *** *** *** *** *** *** *** Fire Explosion *** *** *** *** *** *** *** *** *** *** */

cAnimation_2 :: cAnimation_2( double posx, double posy, unsigned int power /* = 5 */ ) : cAnimation( posx, posy )
{
	animtype = FIRE_EXPLOSION;

	images.push_back( GetImage( "animation/fire_1/4.png" ) );
	images.push_back( GetImage( "animation/fire_1/3.png" ) );
	images.push_back( GetImage( "animation/fire_1/2.png" ) );
	images.push_back( GetImage( "animation/fire_1/1.png" ) );

	for( unsigned int i = 0; i < power; i++ )
	{
		Animation2item temp;

		temp.velx = ( (double)( rand() % ( 50 ) - 25 ) ) / 10;
		temp.vely = ( (double)( rand() % ( 50 ) - 25 ) ) / 10;

		temp.counter = 8 + rand() % ( 5 );

		objects.push_back( temp );
	}
}

cAnimation_2 :: ~cAnimation_2( void )
{
	objects.clear();
}

void cAnimation_2 :: Update( void )
{
	if( !visible )
	{
		return;
	}

	counter += Framerate.speedfactor * fading_speed;

	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		objects[i].posx += objects[i].velx;
		objects[i].posy += objects[i].vely;
	}

	if( counter > 12 || counter < 0 )
	{
		visible = 0;
	}
}

void cAnimation_2 :: Draw( void )
{
	if( !visible )
	{
		return;
	}

	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( objects[i].counter > 8 )
		{
			SetImage( 0 );
		}
		else if( objects[i].counter > 5 )
		{
			SetImage( 1 );
		}
		else if( objects[i].counter > 3 )
		{
			SetImage( 2 );
		}
		else if( objects[i].counter > 0 )
		{
			SetImage( 3 );
		}

		objects[i].scale = objects[i].counter / 10;

		if( image )
		{
			glColor4ub( color.red, color.green, color.blue, color.alpha );
			image->Blit( objects[i].posx - ( cameraposx - posx ), objects[i].posy - ( cameraposy - posy ), NULL, 0, 0, 0, objects[i].scale );
			glColor4f( 1, 1, 1, 1 );
		}
		
		objects[i].counter -= Framerate.speedfactor * fading_speed;
	}
}

/* *** *** *** *** *** *** *** Particle Explosion *** *** *** *** *** *** *** *** *** *** */

cAnimation_3 :: cAnimation_3( double posx, double posy, unsigned int power /* = 5 */, GL_Surface *img /* = NULL */ ) : cAnimation( posx, posy, -1 )
{
	animtype = PARTICLE_EXPLOSION;

	image = img;

	vel = 2;
	vel_rand = 2;
	scale = 1;
	angle_start = 0;
	angle_range = 360;
	size_scale = 1;
	lifetime = 1;
	fadetype = FADE_SIZE;
	fade = 1;
	blending = BLEND_NONE;

	if( !image )
	{
		image = GetImage( "animation/particles/smoke.png" );
	}

	for( unsigned int i = 0; i < power; i++ )
	{
		Animation3item temp;
		objects.push_back( temp );
	}
}

cAnimation_3 :: ~cAnimation_3( void )
{
	objects.clear();
}

void cAnimation_3 :: Init( void )
{
	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		// angle
		double dir_angle = (double)( rand() % (int)( angle_range * 100 ) ) / 100;

		if( angle_start > 0 ) // with start angle
		{
			dir_angle += angle_start;
		}
		// velocity
		double speed = vel;
		if( vel_rand > 0.001 )
		{
			speed += (double)( rand() % (int)( vel_rand * 1000 ) ) / 1000;
		}
		objects[i].velx = (double)cos( dir_angle * 0.017453292519943295 ) * speed;
		objects[i].vely = (double)sin( dir_angle * 0.017453292519943295 ) * speed;
		// basic rotation
		objects[i].rotx = rotx;
		objects[i].roty = roty;
		objects[i].rotz = rotz;
		// constant rotation
		objects[i].const_rotx = const_rotx;
		objects[i].const_roty = const_roty;
		objects[i].const_rotz = const_rotz;
		if( const_rotx_rand > 0.001 )
		{
			objects[i].const_rotx += (double)( rand() % (int)( const_rotx_rand * 100 ) ) / 100;
		}
		if( const_roty_rand > 0.001 )
		{
			objects[i].const_roty += (double)( rand() % (int)( const_roty_rand * 100 ) ) / 100;
		}
		if( const_rotz_rand > 0.001 )
		{
			objects[i].const_rotz += (double)( rand() % (int)( const_rotz_rand * 100 ) ) / 100;
		}
		// scale
		objects[i].scale = scale * size_scale;
		// lifetime
		objects[i].counter = lifetime;
	}

	if( fading_speed < 0 )
	{
		fading_speed = 1;
	}

	if( fadetype == FADE_ALPHA )
	{
		fade = color.alpha;
	}
	else if( fadetype == FADE_COLOR )
	{
		fade = ( color.red + color.green + color.blue ) / 3;
	}
}

void cAnimation_3 :: Update( void )
{
	if( !visible )
	{
		return;
	}

	if( fadetype == FADE_SIZE )
	{
		fade -= ( ( 0.1 * Framerate.speedfactor ) * fading_speed ) / lifetime;
	}
	else if( fadetype == FADE_ALPHA )
	{
		fade -= ( ( 20 * Framerate.speedfactor ) * fading_speed ) / lifetime;
		
		if( fade < 0 )
		{
			visible = 0;
			return;
		}
		
		color.alpha = (Uint8)fade;
	}
	else if( fadetype == FADE_COLOR )
	{
		fade -= ( ( 20 * Framerate.speedfactor ) * fading_speed ) / lifetime;
		
		if( fade < 0 )
		{
			visible = 0;
			return;
		}
		
		color.red = (Uint8)fade;
		color.green = (Uint8)fade;
		color.blue = (Uint8)fade;
	}

	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( objects[i].counter > 0 )
		{
			// position
			objects[i].posx += objects[i].velx * Framerate.speedfactor;
			objects[i].posy += objects[i].vely * Framerate.speedfactor;
			
			// constant rotation
			if( objects[i].const_rotx > 0 )
			{
				objects[i].SetRotationX( objects[i].rotx + ( objects[i].const_rotx * Framerate.speedfactor ) );
			}
			if( objects[i].const_roty > 0 )
			{
				objects[i].SetRotationY( objects[i].roty + ( objects[i].const_roty * Framerate.speedfactor ) );
			}
			if( objects[i].const_rotz > 0 )
			{
				objects[i].SetRotationZ( objects[i].rotz + ( objects[i].const_rotz * Framerate.speedfactor ) );
			}

			if( fadetype == FADE_SIZE )
			{
				objects[i].scale = fade * size_scale;
			}

			if( objects[i].scale < 0 )
			{
				objects[i].counter = 0;
				continue;
			}
		}
		else
		{
			objects.erase( objects.begin() + i );
		}
	}

	if( objects.size() == 0 )
	{
		visible = 0;
	}
}

void cAnimation_3 :: Draw( void )
{
	if( !visible )
	{
		return;
	}

	for( unsigned int i = 0; i < objects.size(); i++ )
	{
		if( blending == BLEND_ADD )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		}
		else if( blending == BLEND_DRIVE )
		{
			glBlendFunc( GL_SRC_COLOR, GL_DST_ALPHA );
		}

		glColor4ub( color.red, color.green, color.blue, color.alpha );
		image->Blit( objects[i].posx - ( cameraposx - posx ), objects[i].posy - ( cameraposy - posy ), NULL, objects[i].rotx, objects[i].roty, objects[i].rotz, objects[i].scale );
		glColor4f( 1, 1, 1, 1 );
		
		if( blending != BLEND_NONE )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
	}
}

void cAnimation_3 :: SetSpeed( double vel_base, double vel_random /* = 1 */ )
{
	vel = vel_base;
	vel_rand = vel_random;
}

void cAnimation_3 :: Set_ConstRotationX( double rot, double rot_random /* = 0 */ )
{
	const_rotx = rot;
	const_rotx_rand = rot_random;
}

void cAnimation_3 :: Set_ConstRotationY( double rot, double rot_random /* = 0 */ )
{
	const_roty = rot;
	const_roty_rand = rot_random;
}

void cAnimation_3 :: Set_ConstRotationZ( double rot, double rot_random /* = 0 */ )
{
	const_rotz = rot;
	const_rotz_rand = rot_random;
}

void cAnimation_3 :: SetDirectionRange( double start, double range /* = 20 */ )
{
	angle_start = start;
	angle_range = range;
}

void cAnimation_3 :: SetScale( double size )
{
	size_scale = size;
}

void cAnimation_3 :: SetLifetimeMod( double time )
{
	lifetime = time;
}

void cAnimation_3 :: SetFading( FadingType type )
{
	fadetype = type;
}

void cAnimation_3 :: SetBlending( BlendingMode mode )
{
	blending = mode;
}

void cAnimation_3 :: SetImage( GL_Surface *img )
{
	if( !img )
	{
		return;
	}

	image = img;
}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cAnimationManager :: cAnimationManager( void )
{
	objects.clear();
}

cAnimationManager :: ~cAnimationManager( void )
{
	DeleteAll();
}

void cAnimationManager :: Update( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		objects[i]->Update();

		// if animation has ended delete the animation sprite
		if( !objects[i]->visible )
		{
			Delete( i );
		}
	}
}

void cAnimationManager :: Draw( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		objects[i]->Draw();
	}
}

void cAnimationManager :: Add( cAnimation *animation )
{
	if( !animation )
	{
		return;
	}

	animation->Init();

	objects.push_back( animation );
}

void cAnimationManager :: Delete( unsigned int num )
{
	if( num > Get_Size() )
	{
		return;
	}

	delete objects[num];
	objects.erase( objects.begin() + num );
}

void cAnimationManager :: DeleteAll( void )
{
	if( !Get_Size() )
	{
		return;
	}

	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( objects[i] ) 
		{
			delete objects[i];
		}
	}

	objects.clear();
}

unsigned int cAnimationManager :: Get_Size( void )
{
	return (unsigned int)objects.size();
}
