/***************************************************************************
 * sprite_manager.cpp  -  Sprite Manager
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

#include "../core/sprite_manager.h"
#include "../core/game_core.h"
#include "../level/level_player.h"
#include "../input/mouse.h"
#include "../overworld/world_player.h"
#include <algorithm>

namespace SMC {

/* *** *** *** *** *** *** cSprite_Manager *** *** *** *** *** *** *** *** *** *** *** */

cSprite_Manager ::cSprite_Manager(unsigned int reserve_items /* = 2000 */,
                                  unsigned int zpos_items /* = 100 */)
    : cObject_Manager<cSprite>() {
  objects.reserve(reserve_items);

  m_z_pos_data.assign(zpos_items, 0.0f);
  m_z_pos_data_editor.assign(zpos_items, 0.0f);
}

cSprite_Manager ::~cSprite_Manager(void) { Delete_All(); }

cSprite* cSprite_Manager ::Add(std::unique_ptr<cSprite> sprite) {
  // empty object
  if (!sprite) {
    return nullptr;
  }

  Set_Pos_Z(sprite.get());

  // Check if an destroyed object can be replaced
  for (auto& slot : objects) {
    // if destroy is set, reuse the slot (the move frees the old sprite and adopts the new)
    if (slot->m_auto_destroy) {
      slot = std::move(sprite);
      return slot.get();
    }
  }

  return cObject_Manager<cSprite>::Add(std::move(sprite));
}

void cSprite_Manager ::Set_Pos_Z(cSprite* sprite) {
  // don't set particle effect z position
  if (sprite->m_type == TYPE_ANIMATION ||
      sprite->m_type == TYPE_PARTICLE_EMITTER) {
    return;
  }

  // set new z position if unset
  if (sprite->m_pos_z <= m_z_pos_data[sprite->m_type]) {
    sprite->m_pos_z = m_z_pos_data[sprite->m_type] + 0.000001f;
  }
  // if editor z position is given
  if (sprite->m_editor_pos_z > 0.0f) {
    if (sprite->m_editor_pos_z <= m_z_pos_data_editor[sprite->m_type]) {
      sprite->m_editor_pos_z = m_z_pos_data_editor[sprite->m_type] + 0.000001f;
    }
  }

  // update z position
  if (sprite->m_pos_z > m_z_pos_data[sprite->m_type]) {
    m_z_pos_data[sprite->m_type] = sprite->m_pos_z;
  }
  // if editor z position is given
  if (sprite->m_editor_pos_z > 0.0f) {
    if (sprite->m_editor_pos_z > m_z_pos_data_editor[sprite->m_type]) {
      m_z_pos_data_editor[sprite->m_type] = sprite->m_editor_pos_z;
    }
  }
}

void cSprite_Manager ::Move_To_Front(cSprite* sprite) {
  // not needed
  if (objects.size() <= 1) {
    return;
  }

  // if already in front
  if (sprite == objects.front().get()) {
    return;
  }

  // get iterator
  auto itr =
      std::find_if(objects.begin(), objects.end(),
                   [sprite](const std::unique_ptr<cSprite>& p) {
                     return p.get() == sprite;
                   });

  // not available
  if (itr == objects.end()) {
    // fixme : should not happen but it does
    return;
  }

  // move the sprite's ownership to the front, keeping the others in order (no free)
  std::unique_ptr<cSprite> owned = std::move(*itr);
  objects.erase(itr);
  objects.insert(objects.begin(), std::move(owned));

  // make it the first z position
  sprite->m_pos_z = Get_First(sprite->m_type)->m_pos_z - 0.000001f;
}

void cSprite_Manager ::Move_To_Back(cSprite* sprite) {
  // not needed
  if (objects.size() <= 1) {
    return;
  }

  // if already in back
  if (sprite == objects.back().get()) {
    return;
  }

  // get iterator
  auto itr =
      std::find_if(objects.begin(), objects.end(),
                   [sprite](const std::unique_ptr<cSprite>& p) {
                     return p.get() == sprite;
                   });

  // not available
  if (itr == objects.end()) {
    // fixme : should not happen but it does
    return;
  }

  // move the sprite's ownership to the back, keeping the others in order (no free)
  std::unique_ptr<cSprite> owned = std::move(*itr);
  objects.erase(itr);
  objects.push_back(std::move(owned));

  // make it the last z position
  sprite->m_pos_z = Get_Last(sprite->m_type)->m_pos_z + 0.000001f;
}

void cSprite_Manager ::Delete_All(bool delayed /* = 0 */) {
  // delayed
  if (delayed) {
    for (const auto& sprite : objects) {
      // get sprite pointer
      sprite->Destroy();
    }
  }
  // instant
  else {
    // remove objects that can not be auto-deleted (the player etc. are owned elsewhere:
    // release before erasing so they are not freed here)
    for (auto itr = objects.begin(); itr != objects.end();) {
      if ((*itr)->m_disallow_managed_delete) {
        itr->release();
        itr = objects.erase(itr);
      }
      // increment
      else {
        ++itr;
      }
    }

    cObject_Manager<cSprite>::Delete_All();
  }

  // clear z position data
  std::fill(m_z_pos_data.begin(), m_z_pos_data.end(), 0.0f);
  std::fill(m_z_pos_data_editor.begin(), m_z_pos_data_editor.end(), 0.0f);
}

cSprite* cSprite_Manager ::Get_First(const SpriteType type) const {
  cSprite* first = nullptr;

  for (const auto& sprite : objects) {
    if (sprite->m_type == type &&
        (!first || sprite->m_pos_z < first->m_pos_z)) {
      first = sprite.get();
    }
  }

  // return result
  return first;
}

cSprite* cSprite_Manager ::Get_Last(const SpriteType type) const {
  cSprite* last = nullptr;

  for (const auto& sprite : objects) {
    if (sprite->m_type == type && (!last || sprite->m_pos_z > last->m_pos_z)) {
      last = sprite.get();
    }
  }

  // return result
  return last;
}

cSprite* cSprite_Manager ::Get_from_Position(
    int start_pos_x, int start_pos_y,
    const SpriteType type /* = TYPE_UNDEFINED */,
    int check_pos /* = 0 */) const {
  for (const auto& sprite : objects) {
    if (static_cast<int>(sprite->m_start_pos_x) != start_pos_x ||
        static_cast<int>(sprite->m_start_pos_y) != start_pos_y) {
      if (check_pos != 2 ||
          (static_cast<int>(sprite->m_pos_x) != start_pos_x &&
           static_cast<int>(sprite->m_pos_y) != start_pos_y)) {
        continue;
      }
    }

    if (check_pos == 1 && (static_cast<int>(sprite->m_pos_x) != start_pos_x ||
                           static_cast<int>(sprite->m_pos_y) != start_pos_y)) {
      continue;
    }

    // if type is given
    if (type != TYPE_UNDEFINED) {
      // skip invalid type
      if (sprite->m_type != type) {
        continue;
      }
    }

    // found
    return sprite.get();
  }

  return nullptr;
}

void cSprite_Manager ::Get_Objects_sorted(cSprite_List& new_objects,
                                          bool editor_sort /* = 0 */,
                                          bool with_player /* = 0 */) const {
  // copy the borrowed pointers (the manager keeps ownership via unique_ptr)
  new_objects.clear();
  new_objects.reserve(objects.size());
  for (const auto& sprite_ptr : objects) {
    new_objects.push_back(sprite_ptr.get());
  }

  if (with_player) {
    new_objects.push_back(pActive_Player);
  }

  // z position sort
  if (!editor_sort) {
    // default
    std::sort(new_objects.begin(), new_objects.end(), zpos_sort());
  } else {
    // editor
    std::sort(new_objects.begin(), new_objects.end(), editor_zpos_sort());
  }
}

void cSprite_Manager ::Get_Colliding_Objects(
    cSprite_List& col_objects, const GL_rect& rect, bool with_player /* = 0 */,
    const cSprite* exclude_sprite /* = nullptr */) const {
  // Check objects
  for (const auto& sprite : objects) {
    // if destroyed sprite
    if (sprite.get() == exclude_sprite || sprite->m_auto_destroy) {
      continue;
    }

    // if rects don't touch
    if (!rect.Intersects(sprite->m_col_rect)) {
      continue;
    }

    col_objects.push_back(sprite.get());
  }

  if (with_player && pActive_Player != exclude_sprite) {
    if (rect.Intersects(pActive_Player->m_col_rect)) {
      col_objects.push_back(pActive_Player);
    }
  }
}

void cSprite_Manager ::Handle_Collision_Items(void) {
  for (const auto& sprite : objects) {
    if (sprite->m_auto_destroy) {
      if (sprite->m_collisions.size()) {
        debug_print("Collision with a destroyed sprite (%s)\n",
                    sprite->m_name.c_str());
        sprite->Clear_Collisions();
      }

      continue;
    }

    // collision and movement handling
    sprite->Collide_Move();
    // handle found collisions
    sprite->Handle_Collisions();
  }
}

unsigned int cSprite_Manager ::Get_Size_Array(const ArrayType sprite_array) {
  unsigned int count = 0;

  for (const auto& sprite : objects) {
    if (sprite->m_sprite_array == sprite_array) {
      count++;
    }
  }

  return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
