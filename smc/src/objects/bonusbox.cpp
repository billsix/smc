/***************************************************************************
           bonusbox.cpp  -  class for bonusbox
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

#include "../objects/bonusbox.h"
#include "../objects/star.h"
#include "../gui/hud.h"
#include "../player/player.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/sprite_manager.h"
#include "../core/camera.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../objects/goldpiece.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

cBonusBox :: cBonusBox( float x, float y )
: cBaseBox( x, y )
{
	Init();
}

cBonusBox :: cBonusBox( XMLAttributes &attributes )
: cBaseBox()
{
	Init();
	Create_from_Stream( attributes );
}

cBonusBox :: ~cBonusBox( void )
{
	for( MovingSpriteList::iterator itr = active_items.begin(), itr_end = active_items.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	active_items.clear();
}

void cBonusBox :: Init( void )
{
	type = TYPE_BONUSBOX;
	force_best_item = 0;
	player_range = 5000;
	counter = 1;

	Set_Animation( "Bonus" );
	gold_color = COL_DEFAULT;
	Set_Goldcolor( COL_YELLOW );

	Create_Name();
}

cBonusBox *cBonusBox :: Copy( void )
{
	cBonusBox *bonusbox = new cBonusBox( startposx, startposy );
	bonusbox->Set_Animation( anim_type );
	bonusbox->Set_Bonus_Type( box_type );
	bonusbox->Set_Invisible( box_invisible );
	bonusbox->Set_Force_best_item( force_best_item );
	bonusbox->Set_Goldcolor( gold_color );
	bonusbox->Set_UseableCount( start_useable_count, 1 );

	return bonusbox;
}

void cBonusBox :: Create_from_Stream( XMLAttributes &attributes )
{
	cBaseBox::Create_from_Stream( attributes );

	// item
	Set_Bonus_Type( (SpriteType)attributes.getValueAsInteger( "item" ) );
	// force best possible item
	Set_Force_best_item( attributes.getValueAsBool( "force_best_item" ) );
	// gold color
	if( box_type == TYPE_GOLDPIECE )
	{
		Set_Goldcolor( Get_Color_id( attributes.getValueAsString( "gold_color", Get_Color_name( gold_color ) ).c_str() ) );
	}
}

void cBonusBox :: Save_to_Stream( ofstream &file )
{
	// begin box
	file << "\t<box>" << std::endl;

	cBaseBox::Save_to_Stream( file );

	// force best possible item
	file << "\t\t<Property name=\"force_best_item\" value=\"" << force_best_item << "\" />" << std::endl;
	// gold color
	if( box_type == TYPE_GOLDPIECE )
	{
		file << "\t\t<Property name=\"gold_color\" value=\"" << Get_Color_name( gold_color ) << "\" />" << std::endl;
	}

	// end box
	file << "\t</box>" << std::endl;
}

void cBonusBox :: Set_UseableCount( int count, bool new_startcount /* = 0 */ )
{
	cBaseBox::Set_UseableCount( count, new_startcount );

	// disable
	if( !useable_count )
	{
		Set_Image( 0, 0, 0 );

		// invisible box
		if( box_invisible )
		{
			// keep empty start/editor image
			start_image = NULL;
		}
	}

	Update_valid_update();
}

void cBonusBox :: Set_Bonus_Type( SpriteType nbonus_type )
{
	// already set
	if( box_type == nbonus_type )
	{
		return;
	}

	box_type = nbonus_type;

	// set item image
	if( box_type == TYPE_UNDEFINED )
	{
		item_image = NULL;
	}
	else if( box_type == TYPE_POWERUP )
	{
		// force always best item
		force_best_item = 1;
		item_image = pVideo->Get_Surface( "game/editor/unknown.png" );
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_red.png" );
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		item_image = pVideo->Get_Surface( "game/items/fire_flower.png" );
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_blue.png" );
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_ghost.png" );
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_green.png" );
	}
	else if( box_type == TYPE_JSTAR )
	{
		item_image = pVideo->Get_Surface( "game/items/star.png" );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		if( gold_color == COL_RED )
		{
			item_image = pVideo->Get_Surface( "game/items/goldpiece/red/1.png" );
		}
		else
		{
			item_image = pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" );
		}
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		item_image = pVideo->Get_Surface( "game/items/mushroom_poison.png" );
	}
	else
	{
		item_image = NULL;
		printf( "Error : Unknown BonusBox Item type : %d\n", box_type );
	}

	// recreate name
	Create_Name();
}

void cBonusBox :: Set_Force_best_item( bool enable )
{
	// can't be set if random 
	if( box_type == TYPE_POWERUP )
	{
		return;
	}

	force_best_item = enable;
}

void cBonusBox :: Set_Goldcolor( DefaultColor ncolor )
{
	// already set
	if( gold_color == ncolor )
	{
		return;
	}

	if( ncolor == COL_DEFAULT )
	{
		gold_color = COL_YELLOW;
	}

	gold_color = ncolor;

	if( box_type != TYPE_GOLDPIECE )
	{
		return;
	}

	if( gold_color == COL_YELLOW )
	{
		item_image = pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" );
		name = "Box Goldpiece Yellow";
	}
	else if( gold_color == COL_RED )
	{
		item_image = pVideo->Get_Surface( "game/items/goldpiece/red/1.png" );
		name = "Box Goldpiece Red";
	}
	else
	{
		printf( "Warning : Unknown Bonusbox Gold Color %d\n", gold_color );
	}
}

void cBonusBox :: Activate( void )
{
	// boxed item
	cMovingSprite *box_item = NULL;

	bool random = 0;

	// random
	if( box_type == TYPE_POWERUP )
	{
		int r = rand() % 5;

		if( r == 0 )
		{
			box_type = TYPE_MUSHROOM_DEFAULT;
		}
		else if( r == 1 )
		{
			box_type = TYPE_FIREPLANT;
		}
		else if( r == 2 )
		{
			box_type = TYPE_MUSHROOM_BLUE;
		}
		else if( r == 3 )
		{
			box_type = TYPE_MUSHROOM_GHOST;
		}
		else if( r == 4 )
		{
			box_type = TYPE_JSTAR;
		}

		random = 1;
	}

	// no item
	if( box_type == TYPE_UNDEFINED )
	{
		pAudio->PlaySound( "item/death_box.ogg" );
	}
	// check if lower item should be used if no force best item
	else if( !force_best_item && ( box_type == TYPE_FIREPLANT || box_type == TYPE_MUSHROOM_BLUE ) && 
		( pPlayer->maryo_type == MARYO_SMALL || ( ( pPlayer->maryo_type == MARYO_FIRE || pPlayer->maryo_type == MARYO_ICE ) && !Itembox->item_id ) ) )
	{
		pAudio->PlaySound( "sprout_1.ogg" );

		cMushroom *mushroom = new cMushroom( startposx - ( ( item_image->w - rect.w ) / 2 ), startposy - ( ( item_image->h - rect.h ) / 2 ) );
		box_item = static_cast<cMovingSprite *>(mushroom);
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		pAudio->PlaySound( "sprout_1.ogg" );
		box_item = static_cast<cMovingSprite *>(new cFirePlant( startposx - ( ( item_image->w - rect.w ) / 2 ), startposy ));
	}
	else if( box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON || box_type == TYPE_MUSHROOM_BLUE || box_type == TYPE_MUSHROOM_GHOST )
	{
		pAudio->PlaySound( "sprout_1.ogg" );

		cMushroom *mushroom = new cMushroom( startposx - ( ( item_image->w - rect.w ) / 2 ), startposy - ( ( item_image->h - rect.h ) / 2 ) );
		mushroom->Set_Type( box_type );
		box_item = static_cast<cMovingSprite *>(mushroom);
	}
	else if( box_type == TYPE_JSTAR )
	{
		pAudio->PlaySound( "sprout_1.ogg" );
		cjStar *star = new cjStar( startposx - ( ( item_image->w - rect.w ) / 2 ), startposy );
		star->Set_onTop( this );
		star->spawned = 1;
		// add to global objects
		pSprite_Manager->Add( star );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		pAudio->PlaySound( "item/goldpiece_1.ogg" );

		cJGoldpiece *goldpiece = new cJGoldpiece( startposx - ( ( item_image->w - rect.w ) / 2 ), posy );
		goldpiece->Set_Goldcolor( gold_color );
		// add to global objects
		pSprite_Manager->Add( goldpiece );
	}
	else
	{
		printf( "Error : Unknown bonusbox item type %d\n", box_type );
		return;
	}

	// set back random state
	if( random )
	{
		box_type = TYPE_POWERUP;
	}

	if( box_item )
	{
		// set posz behind box
		box_item->posz = posz - 0.000001f;
		// set spawned
		box_item->spawned = 1;

		// add to item list
		active_items.push_back( box_item );
		Update_valid_update();
	}
}

void cBonusBox :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	// update active items
	for( MovingSpriteList::iterator itr = active_items.begin(); itr != active_items.end(); )
	{
		cPowerUp *powerup = static_cast<cPowerUp *>(*itr);

		if( !powerup->visible )
		{
			++itr;
			continue;
		}

		// position over the box reached
		if( powerup->posy < posy - powerup->col_rect.h - powerup->col_pos.y )
		{
			// clear animation counter
			powerup->counter = 0;

			// set powerup default posz
			powerup->posz = 0.05f;

			// set the item on top
			powerup->Set_onTop( this, 0 );
			// add the item to the level objects
			pSprite_Manager->Add( powerup );

			// remove from array
			itr = active_items.erase( itr );
			Update_valid_update();
		}
		// move upwards
		else
		{
			powerup->counter += pFramerate->speedfactor;
			powerup->Move( 0, -3.1f );

			++itr;
		}
	}

	cBaseBox::Update();
}

void cBonusBox :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	for( MovingSpriteList::iterator itr = active_items.begin(), itr_end = active_items.end(); itr != itr_end; ++itr )
	{
		cMovingSprite *obj = (*itr);

		if( !obj->visible )
		{
			continue;
		}
		
		obj->Draw();
	}

	cBaseBox::Draw( request );
}

bool cBonusBox :: is_Update_valid( void )
{
	if( active_items.size() )
	{
		return 1;
	}

	return cBaseBox::is_Update_valid();
}

void cBonusBox :: Editor_Activate( void )
{
	// BaseBox Settings first
	cBaseBox::Editor_Activate();

	WindowManager &wmgr = WindowManager::getSingleton();

	// Animation
	Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_animation" ));
	combobox->getEditbox()->setTooltipText( "Animation" );
	Editor_Add( combobox, 120, 100 );

	combobox->addItem( new ListboxTextItem( "Default" ) );
	combobox->addItem( new ListboxTextItem( "Bonus" ) );
	combobox->addItem( new ListboxTextItem( "Power" ) );

	combobox->setText( anim_type.c_str() );

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cBonusBox::Editor_Animation_Select, this ) );

	// Item
	combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_item" ));
	combobox->getEditbox()->setTooltipText( "Item" );
	Editor_Add( combobox, 160, 140 );

	combobox->addItem( new ListboxTextItem( "Empty" ) );
	combobox->addItem( new ListboxTextItem( "Random" ) );
	combobox->addItem( new ListboxTextItem( "Mushroom" ) );
	combobox->addItem( new ListboxTextItem( "Fireplant" ) );
	combobox->addItem( new ListboxTextItem( "Mushroom Blue" ) );
	combobox->addItem( new ListboxTextItem( "Mushroom Ghost" ) );
	combobox->addItem( new ListboxTextItem( "Mushroom 1-UP" ) );
	combobox->addItem( new ListboxTextItem( "Star" ) );
	combobox->addItem( new ListboxTextItem( "Goldpiece" ) );
	combobox->addItem( new ListboxTextItem( "Mushroom Poison" ) );

	if( box_type == TYPE_MUSHROOM_DEFAULT )
	{
		combobox->setText( "Mushroom" );
	}
	else if( box_type == TYPE_FIREPLANT )
	{
		combobox->setText( "Fireplant" );
	}
	else if( box_type == TYPE_MUSHROOM_BLUE )
	{
		combobox->setText( "Mushroom Blue" );
	}
	else if( box_type == TYPE_MUSHROOM_GHOST )
	{
		combobox->setText( "Mushroom Ghost" );
	}
	else if( box_type == TYPE_MUSHROOM_LIVE_1 )
	{
		combobox->setText( "Mushroom 1-UP" );
	}
	else if( box_type == TYPE_JSTAR )
	{
		combobox->setText( "Star" );
	}
	else if( box_type == TYPE_GOLDPIECE )
	{
		combobox->setText( "Goldpiece" );
	}
	else if( box_type == TYPE_MUSHROOM_POISON )
	{
		combobox->setText( "Mushroom Poison" );
	}
	else if( box_type == TYPE_POWERUP )
	{
		combobox->setText( "Random" );
	}
	else
	{
		combobox->setText( "Empty" );
	}

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cBonusBox::Editor_Item_Select, this ) );

	// Force best item
	combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_force_best_item" ));
	combobox->getEditbox()->setTooltipText( "Force best Item" );
	Editor_Add( combobox, 120, 80 );

	combobox->addItem( new ListboxTextItem( "Enabled" ) );
	combobox->addItem( new ListboxTextItem( "Disabled" ) );

	if( force_best_item )
	{
		combobox->setText( "Enabled" );
	}
	else
	{
		combobox->setText( "Disabled" );
	}


	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cBonusBox::Editor_Force_best_item_Select, this ) );

	// gold color
	combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "editor_bonusbox_gold_color" ));
	combobox->getEditbox()->setTooltipText( "Gold Color" );
	Editor_Add( combobox, 100, 80 );

	combobox->addItem( new ListboxTextItem( "yellow" ) );
	combobox->addItem( new ListboxTextItem( "red" ) );
	combobox->setText( Get_Color_name( gold_color ) );

	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cBonusBox::Editor_Gold_Color_Select, this ) );

	// set state
	Editor_state_update();
	// set position
	Editor_pos_update();
}

void cBonusBox :: Editor_state_update( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// Force best item
	Combobox *combobox = static_cast<Combobox *>(wmgr.getWindow( "editor_bonusbox_force_best_item" ));

	if( box_type == TYPE_UNDEFINED || box_type == TYPE_POWERUP || box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON || 
		box_type == TYPE_MUSHROOM_GHOST || box_type == TYPE_JSTAR || box_type == TYPE_GOLDPIECE )
	{
		combobox->setEnabled( 0 );
	}
	else
	{
		combobox->setEnabled( 1 );
	}

	// gold color
	combobox = static_cast<Combobox *>(wmgr.getWindow( "editor_bonusbox_gold_color" ));

	if( box_type != TYPE_GOLDPIECE )
	{
		combobox->setEnabled( 0 );
	}
	else
	{
		combobox->setEnabled( 1 );
	}
}

bool cBonusBox :: Editor_Animation_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Animation( item->getText().c_str() );

	return 1;
}

bool cBonusBox :: Editor_Item_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( "Mushroom" ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_DEFAULT );
	}
	else if( item->getText().compare( "Fireplant" ) == 0 )
	{
		Set_Bonus_Type( TYPE_FIREPLANT );
	}
	else if( item->getText().compare( "Mushroom Blue" ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_BLUE );
	}
	else if( item->getText().compare( "Mushroom Ghost" ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_GHOST );
	}
	else if( item->getText().compare( "Mushroom 1-UP" ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_LIVE_1 );
	}
	else if( item->getText().compare( "Star" ) == 0 )
	{
		Set_Bonus_Type( TYPE_JSTAR );
	}
	else if( item->getText().compare( "Goldpiece" ) == 0 )
	{
		Set_Bonus_Type( TYPE_GOLDPIECE );
	}
	else if( item->getText().compare( "Mushroom Poison" ) == 0 )
	{
		Set_Bonus_Type( TYPE_MUSHROOM_POISON );
	}
	else if( item->getText().compare( "Random" ) == 0 )
	{
		Set_Bonus_Type( TYPE_POWERUP );
	}
	else
	{
		Set_Bonus_Type( TYPE_UNDEFINED );
	}

	Editor_state_update();

	return 1;
}

bool cBonusBox :: Editor_Force_best_item_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	if( item->getText().compare( "Enabled" ) == 0 )
	{
		Set_Force_best_item( 1 );
	}
	else
	{
		Set_Force_best_item( 0 );
	}

	return 1;
}

bool cBonusBox :: Editor_Gold_Color_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Goldcolor( Get_Color_id( item->getText().c_str() ) );

	return 1;
}
