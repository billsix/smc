/***************************************************************************
 * level_entry.cpp  -  entry point to enter a level
 *
 * Copyright (C) 2007 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/level_entry.h"
#include "../player/player.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/main.h"
#include "../video/gl_surface.h"
#include "../video/font.h"
#include "../video/renderer.h"
#include "../level/level.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Entry :: cLevel_Entry( float x, float y )
: cImageObjectSprite( x, y )
{
	cLevel_Entry::Init();
}

cLevel_Entry :: cLevel_Entry( CEGUI::XMLAttributes &attributes )
: cImageObjectSprite()
{
	cLevel_Entry::Init();
	cLevel_Entry::Create_from_Stream( attributes );
}

cLevel_Entry :: ~cLevel_Entry( void )
{
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}
}

void cLevel_Entry :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_LEVEL_ENTRY;
	massivetype = MASS_PASSIVE;
	editor_posz = 0.111f;
	player_range = 1000;

	rect.w = 10;
	rect.h = 20;
	col_rect.w = rect.w;
	col_rect.h = rect.h;
	start_rect.w = rect.w;
	start_rect.h = rect.h;

	Set_Direction( DIR_UP );

	editor_color = lightblue;
	editor_color.alpha = 128;

	editor_entry_name = NULL;
}

cLevel_Entry *cLevel_Entry :: Copy( void )
{
	cLevel_Entry *level_entry = new cLevel_Entry( startposx, startposy );
	level_entry->Set_Direction( start_direction );
	level_entry->Set_Name( entry_name );

	return level_entry;
}

void cLevel_Entry :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	// position
	Set_Pos( static_cast<float>(attributes.getValueAsInteger( "posx" )), static_cast<float>(attributes.getValueAsInteger( "posy" )), 1 );
	// name
	Set_Name( attributes.getValueAsString( "name" ).c_str() );
	// direction
	Set_Direction( Get_Direction_Id( attributes.getValueAsString( "direction", Get_Direction_Name( start_direction ) ).c_str() ) );
}

void cLevel_Entry :: Save_to_Stream( ofstream &file )
{
	// begin level_entry
	file << "\t<level_entry>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << static_cast<int>(startposx) << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << static_cast<int>(startposy) << "\" />" << std::endl;
	// direction
	file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_Name( start_direction ) << "\" />" << std::endl;
	// name
	if( !entry_name.empty() )
	{
		file << "\t\t<Property name=\"name\" value=\"" << string_to_xml_string( entry_name ) << "\" />" << std::endl;
	}

	// end level_entry
	file << "\t</level_entry>" << std::endl;
}

void cLevel_Entry :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	cImageObjectSprite::Set_Direction( dir, 1 );

	Create_Name();
}

void cLevel_Entry :: Create_Name( void )
{
    name = _("Level Entry");

	if( direction == DIR_UP )
	{
		name += " U";
	}
	else if( direction == DIR_LEFT )
	{
		name += " L";
	}
	else if( direction == DIR_DOWN )
	{
		name += " D";
	}
	else if( direction == DIR_RIGHT )
	{
		name += " R";
	}
}

void cLevel_Entry :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// draw color rect
	pVideo->Draw_Rect( col_rect.x - pActive_Camera->x, col_rect.y - pActive_Camera->y, col_rect.w, col_rect.h, editor_posz, &editor_color );

	// draw entry name
	if( editor_entry_name )
	{
		// create request
		cSurfaceRequest *surface_request = new cSurfaceRequest();
		// blit
		editor_entry_name->Blit( col_rect.x + col_rect.w + 5 - pActive_Camera->x, col_rect.y - pActive_Camera->y, editor_posz, surface_request );
		surface_request->shadow_pos = 2;
		surface_request->shadow_color = lightgreyalpha64;
		// add request
		pRenderer->Add( surface_request );
	}
}

void cLevel_Entry :: Activate( void )
{
	pAudio->Play_Sound( "leave_pipe.ogg" );
	
	pPlayer->Stop_Ducking();
	pPlayer->Reset_on_Ground();

	// set position
	pPlayer->Set_Pos( Get_Player_Pos_X(), Get_Player_Pos_Y() );
	// set image
	if( direction == DIR_UP || direction == DIR_DOWN )
	{
		pPlayer->Set_Image( MARYO_IMG_FALL + pPlayer->direction );
	}
	else if( direction == DIR_LEFT || direction == DIR_RIGHT )
	{
		pPlayer->Set_Image( pPlayer->direction );

		// set rotation
		if( direction == DIR_RIGHT )
		{
			pPlayer->Set_Rotation_Z( 90 );
		}
		else if( direction == DIR_LEFT )
		{
			pPlayer->Set_Rotation_Z( 270 );
		}
	}

	// change position z to be before massive for the animation
	float player_posz = pPlayer->posz;
	pPlayer->posz = 0.0799f;

	// move slowly
	for( float i = 0; i < ( speedfactor_fps * 0.9f ); i += pFramerate->speedfactor )
	{
		if( direction == DIR_DOWN )
		{
			pPlayer->Move( 0, 2.8f );
		}
		else if( direction == DIR_UP )
		{
			pPlayer->Move( 0, -2.8f );
		}
		else if( direction == DIR_RIGHT )
		{
			pPlayer->Move( 2, 0 );
		}
		else if( direction == DIR_LEFT )
		{
			pPlayer->Move( -2, 0 );
		}

		// center camera
		pActive_Camera->Center();
		// keep global effect particles on screen
		pActive_Level->pGlobal_effect->Update_Particles();
		// draw
		Draw_Game();

		pVideo->Render();
		pFramerate->Update();
	}

	// set position z back
	pPlayer->posz = player_posz;

	if( direction == DIR_RIGHT || direction == DIR_LEFT )
	{
		pPlayer->Set_Rotation_Z( 0 );
	}

	pPlayer->Collisions_Clear();
}

float cLevel_Entry :: Get_Player_Pos_X( void )
{
	// left
	if( direction == DIR_LEFT )
	{
		return col_rect.x - pPlayer->col_pos.y + col_rect.w;
	}
	// right
	else if( direction == DIR_RIGHT )
	{
		return col_rect.x - pPlayer->col_pos.y - col_rect.w - pPlayer->col_rect.w;
	}

	// up/down
	return col_rect.x - pPlayer->col_pos.x + ( col_rect.w * 0.5f ) - ( pPlayer->col_rect.w * 0.5f );
}

float cLevel_Entry :: Get_Player_Pos_Y( void )
{
	// up
	if( direction == DIR_UP )
	{
		return col_rect.y - pPlayer->col_pos.y + col_rect.h;
	}
	// down
	else if( direction == DIR_DOWN )
	{
		return col_rect.y - pPlayer->col_pos.y - 5 - pPlayer->rect.h;
	}

	// left/right
	return col_rect.y - pPlayer->col_pos.y + ( col_rect.h * 0.5f ) - ( pPlayer->col_rect.h * 0.5f );
}

void cLevel_Entry :: Set_Name( string str_name )
{
	// delete editor image
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}

	// Set new name
	entry_name = str_name;

	// if empty don't create editor image
	if( entry_name.empty() )
	{
		return;
	}

	editor_entry_name = pFont->Render_Text( pFont->font_small, entry_name, white );
}

bool cLevel_Entry :: Is_Draw_Valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		return 0;
	}

	// not visible on the screen
	if( !visible || !Is_Visible_on_Screen() )
	{
		return 0;
	}

	return 1;
}

void cLevel_Entry :: Editor_Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// direction
	CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "level_entry_direction" ));
	Editor_Add( UTF8_("Direction"), UTF8_("The direction to come out"), combobox, 100, 105 );

	combobox->addItem( new CEGUI::ListboxTextItem( "up" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "down" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "right" ) );
	combobox->addItem( new CEGUI::ListboxTextItem( "left" ) );
	combobox->setText( Get_Direction_Name( start_direction ) );

	combobox->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &cLevel_Entry::Editor_Direction_Select, this ) );

	// destination entry
	CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_entry_name" ));
	Editor_Add( UTF8_("Name"), UTF8_("Name for identification"), editbox, 150 );

	editbox->setText( entry_name.c_str() );
	editbox->subscribeEvent( CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber( &cLevel_Entry::Editor_Name_Key, this ) );

	// init
	Editor_Init();
}

bool cLevel_Entry :: Editor_Direction_Select( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	CEGUI::ListboxItem *item = static_cast<CEGUI::Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_Id( item->getText().c_str() ) );

	return 1;
}

bool cLevel_Entry :: Editor_Name_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Name( str_text );

	return 1;
}
