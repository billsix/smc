/***************************************************************************
    level_manager.cpp  -  class for handling levels
                             -------------------
    copyright            : (C) 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../level/level_manager.h"
#include "../level/level.h"


/* *** *** *** *** *** cLevel_manager *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_manager :: cLevel_manager( void )
{
	pLevel = new cLevel();
}

cLevel_manager :: ~cLevel_manager( void )
{
	if( pLevel )
	{
		delete pLevel;
		pLevel = NULL;
	}
}

void cLevel_manager :: Init( void )
{

}

void cLevel_manager :: Unload( void )
{
	pLevel->Unload();
}

bool cLevel_manager :: Load( string filename, bool delayed /* = 0 */, bool sub_level /* = 0 */ )
{
	return 1;
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Level information handler
cLevel_manager *pLevel_manager = NULL;
