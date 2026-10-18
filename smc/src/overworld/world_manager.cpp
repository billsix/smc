/***************************************************************************
 * worlds.cpp  -  class for handling worlds data
 *
 * Copyright (C) 2004 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../overworld/world_manager.h"
#include "../core/game_core.h"
#include "../overworld/overworld.h"
#include "../core/filesystem/filesystem.h"
#include "../core/filesystem/resource_manager.h"
#include "../overworld/world_editor.h"
#include "../input/mouse.h"
#include "../video/animation.h"
// CEGUI
#include "CEGUIXMLParser.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** cOverworld_Manager *** *** *** *** *** *** *** *** *** */

cOverworld_Manager ::cOverworld_Manager(cSprite_Manager* sprite_manager)
    : cObject_Manager<cOverworld>(), m_camera(sprite_manager) {
  m_debug_mode = false;
  m_draw_layer = false;
  m_camera_mode = false;

  Init();
}

cOverworld_Manager ::~cOverworld_Manager(void) { Delete_All(); }

bool cOverworld_Manager ::New(std::string name) {
  string_trim(name, ' ');

  // no name given
  if (name.empty()) {
    return false;
  }

  // name already exists
  if (Get_from_Path(name)) {
    return false;
  }

  auto overworld = std::make_unique<cOverworld>();
  overworld->New(name);
  Add(std::move(overworld));

  return true;
}

void cOverworld_Manager ::Init(void) {
  // if already loaded
  if (!objects.empty()) {
    Delete_All();
  }

  // Load Worlds
  Load_Dir(pResource_Manager->user_data_dir + USER_WORLD_DIR, true);
  Load_Dir(DATA_DIR "/" GAME_OVERWORLD_DIR);
}

void cOverworld_Manager ::Load_Dir(const std::string& dir,
                                   bool user_dir /* = 0 */) {
  // set world directory
  vector<std::string> const subdirs = Get_Directory_Files(dir, "", true, false);

  for (const auto& subdir : subdirs) {
    try {
      std::string const current_dir = subdir;

      // only directories with an existing description
      if (File_Exists(current_dir + "/description.xml")) {
        cOverworld* overworld = Get_from_Path(current_dir);

        // already available
        if (overworld) {
          overworld->m_description.m_user = 2;
          continue;
        }

        auto created = std::make_unique<cOverworld>();

        // get relative path
        std::string const relative_path(current_dir, current_dir.rfind('/'));

        // set relative path
        created->m_description.m_path = relative_path;
        // default name is the path
        created->m_description.m_name = relative_path;
        // set user
        created->m_description.m_user = user_dir;

        overworld = Add(std::move(created));

        overworld->Load();
      }
    } catch (const std::exception& ex) {
      printf("%s %s\n", subdir.c_str(), ex.what());
    }
  }
}

bool cOverworld_Manager ::Set_Active(const std::string& str) {
  return Set_Active(Get(str));
}

bool cOverworld_Manager ::Set_Active(cOverworld* world) {
  if (!world) {
    return false;
  }

  pActive_Overworld = world;

  pWorld_Editor->Set_Sprite_Manager(&world->m_sprite_manager);
  pWorld_Editor->Set_Overworld(world);
  m_camera.Set_Sprite_Manager(&world->m_sprite_manager);
  pOverworld_Player->Set_Sprite_Manager(&world->m_sprite_manager);
  pOverworld_Player->Set_Overworld(world);
  pOverworld_Player->Reset();

  // pre-update animations
  for (const auto& sprite : world->m_sprite_manager.objects) {
    if (sprite->m_type == TYPE_PARTICLE_EMITTER) {
      cParticle_Emitter* emitter =
          static_cast<cParticle_Emitter*>(sprite.get());
      emitter->Pre_Update();
    }
  }

  return true;
}

void cOverworld_Manager ::Reset(void) {
  // default Overworld
  Set_Active("World 1");

  // Set Player to first Waypoint
  pOverworld_Player->Set_Waypoint(
      static_cast<int>(pActive_Overworld->m_player_start_waypoint));

  // Reset all Waypoints
  for (const auto& overworld : objects) {
    overworld->Reset_Waypoints();
  }
}

cOverworld* cOverworld_Manager ::Get(const std::string& str) {
  cOverworld* world = Get_from_Name(str);

  if (world) {
    return world;
  }

  return Get_from_Path(str);
}

cOverworld* cOverworld_Manager ::Get_from_Path(const std::string& path) {
  for (const auto& overworld : objects) {
    if (overworld->m_description.m_path.compare(path) == 0) {
      return overworld.get();
    }
  }

  return nullptr;
}

cOverworld* cOverworld_Manager ::Get_from_Name(const std::string& name) {
  for (const auto& overworld : objects) {
    if (overworld->m_description.m_name.compare(name) == 0) {
      return overworld.get();
    }
  }

  return nullptr;
}

int cOverworld_Manager ::Get_Array_Num(const std::string& path) const {
  for (unsigned int i = 0; i < objects.size(); i++) {
    if (objects[i]->m_description.m_path.compare(path) == 0) {
      return static_cast<int>(i);
    }
  }

  return -1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cOverworld_Manager* pOverworld_Manager = nullptr;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
