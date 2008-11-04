/***************************************************************************
 * text_box.cpp  -  box speaking to you
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

#include "../objects/text_box.h"
#include "../core/obj_manager.h"
#include "../core/framerate.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../core/main.h"
#include "../input/keyboard.h"
#include "../core/i18n.h"

/* *** *** *** *** *** *** *** *** cText_Box *** *** *** *** *** *** *** *** *** */

cText_Box :: cText_Box( float x, float y )
: cBaseBox( x, y )
{
	cText_Box::Init();
}

cText_Box :: cText_Box( CEGUI::XMLAttributes &attributes )
: cBaseBox()
{
	cText_Box::Init();
	cText_Box::Create_from_Stream( attributes );
}

cText_Box :: ~cText_Box( void )
{

}

void cText_Box :: Init( void )
{
	type = TYPE_TEXT_BOX;
	box_type = type;

	// default is infinite times activate-able
	Set_Useable_Count( -1, 1 );
	// Spinbox Animation
	Set_Animation( "Default" );

	// todo : editor image needed
	//item_image = NULL;

	Create_Name();
}

cText_Box *cText_Box :: Copy( void )
{
	cText_Box *text_box = new cText_Box( startposx, startposy );
	text_box->Set_Text( text );
	text_box->Set_Invisible( box_invisible );

	return text_box;
}

void cText_Box :: Create_from_Stream( CEGUI::XMLAttributes &attributes )
{
	cBaseBox::Create_from_Stream( attributes );

	// text
	Set_Text( xml_string_to_string( attributes.getValueAsString( "text" ).c_str() ) );
}

void cText_Box :: Save_to_Stream( ofstream &file )
{
	// begin box
	file << "\t<box>" << std::endl;

	cBaseBox::Save_to_Stream( file );

	// text
	file << "\t\t<Property name=\"text\" value=\"" << string_to_xml_string( text ) << "\" />" << std::endl;

	// end box
	file << "\t</box>" << std::endl;
}

void cText_Box :: Activate( void )
{
	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
	CEGUI::MultiLineEditbox *editbox = static_cast<CEGUI::MultiLineEditbox *>(wmgr.createWindow( "TaharezLook/MultiLineEditbox", "text_box_text" ));

	// add to main window
	pGuiSystem->getGUISheet()->addChildWindow( editbox );


	// set on top
	editbox->setAlwaysOnTop( 1 );
	// set position
	float text_pos_x = posx - 150 + ( rect.w * 0.5f );
	float text_pos_y = posy - 5 - 200;
	editbox->setXPosition( CEGUI::UDim( 0, ( text_pos_x - pActive_Camera->x ) * global_upscalex ) );
	editbox->setYPosition( CEGUI::UDim( 0, ( text_pos_y - pActive_Camera->y ) * global_upscaley ) );
	// set size
	editbox->setWidth( CEGUI::UDim( 0, 300 * global_upscalex ) );
	editbox->setHeight( CEGUI::UDim( 0, 200 * global_upscaley ) );

	// set text
	editbox->setText( reinterpret_cast<const CEGUI::utf8*>(text.c_str()) );
	// always hide horizontal scrollbar
	editbox->getHorzScrollbar()->hide();

	bool display = 1;

	while( display )
	{
		while( SDL_PollEvent( &input_event ) )
		{
			if( input_event.type == SDL_KEYDOWN )
			{
				pKeyboard->keys[input_event.key.keysym.sym] = 1;

				// exit keys
				if( input_event.key.keysym.sym == pPreferences->key_action || input_event.key.keysym.sym == SDLK_ESCAPE || input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_SPACE )
				{
					display = 0;
					break;
				}
			}
			else if( input_event.type == SDL_JOYBUTTONDOWN )
			{
				pJoystick->Set_Button( input_event.jbutton.button, 1 );

				if( input_event.jbutton.button == pPreferences->joy_button_action || input_event.jbutton.button == pPreferences->joy_button_exit )
				{
					display = 0;
					break;
				}
			}
			else if( input_event.type == SDL_KEYUP )
			{
				pKeyboard->keys[input_event.key.keysym.sym] = 0;
			}
			else if( input_event.type == SDL_JOYBUTTONUP )
			{
				pJoystick->Set_Button( input_event.jbutton.button, 0 );
			}
		}

		Uint8 *keys = SDL_GetKeyState( NULL );
		Sint16 joy_ver_axis = 0;

		// if joystick enabled
		if( pPreferences->joy_enabled )
		{
			joy_ver_axis = SDL_JoystickGetAxis( pJoystick->joystick, pPreferences->joy_axis_ver );
		}

		// down
		if( keys[pPreferences->key_down] || joy_ver_axis > pPreferences->joy_axis_threshold )
		{
			editbox->getVertScrollbar()->setScrollPosition( editbox->getVertScrollbar()->getScrollPosition() + ( editbox->getVertScrollbar()->getStepSize() * 0.25f * pFramerate->speedfactor ) );
		}
		// up
		if( keys[pPreferences->key_up] || joy_ver_axis < -pPreferences->joy_axis_threshold )
		{
			editbox->getVertScrollbar()->setScrollPosition( editbox->getVertScrollbar()->getScrollPosition() - ( editbox->getVertScrollbar()->getStepSize() * 0.25f * pFramerate->speedfactor ) );
		}

		// move camera because text could not be completely visible
		if( pActive_Camera->y_offset > 0 )
		{
			pActive_Camera->y_offset -= 2;
			// set position
			pActive_Camera->Center();

			// set position
			editbox->setXPosition( CEGUI::UDim( 0, ( text_pos_x - pActive_Camera->x ) * global_upscalex ) );
			editbox->setYPosition( CEGUI::UDim( 0, ( text_pos_y - pActive_Camera->y ) * global_upscaley ) );
		}

		// update animation
		Update();
		
		// draw
		Draw_Game();
		// render
		pVideo->Render();
		pFramerate->Update();
	}

	wmgr.destroyWindow( editbox );
}

void cText_Box :: Update( void )
{
	if( !valid_update || !is_Player_range() )
	{
		return;
	}

	cBaseBox::Update();
}

void cText_Box :: Set_Text( string str_text )
{
	text = str_text;
}

void cText_Box :: Editor_Activate( void )
{
	// BaseBox Settings first
	cBaseBox::Editor_Activate();

	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

	// text
	CEGUI::MultiLineEditbox *editbox = static_cast<CEGUI::MultiLineEditbox *>(wmgr.createWindow( "TaharezLook/MultiLineEditbox", "text_box_text" ));
	Editor_Add( UTF8_("Text"), UTF8_("Text to display when activated"), editbox, 300, 200 );

	editbox->setText( reinterpret_cast<const CEGUI::utf8*>(text.c_str()) );
	editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventKeyUp, CEGUI::Event::Subscriber( &cText_Box::Editor_Text_Key, this ) );

	// init
	Editor_Init();
}

bool cText_Box :: Editor_Text_Key( const CEGUI::EventArgs &event )
{
	const CEGUI::WindowEventArgs &windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>( event );
	string str_text = static_cast<CEGUI::MultiLineEditbox *>( windowEventArgs.window )->getText().c_str();

	Set_Text( str_text );

	return 1;
}
