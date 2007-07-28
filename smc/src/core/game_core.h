/***************************************************************************
			game_core.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GAME_CORE_H__
#define __GAME_CORE_H__

#include "../core/globals.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

// quit game if true
extern bool done;
// current Game Mode
extern GameMode Game_Mode;
// current Game Mode Type
extern GameModeType Game_Mode_Type;
// next global Game Action
extern GameAction Game_Action;
// Game Action data
extern string Game_Action_Data;
// Game Action pointer
extern void *Game_Action_ptr;
// global debugging
extern bool Game_debug;

// The global editor enabled variables prevent including additional editor headers
// if editor is enabled for the current game mode
extern bool editor_enabled;
// if level editor is active
extern bool editor_level_enabled;
// if world editor is active
extern bool editor_world_enabled;

// global image scale
extern float global_scalex, global_scaley;

// Game Input event
extern SDL_Event input_event;

/* *** *** *** *** *** *** *** *** DialogBox *** *** *** *** *** *** *** *** *** */

class cDialogBox
{
public:
	cDialogBox( void );
	virtual ~cDialogBox( void );

	// initialize
	void Init( void );
	// exit
	void Exit( void );
	
	// draw
	virtual void Draw( void );
	// update
	virtual void Update( void );

	// if finished
	bool finished;
	// base window
	Window *window;
	// layout filename
	string layout_file;

	// hide mouse on exit
	bool mouse_hide;
};

// CEGUI EditBox with header
class cDialogBox_Text : public cDialogBox
{
public:
	cDialogBox_Text( void );
	virtual ~cDialogBox_Text( void );

	// initialize
	void Init( string title_text );

	// enter
	string Enter( string default_text, string title_text, bool auto_no_text = 1 );

	// CEGUI events
	bool Button_window_quit_clicked( const EventArgs &event ); // window quit button

	// editbox
	Editbox *box_editbox;
};

// Button Question Box
class cDialogBox_Question : public cDialogBox
{
public:
	cDialogBox_Question( void );
	virtual ~cDialogBox_Question( void );

	// initialize
	void Init( bool with_cancel );

	// enter
	int Enter( string text, bool with_cancel = 0 );

	// CEGUI events
	bool Button_yes_clicked( const EventArgs &event ); // yes button
	bool Button_no_clicked( const EventArgs &event ); // no button
	bool Button_cancel_clicked( const EventArgs &event ); // cancel button

	// return value
	int return_value;
};

/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

/* Changes the Game Mode to the given mode
 * and disables or enables game mode specific objects
*/
void Change_Game_Mode( GameMode new_mode );

/* Function: Copy_Object
 * Returns the copied object with the given position
 * if no position is given defaults to 0
 * returns NULL if an error occurred
 */
cSprite *Copy_Object( cSprite *copy_object, float x = 0, float y = 0 );

// Function: Time_to_String
// Returns the given time as string
string Time_to_String( time_t t, const char *format );

// Function: Get_filename
// Returns the filename with the given options
string Get_filename( string filename, bool with_dir = 1, bool with_end = 1 );

// Function: ClearInputEvents
// Clears the complete input event queue
void ClearInputEvents( void ); 

// returns the opposite Direction
ObjectDirection Get_OppositeDirection( ObjectDirection direction );
// Returns the Direction Name
string Get_Direction_name( ObjectDirection dir );
// Returns the Direction identifier
ObjectDirection Get_Direction_id( string str_direction );

// Returns the SpriteType identifier
SpriteType Get_Sprite_Type_id( string str_type );

// Returns the Massivetype Name
string Get_Massivetype_name( MassiveType mtype );
// Returns the Massivetype identifier
MassiveType Get_Massivetype_id( string str_massivetype );
// Returns the Color of the given Massivetype
Color Get_Massivetype_Color( MassiveType mtype );

// Returns the Color Name
string Get_Color_name( DefaultColor color );
// Returns the Color identifier
DefaultColor Get_Color_id( string str_color );

// Function: Gui_handle_Time
// GUI generic function
void Gui_handle_Time( void );

/* Draw a Statictext
 * if wait_for_input is given draws the text until the user pressed a key
 */
void Draw_StaticText( string text, Color *color_text = &white, Color *color_bg = NULL, bool wait_for_input = 1 );

/* CEGUI EditBox with header
 *
 * Parameters
 * default_text : default text in the EditBox
 * title_text : box header text
 * auto_no_text : if true and any key is pressed the default text is cleared
 */
string Box_Text_Input( string default_text, string title_text, bool auto_no_text = 1 );

/* Button Question Box
 * returns 1 for Yes, 0 for No and -1 if canceled
 *
 * text : box question text
 * with_cancel : add the cancel button
*/
int Box_Question( string text, bool with_cancel = 0 );

/* Function: Preload_images
 * Preloads the common images into the image manager
 */
void Preload_images( void );

/* Function: Preload_sounds
 * Preloads the common sounds into the sound manager
 */
void Preload_sounds( void );

/* Function: Delete_file
 * Deletes the given file
 * Use with Caution
 *
 * Returns:
 * true on success
*/
bool Delete_file( const string &filename );

// Function: valid_file
// Checks if the file exists
bool valid_file( const string &filename );

/* Function: is_valid_number
 * if accept_floating_point is set also accept floating point values
 * Returns true if the number is valid
*/
bool is_valid_number( string num, bool accept_floating_point = 1 );

// Function: Convert_path_separators
// Converts "\" and "!" to "/"
void Convert_path_separators( string &str );

// Function: int_to_string
// Returns the number as a string
string int_to_string( int number );
// Function: float_to_string
// Returns the number as a string
string float_to_string( const float number );
// Returns the string as a float
float string_to_float( const string &str );
// Function: string_to_int
// Returns the string as a number
int string_to_int( const string &str );
// Function: string_to_double
// Returns the string as a double
double string_to_double( const string &str );
// Returns a valid XML string
string string_to_xml_string( const string &str );
// Returns the real string
string xml_string_to_string( string str );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
