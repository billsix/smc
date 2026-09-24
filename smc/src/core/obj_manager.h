/***************************************************************************
 * obj_manager.h  -  Generic object manager
 *
 * Copyright (C) 2005 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_OBJ_MANAGER_H
#define SMC_OBJ_MANAGER_H

#include "../core/global_basic.h"
#include <algorithm>

namespace SMC {

/* *** *** *** *** *** cObject_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

template <class T>
class cObject_Manager {
 public:
  cObject_Manager(void) {};
  virtual ~cObject_Manager(void) {};

  //	Add the given object
  virtual void Add(T* obj) { objects.push_back(obj); }

  // Delete the given object
  virtual bool Delete(T* obj, bool delete_data = 1) {
    // empty object
    if (!obj) {
      return 0;
    }

    // get iterator
    typename vector<T*>::iterator itr =
        std::find(objects.begin(), objects.end(), obj);

    // available in vector
    if (itr != objects.end()) {
      // erase
      objects.erase(itr);
    }

    if (delete_data) {
      delete obj;
    }

    return 1;
  }

  // Delete all objects
  virtual void Delete_All(void) {
    for (typename vector<T*>::iterator itr = objects.begin();
         itr != objects.end(); ++itr) {
      delete *itr;
    }

    objects.clear();
  }

  /* Return the object pointer
	 * if not found returns NULL
	*/
  virtual T* Get_Pointer(size_t identifier) const {
    if (identifier >= objects.size()) {
      // out of array
      return NULL;
    }

    // available
    return objects[identifier];
  }

  /* Return the object array number
	 * if not found returns -1
	*/
  int Get_Array_Num(T* obj) const {
    // invalid
    if (!obj) {
      return -1;
    }

    typename vector<T*>::const_iterator itr_obj =
        std::find(objects.begin(), objects.end(), obj);

    // not in vector
    if (itr_obj == objects.end()) {
      return -1;
    }

    return std::distance(objects.begin(), itr_obj);
  }

  // Return the object count
  size_t size(void) const { return objects.size(); }

  // Return true if no objects
  bool empty(void) const { return objects.empty(); }

  vector<T*> objects;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
