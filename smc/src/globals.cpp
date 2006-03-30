/***************************************************************************
    globals.cpp  -  all global used variables, as specified in globals.h
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
 

#include "globals.h"
#include "main.h"

#include <algorithm>

// ##### The global variables #########

cFramerate Framerate( DESIRED_FPS );

cImageManager *pImageManager = NULL;
cObjectManager *pObjManager = NULL;
cHudManager *pHudManager = NULL;
cAnimationManager *pAnimationManager = NULL;
cDialogManager *pDialogManager = NULL;
cJoystick *pJoystick = NULL;
OpenGLRenderer *pGuiRenderer = NULL;
System *pGuiSystem = NULL;

cPlayer *pPlayer = NULL;
cLevel *pLevel = NULL;
cOverWorld *pOverWorld = NULL;
cMainMenu *pMenu = NULL;

cAudio *pAudio = NULL;
cPreferences *pPreferences = NULL;
cMouseCursor *pMouseCursor = NULL;
cLevelEditor *pLeveleditor = NULL;
cSavegame *pSavegame = NULL;

cPlayerPoints *pointsdisplay = NULL;
cDebugDisplay *debugdisplay = NULL;
cGoldDisplay *golddisplay = NULL;
cLiveDisplay *livedisplay = NULL;
cTimeDisplay *timedisplay = NULL;
cItemBox *Itembox = NULL;

int Leveleditor_Mode = 0;
GameMode Game_Mode;
bool Game_debug = 0;

bool done;
SDL_Surface *screen;
SDL_Event event;
Color std_bgcolor, blue, darkblue, lightblue, black, white, grey, lightgrey, lightgreyalpha, green, lightgreenalpha, greenyellow, darkgreen, red, lightred, lila, orange;
TTF_Font *font_normal, *font_small;
Uint8 *keys;

double cameraposx = 0, cameraposy = 0;

double global_scalex, global_scaley;

// uses the is_valid_number function
class nondigit
{
public:
	bool operator() (char c) { return !isdigit(c); }
};


bool is_valid_number( string num )
{
	if( num.find( '-' ) == 0 ) // should also accept negative numbers
	{
		num.erase( 0, 1 );
	}

	if( std::find_if( num.begin(), num.end(), nondigit() ) == num.end() )
	{
		return 1;
	}

	return 0;
}

cSprite *Copy_Object( cSprite *CopyObject, double x, double y )
{
	cSprite *new_sprite = NULL;

	if( CopyObject->type == TYPE_GOLDPIECE )
	{
		cGoldPiece *active = (cGoldPiece*)CopyObject;

		new_sprite = (cSprite *)new cGoldPiece( x, y, active->color );
	}
	if( CopyObject->type == TYPE_MUSHROOM_DEFAULT || CopyObject->type == TYPE_MUSHROOM_LIVE_1 )
	{
		cMushroom *active = (cMushroom*)CopyObject;

		new_sprite = (cSprite *)new cMushroom( x, y, active->type );
	}
	else if( CopyObject->type == TYPE_MOON )
	{
		new_sprite = (cSprite *)new cMoon( x, y );
	}
	else if( CopyObject->type == TYPE_JSTAR )
	{
		new_sprite = (cSprite *)new cjStar( x, y );
	}
	else if( CopyObject->type == TYPE_CLOUD )
	{
		new_sprite = (cSprite *)new cCloud( x, y );
	}
	else if( CopyObject->type == TYPE_MOVING_PLATFORM )
	{
		cMoving_Platform *active = (cMoving_Platform *)CopyObject;

		new_sprite = (cSprite *)new cMoving_Platform( x, y, active->platform_type, active->max_area, active->speed );
	}
	else if( CopyObject->type == TYPE_LEVELEXIT )
	{
		cLevelExit *active = (cLevelExit *)CopyObject;

		new_sprite = (cSprite *)new cLevelExit( x, y, active->levelchange, active->levelname );
	}
	else if( CopyObject->type == TYPE_GOLDBOX )
	{
		cGoldBox *active = (cGoldBox *)CopyObject;
		cGoldBox *goldbox = new cGoldBox( x, y, active->color );
		goldbox->Set_UseableCount( active->start_useable_count, 1 );

		new_sprite = (cSprite *)goldbox;
	}
	else if( CopyObject->type == TYPE_SPINBOX )
	{
		new_sprite = (cSprite *)new cSpinBox( x, y );
	}
	else if( CopyObject->type == TYPE_GOOMBA )
	{
		cGoomba *enemy = (cGoomba*)CopyObject;

		new_sprite = (cSprite *)new cGoomba( x, y, enemy->color );
		new_sprite->Set_Direction( enemy->start_direction, 1 );
	}
	else if( CopyObject->type == TYPE_TURTLE )
	{
		new_sprite = (cSprite *)new cTurtle( x, y, 1 );
	}
	else if( CopyObject->type == TYPE_JPIRANHA )
	{
		cjPiranha *enemy = (cjPiranha*)CopyObject;

		new_sprite = (cSprite *)new cjPiranha( x, y, CopyObject->direction, enemy->max_distance );
	} 
	else if( CopyObject->type == TYPE_BANZAI_BILL )
	{
		new_sprite = (cSprite *)new cbanzai_bill( x, y, CopyObject->direction );
	}
	else if( CopyObject->type == TYPE_REX )
	{
		new_sprite = (cSprite *)new cRex( x, y );
	}
	else if( CopyObject->type == TYPE_BONUSBOX_MUSHROOM_FIRE )
	{
		new_sprite = (cSprite*)new cBonusBox( x, y,TYPE_BONUSBOX_MUSHROOM_FIRE );
	}
	else if( CopyObject->type == TYPE_BONUSBOX_LIVE )
	{
		new_sprite = (cSprite *)new cBonusBox( x, y, TYPE_BONUSBOX_LIVE );
	}
	else if( CopyObject->type == TYPE_BONUSBOX_JSTAR )
	{
		new_sprite = (cSprite *)new cBonusBox( x, y, TYPE_BONUSBOX_JSTAR );
	}
	else if( CopyObject->type == TYPE_ENEMYSTOPPER )
	{
		new_sprite = (cSprite *)new cEnemyStopper( x, y );
	}
	else if( CopyObject->type == TYPE_PLAYER )
	{
		// nothing
	}
	else // standard sprite
	{
		new_sprite = new cSprite( GetImage( CopyObject->image->filename ), x, y );

		if( CopyObject->massivetype == MASS_PASSIVE )
		{
			new_sprite->array = ARRAY_PASSIVE;
			new_sprite->massivetype = MASS_PASSIVE;
		}
		else if( CopyObject->massivetype == MASS_MASSIVE )
		{
			new_sprite->array = ARRAY_MASSIVE;
			new_sprite->massivetype = MASS_MASSIVE;
		}
		else if( CopyObject->massivetype == MASS_HALFMASSIVE )
		{
			new_sprite->array = ARRAY_ACTIVE;
			new_sprite->type = TYPE_HALFMASSIVE;
			new_sprite->massivetype = MASS_HALFMASSIVE;
		}
		else if( CopyObject->massivetype == MASS_CLIMBABLE )
		{
			new_sprite->array = ARRAY_ACTIVE;
			new_sprite->type = TYPE_HALFMASSIVE;
			new_sprite->massivetype = MASS_CLIMBABLE;
		}
	}

	if( new_sprite )
	{
		new_sprite->massivetype = CopyObject->massivetype;
	}

	return new_sprite;
}

string Get_current_time( void )
{
   time_t t;
   struct tm *area;

   t = time( NULL );
   area = localtime( &t );
   return asctime( area );
}

void ClearEvents( void )
{
	SDL_Event inEvent;

	while( SDL_PollEvent( &inEvent ) )
	{
		// ignore
	}
}

void Gui_handle_Mouse_down( Uint8 button )
{
	switch ( button )
	{
		// mouse buttons
		case SDL_BUTTON_LEFT:
		{
			System::getSingleton().injectMouseButtonDown( LeftButton );
			break;
		}
		case SDL_BUTTON_MIDDLE:
		{
			System::getSingleton().injectMouseButtonDown( MiddleButton );
			break;
		}
		case SDL_BUTTON_RIGHT:
		{
			System::getSingleton().injectMouseButtonDown( RightButton );
			break;
		}
		// mouse wheel
		case SDL_BUTTON_WHEELDOWN:
		{
			System::getSingleton().injectMouseWheelChange( -1 );
			break;
		}
		case SDL_BUTTON_WHEELUP:
		{
			System::getSingleton().injectMouseWheelChange( +1 );
			break;
		}
		default:
		{
			break;
		}
	}
}

void Gui_handle_Mouse_up( Uint8 button )
{
	switch ( button )
	{
		case SDL_BUTTON_LEFT:
		{
			System::getSingleton().injectMouseButtonUp( LeftButton );
		}
		break;
		case SDL_BUTTON_MIDDLE:
		{
			System::getSingleton().injectMouseButtonUp( MiddleButton );
		}
		break;
		case SDL_BUTTON_RIGHT:
		{
			System::getSingleton().injectMouseButtonUp( RightButton );
		}
		break;
	}
}

double last_time_pulse = 0;

void Gui_handle_Time( void )
{
	// get current "run-time" in seconds
	double t = 0.001 * SDL_GetTicks();

	// inject the time that passed since the last call 
	System::getSingleton().injectTimePulse( float( t - last_time_pulse ) );

	// store the new time as the last time
	last_time_pulse = t;
}

string EditMessageBox( string default_text, string title_text, double pos_x, double pos_y, bool auto_no_text /* = 1  */)
{
	string description = default_text;

	bool finished = 0;

	GL_rect rect_bg; // default background color rect
	GL_rect rect_title, rect_text; // title and description rect
	GL_rect rect_top, rect_top_in; // top box background color rects

	GL_Surface *title_img = RenderText( font_normal, title_text, white );
	GL_Surface *description_img = RenderText( font_normal, description, black );
	
	GL_rect pos_rect;

	pos_rect.x = pos_x;
	pos_rect.y = pos_y;
	pos_rect.w = 350;
	pos_rect.h = description_img->h + 2;

	rect_bg = pos_rect;
	rect_bg.x += 1;
	rect_bg.y += 1;
	rect_bg.h -= 2;
	rect_bg.w -= 2;
	
	rect_title = rect_bg;
	rect_title.y -= rect_bg.h + 2;
	rect_title.x += ( pos_rect.w / 2 ) - ( title_img->w / 2 ) - 5;
	rect_text = rect_bg;
	

	rect_top = rect_title;
	rect_top.x -= 2;
	rect_top.y -= 2;
	rect_top.h = title_img->h + 4;
	rect_top.w = title_img->w + 4;

	rect_top_in = rect_top;
	rect_top_in.x += 3;
	rect_top_in.y += 3;
	rect_top_in.h -= 6;
	rect_top_in.w -= 6;

	rect_title.x += 15;
	rect_text.x += 10;

	description.reserve( 30 );

	while( !finished )
	{
		DrawRect( &pos_rect, &darkgreen );
		DrawRect( &rect_bg, &white );

		DrawRect( &rect_top, &black );
		DrawRect( &rect_top_in, &darkblue );
		
		title_img->Blit( rect_title.x, rect_title.y, &black );
		description_img->Blit( rect_text.x, rect_text.y, &lightgreyalpha );

		SDL_GL_SwapBuffers();

		while( SDL_PollEvent( &event ) )
		{
			keys = SDL_GetKeyState( NULL );

			if( event.type == SDL_KEYDOWN )
			{
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					description = "";
					finished = 1;
				}
				else if( event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER )
				{
					finished = 1;
				}
				else if( event.key.keysym.sym == SDLK_BACKSPACE )
				{
					if( description.length() <= 1 || ( description.compare( default_text ) == 0 && auto_no_text ) )
					{
						description = "";

						delete description_img;
						description_img = RenderText( font_normal, " ", black );
					}
					else
					{
						description.erase( description.length() - 1, 1 );
						
						delete description_img;
						description_img = RenderText( font_normal, description, black );
					}
				}
				else
				{
					if( event.key.keysym.unicode && description.length() < 30 )
					{
						if( description.compare( default_text ) == 0 && auto_no_text )
						{
							description = "";
						}
						
						description.insert( description.length(), 1, (char)event.key.keysym.unicode );
						
						delete description_img;
						description_img = RenderText( font_normal, description, black );
					}
				}
			}
		}
		
		Framerate.Update();
	}

	delete description_img;
	delete title_img;

	return description;
}

void Preload_images( void )
{
	// Mushrooms
	GetImage( "game/items/mushroom_red.png" );
	GetImage( "game/items/mushroom_green.png" );
	// Flower
	GetImage( "game/items/flower_red_1.png" );
	GetImage( "game/items/flower_red_2.png" );

	// Box
	GetImage( "game/box/brown1_1.png" );
	GetImage( "game/box/yellow1_1.png" );
	GetImage( "game/box/yellow3_1.png" );
	GetImage( "game/box/yellow3_anim_1.png" );
	GetImage( "game/box/yellow3_anim_2.png" );
	GetImage( "game/box/yellow3_anim_3.png" );

	// Goldpiece animation
	GetImage( "animation/light_1/1.png" );
	GetImage( "animation/light_1/2.png" );
	GetImage( "animation/light_1/3.png" );
	// Particle animation
	GetImage( "animation/particles/smoke.png" );
	GetImage( "animation/particles/light.png" );

	// The Fireball
	GetImage( "animation/fireball/1.png" );

	// HUD
	GetImage( "game/maryo_l.png" );
	GetImage( "game/gold_m.png" );
	GetImage( "game/itembox.png" );
}

bool Delete_file( string filename )
{
	if( remove( filename.c_str() ) == 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool valid_file( string filename )
{
	ifstream ifs( filename.c_str(), ios::in );

	if( ifs ) 
	{
		ifs.close();
		return true;
	}

	return false;
}

string int_to_string( int number )
{
	std::ostringstream temp;
	temp << number;

	return temp.str();
}

string double_to_string( double number )
{
	std::ostringstream temp;
	temp << number;

	return temp.str();
}

float string_to_float( const string &str )
{
	float val = 0;
	sscanf( str.c_str(), " %f", &val );

	return val;
}

int string_to_int( string str )
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

double string_to_double( string str )
{
	double num;

	std::istringstream temp( str );
	temp >> num;

	return num;
}