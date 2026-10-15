/***************************************************************************
 * world_sprite_manager.cpp  -  World Sprite Manager
 *
 * Copyright (C) 2008 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../overworld/world_sprite_manager.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../overworld/world_layer.h"
#include "../overworld/world_waypoint.h"

namespace SMC {

/* *** *** *** *** *** *** cWorld_Sprite_Manager *** *** *** *** *** *** *** *** *** *** *** */

cWorld_Sprite_Manager ::cWorld_Sprite_Manager(cOverworld* overworld)
    : cSprite_Manager(500) {
  m_overworld = overworld;
}

cWorld_Sprite_Manager ::~cWorld_Sprite_Manager(void) { Delete_All(); }

cSprite* cWorld_Sprite_Manager ::Add(std::unique_ptr<cSprite> sprite) {
  // empty sprite
  if (!sprite) {
    return nullptr;
  }

  // add (the manager owns it from here; `added` is the borrowed pointer)
  cSprite* added = cSprite_Manager::Add(std::move(sprite));

  // Add to Waypoints array
  if (added->m_type == TYPE_OW_WAYPOINT) {
    m_overworld->m_waypoints.push_back(static_cast<cWaypoint*>(added));
  }
  // Register layer line point start with the world layer (which borrows it)
  else if (added->m_type == TYPE_OW_LINE_START) {
    m_overworld->m_layer.Register(static_cast<cLayer_Line_Point_Start*>(added));
  }

  return added;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
