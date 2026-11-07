/***************************************************************************
 * owning_vector.h  -  a vector that owns its objects, and the algorithms over it
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

#ifndef SMC_OWNING_VECTOR_H
#define SMC_OWNING_VECTOR_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace SMC {

// A class that owns a list of objects holds one of these as a PRIVATE member: ownership arrives
// as a unique_ptr (a std::move at the call site) and everyone else borrows a T*. The functions
// below are the lookups the owners share; each owner exposes only the operations its callers use.
template <class T>
using Owning_Vector = std::vector<std::unique_ptr<T>>;

// The owning slot of the given borrowed pointer, or end()
template <class T>
typename Owning_Vector<T>::iterator Find_Owned(Owning_Vector<T>& objects,
                                               const T* obj) {
  if (!obj) {
    return objects.end();
  }

  return std::ranges::find_if(
      objects, [obj](const std::unique_ptr<T>& p) { return p.get() == obj; });
}

// Remove and free the given object; false if it is not in the vector
template <class T>
bool Erase_Owned(Owning_Vector<T>& objects, const T* obj) {
  auto itr = Find_Owned(objects, obj);
  if (itr == objects.end()) {
    return false;
  }

  objects.erase(itr);
  return true;
}

// Remove the given object WITHOUT freeing it: ownership passes back to the caller
// (an empty unique_ptr if it is not in the vector)
template <class T>
[[nodiscard]] std::unique_ptr<T> Release_Owned(Owning_Vector<T>& objects,
                                               const T* obj) {
  auto itr = Find_Owned(objects, obj);
  if (itr == objects.end()) {
    return nullptr;
  }

  std::unique_ptr<T> owned = std::move(*itr);
  objects.erase(itr);
  return owned;
}

// The object at the given index, or nullptr when the index is out of range
template <class T>
T* Borrow_At(const Owning_Vector<T>& objects, size_t index) {
  return index < objects.size() ? objects[index].get() : nullptr;
}

// The index of the given object, or -1 when it is null or not in the vector
template <class T>
int Index_Of(const Owning_Vector<T>& objects, const T* obj) {
  if (!obj) {
    return -1;
  }

  auto itr = std::ranges::find_if(
      objects, [obj](const std::unique_ptr<T>& p) { return p.get() == obj; });
  return itr == objects.end() ? -1 : static_cast<int>(itr - objects.begin());
}

// A borrowing view of the objects (each element a T*), for range-for. Like iterating the vector
// itself, the view must not outlive it, and objects added during the loop are not visited.
template <class T>
auto Borrowed(const Owning_Vector<T>& objects) {
  return objects | std::views::transform(
                       [](const std::unique_ptr<T>& p) { return p.get(); });
}

}  // namespace SMC

#endif
