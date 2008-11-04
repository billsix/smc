/***************************************************************************
 * game_core.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_GAME_CORE_H
#define SMC_GAME_CORE_H

#include "../core/globals.h"
#include "../objects/movingsprite.h"
#include "../core/camera.h"

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

// quit game if true
extern bool game_exit;
// current Game Mode
extern GameMode Game_Mode;
// current Game Mode Type
extern GameModeType Game_Mode_Type;
// next global Game Action
extern GameAction Game_Action;
// Game Action data
extern CEGUI::XMLAttributes Game_Action_Data;
// Game Action pointer
extern void *Game_Action_ptr;

// internal game resolution and is used for global scaling
extern int game_res_w;
extern int game_res_h;

// user data directory
extern std::string user_data_dir;

// global debugging
extern bool game_debug, game_debug_performance;

// Game Input event
extern SDL_Event input_event;

// global up scale ( f.e. default image scale )
extern float global_upscalex, global_upscaley;
// global down scale ( f.e. mouse/CEGUI scale )
extern float global_downscalex, global_downscaley;

// The global editor enabled variables prevent including additional editor header files
// if editor is enabled for the current game mode
extern bool editor_enabled;
// if level editor is active
extern bool editor_level_enabled;
// if world editor is active
extern bool editor_world_enabled;

// Active sprite manager
extern cSprite_Manager *pActive_Sprite_Manager;
// Active camera class
extern cCamera *pActive_Camera;
// Active player
extern cSprite *pActive_Player;


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
	CEGUI::Window *window;
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
	void Init( std::string title_text );

	// enter
	string Enter( std::string default_text, std::string title_text, bool auto_no_text = 1 );

	// CEGUI events
	bool Button_window_quit_clicked( const CEGUI::EventArgs &event ); // window quit button

	// editbox
	CEGUI::Editbox *box_editbox;
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
	int Enter( std::string text, bool with_cancel = 0 );

	// CEGUI events
	bool Button_yes_clicked( const CEGUI::EventArgs &event ); // yes button
	bool Button_no_clicked( const CEGUI::EventArgs &event ); // no button
	bool Button_cancel_clicked( const CEGUI::EventArgs &event ); // cancel button

	// return value
	int return_value;
};

/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

/* Change the Game Mode to the given mode
 * and disables or enables game mode specific objects
*/
void Change_Game_Mode( GameMode new_mode );

// Return the given time as string
std::string Time_to_String( time_t t, const char *format );

// Return the filename with the given options
std::string Get_Filename( std::string filename, bool with_dir = 1, bool with_end = 1 );

// Clear the complete input event queue
void Clear_Input_Events( void ); 

// Return the opposite Direction
ObjectDirection Get_Opposite_Direction( ObjectDirection direction );
// Return the Direction Name
std::string Get_Direction_Name( ObjectDirection dir );
// Return the Direction identifier
ObjectDirection Get_Direction_Id( std::string str_direction );

// Return the SpriteType identifier
SpriteType Get_Sprite_Type_Id( std::string str_type );
/* Return the Color of the given Sprite
 * based mostly on sprite array
*/
Color Get_Sprite_Color( cSprite *sprite );

// Return the massive type Name
std::string Get_Massive_Type_Name( MassiveType mtype );
// Return the massive type identifier
MassiveType Get_Massive_Type_Id( std::string str_massivetype );
// Return the Color of the given Massivetype
Color Get_Massive_Type_Color( MassiveType mtype );

// Return the ground type name
std::string Get_Ground_Type_Name( GroundType gtype );
// Return the ground type identifier
GroundType Get_Ground_Type_Id( std::string str_groundtype );

// Return the Color Name
std::string Get_Color_Name( DefaultColor color );
// Return the Color identifier
DefaultColor Get_Color_Id( std::string str_color );

// Update The GUI time handler
void Gui_Handle_Time( void );

/* Draw a Statictext
 * if wait_for_input is given draws the text until the user pressed a key
 */
void Draw_Static_Text( std::string text, const Color *color_text = &white, const Color *color_bg = NULL, bool wait_for_input = 1 );

/* CEGUI EditBox with header
 *
 * Parameters
 * default_text : default text in the EditBox
 * title_text : box header text
 * auto_no_text : if true and any key is pressed the default text is cleared
 */
std::string Box_Text_Input( std::string default_text, std::string title_text, bool auto_no_text = 1 );

/* Button Question Box
 * returns 1 for Yes, 0 for No and -1 if canceled
 *
 * text : box question text
 * with_cancel : add the cancel button
*/
int Box_Question( std::string text, bool with_cancel = 0 );

/* Preload the common images into the image manager
 * draw_gui : if set use the loading screen gui for drawing
 */
void Preload_Images( bool draw_gui = 0 );

/* Preload the common sounds into the sound manager
 * draw_gui : if set use the loading screen gui for drawing
 */
void Preload_Sounds( bool draw_gui = 0 );

/* Deletes the given file
 * Use with Caution
 *
 * Returns:
 * true on success
*/
bool Delete_File( const std::string &filename );
// Check if the file exists
bool File_Exists( const std::string &filename );

// Converts "\" and "!" to "/"
void Convert_Path_Separators( std::string &str );

// Changes the image path in the given xml attributes to the new one
void Relocate_Image( CEGUI::XMLAttributes &xml_attributes, std::string filename_old, std::string filename_new );

/* Get all files from the directory
 * dir : the directory to scan
 * file_type : if set only this file type is returned
 * with_directories : if set adds directories to the returned objects
 * search_in_sub_directories : searches in every sub-directory
*/
vector<std::string> Get_Directory_Files( std::string dir, std::string file_type = "", bool with_directories = 0, bool search_in_sub_directories = 1 );

// Return the operating system temporary files directory
std::string Get_Temp_Directory( void );
// Return the default smc user directory in the operating system application/home directory
string Get_User_Directory( void );
// Create the necessary folders in the user directory
void Init_User_Directory( void );
// Return the clipboard content
std::string Get_Clipboard_Content( void );
// Set the clipboard content
void Set_Clipboard_Content( std::string str );
/* Copy selected GUI text to the Clipboard
 * cut: if set cut the text
*/
bool GUI_Copy_To_Clipboard( bool cut = 0 );
// Paste text from the clipboard to the GUI
bool GUI_Paste_From_Clipboard( void );
// Trim the string from the end with the given character
std::string string_trim_from_end( std::string str, const char c );
// Return the number as a string
std::string int_to_string( const int number );
/* Return the float as a string
 * prec: the precision after the decimal point
*/
std::string float_to_string( const float number, int prec = 6 );
// Return the string as a float
float string_to_float( const std::string &str );
// Return the string as a number
int string_to_int( const std::string &str );
// Return the string as a double
double string_to_double( const std::string &str );
// Return a valid XML string
std::string string_to_xml_string( const std::string &str );
// Return the real string
std::string xml_string_to_string( std::string str );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
