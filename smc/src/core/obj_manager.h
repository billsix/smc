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
#include <memory>

namespace SMC {

/* *** *** *** *** *** cObject_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

// Owns its objects: each is held in a unique_ptr, so nothing is ever hand-deleted and the
// destructor frees the lot. `objects` stays a public vector; callers read borrowed T* via
// `.get()` (Get_Pointer) or range-for over the unique_ptrs.
template <class T>
class cObject_Manager {
 public:
  using OwningList = vector<std::unique_ptr<T>>;

  cObject_Manager(void) = default;
  virtual ~cObject_Manager(void) = default;

  //	Add the given object (takes ownership)
  virtual void Add(T* obj) { objects.emplace_back(obj); }

  // Delete the given object
  virtual bool Delete(T* obj, bool delete_data = true) {
    // empty object
    if (!obj) {
      return false;
    }

    // get iterator
    typename OwningList::iterator itr = std::find_if(
        objects.begin(), objects.end(),
        [obj](const std::unique_ptr<T>& p) { return p.get() == obj; });

    // available in vector
    if (itr != objects.end()) {
      if (!delete_data) {
        // caller keeps ownership: drop it from the vector without freeing
        itr->release();
      }
      // erase frees it when we still own it
      objects.erase(itr);
    } else if (delete_data) {
      // not managed here, but the old contract still freed a passed-in obj
      delete obj;
    }

    return true;
  }

  // Delete all objects (unique_ptr frees each)
  virtual void Delete_All(void) { objects.clear(); }

  /* Return the object pointer
	 * if not found returns NULL
	*/
  virtual T* Get_Pointer(size_t identifier) const {
    if (identifier >= objects.size()) {
      // out of array
      return nullptr;
    }

    // available
    return objects[identifier].get();
  }

  /* Return the object array number
	 * if not found returns -1
	*/
  int Get_Array_Num(const T* obj) const {
    // invalid
    if (!obj) {
      return -1;
    }

    typename OwningList::const_iterator itr_obj = std::find_if(
        objects.begin(), objects.end(),
        [obj](const std::unique_ptr<T>& p) { return p.get() == obj; });

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

  OwningList objects;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
