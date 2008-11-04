/***************************************************************************
 * powerup.cpp  -  powerup classes
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/powerup.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../video/animation.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** cPowerUp *** *** *** *** *** *** *** *** *** *** *** */

cPowerUp :: cPowerUp( float x, float y )
: cImageObjectSprite( x, y )
{
	sprite_array = ARRAY_ACTIVE;
	massivetype = MASS_PASSIVE;
	type = TYPE_POWERUP;
	posz = 0.05f;

	counter = 0;
}

cPowerUp :: ~cPowerUp( void )
{
	//
}

void cPowerUp :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	// visible
	bool save_visible = string_to_int( save_object->Get_Value( "visible" ) ) > 0;
	Set_Visible( save_visible );
}

cSave_Level_Object *cPowerUp :: Save_to_Savegame( void )
{
	// only save if needed
	if( visible )
	{
		return NULL;
	}

	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->type = type;
	save_object->properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(startposx) ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(startposy) ) ) );

	// visible
	save_object->properties.push_back( cSave_Level_Object_Property( "visible", int_to_string( visible ) ) );

	return save_object;
}

void cPowerUp :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// don't draw in editor mode if spawned
	if( spawned && editor_level_enabled )
	{
		return;
	}

	cImageObjectSprite::Draw( request );
}

bool cPowerUp :: Is_Update_Valid( void )
{
	// if not visible
	if( !visible )
	{
		return 0;
	}

	return 1;
}

unsigned int cPowerUp :: Validate_Collision( cSprite *obj )
{
	// basic validation checking
	int basic_valid = Validate_Collision_Ghost( obj );

	// found valid collision
	if( basic_valid > -1 )
	{
		return basic_valid;
	}

	if( obj->massivetype == MASS_MASSIVE )
	{
		if( obj->type == TYPE_PLAYER )
		{
			return 0;
		}
		if( obj->sprite_array == ARRAY_ENEMY )
		{
			return 0;
		}
		// don't collide with balls
		if( obj->type == TYPE_BALL )
		{
			return 0;
		}

		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && Is_on_Top( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cPowerUp :: Handle_out_of_Level( ObjectDirection dir )
{
	if( dir == DIR_LEFT )
	{
		posx = pActive_Camera->limit_rect.x;
	}
	else if( dir == DIR_RIGHT )
	{
		posx = pActive_Camera->limit_rect.x + pActive_Camera->limit_rect.w - col_pos.x - col_rect.w - 0.01f;
	}

	Turn_Around( dir );
}

/* *** *** *** *** *** *** cMushroom *** *** *** *** *** *** *** *** *** *** *** */

cMushroom :: cMushroom( float x, float y )
: cPowerUp( x, y )
{
	Init();
}

cMushroom :: cMushroom( CEGUI::XMLAttributes &attributes )
: cPowerUp()
{
	Init();
	Create_from_Stream( attributes );
}

cMushroom :: ~cMushroom( void )
{
	//
}

void cMushroom :: Init( void )
{
	velx = 3;
	player_range = 5000;

	type = TYPE_UNDEFINED;
	Set_Type( TYPE_MUSHROOM_DEFAULT );

	Update_Direction();

	glim_mod = 1;
}

cMushroom *cMushroom :: Copy( void )
{
	cMushroom *mushroom = new cMushroom( startposx, startposy );
	mushroom->Set_Type( type );

	return mushroom;
}

void cMushroom :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )) );
	// type
	Set_Type( (SpriteType)attributes.getValueAsInteger( "mushroom_type", TYPE_MUSHROOM_DEFAULT ) );
}

void cMushroom :: Save_to_Stream( ofstream &file )
{
	// begin item
	file << "\t<item>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"mushroom\" />" << std::endl;
	// mushroom type
	file << "\t\t<Property name=\"mushroom_type\" value=\"" << type << "\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;

	// end item
	file << "\t</item>" << std::endl;
}

void cMushroom :: Set_Type( SpriteType ntype )
{
	// already set
	if( type == ntype )
	{
		return;
	}

	Set_Color_Combine( 0, 0, 0, 0 );
	Clear_Images();

	if( ntype == TYPE_MUSHROOM_DEFAULT )
	{
		images.push_back( pVideo->Get_Surface( "game/items/mushroom_red.png" ) );
		name = _("Mushroom Red");
	}
	else if( ntype == TYPE_MUSHROOM_LIVE_1 )
	{
		images.push_back( pVideo->Get_Surface( "game/items/mushroom_green.png" ) );
		name = _("Mushroom 1-UP");
	}
	else if( ntype == TYPE_MUSHROOM_POISON )
	{
		images.push_back( pVideo->Get_Surface( "game/items/mushroom_poison.png" ) );
		name = _("Mushroom Poison");
	}
	else if( ntype == TYPE_MUSHROOM_BLUE )
	{
		images.push_back( pVideo->Get_Surface( "game/items/mushroom_blue.png" ) );
		name = _("Mushroom Blue");
	}
	else if( ntype == TYPE_MUSHROOM_GHOST )
	{
		images.push_back( pVideo->Get_Surface( "game/items/mushroom_ghost.png" ) );
		name = _("Mushroom Ghost");
	}
	else
	{
		printf( "Warning Unknown Mushroom type : %d\n", ntype );
		return;
	}
	
	type = ntype;
	
	Set_Image( 0, 1, 0 );
}

void cMushroom :: Activate( void )
{
	pPlayer->Get_Item( type );

	if( type == TYPE_MUSHROOM_DEFAULT )
	{
		pointsdisplay->Add_Points( 500, posx + image->w / 2, posy + 3 );
	}
	else if( type == TYPE_MUSHROOM_LIVE_1 )
	{
		pointsdisplay->Add_Points( 1000, posx + image->w / 2, posy + 3 );
	}
	else if( type == TYPE_MUSHROOM_POISON )
	{
		// nothing
	}
	else if( type == TYPE_MUSHROOM_BLUE )
	{
		pointsdisplay->Add_Points( 700, posx + image->w / 2, posy + 3 );
	}
	else if( type == TYPE_MUSHROOM_GHOST )
	{
		pointsdisplay->Add_Points( 800, posx + image->w / 2, posy + 3 );
	}

	// if spawned destroy
	if( spawned )
	{
		Destroy();
	}
	// hide
	else
	{
		Set_Visible( 0 );
	}
}

void cMushroom :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// Collision with Player
	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		Activate();
	}

	if( !ground_object )
	{
		if( vely < 25 )
		{
			Add_Velocity( 0, 1.5f );
		}
	}
	else
	{
		vely = 0;
	}

	// particles
	if( type == TYPE_MUSHROOM_LIVE_1 || type == TYPE_MUSHROOM_POISON )
	{
		counter += pFramerate->speedfactor;

		while( counter > 1 )
		{
			// randomly generated
			if( ( rand() % 3 ) == 0 )
			{
				cParticle_Emitter *anim = new cParticle_Emitter();
				anim->Set_Pos( col_rect.x + Get_Random_Float( 0, col_rect.w ), col_rect.y + Get_Random_Float( 0, col_rect.h ) );
				anim->Set_Direction_Range( 180, 180 );
				anim->Set_Time_to_Live( 0.3f );
				anim->Set_Scale( 0.6f, 0.1f );
				// 1-UP
				if( type ==  TYPE_MUSHROOM_LIVE_1 )
				{
					anim->Set_Color( Color( static_cast<Uint8>(rand() % 30), 100 + ( rand() % 150 ), rand() % 30 ) );
					anim->Set_Speed( 0.5f, 0.5f );
					anim->Set_Blending( BLEND_ADD );
				}
				// Poison
				else if( type == TYPE_MUSHROOM_POISON )
				{
					anim->Set_Color( Color( static_cast<Uint8>( 120 + rand() % 30 ), 80 + ( rand() % 150 ), rand() % 30 ) );
					anim->Set_Speed( 0.7f, 0.5f );
				}
				
				anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );

				pAnimation_Manager->Add( anim );
			}

			counter--;
		}
	}
	else if( type == TYPE_MUSHROOM_BLUE )
	{
		// animation only if on ground
		if( ground_object )
		{
			counter += pFramerate->speedfactor * 2;


			while( counter > 1 )
			{
				// randomly generated
				if( ( rand() % 3 ) == 0 )
				{
					cParticle_Emitter *anim = new cParticle_Emitter();
					anim->Set_Pos( col_rect.x + col_rect.w * 0.3f + Get_Random_Float( 0, col_rect.w * 0.4f ), col_rect.y + col_rect.h * 0.72f + Get_Random_Float( 0, col_rect.h * 0.1f ) );
					anim->Set_Time_to_Live( 1.4f, 1 );
					anim->Set_Scale( 0.3f, 0.1f );
					anim->Set_Color( Color( static_cast<Uint8>(100), 100, 255 ) );
					anim->Set_Blending( BLEND_ADD );
					anim->Set_Speed( 0, 0.01f );
					anim->Set_Image( pVideo->Get_Surface( "animation/particles/ice_1.png" ) );

					pAnimation_Manager->Add( anim );
				}

				counter--;
			}
		}

		// glimming
		float new_glim = combine_col[2]; 

		if( glim_mod )
		{
			new_glim += pFramerate->speedfactor * 0.05f;

			if( new_glim > 0.5f )
			{
				new_glim = 1;
				glim_mod = 0;
			}
		}
		else
		{
			new_glim -= pFramerate->speedfactor * 0.05f;

			if( new_glim < 0 )
			{
				new_glim = 0;
				glim_mod = 1;
			}
		}

		Set_Color_Combine( new_glim * 0.1f, new_glim * 0.1f, new_glim, GL_ADD );
	}
}

void cMushroom :: Handle_Collision_Massive( cObjectCollision *collision )
{
	Turn_Around( collision->direction );
}

void cMushroom :: Handle_Collision_Box( ObjectDirection cdirection, GL_rect *r2 )
{
	if( cdirection == DIR_DOWN )
	{
		vely = -30;

		// left
		if( posx > r2->x && velx < 0 )
		{
			Turn_Around( DIR_LEFT );
		}
		// right
		else if( posx < r2->x && velx > 0 )
		{
			Turn_Around( DIR_RIGHT );
		}
	}
	else if( cdirection == DIR_LEFT || cdirection == DIR_RIGHT )
	{
		vely = -13;
		Turn_Around( cdirection );
	}
	// unsupported collision direction
	else
	{
		return;
	}

	Reset_on_Ground();
}

/* *** *** *** *** *** *** cFirePlant *** *** *** *** *** *** *** *** *** *** *** */

cFirePlant :: cFirePlant( float x, float y )
: cPowerUp( x, y )
{
	type = TYPE_FIREPLANT;

	images.push_back( pVideo->Get_Surface( "game/items/fireplant.png" ) );
	images.push_back( pVideo->Get_Surface( "game/items/fireplant_left.png" ) );
	images.push_back( pVideo->Get_Surface( "game/items/fireplant_right.png" ) );

	Set_Image( 0, 1, 0 );

	name = _("Fireplant");
}

cFirePlant :: ~cFirePlant( void )
{
	//
}

cFirePlant *cFirePlant :: Copy( void )
{
	cFirePlant *fireplant = new cFirePlant( startposx, startposy );

	return fireplant;
}

void cFirePlant :: Update( void )
{
	if( !valid_update || !Is_Visible_on_Screen() )
	{
		return;
	}

	counter += pFramerate->speedfactor;

	if( counter > speedfactor_fps * 2.5f )
	{
		// if no direction image set
		if( curr_img == 0 )
		{
			Set_Image( 1 + ( rand() % 2 ), 0, 0 );
		}
		// direction image is set
		else
		{
			Set_Image( 0, 0, 0 );
		}

		counter = 0;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		pPlayer->Get_Item( TYPE_FIREPLANT );

		pointsdisplay->Add_Points( 700, posx + image->w / 2, posy );
		
		// if spawned destroy
		if( spawned )
		{
			Destroy();
		}
		// hide
		else
		{
			Set_Visible( 0 );
		}
	}
}


/* *** *** *** *** *** *** *** cMoon *** *** *** *** *** *** *** *** *** *** */

cMoon :: cMoon( float x, float y )
: cPowerUp( x, y )
{
	Init();
}

cMoon :: cMoon( CEGUI::XMLAttributes &attributes )
: cPowerUp()
{
	Init();
	Create_from_Stream( attributes );
}

cMoon :: ~cMoon( void )
{
	//
}

void cMoon :: Init( void )
{
	type = TYPE_MOON;

	images.push_back( pVideo->Get_Surface( "game/items/moon_1.png" ) );
	images.push_back( pVideo->Get_Surface( "game/items/moon_2.png" ) );

	Set_Image( 0, 1, 0 );

	name = _("Moon (3-UP)");
}

cMoon *cMoon :: Copy( void )
{
	cMoon *moon = new cMoon( startposx, startposy );

	return moon;
}

void cMoon :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )) );
}

void cMoon :: Save_to_Stream( ofstream &file )
{
	// begin item
	file << "\t<item>" << std::endl;

	// type
	file << "\t\t<Property name=\"type\" value=\"moon\" />" << std::endl;
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;

	// end item
	file << "\t</item>" << std::endl;
}

void cMoon :: Update( void )
{
	if( !valid_update || !Is_Visible_on_Screen() )
	{
		return;
	}

	counter += pFramerate->speedfactor;

	if( counter > speedfactor_fps * 2.8f )
	{
		Set_Image( 1, 0, 0 );
	}
	else if( counter > 0 )
	{
		Set_Image( 0, 0, 0 );
	}

	if( counter > speedfactor_fps * 3.3f )
	{
		counter = 0;
	}

	if( Col_Box( &col_rect, &pPlayer->col_rect ) )
	{
		pPlayer->Get_Item( TYPE_MOON );

		pointsdisplay->Add_Points( 4000, posx + image->w / 2, posy );

		// if spawned destroy
		if( spawned )
		{
			Destroy();
		}
		// hide
		else
		{
			Set_Visible( 0 );
		}
	}

	particle_counter += pFramerate->speedfactor;

	// particles
	if( particle_counter > 5 )
	{
		if( ( rand() % 5 ) == 0 )
		{
			cParticle_Emitter *anim = new cParticle_Emitter();
			anim->Set_Pos( posx + 12 + ( rand() % 30 ), posy + 10 + ( rand() % 20 ) );
			anim->Set_Speed( 0.2f, 0.2f );
			anim->Set_Scale( 0.2f );
			anim->Set_Time_to_Live( 0.3f );
			anim->Set_Color( Color( static_cast<Uint8>(200), 200, 0 ) );
			anim->Set_Blending( BLEND_ADD );
			anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
			pAnimation_Manager->Add( anim );
		}

		particle_counter = 0;
	}
}
