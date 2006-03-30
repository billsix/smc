/***************************************************************************
                levelexit.h  -  header for the corresponding cpp file
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

#ifndef __LEVELEXIT_H__
#define __LEVELEXIT_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** ## Levelchange types ## *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	LVLCHANGE_BEAM		= 0,	// no special animation ( e.g. doors )
	LVLCHANGE_WARP		= 1		// the player slowly moves into the object ( e.g. pipes )
} Levelchange_type;


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* The Levelexit
 * if a level filename is given the given level is the destination
*/
class cLevelExit : public cActiveSprite
{
public:
	cLevelExit( double x, double y, Levelchange_type ntype = LVLCHANGE_BEAM, string nlevelname = "" );
	virtual ~cLevelExit( void );

	virtual void Update( void );
	virtual void Draw( void );

	// Sets the destination level
	void Set_Level( string filename );
	// Returns the destination level
	string Get_Level( bool with_dir = 1, bool with_end = 1 );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor deactivation
	virtual void Editor_Deactivate( void );
	// leveleditor position update
	virtual void Editor_pos_update( void );
	// leveleditor value update
	virtual void Editor_val_update( void );

	// levelchange type
	Levelchange_type levelchange;
	// destination level if type is not exit
	string levelname;

	// colors for the type
	Color col_exit, col_change;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
