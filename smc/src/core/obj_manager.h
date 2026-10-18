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

// Owns its objects. Ownership arrives through Add (a unique_ptr, so the hand-off is a
// std::move at the call site) and leaves only through Release; everything else borrows a T*.
// `objects` stays a public vector of owning pointers: range-for over it, `.get()` only where a
// raw T* has to be stored or compared.
template <class T>
class cObject_Manager {
 public:
  using OwningList = vector<std::unique_ptr<T>>;

  cObject_Manager(void) = default;
  virtual ~cObject_Manager(void) = default;

  // Take ownership of the given object; returns the borrowed pointer for further use
  virtual T* Add(std::unique_ptr<T> obj) {
    T* raw = obj.get();
    objects.push_back(std::move(obj));
    return raw;
  }

  // Remove and free the given object; false if it is not managed here
  virtual bool Delete(T* obj) {
    auto itr = Find(obj);
    if (itr == objects.end()) {
      return false;
    }

    objects.erase(itr);
    return true;
  }

  // Remove the given object WITHOUT freeing it: ownership passes back to the caller
  // (an empty unique_ptr if it is not managed here)
  [[nodiscard]] std::unique_ptr<T> Release(T* obj) {
    auto itr = Find(obj);
    if (itr == objects.end()) {
      return nullptr;
    }

    std::unique_ptr<T> owned = std::move(*itr);
    objects.erase(itr);
    return owned;
  }

  // Delete all objects (unique_ptr frees each). cSprite_Manager intentionally narrows this to a
  // delayed-delete overload; the suppression lives on this base decl, where -Woverloaded-virtual is
  // attributed (maintainer, 2026-10-18; tasks/smc-overloaded-virtual.md).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  virtual void Delete_All(void) { objects.clear(); }
#pragma GCC diagnostic pop

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

    auto itr_obj = std::find_if(
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

 protected:
  // The owning slot of the given (borrowed) pointer, or end()
  typename OwningList::iterator Find(const T* obj) {
    if (!obj) {
      return objects.end();
    }

    return std::find_if(
        objects.begin(), objects.end(),
        [obj](const std::unique_ptr<T>& p) { return p.get() == obj; });
  }
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
