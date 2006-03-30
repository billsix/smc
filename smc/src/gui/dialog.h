/***************************************************************************
			dialog.h  -  header for the corresponding cpp file
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

#ifndef __DIALOG_H__
#define __DIALOG_H__

/* *** *** *** *** *** *** *** Dialog *** *** *** *** *** *** *** *** *** *** */

class cDialog  : public cSprite 
{
public:
	cDialog( double nposx, double nposy, string nidentifier, string ntext, SpriteType dialogtype, unsigned int nmax_length, unsigned int nmin_width, bool no_cam );
	~cDialog( void );

	// draw the dialog
	virtual void Draw( void );

	// returns true if the text got changed
	bool gotChanged( void );

	// Returns the Value and resets the changed state
	int Get_Value_int( bool reset_changed = 1 );
	string Get_Value_string( bool reset_changed = 1 );
	// the dialog gets the edit focus
	void Get_Focus( void );

	/* Sets the text as value
	 * if set_changed is true also sets the text changed value
	*/
	void Set_Text( string text_str, bool set_changed = 0 );

	// Set the position
	void SetPos( double x, double y );
	// Sets the outer Boarder size
	void Set_Boardersize_out( Uint8 size );
	// Sets the Colors
	void SetColors( Color outer, Color inner, Color text_area, Color text );
	// Sets the inner Boarder size
	void Set_Boardersize_in( Uint8 size );

	// Sets the Additional Text area size
	void Additional_Textareasize( Uint8 size );

	// Returns a rect with the real size
	SDL_Rect Get_Rect( void );

	// current text string
	string text;

	// number value if the type is number only
	int text_number;

	// minimal width
	unsigned int min_width;

	// identifier
	string identifier;

	// maximum character or number length
	unsigned int max_length;

	// boarder and text colors
	Color boarder_out_color, boarder_in_color, text_area_color, text_color;

private:
	void Update_Boarders( void );
	void Update_Text( void );

	SDL_Rect rect_boarder_out, rect_boarder_in, rect_text_area;
	
	Uint8 boarder_out, boarder_in, text_area;

	// true if text changed
	bool changed;
};

/* *** *** *** *** *** *** *** Dialog Manager *** *** *** *** *** *** *** *** *** *** */

typedef vector<cDialog *> DialogList;

class cDialogManager
{
public:
	cDialogManager( void );
	~cDialogManager( void );

	// Adds an Dialog
	void Add( double posx, double posy, string identifier, string ntext, SpriteType dialogtype = TYPE_DIALOG_ALL, int max_length = 20, unsigned int min_width = 70, bool no_camera = 0 );
	// Deletes the Dialog
	void Delete( unsigned int num );
	bool Delete( string name );
	/* Returns the Dialog id
	*  if not found returns -1 */ 
	int Get_num( string name );
	/* Returns the Dialog pointer
	*  if not found returns NULL */
	cDialog *Get( string name );

	// Draw all Dialogs
	void Draw( void );

	// Delete all Dialogs
	void DeleteAll( void );

	/* Returns the Collision number
	* if no collision returns -1 */
	int Get_Collision( int x, int y );

	// Returns the current size
	unsigned int Get_Size( void );

	// dialog objects
	DialogList Objects;
};

#endif
