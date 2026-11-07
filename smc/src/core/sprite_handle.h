/***************************************************************************
 * sprite_handle.h  -  a stored reference to a sprite that knows when it is gone
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_SPRITE_HANDLE_H
#define SMC_SPRITE_HANDLE_H

#include <cstddef>
#include <cstdint>

namespace SMC {

class cSprite;

/* *** *** *** *** *** cSprite_Serial *** *** *** *** *** *** *** *** *** *** *** *** */

// Every cSprite holds one. It gives the sprite a serial number that is never reused and records
// serial -> sprite in a registry for as long as the sprite exists, so a cSprite_Handle can tell a
// live sprite from a freed one (even when a new sprite is later allocated at the same address).
class cSprite_Serial {
 public:
  explicit cSprite_Serial(cSprite* sprite);
  ~cSprite_Serial(void);
  cSprite_Serial(const cSprite_Serial&) = delete;
  cSprite_Serial& operator=(const cSprite_Serial&) = delete;

  uint64_t Value(void) const { return m_value; }

  // The sprite with the given serial, or nullptr once it has been freed (0 is never issued)
  static cSprite* Find(uint64_t serial);

 private:
  uint64_t m_value;
};

// The serial of the given sprite, or 0 for nullptr
uint64_t Sprite_Serial_Of(const cSprite* sprite);

/* *** *** *** *** *** cSprite_Handle *** *** *** *** *** *** *** *** *** *** *** *** */

// A stored, non-owning reference to a sprite owned elsewhere (a sprite manager, the level
// player, ...): the member form of a cSprite* whose target can be freed first. It reads like a
// pointer -- assign a T*, test it, compare it, use -> -- but resolves through the registry on
// every use, so once the sprite is freed it reads as nullptr instead of dangling. T is cSprite or
// a class derived from it; the handle is only ever assigned a T*.
template <class T = cSprite>
class cSprite_Handle {
 public:
  cSprite_Handle(void) = default;
  cSprite_Handle(std::nullptr_t) {}
  cSprite_Handle(T* sprite) : m_serial(Sprite_Serial_Of(sprite)) {}

  // The sprite, or nullptr if it was never set or has been freed
  T* Get(void) const { return static_cast<T*>(cSprite_Serial::Find(m_serial)); }

  operator T*(void) const { return Get(); }
  T* operator->(void) const { return Get(); }

 private:
  uint64_t m_serial = 0;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
