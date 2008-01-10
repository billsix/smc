/***************************************************************************
           box.cpp  -  class for the basic box handler
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
 
#include "../objects/box.h"
#include "../audio/audio.h"
#include "../core/camera.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../core/game_core.h"
#include "../video/animation.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../user/savegame.h"

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

cBaseBox :: cBaseBox( float x /* = 0 */, float y /* = 0 */ )
: cImageObjectSprite( x, y )
{
	type = TYPE_ACTIVESPRITE;
	sprite_array = ARRAY_ACTIVE;
	massivetype = MASS_MASSIVE;
	can_be_ground = 1;

	counter = 0;
	box_type = TYPE_UNDEFINED;
	item_image = NULL;
	posz = 0.055f;

	move_col_dir = DIR_UNDEFINED;
	move_counter = 0;
	move_back = 0;
	useable_count = 1; // default = 1 time usable
	start_useable_count = 1;

	box_invisible = 0;

	particle_counter_active = 0;
}

cBaseBox :: ~cBaseBox( void )
{
	//
}

void cBaseBox :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	if( box_type != TYPE_SPINBOX && box_type != TYPE_TEXT_BOX )
	{
		// animation
		Set_Animation( attributes.getValueAsString( "animation", anim_type ).c_str() );
	}
	// invisible
	Set_Invisible( attributes.getValueAsInteger( "invisible" ) );
	// useable count
	Set_UseableCount( attributes.getValueAsInteger( "useable_count", start_useable_count ), 1 );
}

void cBaseBox :: Save_to_Stream( ofstream &file )
{
	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// type
	if( box_type == TYPE_SPINBOX )
	{
		file << "\t\t<Property name=\"type\" value=\"spin\" />" << std::endl;
	}
	else if( box_type == TYPE_TEXT_BOX )
	{
		file << "\t\t<Property name=\"type\" value=\"text\" />" << std::endl;
	}
	else
	{
		file << "\t\t<Property name=\"type\" value=\"bonus\" />" << std::endl;
		// animation type
		file << "\t\t<Property name=\"animation\" value=\"" << anim_type << "\" />" << std::endl;
		// best possible item
		file << "\t\t<Property name=\"item\" value=\"" << box_type << "\" />" << std::endl;
	}
	// invisible
	file << "\t\t<Property name=\"invisible\" value=\"" << box_invisible << "\" />" << std::endl;
	// useable count
	file << "\t\t<Property name=\"useable_count\" value=\"" << start_useable_count << "\" />" << std::endl;
}

void cBaseBox :: Load_from_Savegame( cSave_Level_Object *save_object )
{
	// useable count
	int save_useable_count = string_to_int( save_object->Get_Value( "useable_count" ) );
	Set_UseableCount( save_useable_count );
}

cSave_Level_Object *cBaseBox :: Save_to_Savegame( void )
{
	// only save if needed
	if( useable_count == start_useable_count )
	{
		return NULL;
	}

	cSave_Level_Object *save_object = new cSave_Level_Object();

	// default values
	save_object->type = type;
	save_object->properties.push_back( cSave_Level_Object_Property( "posx", int_to_string( static_cast<int>(startposx) ) ) );
	save_object->properties.push_back( cSave_Level_Object_Property( "posy", int_to_string( static_cast<int>(startposy) ) ) );

	// Useable Count
	save_object->properties.push_back( cSave_Level_Object_Property( "useable_count", int_to_string( useable_count ) ) );

	return save_object;
}

void cBaseBox :: Set_Animation( string new_anim_type )
{
	// already set
	if( anim_type.compare( new_anim_type ) == 0 )
	{
		return;
	}

	Clear_Images();
	anim_type = new_anim_type;

	if( anim_type.compare( "Bonus" ) == 0 )
	{
		// disabled image
		images.push_back( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// animation images
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/1.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/2.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/3.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/4.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/5.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/bonus/6.png" ) );

		anim_counter_min = 1;
		anim_counter_max = 6;
	}
	else if( anim_type.compare( "Default" ) == 0 )
	{
		// disabled image
		images.push_back( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// default
		images.push_back( pVideo->Get_Surface( "game/box/yellow/default.png" ) );

		anim_counter_min = 1;
		anim_counter_max = 1;
	}
	else if( anim_type.compare( "Power" ) == 0 )
	{
		// disabled image
		images.push_back( pVideo->Get_Surface( "game/box/brown1_1.png" ) );
		// default
		images.push_back( pVideo->Get_Surface( "game/box/yellow/power_1.png" ) );

		anim_counter_min = 1;
		anim_counter_max = 1;
	}
	else if( anim_type.compare( "Spin" ) == 0 )
	{
		// disabled image
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/disabled.png" ) );
		// animation images
		images.push_back( pVideo->Get_Surface( "game/box/yellow/default.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/1.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/2.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/3.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/4.png" ) );
		images.push_back( pVideo->Get_Surface( "game/box/yellow/spin/5.png" ) );

		anim_counter_min = 1;
		anim_counter_max = 6;
	}
	else
	{
		printf( "Warning : Unknown Box Animation Type %s\n", anim_type.c_str() );
		Set_Animation( "Bonus" );
	}

	counter = static_cast<float>(anim_counter_min);
	Set_Image( static_cast<int>(counter), 1, 0 );
}

void cBaseBox :: Set_UseableCount( int count, bool new_startcount /* = 0 */ )
{
	// unlimited
	if( count < -1 )
	{
		count = -1;
	}

	useable_count = count;

	if( new_startcount )
	{
		start_useable_count = useable_count;
	}
}

void cBaseBox :: Set_Invisible( unsigned int type )
{
	// already set
	if( box_invisible == type )
	{
		return;
	}

	// was invisible
	if( box_invisible == 1 )
	{
		Set_Image( static_cast<int>(counter), 1, 0 );
	}
	// was ghost
	else if( box_invisible == 2 )
	{
		Set_Color( 255, 255, 255, 255 );
		Set_Color_Combine( 0, 0, 0, 0 );
	}

	// got invisible
	if( type == 1 )
	{
		Set_Image( -1, 1, 0 );

		col_pos = images[0]->col_pos;
		col_rect.w = images[0]->col_w;
		col_rect.h = images[0]->col_h;
		rect.w = images[0]->col_w;
		rect.h = images[0]->col_h;
		start_rect.w = images[0]->col_w;
		start_rect.h = images[0]->col_h;
	}
	// got ghost
	else if( type == 2 )
	{
		Set_Color( 192, 192, 255, 128 );
		Set_Color_Combine( 0.2f, 0.2f, 0.55f, GL_ADD );
	}

	box_invisible = type;

	// create name again
	Create_Name();
}

void cBaseBox :: Activate_collision( ObjectDirection cdirection )
{
	// if already active ignore event
	if( move_col_dir != DIR_UNDEFINED )
	{
		return;
	}

	// not useable
	if( useable_count == 0 )
	{
		return;
	}

	// if invisible go visible
	if( box_invisible )
	{
		Set_Image( static_cast<int>(counter), 0, 0 );
	}

	// set collision direction
	move_col_dir = cdirection;
	Update_valid_update();

	Check_Collision( Get_OppositeDirection( move_col_dir ) );
	Activate();

	// set useable count
	if( useable_count > 0 )
	{
		Set_UseableCount( useable_count - 1 );
	}
}

void cBaseBox :: Update_collision( void )
{
	if( move_col_dir == DIR_UNDEFINED )
	{
		return;
	}

	// moving value
	float xcheck = 0, ycheck = 0;
	/* scale value 
	 * image scale using the width/height
	 * todo : implement this for left/right to allow smaller y_scale
	*/
	float x_scale = 0.5f, y_scale = 0.5f;

	if( move_col_dir == DIR_UP )
	{
		ycheck = -2.5f;
	}
	else if( move_col_dir == DIR_RIGHT )
	{
		ycheck = -1;
		xcheck = 2.5f;
		//y_scale = 0.25f;
	}
	else if( move_col_dir == DIR_LEFT )
	{
		ycheck = -1;
		xcheck = -2.5f;
		//y_scale = 0.25f;
	}
	else
	{
		printf( "Warning : wrong box collision direction %d\n", move_col_dir );
		move_col_dir = DIR_UNDEFINED;
		Update_valid_update();
		return;
	}

	// speed mod
	float mod = pFramerate->speedfactor * 0.05f;
	// counter
	move_counter += pFramerate->speedfactor * 0.2f;

	// move into the given direction
	if( !move_back )
	{
		// final move value
		float move_x = ( xcheck * pFramerate->speedfactor ) - ( ( rect.w * x_scale ) * mod );
		float move_y = ( ycheck * pFramerate->speedfactor ) - ( ( rect.h * y_scale ) * mod );

		// retain collision pos
		col_pos.x -= move_x;
		col_pos.y -= move_y;
		// move
		Move( move_x, move_y, 1 );
		// scale
		Add_Scale( mod );

		// Particles
		Generate_active_Particles();

		// check if reached final position
		if( move_counter > 1 )
		{
			move_back = 1;
			move_counter = 0;
		}
	}
	// move back to the original position
	else
	{
		// final move value
		float move_x = ( -xcheck * pFramerate->speedfactor ) + ( ( rect.w * x_scale ) * mod );
		float move_y = ( -ycheck * pFramerate->speedfactor ) + ( ( rect.h * y_scale ) * mod );

		// retain collision pos
		col_pos.x -= move_x;
		col_pos.y -= move_y;
		// move
		Move( move_x, move_y, 1 );
		// scale
		Add_Scale( -mod );

		// check if reached original position
		if(	move_counter > 1 )
		{
			move_col_dir = DIR_UNDEFINED;
			Update_valid_update();
			move_back = 0;
			move_counter = 0;

			// reset rect
			col_pos = image->col_pos;
			// reset scale
			Set_Scale( 1 );
			// reset position
			Set_Pos( startposx, startposy );
		}
	}
}

void cBaseBox :: Check_Collision( ObjectDirection cdirection )
{
	// additional direction based check position
	float check_x = 0;
	float check_y = 0;


	// set the collision size based on the collision direction
	if( cdirection == DIR_BOTTOM )
	{
		check_y -= 10;
	}
	else if( cdirection == DIR_TOP )
	{
		check_y += 10;
	}
	else if( cdirection == DIR_LEFT ) 
	{
		check_x += 5;
	}
	else if( cdirection == DIR_RIGHT )
	{
		check_x -= 5;
	}

	// collision count
	unsigned int col_count = Collision_Check_Relative( check_x, check_y, col_rect.w - ( check_x * 0.5f ), col_rect.h - ( check_y * 0.5f ) ).size();

	// handle collisions
	for( unsigned int i = 0; i < col_count; i++ )
	{
		cObjectCollision *col = Collision_Get_last();
		cSprite *col_obj = pLevel->pSprite_Manager->Get_Pointer( col->number );

		// send box collision
		col_obj->Handle_Collision_Box( Get_OppositeDirection( col->direction ), &col_rect );

		// Enemy collision
		if( col->type == CO_ENEMY )
		{
			Col_Enemy( col_obj );
		}

		Collision_Delete( col );
	}
}

void cBaseBox :: Col_Enemy( cSprite *obj )
{
	// only valid enemies
	if( obj->type == TYPE_GUMBA || obj->type == TYPE_TURTLE || obj->type == TYPE_REX )
	{
		pAudio->PlaySound( "death_box.ogg" );
		static_cast<cMovingSprite *>(obj)->DownGrade( 1 );
	}
}

void cBaseBox :: Activate( void )
{
	// virtual
}

void cBaseBox :: Update( void )
{
	// animate visible box only or an activated invisible box
	if( !box_invisible || image )
	{
		// Spinbox uses anim_counter for animation handling
		if( ( type == TYPE_SPINBOX || useable_count != 0 ) && anim_counter_min != anim_counter_max )
		{
			counter += pFramerate->speedfactor * 0.4f;

			if( counter >= anim_counter_max + 1 )
			{
				counter = static_cast<float>(anim_counter_min);
			}

			// Set_Image overwrites col_pos
			GL_point col_pos_temp = col_pos;

			Set_Image( static_cast<int>(counter) );

			// save col_pos
			col_pos = col_pos_temp;
		}
	}

	Update_collision();
}

void cBaseBox :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// editor disabled
	if( !editor_level_enabled )
	{
		// visible box or activated invisible box
		cImageObjectSprite::Draw( request );
	}
	// editor enabled
	else
	{
		// draw invisible box only in editor mode
		if( box_invisible )
		{
			Color color;

			// default invisible
			if( box_invisible == 1 )
			{
				color = Color( static_cast<Uint8>(240), 0, 30, 128 );
			}
			// ghost
			else if( box_invisible == 2 )
			{
				color = Color( static_cast<Uint8>(20), 20, 150, 128 );
			}

			pVideo->Draw_Rect( startposx - pCamera->x, startposy - pCamera->y, col_rect.w, col_rect.h, posz, &color );
		}
		// visible box
		else
		{
			cImageObjectSprite::Draw( request );
		}

		// draw item image
		if( item_image )
		{
			// auto position
			item_image->Blit( startposx - ( ( item_image->w - rect.w ) / 2 ) - pCamera->x, startposy - ( ( item_image->h - rect.h ) / 2 ) - pCamera->y, posz + 0.000003f );
		}
	}
}

void cBaseBox :: Generate_active_Particles( void )
{
	// no default/unimportant boxes
	if( type == TYPE_SPINBOX || type == TYPE_TEXT_BOX || box_type == TYPE_GOLDPIECE )
	{
		return;
	}

	particle_counter_active += pFramerate->speedfactor;

	while( particle_counter_active > 0 )
	{
		cParticleAnimation *anim = new cParticleAnimation( posx + ( rand() % static_cast<int>(rect.w) ), posy );
		anim->Set_DirectionRange( 180, 180 );
		anim->Set_Speed( 3.1f, 0.5f );
		anim->Set_Scale( 0.7f, 0.1f );

		Color color = white;

		if( box_type == TYPE_MUSHROOM_DEFAULT )
		{
			color = Color( static_cast<Uint8>( 100 + ( rand() % 100 ) ), 20 + ( rand() % 50 ), 10 + ( rand() % 30 ) );
		}
		else if( box_type == TYPE_FIREPLANT )
		{
			color = Color( static_cast<Uint8>( 110 + ( rand() % 150 ) ), rand() % 50, rand() % 30 );
		}
		else if( box_type == TYPE_MUSHROOM_BLUE )
		{
			color = Color( static_cast<Uint8>( rand() % 30 ), ( rand() % 30 ), 150 + ( rand() % 50 ) );
		}
		else if( box_type == TYPE_MUSHROOM_GHOST )
		{
			color = Color( static_cast<Uint8>( 100 + ( rand() % 50 ) ), 100 + ( rand() % 50 ), 100 + ( rand() % 50 ) );
		}
		else if( box_type == TYPE_MUSHROOM_LIVE_1 )
		{
			color = Color( static_cast<Uint8>( rand() % 30 ), 100 + ( rand() % 150 ), rand() % 30 );
		}
		else if( box_type == TYPE_JSTAR )
		{
			color = Color( static_cast<Uint8>( 110 + ( rand() % 150 ) ), 80 + ( rand() % 50 ), rand() % 20 );
		}
		else if( box_type == TYPE_MUSHROOM_POISON )
		{
			color = Color( static_cast<Uint8>( 50 + rand() % 50 ), 100 + ( rand() % 150 ), rand() % 10 );
		}

		anim->Set_Time_to_Live( 0.3f );
		anim->Set_Color( color );
		anim->Set_Blending( BLEND_ADD );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/light.png" ) );
		pAnimationManager->Add( anim );

		particle_counter_active--;
	}
}

bool cBaseBox :: is_Update_valid( void )
{
	if( !useable_count && move_col_dir == DIR_UNDEFINED )
	{
		return 0;
	}

	return 1;
}

bool cBaseBox :: is_Draw_valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		// not visible
		if( !visible || !image )
		{
			return 0;
		}

		// ghost
		if( box_invisible == 2 )
		{
			// maryo is not ghost
			if( pPlayer->maryo_type != MARYO_GHOST )
			{
				return 0;
			}
		}
	}
	// editor enabled
	else
	{
		// no image
		if( !start_image && !box_invisible )
		{
			return 0;
		}
	}

	// not visible on the screen
	if( !is_Visible_onScreen() )
	{
		return 0;
	}

	return 1;
}

unsigned int cBaseBox :: Validate_Collision( cSprite *obj )
{
	if( obj->massivetype == MASS_MASSIVE )
	{
		return 2;
	}
	// items
	if( obj->type == TYPE_MUSHROOM_LIVE_1 || obj->type == TYPE_MUSHROOM_DEFAULT || obj->type == TYPE_MUSHROOM_POISON 
		|| obj->type == TYPE_MUSHROOM_BLUE || obj->type == TYPE_MUSHROOM_GHOST || obj->type == TYPE_FGOLDPIECE )
	{
		return 2;
	}
	if( obj->massivetype == MASS_HALFMASSIVE )
	{
		// if moving downwards and object is on top
		if( vely >= 0 && is_onTop( obj ) )
		{
			return 2;
		}
	}

	return 0;
}

void cBaseBox :: Handle_Collision_Player( cObjectCollision *collision )
{
	// if player jumps from below or fly's against it
	if( collision->direction == DIR_BOTTOM && pPlayer->state != STA_FLY )
	{
		if( useable_count != 0 )
		{
			Activate_collision( Get_OppositeDirection( collision->direction ) );
		}
		else
		{
			if( is_Visible_onScreen() )
			{
				pAudio->PlaySound( "tock.ogg" );
			}
		}
	}
}

void cBaseBox :: Handle_Collision_Enemy( cObjectCollision *collision )
{
	if( collision->direction == DIR_RIGHT || collision->direction == DIR_LEFT || collision->direction == DIR_BOTTOM )
	{
		if( useable_count != 0 )
		{
			Activate_collision( Get_OppositeDirection( collision->direction ) );
		}
		else
		{
			if( is_Visible_onScreen() )
			{
				pAudio->PlaySound( "tock.ogg" );
			}
		}
	}
}


void cBaseBox :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	if( type != TYPE_TEXT_BOX )
	{
		// useable count
		Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "editor_basebox_useable_count" ));
		editbox->setTooltipText( "Useable Count" );
		Editor_Add( editbox, 80 );

		editbox->setText( int_to_string( start_useable_count ) );
		editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cBaseBox::Editor_Useable_Count_Key, this ) );
	}

	// Invisible
	Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_basebox_invisible" ));
	combobox->getEditbox()->setTooltipText( "Invisible" );
	Editor_Add( combobox, 120, 100 );

	combobox->addItem( new ListboxTextItem( "Enabled" ) );
	combobox->addItem( new ListboxTextItem( "Disabled" ) );
	combobox->addItem( new ListboxTextItem( "Ghost" ) );

	if( box_invisible == 1 )
	{
		combobox->setText( "Enabled" );
	}
	else if( box_invisible == 2 )
	{
		combobox->setText( "Ghost" );
	}
	else
	{
		combobox->setText( "Disabled" );
	}

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cBaseBox::Editor_Invisible_Select, this ) );

	if( type == TYPE_SPINBOX )
	{
		// set position
		Editor_pos_update();
	}
}

bool cBaseBox :: Editor_Useable_Count_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_UseableCount( string_to_int( str_text ), 1 );

	return 1;
}

bool cBaseBox :: Editor_Invisible_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( "Enabled" ) == 0 )
	{
		Set_Invisible( 1 );
	}
	else if( item->getText().compare( "Ghost" ) == 0 )
	{
		Set_Invisible( 2 );
	}
	else
	{
		Set_Invisible( 0 );
	}

	return 1;
}

void cBaseBox :: Create_Name( void )
{
	if( box_type == TYPE_UNDEFINED )
	{
		name = "Box Empty";
	}
	else if( box_type == TYPE_POWERUP )
	{
		name = "Box Random";
	}
	else if( box_type == TYPE_SPINBOX )
	{
		name = "Spinbox";
	}
	else if( box_type == TYPE_TEXT_BOX )
	{
		name = "Textbox";
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		name = "Box Mushroom";
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		name = "Box Mushroom - Fireplant";
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		name = "Box Mushroom - Blue Mushroom";
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		name = "Box Mushroom - Ghost Mushroom";
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		name = "Box 1-UP";
	}
	else if( box_type == TYPE_JSTAR )
	{
		name = "Box Star";
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		name = "Box Goldpiece";
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		name = "Box Mushroom Poison";
	}
	else
	{
		name = "Box Unknown Item Type";
	}

	if( box_invisible )
	{
		name.insert( 0, "Invisible " );
	}
}
