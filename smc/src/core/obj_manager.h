/***************************************************************************
                obj_manager.h  -  Generic object manager
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

#ifndef __OBJ_MANAGER_H__
#define __OBJ_MANAGER_H__

/* *** *** *** *** *** cObject_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

template<class T> class cObject_Manager
{
public:
	cObject_Manager( void ) {};
	virtual ~cObject_Manager( void ) {};

	/*	Add the given object
	 */
	virtual void Add( T *obj )
	{
		objects.push_back( obj );
	}

	/* Delete the object from given array number
	 */
	virtual bool Delete( unsigned int array_num, bool delete_data = 1 )
	{
		// out of array
		if( array_num >= objects.size() )
		{
			return 0;
		}

		if( delete_data )
		{
			delete objects[array_num];
		}

		objects.erase( objects.begin() + array_num );

		return 1;
	}

	/* Delete the given object
	 */
	virtual bool Delete( T *obj, bool delete_data = 1 )
	{
		// empty object
		if( !obj )
		{
			return 0;
		}

		return Delete( Get_Array_num( obj ), delete_data );
	}

	/* Delete all objects
	 */
	virtual void Delete_All( void )
	{
		for( typename vector<T *>::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
		{
			delete *itr;
		}

		objects.clear();
	}

	/* Return the object pointer
	 * if not found returns NULL
	*/
	virtual T *Get_Pointer( unsigned int identifier )
	{
		if( identifier >= objects.size() )
		{
			// out of array
			return NULL;
		}

		// available
		return objects[identifier];
	}

	// Switch objects array position
	bool Switch_Array_num( T *obj1, T *obj2 )
	{
		// empty object
		if( !obj1 || !obj2 )
		{
			return 0;
		}

		// if the same
		if( obj1 == obj2 )
		{
			return 0;
		}

		int obj1_pos = Get_Array_num( obj1 );
		int obj2_pos = Get_Array_num( obj2 );
		
		// not found
		if( obj1_pos < 0 || obj2_pos < 0 )
		{
			return 0;
		}

		objects[obj1_pos] = obj2;
		objects[obj2_pos] = obj1;

		return 1;
	}

	/* Return the object array number
	 * if not found returns -1
	*/
	int Get_Array_num( T *obj )
	{
		// invalid
		if( !obj )
		{
			return -1;
		}

		for( unsigned int i = 0; i < objects.size(); i++ )
		{
			// check if the same
			if( objects[i] == obj )
			{
				// found
				return i;
			}
		}

		// not found
		return -1;
	}

	/* Returns the object count
	 */
	unsigned int size( void )
	{
		return objects.size();
	}

	vector<T*> objects;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
