/***************************************************************************
 * sprite_handle.cpp  -  the live-sprite registry behind cSprite_Handle
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/sprite_handle.h"
#include "../objects/sprite.h"
#include <unordered_map>

namespace SMC {

namespace {

// serial -> live sprite; a function-local static so it exists before the first sprite and
// outlives the last one (the game is single-threaded)
std::unordered_map<uint64_t, cSprite*>& Live_Sprites(void) {
  static std::unordered_map<uint64_t, cSprite*> live;
  return live;
}

uint64_t last_serial = 0;

}  // namespace

cSprite_Serial::cSprite_Serial(cSprite* sprite) : m_value(++last_serial) {
  Live_Sprites().emplace(m_value, sprite);
}

cSprite_Serial::~cSprite_Serial(void) { Live_Sprites().erase(m_value); }

cSprite* cSprite_Serial::Find(uint64_t serial) {
  if (serial == 0) {
    return nullptr;
  }

  auto itr = Live_Sprites().find(serial);
  return itr == Live_Sprites().end() ? nullptr : itr->second;
}

uint64_t Sprite_Serial_Of(const cSprite* sprite) {
  return sprite ? sprite->m_serial.Value() : 0;
}

}  // namespace SMC
