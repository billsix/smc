/***************************************************************************
			game_core.cpp  -  globally used variables and functions
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

#include "../core/game_core.h"
#include "../core/main.h"
#include "../video/font.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/camera.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../player/player.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../level/level.h"

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

bool done = 0;

GameMode Game_Mode = MODE_NOTHING;
GameModeType Game_Mode_Type = MODE_TYPE_DEFAULT;
GameAction Game_Action = GA_NONE;
string Game_Action_Data;
void *Game_Action_ptr = NULL;
bool Game_debug = 0;

bool editor_enabled = 0;
bool editor_level_enabled = 0;
bool editor_world_enabled = 0;

float global_scalex = 1, global_scaley = 1;

SDL_Event input_event;

/* non digit class
 * returns true if not a number was found
 * used by the valid_number function
*/
class nondigit
{
public:
	bool operator() (char c) { return !isdigit( c ); }
};

/* *** *** *** *** *** *** *** cDialogBox *** *** *** *** *** *** *** *** *** *** */

cDialogBox :: cDialogBox( void )
{
	mouse_hide = 0;
}

cDialogBox :: ~cDialogBox( void )
{

}

void cDialogBox :: Init( void )
{
	// load layout
	window = WindowManager::getSingleton().loadWindowLayout( layout_file );
	pGuiSystem->getGUISheet()->addChildWindow( window );

	// hide mouse on exit
	if( !pMouseCursor->visible )
	{
		mouse_hide = 1;
		pMouseCursor->visible = 1;
	}
}

void cDialogBox :: Exit( void )
{
	// hide mouse
	if( mouse_hide )
	{
		pMouseCursor->Set_Visible( 0 );
	}

	pGuiSystem->getGUISheet()->removeChildWindow( window );
	WindowManager::getSingleton().destroyWindow( window );
}

void cDialogBox :: Draw( void )
{
	Draw_Game();
	pVideo->Draw_Gradient( NULL, 0.905f, &whitealpha128, &black, DIR_VERTICAL );

	pVideo->Render();

	pMouseCursor->Update_Position();
}

void cDialogBox :: Update( void )
{
	pFramerate->Update();
	Gui_handle_Time();
}

/* *** *** *** *** *** *** *** cDialogBox_Text *** *** *** *** *** *** *** *** *** *** */

cDialogBox_Text :: cDialogBox_Text( void )
: cDialogBox()
{
	layout_file = "box_text.layout";
}

cDialogBox_Text :: ~cDialogBox_Text( void )
{

}

void cDialogBox_Text :: Init( string title_text )
{
	cDialogBox::Init();

	// get window
	FrameWindow *box_window = static_cast<FrameWindow *>(WindowManager::getSingleton().getWindow( "box_text_window" ));
	box_window->setText( title_text );
	box_window->setSizingEnabled( 0 );
	box_window->getCloseButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Text::Button_window_quit_clicked, this ) );

	// get editbox
	box_editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "box_text_editbox" ));
}

string cDialogBox_Text :: Enter( string default_text, string title_text, bool auto_no_text /* = 1 */ )
{
	Init( title_text );

	box_editbox->setText( default_text );
	box_editbox->setTooltipText( title_text );
	box_editbox->activate();
	box_editbox->setCaratIndex( default_text.length() );

	finished = 0;

	while( !finished )
	{
		while( SDL_PollEvent( &input_event ) )
		{
			if( input_event.type == SDL_KEYDOWN )
			{
				if( auto_no_text && default_text.compare( box_editbox->getText().c_str() ) == 0 )
				{
					box_editbox->setText( "" );
					// only 1 time
					auto_no_text = 0;
				}

				if( input_event.key.keysym.sym == SDLK_ESCAPE )
				{
					box_editbox->setText( "" );
					finished = 1;
				}
				else if( input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_KP_ENTER )
				{
					finished = 1;
				}
				else
				{
					pKeyboard->CEGUI_Handle_Key_Down( input_event.key.keysym.sym );
				}
			}
			else if( input_event.type == SDL_KEYUP )
			{
				pKeyboard->CEGUI_Handle_Key_Up( input_event.key.keysym.sym );
			}
			else
			{
				pMouseCursor->Handle_Event( &input_event );
			}
		}

		Update();
		Draw();
	}

	string return_string = box_editbox->getText().c_str();

	Exit();
	return return_string;
}

bool cDialogBox_Text :: Button_window_quit_clicked( const EventArgs &event )
{
	box_editbox->setText( "" );
	finished = 1;
	return 1;
}

/* *** *** *** *** *** *** *** cDialogBox_Question *** *** *** *** *** *** *** *** *** *** */

cDialogBox_Question :: cDialogBox_Question( void )
: cDialogBox()
{
	layout_file = "box_question.layout";
	return_value = -1;
}

cDialogBox_Question :: ~cDialogBox_Question( void )
{

}

void cDialogBox_Question :: Init( bool with_cancel )
{
	cDialogBox::Init();

	// get window
	FrameWindow *box_window = static_cast<FrameWindow *>(WindowManager::getSingleton().getWindow( "box_question_window" ));
	box_window->setSizingEnabled( 0 );
	// subscribe close button
	if( with_cancel )
	{
		box_window->getCloseButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Question::Button_cancel_clicked, this ) );
	}
	else
	{
		box_window->getCloseButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Question::Button_no_clicked, this ) );
	}
}

int cDialogBox_Question :: Enter( string text, bool with_cancel /* = 0 */ )
{
	Init( with_cancel );

	// get text
	Editbox *box_text = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "box_question_text" ));
	box_text->setText( text.c_str() );

	// Button Yes
	PushButton *button_yes = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "box_question_button_yes" ));
	// Button No
	PushButton *button_no = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "box_question_button_no" ));
	// Button Cancel
	PushButton *button_cancel = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "box_question_button_cancel" ));

	// if without cancel
	if( !with_cancel )
	{
		button_cancel->hide();
	}

	// events
	button_yes->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Question::Button_yes_clicked, this ) );
	button_no->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Question::Button_no_clicked, this ) );
	button_cancel->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cDialogBox_Question::Button_cancel_clicked, this ) );

	finished = 0;

	while( !finished )
	{
		Draw();

		while( SDL_PollEvent( &input_event ) )
		{
			if( input_event.type == SDL_KEYDOWN )
			{
				if( input_event.key.keysym.sym == SDLK_ESCAPE )
				{
					if( with_cancel )
					{
						return_value = -1;
					}
					else
					{
						return_value = 0;
					}

					finished = 1;
				}
				else if( input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_KP_ENTER )
				{
					return_value = 1;
					finished = 1;
				}
				else
				{
					pKeyboard->CEGUI_Handle_Key_Down( input_event.key.keysym.sym );
				}
			}
			else if( input_event.type == SDL_KEYUP )
			{
				pKeyboard->CEGUI_Handle_Key_Up( input_event.key.keysym.sym );
			}
			else
			{
				pMouseCursor->Handle_Event( &input_event );
			}
		}

		Update();
	}

	Exit();

	return return_value;
}

bool cDialogBox_Question :: Button_yes_clicked( const EventArgs &event )
{
	return_value = 1;
	finished = 1;
	return 1;
}

bool cDialogBox_Question :: Button_no_clicked( const EventArgs &event )
{
	return_value = 0;
	finished = 1;
	return 1;
}

bool cDialogBox_Question :: Button_cancel_clicked( const EventArgs &event )
{
	return_value = -1;
	finished = 1;
	return 1;
}

/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

void Change_Game_Mode( GameMode new_mode )
{
	// already set
	if( Game_Mode == new_mode )
	{
		return;
	}

	// remember old mode
	GameMode old_mode = Game_Mode;
	// set new mode first
	Game_Mode = new_mode;

	// mode was level
	if( old_mode == MODE_LEVEL )
	{
		// hide editor window if visible
		if( pLevel_Editor->enabled )
		{
			if( pLevel_Editor->editor_window->isVisible() )
			{
				pLevel_Editor->editor_window->hide();
			}
		}

		// level to overworld
		if( new_mode == MODE_OVERWORLD )
		{
			// update hud
			pHudManager->Update_Text();
		}
	}
	// mode was overworld
	else if( old_mode == MODE_OVERWORLD )
	{
		// hide editor window if visible
		if( pWorld_Editor->enabled )
		{
			if( pWorld_Editor->editor_window->isVisible() )
			{
				pWorld_Editor->editor_window->hide();
			}
		}

		// overworld to level
		if( new_mode == MODE_LEVEL )
		{
			// update hud
			pHudManager->Update_Text();

			// fade out music
			pAudio->FadeOutMusic( 1000 );

			// reset speedfactor
			pFramerate->Reset();

			// center camera position
			pCamera->Center();

			// clear input
			ClearInputEvents();
		}
	}
	// mode was menu
	else if( old_mode == MODE_MENU )
	{
		// hide mouse
		if( !editor_enabled )
		{
			pMouseCursor->Set_Visible( 0 );
		}

		// menu to level
		if( new_mode == MODE_LEVEL )
		{
			pCamera->Center();
		}
		// menu to overworld
		else if( new_mode == MODE_OVERWORLD )
		{
			pHudManager->Update_Text();
		}
	}


	// mode gets level
	if( new_mode == MODE_LEVEL )
	{
		// play music
		if( pLevel->valid_music )
		{
			pAudio->PlayMusic( pLevel->musicfile, -1, 0, 1000 );
		}
		else if( pAudio->music_enabled )
		{
			printf( "Warning : Music file not found %s\n", pLevel->musicfile.c_str() );
		}

		// disable world editor
		pWorld_Editor->Disable();

		// set editor enabled state
		editor_enabled = pLevel_Editor->enabled;

		if( pLevel_Editor->enabled )
		{
			if( !pLevel_Editor->editor_window->isVisible() )
			{
				pLevel_Editor->editor_window->show();
				pMouseCursor->visible = 1;
			}
		}
	}
	// mode gets overworld
	else if( new_mode == MODE_OVERWORLD )
	{
		// reset custom level mode type
		if( Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM )
		{
			Game_Mode_Type = MODE_TYPE_DEFAULT;
		}

		// disable level editor
		pLevel_Editor->Disable();

		// set editor enabled state
		editor_enabled = pWorld_Editor->enabled;

		if( pWorld_Editor->enabled )
		{
			if( !pWorld_Editor->editor_window->isVisible() )
			{
				pWorld_Editor->editor_window->show();
				pMouseCursor->visible = 1;
			}
		}
	}
	// mode gets menu
	else if( new_mode == MODE_MENU )
	{
		editor_enabled = 0;

		// reset camera
		pCamera->Set_Pos( 0, 0 );
		// show mouse
		pMouseCursor->visible = 1;
		// position HUD
		pHudManager->Update_Text();
	}
	// mode gets settings
	else if( new_mode == MODE_LEVEL_SETTINGS )
	{
		editor_enabled = 0;

		if( pMouseCursor->active_object )
		{
			pMouseCursor->Clear_active_object();
		}
	}
}

cSprite *Copy_Object( cSprite *copy_object, float x, float y )
{
	if( !copy_object )
	{
		return NULL;
	}

	//  copy moving sprite
	cSprite *new_sprite = copy_object->Copy();

	// if not copyable object
	if( !new_sprite )
	{
		return NULL;
	}

	// set new position
	new_sprite->Set_Pos( x, y, 1 );

	return new_sprite;
}

string Time_to_String( time_t t, const char *format )
{
	char str_time[60];
	strftime( str_time, 60, format,localtime( &t ) );

	return str_time;
}

string Get_filename( string filename, bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	if( !with_dir && filename.find( "/" ) != string::npos ) 
	{
		filename.erase( 0, filename.rfind( "/" ) + 1 );
	}

	if( !with_end && filename.rfind( "." ) != string::npos ) 
	{
		filename.erase( filename.rfind( "." ) );
	}

	return filename;
}

void ClearInputEvents( void )
{
	while( SDL_PollEvent( &input_event ) )
	{
		// todo : Windowmanager quit events ?
		// ignore all events
	}

	// Reset keys
	pKeyboard->Reset_keys();
	pMouseCursor->Reset_keys();
	pJoystick->Reset_keys();
}

ObjectDirection Get_OppositeDirection( ObjectDirection direction )
{
	switch( direction )
	{
	case DIR_UP:
	{
		return DIR_DOWN;
	}
	case DIR_DOWN:
	{
		return DIR_UP;
	}
	case DIR_LEFT:
	{
		return DIR_RIGHT;
	}
	case DIR_RIGHT:
	{
		return DIR_LEFT;
	}
	case DIR_HORIZONTAL:
	{
		return DIR_VERTICAL;
	}
	case DIR_VERTICAL:
	{
		return DIR_HORIZONTAL;
	}
	default:
	    break;
	}

	return DIR_UNDEFINED;
}

string Get_Direction_name( ObjectDirection dir )
{
	switch( dir )
	{
	case DIR_UNDEFINED:
	{
		return "undefined";
	}
	case DIR_LEFT:
	{
		return "left";
	}
	case DIR_RIGHT:
	{
		return "right";
	}
	case DIR_UP:
	{
		return "up";
	}
	case DIR_DOWN:
	{
		return "down";
	}
	case DIR_TOP_LEFT:
	{
		return "top_left";
	}
	case DIR_TOP_RIGHT:
	{
		return "top_right";
	}
	case DIR_BOTTOM_LEFT:
	{
		return "bottom_left";
	}
	case DIR_BOTTOM_RIGHT:
	{
		return "bottom_right";
	}
	case DIR_LEFT_TOP:
	{
		return "left_top";
	}
	case DIR_LEFT_BOTTOM:
	{
		return "left_bottom";
	}
	case DIR_RIGHT_TOP:
	{
		return "right_top";
	}
	case DIR_RIGHT_BOTTOM:
	{
		return "right_bottom";
	}
	case DIR_HORIZONTAL:
	{
		return "horizontal";
	}
	case DIR_VERTICAL:
	{
		return "vertical";
	}
	case DIR_ALL:
	{
		return "all";
	}
	case DIR_FIRST:
	{
		return "first";
	}
	case DIR_LAST:
	{
		return "last";
	}
	default:
	{
	    break;
	}
	}

	return "";
}

ObjectDirection Get_Direction_id( string str_direction )
{
	if( str_direction.compare( "undefined" ) == 0 )
	{
		return DIR_UNDEFINED;
	}
	else if( str_direction.compare( "left" ) == 0 )
	{
		return DIR_LEFT;
	}
	else if( str_direction.compare( "right" ) == 0 )
	{
		return DIR_RIGHT;
	}
	else if( str_direction.compare( "up" ) == 0 )
	{
		return DIR_UP;
	}
	else if( str_direction.compare( "down" ) == 0 )
	{
		return DIR_DOWN;
	}
	else if( str_direction.compare( "top_left" ) == 0 )
	{
		return DIR_TOP_LEFT;
	}
	else if( str_direction.compare( "top_right" ) == 0 )
	{
		return DIR_TOP_RIGHT;
	}
	else if( str_direction.compare( "bottom_left" ) == 0 )
	{
		return DIR_BOTTOM_LEFT;
	}
	else if( str_direction.compare( "bottom_right" ) == 0 )
	{
		return DIR_BOTTOM_RIGHT;
	}
	else if( str_direction.compare( "left_top" ) == 0 )
	{
		return DIR_LEFT_TOP;
	}
	else if( str_direction.compare( "left_bottom" ) == 0 )
	{
		return DIR_LEFT_BOTTOM;
	}
	else if( str_direction.compare( "right_top" ) == 0 )
	{
		return DIR_RIGHT_TOP;
	}
	else if( str_direction.compare( "right_bottom" ) == 0 )
	{
		return DIR_RIGHT_BOTTOM;
	}
	else if( str_direction.compare( "horizontal" ) == 0 )
	{
		return DIR_HORIZONTAL;
	}
	else if( str_direction.compare( "vertical" ) == 0 )
	{
		return DIR_VERTICAL;
	}
	else if( str_direction.compare( "all" ) == 0 )
	{
		return DIR_ALL;
	}
	else if( str_direction.compare( "first" ) == 0 )
	{
		return DIR_FIRST;
	}
	else if( str_direction.compare( "last" ) == 0 )
	{
		return DIR_LAST;
	}

	return DIR_UNDEFINED;
}


SpriteType Get_Sprite_Type_id( string str_type )
{
	if( str_type.compare( "massive" ) == 0 )
	{
		return TYPE_MASSIVE;
	}
	else if( str_type.compare( "passive" ) == 0 )
	{
		return TYPE_PASSIVE;
	}
	else if( str_type.compare( "front_passive" ) == 0 )
	{
		return TYPE_FRONT_PASSIVE;
	}
	else if( str_type.compare( "halfmassive" ) == 0 )
	{
		return TYPE_HALFMASSIVE;
	}
	else if( str_type.compare( "climbable" ) == 0 )
	{
		return TYPE_CLIMBABLE;
	}
	else
	{
		printf( "Warning : Unknown Sprite Type String %s\n", str_type.c_str() );
	}
	
	return TYPE_UNDEFINED;
}

string Get_Massivetype_name( MassiveType mtype )
{
	switch( mtype )
	{
	case MASS_PASSIVE:
	{
		return "passive";
	}
	case MASS_MASSIVE:
	{
		return "massive";
	}
	case MASS_HALFMASSIVE:
	{
		return "halfmassive";
	}
	case MASS_CLIMBABLE:
	{
		return "climbable";
	}
	default:
	    break;
	}

	return "";
}

MassiveType Get_Massivetype_id( string str_massivetype )
{
	if( str_massivetype.compare( "passive" ) == 0 )
	{
		return MASS_PASSIVE;
	}
	else if( str_massivetype.compare( "massive" ) == 0 )
	{
		return MASS_MASSIVE;
	}
	else if( str_massivetype.compare( "halfmassive" ) == 0 )
	{
		return MASS_HALFMASSIVE;
	}
	else if( str_massivetype.compare( "climbable" ) == 0 )
	{
		return MASS_CLIMBABLE;
	}

	return MASS_PASSIVE;
}

Color Get_Massivetype_Color( MassiveType mtype )
{
	switch( mtype )
	{
	case MASS_MASSIVE:
	{
		return lightred;
	}
	case MASS_HALFMASSIVE:
	{
		return orange;
	}
	case MASS_PASSIVE:
	{
		return lightgreen;
	}
	case MASS_CLIMBABLE:
	{
		return lila;
	}
	default:
	    break;
	}

	return white;
}

string Get_Color_name( DefaultColor color )
{
	switch( color )
	{
	case COL_DEFAULT:
	{
		return "default";
	}
	case COL_WHITE:
	{
		return "white";
	}
	case COL_BLACK:
	{
		return "black";
	}
	case COL_RED:
	{
		return "red";
	}
	case COL_ORANGE:
	{
		return "orange";
	}
	case COL_YELLOW:
	{
		return "yellow";
	}
	case COL_GREEN:
	{
		return "green";
	}
	case COL_BLUE:
	{
		return "blue";
	}
	case COL_BROWN:
	{
		return "brown";
	}
	case COL_GREY:
	{
		return "grey";
	}
	default:
	    break;
	}

	return "";
}

DefaultColor Get_Color_id( string str_color )
{
	if( str_color.compare( "white" ) == 0 )
	{
		return COL_WHITE;
	}
	else if( str_color.compare( "black" ) == 0 )
	{
		return COL_BLACK;
	}
	else if( str_color.compare( "red" ) == 0 )
	{
		return COL_RED;
	}
	else if( str_color.compare( "orange" ) == 0 )
	{
		return COL_ORANGE;
	}
	else if( str_color.compare( "yellow" ) == 0 )
	{
		return COL_YELLOW;
	}
	else if( str_color.compare( "green" ) == 0 )
	{
		return COL_GREEN;
	}
	else if( str_color.compare( "blue" ) == 0 )
	{
		return COL_BLUE;
	}
	else if( str_color.compare( "brown" ) == 0 )
	{
		return COL_BROWN;
	}
	else if( str_color == "grey" )
	{
		return COL_GREY;
	}

	return COL_DEFAULT;
}

float last_time_pulse = 0;

void Gui_handle_Time( void )
{
	// get current "run-time" in seconds
	float t = 0.001f * SDL_GetTicks();

	// inject the time that passed since the last call
	System::getSingleton().injectTimePulse( float( t - last_time_pulse ) );

	// store the new time as the last time
	last_time_pulse = t;
}

void Draw_StaticText( string text, Color *color_text /* = &white */, Color *color_bg /* = NULL */, bool wait_for_input /* = 1 */ )
{
	bool draw = 1;

	// Statictext window
	Window *window_statictext = WindowManager::getSingleton().loadWindowLayout( "statictext.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( window_statictext );
	// get default text
	Window *text_default = static_cast<Window *>(WindowManager::getSingleton().getWindow( "text_default" ));

	// set text
	text_default->setProperty( "TextColours", PropertyHelper::colourToString( colour( (float)color_text->red / 255, (float)color_text->green / 255, (float)color_text->blue / 255, 1 ) ) );
	text_default->setText( text.c_str() );

	// align text
	Font *font = FontManager::getSingleton().getFont( "bluebold_medium" );
	float text_width = font->getTextExtent( text.c_str() );
	text_default->setWidth( UDim( 0, text_width + 10 ) );
	text_default->setXPosition( UDim( 0, GAME_RES_W / 2 - text_width / 2 ) );
	text_default->moveToFront();

	while( draw )
	{
		Draw_Game();

		// draw background
		if( color_bg )
		{
			// create request
			cRectRequest *request = new cRectRequest();

			pVideo->Draw_Rect( NULL, 0.9f, color_bg, request );
			request->render_count = wait_for_input ? 4 : 1;

			// add request
			pRenderer->Add( request );
		}

		pVideo->Render();

		if( wait_for_input )
		{
			while( SDL_PollEvent( &input_event ) )
			{
				if( input_event.type == SDL_KEYDOWN || input_event.type == SDL_JOYBUTTONDOWN || input_event.type == SDL_MOUSEBUTTONDOWN )
				{
					draw = 0;
				}
			}

			// slow down
			CorrectFrameTime( 60 );
		}
		else
		{
			draw = 0;
		}

		pFramerate->Update();
	}

	// Clear possible active input
	if( wait_for_input )
	{
		ClearInputEvents();
	}

	pGuiSystem->getGUISheet()->removeChildWindow( window_statictext );
	WindowManager::getSingleton().destroyWindow( window_statictext );
}

string Box_Text_Input( string default_text, string title_text, bool auto_no_text /* = 1 */ )
{
	cDialogBox_Text *box = new cDialogBox_Text();
	string return_value = box->Enter( default_text, title_text, auto_no_text );
	delete box;

	return return_value;
}

int Box_Question( string text, bool with_cancel /* = 0 */ )
{
	cDialogBox_Question *box = new cDialogBox_Question();
	int return_value = box->Enter( text, with_cancel );
	delete box;

	return return_value;
}

void Preload_images( void )
{
	// Maryo
	Maryo_type old_maryo_type = pPlayer->maryo_type;

	// if active object
	cMovingSprite *old_active_object = NULL;

	if( pPlayer->active_object )
	{
		old_active_object = pPlayer->active_object;
		pPlayer->active_object = NULL;
	}

	// small
	pPlayer->maryo_type = MARYO_SMALL;
	pPlayer->Load_Images();
	// big
	pPlayer->maryo_type = MARYO_BIG;
	pPlayer->Load_Images();
	// fire
	pPlayer->maryo_type = MARYO_FIRE;
	pPlayer->Load_Images();
	// ice
	pPlayer->maryo_type = MARYO_ICE;
	pPlayer->Load_Images();
	// ghost
	pPlayer->maryo_type = MARYO_GHOST;
	pPlayer->Load_Images();

	// with holding images
	pPlayer->active_object = pPlayer;

	// small
	pPlayer->maryo_type = MARYO_SMALL;
	pPlayer->Load_Images();
	// big
	pPlayer->maryo_type = MARYO_BIG;
	pPlayer->Load_Images();
	// fire
	pPlayer->maryo_type = MARYO_FIRE;
	pPlayer->Load_Images();
	// ice
	pPlayer->maryo_type = MARYO_ICE;
	pPlayer->Load_Images();
	// ghost
	pPlayer->maryo_type = MARYO_GHOST;
	pPlayer->Load_Images();

	// restore old type
	pPlayer->active_object = old_active_object;
	pPlayer->maryo_type = old_maryo_type;
	pPlayer->Load_Images();

	// Mushrooms
	pVideo->Get_Surface( "game/items/mushroom_red.png" );
	pVideo->Get_Surface( "game/items/mushroom_green.png" );
	// Flower
	pVideo->Get_Surface( "game/items/fire_flower.png" );
	pVideo->Get_Surface( "game/items/fire_flower_left.png" );
	pVideo->Get_Surface( "game/items/fire_flower_right.png" );
	// Star
	pVideo->Get_Surface( "game/items/star.png" );
	// Yellow Goldpiece
	pVideo->Get_Surface( "game/items/goldpiece/yellow/1.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/2.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/3.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/4.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/5.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/6.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/7.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/8.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/9.png" );
	pVideo->Get_Surface( "game/items/goldpiece/yellow/10.png" );
	// Red Goldpiece
	pVideo->Get_Surface( "game/items/goldpiece/red/1.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/2.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/3.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/4.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/5.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/6.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/7.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/8.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/9.png" );
	pVideo->Get_Surface( "game/items/goldpiece/red/10.png" );

	// Brown Box
	pVideo->Get_Surface( "game/box/brown1_1.png" );	
	// Bonus Box
	pVideo->Get_Surface( "game/box/yellow/bonus/1.png" );
	pVideo->Get_Surface( "game/box/yellow/bonus/2.png" );
	pVideo->Get_Surface( "game/box/yellow/bonus/3.png" );
	pVideo->Get_Surface( "game/box/yellow/bonus/4.png" );
	pVideo->Get_Surface( "game/box/yellow/bonus/5.png" );
	pVideo->Get_Surface( "game/box/yellow/bonus/6.png" );
	// Default Box
	pVideo->Get_Surface( "game/box/yellow/default.png" );
	// Spin Box
	pVideo->Get_Surface( "game/box/yellow/spin/1.png" );
	pVideo->Get_Surface( "game/box/yellow/spin/2.png" );
	pVideo->Get_Surface( "game/box/yellow/spin/3.png" );
	pVideo->Get_Surface( "game/box/yellow/spin/4.png" );
	pVideo->Get_Surface( "game/box/yellow/spin/5.png" );

	// Goldpiece animation
	pVideo->Get_Surface( "animation/light_1/1.png" );
	pVideo->Get_Surface( "animation/light_1/2.png" );
	pVideo->Get_Surface( "animation/light_1/3.png" );
	// Particle animation
	pVideo->Get_Surface( "animation/particles/smoke.png" );
	pVideo->Get_Surface( "animation/particles/light.png" );

	// Fireball
	pVideo->Get_Surface( "animation/fireball/1.png" );

	// HUD
	pVideo->Get_Surface( "game/maryo_l.png" );
	pVideo->Get_Surface( "game/gold_m.png" );
	pVideo->Get_Surface( "game/itembox.png" );
}

void Preload_sounds( void )
{
	// player
	pAudio->Get_Sound( "tock.ogg" );
	pAudio->Get_Sound( "player/dead.ogg" );
	pAudio->Get_Sound( "itembox_get.ogg" );
	pAudio->Get_Sound( "itembox_set.ogg" );
	pAudio->Get_Sound( "player/jump_small.ogg" );
	pAudio->Get_Sound( "player/jump_big.ogg" );
	pAudio->Get_Sound( "player/jump_ghost.ogg" );
	// todo : create again
	//pAudio->Get_Sound( "player/maryo_au.ogg" );
	pAudio->Get_Sound( "player/powerdown.ogg" );
	pAudio->Get_Sound( "player/ghost_end.ogg" );
	pAudio->Get_Sound( "enter_pipe.ogg" );
	pAudio->Get_Sound( "leave_pipe.ogg" );

	// items
	pAudio->Get_Sound( "item/star_kill.ogg" );
	pAudio->Get_Sound( "item/fireball.ogg" );
	pAudio->Get_Sound( "item/fireball_2.ogg" );
	pAudio->Get_Sound( "item/fireplant.ogg" );
	pAudio->Get_Sound( "item/goldpiece_1.ogg" );
	pAudio->Get_Sound( "item/live_up.ogg" );
	pAudio->Get_Sound( "item/mushroom.ogg" );
	pAudio->Get_Sound( "item/mushroom_ghost.ogg" );
	pAudio->Get_Sound( "item/moon.ogg" );

	// box
	pAudio->Get_Sound( "death_box.ogg" );

	// enemies
	// eato
	pAudio->Get_Sound( "enemy/eato/die.ogg" );
	// gee
	pAudio->Get_Sound( "enemy/gee/die.ogg" );
	// gumba
	pAudio->Get_Sound( "enemy/gumba/die.ogg" );
	// jpiranha
	pAudio->Get_Sound( "enemy/jpiranha/die.ogg" );
	// rex
	pAudio->Get_Sound( "enemy/rex/die.ogg" );
	// rokko
	pAudio->Get_Sound( "enemy/rokko/activate.ogg" );
	// spika
	pAudio->Get_Sound( "enemy/spika/move.ogg" );
	// thromp
	pAudio->Get_Sound( "enemy/thromp/hit.ogg" );
	pAudio->Get_Sound( "enemy/thromp/die.ogg" );
	// turtle
	pAudio->Get_Sound( "enemy/turtle/hit.ogg" );
	pAudio->Get_Sound( "enemy/turtle/shell/hit.ogg" );
	// todo : create again
	//pAudio->Get_Sound( "enemy/turtle/power_up.ogg" );

	// default
	pAudio->Get_Sound( "sprout_1.ogg" );
	pAudio->Get_Sound( "stomp_1.ogg" );
	pAudio->Get_Sound( "stomp_2.ogg" );
	pAudio->Get_Sound( "stomp_4.ogg" );
	// boss
	pAudio->Get_Sound( "enemy/boss/turtle/big_hit.ogg" );
	pAudio->Get_Sound( "enemy/boss/turtle/shell_attack.ogg" );
	pAudio->Get_Sound( "enemy/boss/turtle/power_up.ogg" );

	// savegame
	pAudio->Get_Sound( "savegame_load.ogg" );
	pAudio->Get_Sound( "savegame_save.ogg" );

	// overworld
	pAudio->Get_Sound( "waypoint_reached.ogg" );
}

bool Delete_file( const string &filename )
{
	if( remove( filename.c_str() ) == 0 )
	{
		return 1;
	}

	return 0;
}

bool valid_file( const string &filename )
{
	ifstream ifs( filename.c_str(), ios::in );

	if( ifs )
	{
		ifs.close();
		return 1;
	}

	return 0;
}

bool is_valid_number( string num, bool accept_floating_point /* = 1 */ )
{
	// accept negative numbers
	if( num.find( '-' ) == 0 )
	{
		num.erase( 0, 1 );
	}

	// accept numbers with a point if given
	if( accept_floating_point && num.find( '.' ) != string::npos )
	{
		num.erase( num.find( '.' ), 1 );
	}

	if( std::find_if( num.begin(), num.end(), nondigit() ) == num.end() )
	{
		return 1;
	}

	return 0;
}

void Convert_path_separators( string &str )
{
	for( string::iterator itr = str.begin(); itr != str.end(); ++itr )
	{
		// fix it
		if( *itr == '\\' || *itr == '!' )
		{
			*itr = '/';
		}
	}
}

string int_to_string( int number )
{
	std::ostringstream temp;
	temp << number;

	return temp.str();
}

string float_to_string( const float number )
{
	std::ostringstream temp;
	temp.setf( std::ios_base::fixed );
	temp << number;

	return temp.str();
}

float string_to_float( const string &str )
{
	float val = 0;
	sscanf( str.c_str(), " %f", &val );

	return val;
}

int string_to_int( const string &str )
{
	int num;

	if( str.empty() )
	{
		return 0;
	}

	std::istringstream temp( str );
	temp >> num;

	return num;
}

double string_to_double( const string &str )
{
	double num;

	std::istringstream temp( str );
	temp >> num;

	return num;
}

string string_to_xml_string( const string &str )
{
	string res;
	res.reserve( str.size() * 2 );

	const string::const_iterator iterEnd = str.end();
	for( string::const_iterator iter = str.begin(); iter != iterEnd ; ++iter )
	{
		switch( *iter )
		{
			case '<':
				res += "&lt;";
				break;

			case '>':
				res += "&gt;";
				break;

			case '&':
				res += "&amp;";
				break;

			case '\'':
				res += "&apos;";
				break;

			case '"':
				res += "&quot;";
				break;

			case '\n':
				res += "&lt;br/&gt;";
				break;

			default:
				res += *iter;
		}
	}

	return res;
}

string xml_string_to_string( string str )
{
	while( str.find( "<br/>" ) != string::npos )
	{
		str.replace( str.find( "<br/>" ), 5, "\n" );
	}

	return str;
}
