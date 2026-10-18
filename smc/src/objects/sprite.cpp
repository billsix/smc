/***************************************************************************
 * sprite.cpp  -  basic sprite class
 *
 * Copyright (C) 2003 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ranges>

#include "../objects/sprite.h"
#include "../user/savegame.h"
#include "../objects/movingsprite.h"
#include "../core/game_core.h"
#include "../level/level.h"
#include "../core/framerate.h"
#include "../level/level_player.h"
#include "../gui/hud.h"
#include "../video/gl_surface.h"
#include "../video/renderer.h"
#include "../core/sprite_manager.h"
#include "../core/editor.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIWindowManager.h"
#include "CEGUIFontManager.h"
#include "elements/CEGUIEditbox.h"
#include "elements/CEGUICombobox.h"
#include "elements/CEGUIComboDropList.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** cCollidingSprite *** *** *** *** *** *** *** *** *** */

cCollidingSprite ::cCollidingSprite(cSprite_Manager* sprite_manager) {
  m_sprite_manager = sprite_manager;
}

cCollidingSprite ::~cCollidingSprite(void) { Clear_Collisions(); }

void cCollidingSprite ::Set_Sprite_Manager(cSprite_Manager* sprite_manager) {
  m_sprite_manager = sprite_manager;
}

void cCollidingSprite ::Handle_Collisions(void) {
  // take the collision list (handlers may add new collisions meanwhile); it frees its
  // entries when this function returns
  cObjectCollision_List col_list;
  m_collisions.swap(col_list);

  // parse the given collisions
  for (const auto& collision : col_list) {
    // handle it
    Handle_Collision(collision.get());
  }
}

std::unique_ptr<cObjectCollision> cCollidingSprite ::Create_Collision_Object(
    const cSprite* base, cSprite* col, Col_Valid_Type valid_type) const {
  // if invalid
  if (!base || valid_type == COL_VTYPE_NOT_VALID) {
    return nullptr;
  }

  // create
  auto collision = std::make_unique<cObjectCollision>();

  // if col object is available
  if (col) {
    // object
    collision->m_obj = col;
    // identifier
    if (col->m_sprite_array != ARRAY_PLAYER) {
      collision->m_number = m_sprite_manager->Get_Array_Num(col);
    }
    // type
    collision->m_array = col->m_sprite_array;
    // direction
    collision->Set_Direction(base, col);
  }

  // valid type
  collision->m_valid_type = valid_type;

  return collision;
}

bool cCollidingSprite ::Add_Collision(
    std::unique_ptr<cObjectCollision> collision, bool add_if_new /* = 0 */) {
  // invalid collision data
  if (!collision) {
    return false;
  }

  // check if collision data is new
  if (add_if_new) {
    // already in list (the duplicate is freed on return)
    if (Is_Collision_Included(collision->m_obj)) {
      return false;
    }
  }

  m_collisions.push_back(std::move(collision));

  return true;
}

void cCollidingSprite ::Add_Collisions(cObjectCollisionType* col_list,
                                       bool add_if_new /* = 0 */) {
  // move ownership of each collision out of col_list into m_collisions
  for (auto& collision : col_list->objects) {
    Add_Collision(std::move(collision), add_if_new);
  }

  col_list->objects.clear();
}

void cCollidingSprite ::Delete_Collision(cObjectCollision* collision) {
  if (!collision) {
    return;
  }

  // erasing the owning slot frees it
  std::erase_if(m_collisions,
                [collision](const std::unique_ptr<cObjectCollision>& p) {
                  return p.get() == collision;
                });
}

void cCollidingSprite ::Delete_Last_Collision(void) {
  if (m_collisions.empty()) {
    return;
  }

  m_collisions.pop_back();
}

int cCollidingSprite ::Is_Collision_In_Direction(
    const ObjectDirection dir) const {
  int pos = 0;

  for (const auto& col : m_collisions) {
    if (col->m_direction == dir) {
      return pos;
    }

    pos++;
  }

  return -1;
}

cObjectCollision* cCollidingSprite ::Get_Last_Collision(
    bool only_blocking /* = 0 */) const {
  // no collisions available
  if (m_collisions.empty()) {
    return nullptr;
  }

  if (only_blocking) {
    for (const auto& col : std::views::reverse(m_collisions)) {
      // get object pointer
      cSprite const* col_obj =
          m_sprite_manager->Get_Pointer(static_cast<size_t>(col->m_number));

      // ignore passive
      if (col_obj->m_massive_type == MASS_PASSIVE) {
        continue;
      }
      // if active check if not climbable
      if (col->m_array == ARRAY_ACTIVE) {
        // not a valid object
        if (col_obj->m_massive_type == MASS_CLIMBABLE) {
          continue;
        }
      }

      // is blocking
      return col.get();
    }

    // not found
    return nullptr;
  }

  return m_collisions.back().get();
}

bool cCollidingSprite ::Is_Collision_Included(const cSprite* obj) const {
  // check if in collisions list
  for (const auto& col : m_collisions) {
    // get object pointer
    // is in list
    if (col->m_obj == obj) {
      return true;
    }
  }

  // not found
  return false;
}

void cCollidingSprite ::Clear_Collisions(void) { m_collisions.clear(); }

void cCollidingSprite ::Handle_Collision(cObjectCollision* collision) {
  // player
  if (collision->m_array == ARRAY_PLAYER) {
    Handle_Collision_Player(collision);
  }
  // enemy
  else if (collision->m_array == ARRAY_ENEMY) {
    Handle_Collision_Enemy(collision);
  }
  // massive
  else if (collision->m_array == ARRAY_MASSIVE ||
           collision->m_array == ARRAY_ACTIVE) {
    Handle_Collision_Massive(collision);
  }
  // passive
  else if (collision->m_array == ARRAY_PASSIVE) {
    Handle_Collision_Passive(collision);
  }
}

/* *** *** *** *** *** *** *** cSprite *** *** *** *** *** *** *** *** *** *** */

const float cSprite::m_pos_z_passive_start = 0.01f;
const float cSprite::m_pos_z_massive_start = 0.08f;
const float cSprite::m_pos_z_front_passive_start = 0.1f;
const float cSprite::m_pos_z_halfmassive_start = 0.04f;

cSprite ::cSprite(cSprite_Manager* sprite_manager,
                  const std::string type_name /* = "sprite" */)
    : cCollidingSprite(sprite_manager), m_type_name(type_name) {
  cSprite::Init();
}

cSprite ::cSprite(CEGUI::XMLAttributes& attributes,
                  cSprite_Manager* sprite_manager,
                  const std::string type_name /* = "sprite" */)
    : cCollidingSprite(sprite_manager), m_type_name(type_name) {
  cSprite::Init();
  cSprite::Load_From_XML(attributes);
}

// Out of line on purpose: a unique_ptr to a type the header only forward-declares cannot
// be destroyed inline (the deleter needs the complete type), so the body lives here
std::unique_ptr<cSave_Level_Object> cSprite ::Save_To_Savegame(void) {
  return nullptr;
}

cSprite ::~cSprite(void) = default;

void cSprite ::Init(void) {
  // undefined
  m_type = TYPE_UNDEFINED;
  m_sprite_array = ARRAY_UNDEFINED;

  // collision data
  m_col_pos.m_x = 0.0f;
  m_col_pos.m_y = 0.0f;
  m_col_rect.m_x = 0.0f;
  m_col_rect.m_y = 0.0f;
  m_col_rect.m_w = 0.0f;
  m_col_rect.m_h = 0.0f;
  // image data
  m_rect.m_x = 0.0f;
  m_rect.m_y = 0.0f;
  m_rect.m_w = 0.0f;
  m_rect.m_h = 0.0f;
  m_start_rect.clear();

  m_start_pos_x = 0.0f;
  m_start_pos_y = 0.0f;

  m_start_image = nullptr;
  m_image = nullptr;
  m_auto_destroy = false;
  m_shadow_pos = 0.0f;
  m_shadow_color = black;
  m_no_camera = false;

  m_color = static_cast<Uint8>(255);

  m_combine_type = 0;
  m_combine_color[0] = 0.0f;
  m_combine_color[1] = 0.0f;
  m_combine_color[2] = 0.0f;

  m_pos_x = 0.0f;
  m_pos_y = 0.0f;
  m_pos_z = 0.0f;
  m_editor_pos_z = 0.0f;

  m_massive_type = MASS_PASSIVE;
  m_active = true;
  m_spawned = false;
  m_camera_range = 1000;
  m_can_be_ground = false;
  m_disallow_managed_delete = false;

  // rotation
  m_rotation_affects_rect = false;
  m_start_rot_x = 0.0f;
  m_start_rot_y = 0.0f;
  m_start_rot_z = 0.0f;
  m_rot_x = 0.0f;
  m_rot_y = 0.0f;
  m_rot_z = 0.0f;
  // scale
  m_scale_affects_rect = false;
  m_scale_up = false;
  m_scale_down = true;
  m_scale_left = false;
  m_scale_right = true;
  m_start_scale_x = 1.0f;
  m_start_scale_y = 1.0f;
  m_scale_x = 1.0f;
  m_scale_y = 1.0f;

  m_valid_draw = true;
  m_valid_update = true;

  m_editor_window_name_width = 0.0f;
}

std::unique_ptr<cSprite> cSprite ::Copy(void) const {
  auto basic_sprite = std::make_unique<cSprite>(m_sprite_manager);
  basic_sprite->Set_Image(m_start_image, true);
  basic_sprite->Set_Pos(m_start_pos_x, m_start_pos_y, true);
  basic_sprite->m_type = m_type;
  basic_sprite->m_sprite_array = m_sprite_array;
  basic_sprite->Set_Massive_Type(m_massive_type);
  basic_sprite->m_can_be_ground = m_can_be_ground;
  basic_sprite->Set_Rotation_Affects_Rect(m_rotation_affects_rect);
  basic_sprite->Set_Scale_Affects_Rect(m_scale_affects_rect);
  basic_sprite->Set_Scale_Directions(m_scale_up, m_scale_down, m_scale_left,
                                     m_scale_right);
  basic_sprite->Set_Ignore_Camera(m_no_camera);
  basic_sprite->Set_Shadow_Pos(m_shadow_pos);
  basic_sprite->Set_Shadow_Color(m_shadow_color);
  basic_sprite->Set_Spawned(m_spawned);
  return basic_sprite;
}

void cSprite ::Load_From_XML(CEGUI::XMLAttributes& attributes) {
  // position
  Set_Pos(static_cast<float>(attributes.getValueAsInteger("posx")),
          static_cast<float>(attributes.getValueAsInteger("posy")), true);
  // image
  Set_Image(pVideo->Get_Surface(attributes.getValueAsString("image").c_str()),
            true);
  // type
  Set_Sprite_Type(
      Get_Sprite_Type_Id(attributes.getValueAsString("type").c_str()));
}

void cSprite ::Save_To_XML(CEGUI::XMLSerializer& stream) {
  // begin
  stream.openTag(m_type_name);

  // position
  Write_Property(stream, "posx", static_cast<int>(m_start_pos_x));
  Write_Property(stream, "posy", static_cast<int>(m_start_pos_y));
  // image
  std::string img_filename;

  if (m_start_image) {
    img_filename = m_start_image->m_filename;
  } else if (m_image) {
    img_filename = m_image->m_filename;
  } else {
    printf("Warning: cSprite::Save_To_XML no image from type %d\n", m_type);
  }

  // remove pixmaps directory from string
  if (img_filename.find(DATA_DIR "/" GAME_PIXMAPS_DIR "/") == 0) {
    img_filename.erase(0, strlen(DATA_DIR "/" GAME_PIXMAPS_DIR "/"));
  }
  Write_Property(stream, "image", img_filename);
  // type
  Write_Property(stream, "type", Get_Sprite_Type_String());

  // end
  stream.closeTag();
}

void cSprite ::Set_Image(std::unique_ptr<cGL_Surface> owned_image,
                         bool new_start_image /* = 0 */) {
  cGL_Surface* view = owned_image.get();
  Set_Image(view, new_start_image);
  m_owned_image = std::move(owned_image);
}

void cSprite ::Set_Image(cGL_Surface* new_image,
                         bool new_start_image /* = 0 */) {
  // an image this sprite owned is freed once something else replaces it
  if (m_owned_image && new_image != m_owned_image.get()) {
    if (m_start_image == m_owned_image.get()) {
      m_start_image = nullptr;
    }
    m_owned_image.reset();
  }

  m_image = new_image;

  if (m_image) {
    // collision data
    m_col_pos = m_image->m_col_pos;
    // scale affects the rect
    if (m_scale_affects_rect) {
      m_col_rect.m_w = m_image->m_col_w * m_scale_x;
      m_col_rect.m_h = m_image->m_col_h * m_scale_y;
      // image data
      m_rect.m_w = m_image->m_w * m_scale_x;
      m_rect.m_h = m_image->m_h * m_scale_y;
    }
    // scale does not affect the rect
    else {
      m_col_rect.m_w = m_image->m_col_w;
      m_col_rect.m_h = m_image->m_col_h;
      // image data
      m_rect.m_w = m_image->m_w;
      m_rect.m_h = m_image->m_h;
    }
    // rotation affects the rect
    if (m_rotation_affects_rect) {
      Update_Rect_Rotation();
    }

    // if no name is set use the first image name
    if (m_name.empty()) {
      m_name = m_image->m_name;
    }
    // if no editor tags are set use the first image editor tags
    if (m_editor_tags.empty()) {
      m_editor_tags = m_image->m_editor_tags;
    }
  } else {
    // clear image data
    m_col_pos.m_x = 0.0f;
    m_col_pos.m_y = 0.0f;
    m_col_rect.m_w = 0.0f;
    m_col_rect.m_h = 0.0f;
    m_rect.m_w = 0.0f;
    m_rect.m_h = 0.0f;
  }

  if (!m_start_image || new_start_image) {
    m_start_image = new_image;

    if (m_start_image) {
      m_start_rect.m_w = m_start_image->m_w;
      m_start_rect.m_h = m_start_image->m_h;

      // always set the image name for basic sprites
      if (Is_Basic_Sprite()) {
        m_name = m_start_image->m_name;
      }
    } else {
      m_start_rect.m_w = 0.0f;
      m_start_rect.m_h = 0.0f;
    }
  }

  // because col_pos could have changed
  Update_Position_Rect();
}

void cSprite ::Set_Sprite_Type(SpriteType type) {
  // set first because of massive-type z calculation
  m_type = type;

  if (m_type == TYPE_MASSIVE) {
    m_sprite_array = ARRAY_MASSIVE;
    Set_Massive_Type(MASS_MASSIVE);
    m_can_be_ground = true;
  } else if (m_type == TYPE_PASSIVE) {
    m_sprite_array = ARRAY_PASSIVE;
    Set_Massive_Type(MASS_PASSIVE);
    m_can_be_ground = false;
  } else if (m_type == TYPE_FRONT_PASSIVE) {
    m_sprite_array = ARRAY_PASSIVE;
    Set_Massive_Type(MASS_PASSIVE);
    m_can_be_ground = false;
  } else if (m_type == TYPE_HALFMASSIVE) {
    m_sprite_array = ARRAY_ACTIVE;
    Set_Massive_Type(MASS_HALFMASSIVE);
    m_can_be_ground = true;
  } else if (m_type == TYPE_CLIMBABLE) {
    m_sprite_array = ARRAY_ACTIVE;
    Set_Massive_Type(MASS_CLIMBABLE);
    m_can_be_ground = false;
  }
}

std::string cSprite ::Get_Sprite_Type_String(void) const {
  if (m_sprite_array == ARRAY_UNDEFINED) {
    return "undefined";
  } else if (m_sprite_array == ARRAY_PASSIVE) {
    if (m_type == TYPE_FRONT_PASSIVE) {
      return "front_passive";
    }

    return "passive";
  } else if (m_sprite_array == ARRAY_ACTIVE) {
    if (m_type == TYPE_HALFMASSIVE) {
      return "halfmassive";
    } else if (m_type == TYPE_CLIMBABLE) {
      return "climbable";
    } else {
      printf("Warning : Sprite array set as active but unknown type %d\n",
             m_type);
      return "active";
    }
  } else if (m_sprite_array == ARRAY_MASSIVE) {
    return "massive";
  } else if (m_sprite_array == ARRAY_HUD) {
    return "hud";
  } else if (m_sprite_array == ARRAY_ANIM) {
    return "animation";
  } else {
    printf("Warning : Sprite unknown array %d\n", m_sprite_array);
  }

  return "";
}

void cSprite ::Set_Ignore_Camera(bool enable /* = 0 */) {
  // already set
  if (m_no_camera == enable) {
    return;
  }

  m_no_camera = enable;

  Update_Valid_Draw();
}

void cSprite ::Set_Pos(float x, float y, bool new_startpos /* = 0 */) {
  m_pos_x = x;
  m_pos_y = y;

  if (new_startpos || (Is_Float_Equal(m_start_pos_x, 0.0f) &&
                       Is_Float_Equal(m_start_pos_y, 0.0f))) {
    m_start_pos_x = x;
    m_start_pos_y = y;
  }

  Update_Position_Rect();
}

void cSprite ::Set_Pos_X(float x, bool new_startpos /* = 0 */) {
  m_pos_x = x;

  if (new_startpos) {
    m_start_pos_x = x;
  }

  Update_Position_Rect();
}

void cSprite ::Set_Pos_Y(float y, bool new_startpos /* = 0 */) {
  m_pos_y = y;

  if (new_startpos) {
    m_start_pos_y = y;
  }

  Update_Position_Rect();
}

void cSprite ::Set_Active(bool enabled) {
  // already set
  if (m_active == enabled) {
    return;
  }

  m_active = enabled;

  Update_Valid_Draw();
  Update_Valid_Update();
}

void cSprite ::Set_Color_Combine(const float red_value, const float green_value,
                                 const float blue_value, const GLint com_type) {
  m_combine_type = com_type;
  m_combine_color[0] = Clamp(red_value, 0.000001f, 1.0f);
  m_combine_color[1] = Clamp(green_value, 0.000001f, 1.0f);
  m_combine_color[2] = Clamp(blue_value, 0.000001f, 1.0f);
}

void cSprite ::Update_Rect_Rotation_Z(void) {
  // rotate 270�
  if (m_rot_z >= 270.0f) {
    // rotate collision position
    float const orig_x = m_col_pos.m_x;
    m_col_pos.m_x = m_col_pos.m_y;
    m_col_pos.m_y = orig_x;

    // switch width and height
    float const orig_w = m_rect.m_w;
    m_rect.m_w = m_rect.m_h;
    m_rect.m_h = orig_w;
    // switch collision width and height
    float const orig_col_w = m_col_rect.m_w;
    m_col_rect.m_w = m_col_rect.m_h;
    m_col_rect.m_h = orig_col_w;
  }
  // mirror
  else if (m_rot_z >= 180.0f) {
    m_col_pos.m_x = m_rect.m_w - (m_col_rect.m_w + m_col_pos.m_x);
    m_col_pos.m_y = m_rect.m_h - (m_col_rect.m_h + m_col_pos.m_y);
  }
  // rotate 90�
  else if (m_rot_z >= 0.00001f) {
    // rotate collision position
    float const orig_x = m_col_pos.m_x;
    m_col_pos.m_x = m_rect.m_h - (m_col_rect.m_h + m_col_pos.m_y);
    m_col_pos.m_y = orig_x;

    // switch width and height
    float const orig_w = m_rect.m_w;
    m_rect.m_w = m_rect.m_h;
    m_rect.m_h = orig_w;
    // switch collision width and height
    float const orig_col_w = m_col_rect.m_w;
    m_col_rect.m_w = m_col_rect.m_h;
    m_col_rect.m_h = orig_col_w;
  }
}

void cSprite ::Set_Rotation_X(float rot, bool new_start_rot /* = 0 */) {
  m_rot_x = fmod(rot, 360.0f);

  if (new_start_rot) {
    m_start_rot_x = m_rot_x;
  }

  if (m_rotation_affects_rect) {
    Update_Rect_Rotation_X();
  }
}

void cSprite ::Set_Rotation_Y(float rot, bool new_start_rot /* = 0 */) {
  m_rot_y = fmod(rot, 360.0f);

  if (new_start_rot) {
    m_start_rot_y = m_rot_y;
  }

  if (m_rotation_affects_rect) {
    Update_Rect_Rotation_Y();
  }
}

void cSprite ::Set_Rotation_Z(float rot, bool new_start_rot /* = 0 */) {
  m_rot_z = fmod(rot, 360.0f);

  if (new_start_rot) {
    m_start_rot_z = m_rot_z;
  }

  if (m_rotation_affects_rect) {
    Update_Rect_Rotation_Z();
  }
}
void cSprite ::Set_Scale_X(const float scale,
                           const bool new_startscale /* = 0 */) {
  // invalid value
  if (Is_Float_Equal(scale, 0.0f)) {
    return;
  }

  // undo previous scale from rect
  if (m_scale_affects_rect && m_scale_x != 1.0f) {
    m_col_rect.m_w /= m_scale_x;
    m_rect.m_w /= m_scale_x;
  }

  m_scale_x = scale;

  // set new scale to rect
  if (m_scale_affects_rect && m_scale_x != 1.0f) {
    m_col_rect.m_w *= m_scale_x;
    m_rect.m_w *= m_scale_x;
  }

  if (new_startscale) {
    m_start_scale_x = m_scale_x;
  }
}

void cSprite ::Set_Scale_Y(const float scale,
                           const bool new_startscale /* = 0 */) {
  // invalid value
  if (Is_Float_Equal(scale, 0.0f)) {
    return;
  }

  // undo previous scale from rect
  if (m_scale_affects_rect && m_scale_y != 1.0f) {
    m_col_rect.m_h /= m_scale_y;
    m_rect.m_h /= m_scale_y;
  }

  m_scale_y = scale;

  // set new scale to rect
  if (m_scale_affects_rect && m_scale_y != 1.0f) {
    m_col_rect.m_h *= m_scale_y;
    m_rect.m_h *= m_scale_y;
  }

  if (new_startscale) {
    m_start_scale_y = m_scale_y;
  }
}
void cSprite ::Set_On_Top(const cSprite* sprite,
                          bool optimize_hor_pos /* = 1 */) {
  // set ground position 0.1f over it
  m_pos_y = sprite->m_col_rect.m_y - m_col_pos.m_y - m_col_rect.m_h - 0.1f;

  // optimize the horizontal position if given
  if (optimize_hor_pos &&
      (m_pos_x < sprite->m_pos_x ||
       m_pos_x > sprite->m_pos_x + sprite->m_col_rect.m_w)) {
    m_pos_x = sprite->m_pos_x + sprite->m_col_rect.m_w / 3;
  }

  Update_Position_Rect();
}

void cSprite ::Move(float move_x, float move_y, const bool real /* = 0 */) {
  if (Is_Float_Equal(move_x, 0.0f) && Is_Float_Equal(move_y, 0.0f)) {
    return;
  }

  if (!real) {
    move_x *= pFramerate->m_speed_factor;
    move_y *= pFramerate->m_speed_factor;
  }

  m_pos_x += move_x;
  m_pos_y += move_y;

  Update_Position_Rect();
}

void cSprite ::Update_Position_Rect(void) {
  // if not editor mode
  if (!editor_enabled) {
    m_rect.m_x = m_pos_x;
    m_rect.m_y = m_pos_y;
    // editor rect
    m_start_rect.m_x = m_pos_x;
    m_start_rect.m_y = m_pos_y;
    // collision rect
    m_col_rect.m_x = m_pos_x + m_col_pos.m_x;
    m_col_rect.m_y = m_pos_y + m_col_pos.m_y;
  }
  // editor mode
  else {
    m_rect.m_x = m_start_pos_x;
    m_rect.m_y = m_start_pos_y;
    m_start_rect.m_x = m_start_pos_x;
    m_start_rect.m_y = m_start_pos_y;
    // Do not use m_start_pos_x/m_start_pos_y because col_rect is not the editor/start rect
    m_col_rect.m_x = m_pos_x + m_col_pos.m_x;  // todo : startcol_pos ?
    m_col_rect.m_y = m_pos_y + m_col_pos.m_y;
  }

  Update_Valid_Draw();
}

void cSprite ::Update_Valid_Draw(void) { m_valid_draw = Is_Draw_Valid(); }

void cSprite ::Update_Valid_Update(void) { m_valid_update = Is_Update_Valid(); }

void cSprite ::Draw(cSurface_Request* request /* = nullptr */) {
  if (!m_valid_draw) {
    return;
  }

  Draw_Image(request);

  // draw debugging collision rects
  if (game_debug) {
    // - image rect
    // create request
    auto rect_request = std::make_unique<cRect_Request>();
    // draw
    pVideo->Draw_Rect(&m_rect, m_pos_z + 0.000008f, &lightgrey,
                      rect_request.get());
    rect_request->m_no_camera = m_no_camera;
    rect_request->m_filled = false;
    rect_request->m_blend_sfactor = GL_SRC_COLOR;
    rect_request->m_blend_dfactor = GL_DST_ALPHA;
    // scale
    if (!m_scale_affects_rect) {
      // scale position y
      if (m_scale_up) {
        rect_request->m_rect.m_y +=
            (m_image->m_int_y * m_scale_y) -
            ((m_image->m_h * 0.5f) * (m_scale_y - 1.0f));
      }

      // scale height
      if (m_scale_down) {
        rect_request->m_rect.m_h += m_image->m_h * (m_scale_y - 1.0f);
      }

      // scale position x
      if (m_scale_left) {
        rect_request->m_rect.m_x +=
            (m_image->m_int_x * m_scale_x) -
            ((m_image->m_w * 0.5f) * (m_scale_x - 1.0f));
      }

      // scale width
      if (m_scale_right) {
        rect_request->m_rect.m_w += m_image->m_w * (m_scale_x - 1.0f);
      }
    }
    // add request
    pRenderer->Add(std::move(rect_request));

    // - collision rect
    // create request
    rect_request = std::make_unique<cRect_Request>();
    // draw
    Color const sprite_color = Get_Sprite_Color(this);
    pVideo->Draw_Rect(&m_col_rect, m_pos_z + 0.000007f, &sprite_color,
                      rect_request.get());
    rect_request->m_no_camera = m_no_camera;
    // blending
    rect_request->m_blend_sfactor = GL_SRC_COLOR;
    rect_request->m_blend_dfactor = GL_DST_ALPHA;

    // add request
    pRenderer->Add(std::move(rect_request));
  }

  // show obsolete images in editor
  if (editor_enabled && m_image && m_image->m_obsolete) {
    // create request
    auto rect_request = std::make_unique<cRect_Request>();
    // draw
    pVideo->Draw_Rect(&m_col_rect, m_pos_z + 0.000005f, &red,
                      rect_request.get());
    rect_request->m_no_camera = false;

    // blending
    rect_request->m_blend_sfactor = GL_SRC_COLOR;
    rect_request->m_blend_dfactor = GL_DST_ALPHA;

    // add request
    pRenderer->Add(std::move(rect_request));
  }
}

void cSprite ::Draw_Image(cSurface_Request* request /* = nullptr */) const {
  if (!m_valid_draw) {
    return;
  }

  // no request given: make one here and hand it to the renderer at the end
  std::unique_ptr<cSurface_Request> owned_request;
  if (!request) {
    owned_request = std::make_unique<cSurface_Request>();
    request = owned_request.get();
  }

  // editor
  if (editor_enabled) {
    Draw_Image_Editor(request);
  }
  // no editor
  else {
    Draw_Image_Normal(request);
  }

  if (owned_request) {
    pRenderer->Add(std::move(owned_request));
  }
}

void cSprite ::Draw_Image_Normal(
    cSurface_Request* request /* = nullptr */) const {
  // texture id
  request->m_texture_id = m_image->m_image;

  // size
  request->m_w = m_image->m_start_w;
  request->m_h = m_image->m_start_h;

  // rotation
  request->m_rot_x += m_rot_x + m_image->m_base_rot_x;
  request->m_rot_y += m_rot_y + m_image->m_base_rot_y;
  request->m_rot_z += m_rot_z + m_image->m_base_rot_z;

  // position x and
  // scalex
  if (m_scale_x != 1.0f) {
    // scale to the right and left
    if (m_scale_right && m_scale_left) {
      request->m_scale_x = m_scale_x;
      request->m_pos_x = m_pos_x + (m_image->m_int_x * m_scale_x) -
                         ((m_image->m_w * 0.5f) * (m_scale_x - 1.0f));
    }
    // scale to the right only
    else if (m_scale_right) {
      request->m_scale_x = m_scale_x;
      request->m_pos_x = m_pos_x + (m_image->m_int_x * m_scale_x);
    }
    // scale to the left only
    else if (m_scale_left) {
      request->m_scale_x = m_scale_x;
      request->m_pos_x = m_pos_x + (m_image->m_int_x * m_scale_x) -
                         ((m_image->m_w) * (m_scale_x - 1.0f));
    }
    // no scaling
    else {
      request->m_pos_x = m_pos_x + m_image->m_int_x;
    }
  }
  // no scalex
  else {
    request->m_pos_x = m_pos_x + m_image->m_int_x;
  }
  // position y and
  // scaley
  if (m_scale_y != 1.0f) {
    // scale down and up
    if (m_scale_down && m_scale_up) {
      request->m_scale_y = m_scale_y;
      request->m_pos_y = m_pos_y + (m_image->m_int_y * m_scale_y) -
                         ((m_image->m_h * 0.5f) * (m_scale_y - 1.0f));
    }
    // scale down only
    else if (m_scale_down) {
      request->m_scale_y = m_scale_y;
      request->m_pos_y = m_pos_y + (m_image->m_int_y * m_scale_y);
    }
    // scale up only
    else if (m_scale_up) {
      request->m_scale_y = m_scale_y;
      request->m_pos_y = m_pos_y + (m_image->m_int_y * m_scale_y) -
                         ((m_image->m_h) * (m_scale_y - 1.0f));
    }
    // no scaling
    else {
      request->m_pos_y = m_pos_y + m_image->m_int_y;
    }
  }
  // no scaley
  else {
    request->m_pos_y = m_pos_y + m_image->m_int_y;
  }

  // position z
  request->m_pos_z = m_pos_z;

  // no camera setting
  request->m_no_camera = m_no_camera;

  // color
  request->m_color = m_color;
  // combine color
  if (m_combine_type) {
    request->m_combine_type = m_combine_type;
    request->m_combine_color[0] = m_combine_color[0];
    request->m_combine_color[1] = m_combine_color[1];
    request->m_combine_color[2] = m_combine_color[2];
  }

  // shadow
  if (m_shadow_pos) {
    request->m_shadow_pos = m_shadow_pos;
    request->m_shadow_color = m_shadow_color;
  }
}

void cSprite ::Draw_Image_Editor(
    cSurface_Request* request /* = nullptr */) const {
  // texture id
  request->m_texture_id = m_start_image->m_image;

  // size
  request->m_w = m_start_image->m_start_w;
  request->m_h = m_start_image->m_start_h;

  // rotation
  request->m_rot_x += m_start_rot_x + m_start_image->m_base_rot_x;
  request->m_rot_y += m_start_rot_y + m_start_image->m_base_rot_y;
  request->m_rot_z += m_start_rot_z + m_start_image->m_base_rot_z;

  // position x and
  // scalex
  if (m_start_scale_x != 1.0f) {
    // scale to the right and left
    if (m_scale_right && m_scale_left) {
      request->m_scale_x = m_start_scale_x;
      request->m_pos_x =
          m_start_pos_x + (m_start_image->m_int_x * m_start_scale_x) -
          ((m_start_image->m_w * 0.5f) * (m_start_scale_x - 1.0f));
    }
    // scale to the right only
    else if (m_scale_right) {
      request->m_scale_x = m_start_scale_x;
      request->m_pos_x =
          m_start_pos_x + (m_start_image->m_int_x * m_start_scale_x);
    }
    // scale to the left only
    else if (m_scale_left) {
      request->m_scale_x = m_start_scale_x;
      request->m_pos_x = m_start_pos_x +
                         (m_start_image->m_int_x * m_start_scale_x) -
                         ((m_start_image->m_w) * (m_start_scale_x - 1.0f));
    }
    // no scaling
    else {
      request->m_pos_x = m_start_pos_x + m_start_image->m_int_x;
    }
  }
  // no scalex
  else {
    request->m_pos_x = m_start_pos_x + m_start_image->m_int_x;
  }
  // position y and
  // scaley
  if (m_start_scale_y != 1.0f) {
    // scale down and up
    if (m_scale_down && m_scale_up) {
      request->m_scale_y = m_start_scale_y;
      request->m_pos_y =
          m_start_pos_y + (m_start_image->m_int_y * m_start_scale_y) -
          ((m_start_image->m_h * 0.5f) * (m_start_scale_y - 1.0f));
    }
    // scale down only
    else if (m_scale_down) {
      request->m_scale_y = m_start_scale_y;
      request->m_pos_y =
          m_start_pos_y + (m_start_image->m_int_y * m_start_scale_y);
    }
    // scale up only
    else if (m_scale_up) {
      request->m_scale_y = m_start_scale_y;
      request->m_pos_y = m_start_pos_y +
                         (m_start_image->m_int_y * m_start_scale_y) -
                         ((m_start_image->m_h) * (m_start_scale_y - 1.0f));
    }
    // no scaling
    else {
      request->m_pos_y = m_start_pos_y + m_start_image->m_int_y;
    }
  }
  // no scaley
  else {
    request->m_pos_y = m_start_pos_y + m_start_image->m_int_y;
  }

  // if editor z position is given
  if (m_editor_pos_z > 0.0f) {
    request->m_pos_z = m_editor_pos_z;
  }
  // normal position z
  else {
    request->m_pos_z = m_pos_z;
  }

  // no camera setting
  request->m_no_camera = m_no_camera;

  // color
  request->m_color = m_color;
  // combine color
  if (m_combine_type) {
    request->m_combine_type = m_combine_type;
    request->m_combine_color[0] = m_combine_color[0];
    request->m_combine_color[1] = m_combine_color[1];
    request->m_combine_color[2] = m_combine_color[2];
  }

  // shadow
  if (m_shadow_pos) {
    request->m_shadow_pos = m_shadow_pos;
    request->m_shadow_color = m_shadow_color;
  }
}

void cSprite ::Set_Massive_Type(MassiveType type) {
  m_massive_type = type;

  // set massive-type z position
  if (m_massive_type == MASS_MASSIVE) {
    m_pos_z = m_pos_z_massive_start;
  } else if (m_massive_type == MASS_PASSIVE) {
    if (m_type == TYPE_FRONT_PASSIVE) {
      m_pos_z = m_pos_z_front_passive_start;
    } else {
      m_pos_z = m_pos_z_passive_start;
    }
  } else if (m_massive_type == MASS_CLIMBABLE ||
             m_massive_type == MASS_HALFMASSIVE) {
    m_pos_z = m_pos_z_halfmassive_start;
  }

  // make it the latest sprite
  m_sprite_manager->Move_To_Back(this);
}

bool cSprite ::Is_On_Top(const cSprite* obj) const {
  // invalid
  if (!obj) {
    return false;
  }

  // always collide upwards because of the image size collision checking
  if (m_col_rect.m_x + m_col_rect.m_w > obj->m_col_rect.m_x &&
      m_col_rect.m_x < obj->m_col_rect.m_x + obj->m_col_rect.m_w &&
      m_col_rect.m_y + m_col_rect.m_h < obj->m_col_rect.m_y) {
    return true;
  }

  return false;
}

bool cSprite ::Is_Visible_On_Screen(void) const {
  // camera position
  float cam_x = 0.0f;
  float cam_y = 0.0f;

  if (!m_no_camera) {
    cam_x = pActive_Camera->m_x;
    cam_y = pActive_Camera->m_y;
  }

  // not visible left
  if (m_rect.m_x + m_rect.m_w < cam_x) {
    return false;
  }
  // not visible right
  else if (m_rect.m_x > cam_x + game_res_w) {
    return false;
  }
  // not visible down
  else if (m_rect.m_y + m_rect.m_h < cam_y) {
    return false;
  }
  // not visible up
  else if (m_rect.m_y > cam_y + game_res_h) {
    return false;
  }

  return true;
}

bool cSprite ::Is_In_Range(void) const {
  // no camera range set
  if (m_camera_range < 300) {
    return Is_Visible_On_Screen();
  }

  // check if not in range
  if (m_rect.m_x + (m_rect.m_w * 0.5f) <
          pActive_Camera->m_x + (game_res_w / 2) - m_camera_range ||
      m_rect.m_y + (m_rect.m_h * 0.5f) <
          pActive_Camera->m_y + (game_res_h / 2) - m_camera_range ||
      m_rect.m_x + (m_rect.m_w * 0.5f) >
          pActive_Camera->m_x + (game_res_w / 2) + m_camera_range ||
      m_rect.m_y + (m_rect.m_h * 0.5f) >
          pActive_Camera->m_y + (game_res_h / 2) + m_camera_range) {
    return false;
  }

  // is in range
  return true;
}

bool cSprite ::Is_Update_Valid(void) {
  // if destroyed
  if (m_auto_destroy) {
    return false;
  }

  return true;
}

bool cSprite ::Is_Draw_Valid(void) {
  // if editor not enabled
  if (!editor_enabled) {
    // if not active or no image is set
    if (!m_active || !m_image) {
      return false;
    }
  }
  // editor enabled
  else {
    // if destroyed
    if (m_auto_destroy) {
      return false;
    }

    // no image
    if (!m_start_image) {
      return false;
    }
  }

  // not visible on the screen
  if (!Is_Visible_On_Screen()) {
    return false;
  }

  return true;
}

void cSprite ::Destroy(void) {
  // already destroyed
  if (m_auto_destroy) {
    return;
  }

  Clear_Collisions();

  // if this can not be auto-deleted
  if (m_disallow_managed_delete) {
    return;
  }

  m_auto_destroy = true;
  m_active = false;
  m_valid_draw = false;
  m_valid_update = false;
  Set_Image(nullptr, true);
}

void cSprite ::Editor_Add(const CEGUI::String& name,
                          const CEGUI::String& tooltip,
                          CEGUI::Window* window_setting, float obj_width,
                          float obj_height /* = 28 */,
                          bool advance_row /* = 1 */) {
  if (obj_height < 28.0f) {
    obj_height = 28.0f;
  }

  // get gui sheet
  CEGUI::Window* guisheet = pGuiSystem->getGUISheet();
  // get window manager
  CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();

  // create name window
  CEGUI::Window* window_name = wmgr.createWindow(
      "TaharezLook/StaticText", "text_" + window_setting->getName());
  window_name->setText(name);
  window_name->setTooltipText(tooltip);
  // get text width
  CEGUI::Font const* font =
      &CEGUI::FontManager::getSingleton().get("bluebold_medium");
  float const text_width =
      12.0f + font->getTextExtent(name) * global_downscalex;
  // all names should have the same width
  if (text_width > m_editor_window_name_width) {
    m_editor_window_name_width = text_width;
  }
  // set size
  window_name->setWidth(CEGUI::UDim(0, text_width * global_upscalex));
  window_name->setHeight(CEGUI::UDim(0, 28 * global_upscaley));

  // create settings window
  auto settings_item = std::make_unique<cEditor_Object_Settings_Item>();

  // set size
  window_setting->setWidth(CEGUI::UDim(0, obj_width * global_upscalex));
  window_setting->setHeight(CEGUI::UDim(0, obj_height * global_upscaley));

  settings_item->window_name = window_name;
  settings_item->window_setting = window_setting;
  settings_item->advance_row = advance_row;

  m_editor_windows.push_back(std::move(settings_item));

  // add to main window
  guisheet->addChildWindow(window_name);
  guisheet->addChildWindow(window_setting);
}

void cSprite ::Editor_Activate(void) {
  // if this is not a basic sprite
  if (!Is_Basic_Sprite()) {
    return;
  }

  // get window manager
  CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();

  // image
  CEGUI::Editbox* editbox = static_cast<CEGUI::Editbox*>(
      wmgr.createWindow("TaharezLook/Editbox", "editor_sprite_image"));
  Editor_Add(UTF8_("Image"), UTF8_("Image filename"), editbox, 200);

  editbox->setText(m_start_image->Get_Filename(1));
  editbox->subscribeEvent(
      CEGUI::Editbox::EventTextChanged,
      CEGUI::Event::Subscriber(&cSprite::Editor_Image_Text_Changed, this));

  // init
  Editor_Init();
}

void cSprite ::Editor_Deactivate(void) {
  // remove editor controls
  m_editor_windows.clear();
}

void cSprite ::Editor_Init(void) {
  // set state
  Editor_State_Update();

  // init
  for (const auto& editor_window : m_editor_windows) {
    CEGUI::Window* window_name = editor_window->window_name;

    // set first row width
    if (editor_window->advance_row) {
      window_name->setWidth(
          CEGUI::UDim(0, m_editor_window_name_width * global_upscalex));
    }
  }

  // set position
  Editor_Position_Update();
}

void cSprite ::Editor_Position_Update(void) {
  float obj_posx = 0.0f;
  float obj_posy = 0.0f;
  float row_height = 0.0f;

  // set all positions
  for (const auto& editor_window : m_editor_windows) {
    CEGUI::Window* window_name = editor_window->window_name;
    CEGUI::Window* window_setting = editor_window->window_setting;

    // start a new row
    if (editor_window->advance_row) {
      obj_posx = 0.0f;
      obj_posy += row_height;
      row_height = 0.0f;
    }

    // get window text width
    float const window_name_width =
        window_name->getWidth().asAbsolute(static_cast<float>(game_res_w)) *
        global_downscalex;
    float const window_name_height =
        window_name->getHeight().asAbsolute(static_cast<float>(game_res_h)) *
        global_downscaley;

    // get window setting width
    float window_setting_width;
    float window_setting_height;

    // if combobox get the editbox/droplist dimension
    if (window_setting->getType() == "TaharezLook/Combobox") {
      window_setting_width = static_cast<CEGUI::Combobox*>(window_setting)
                                 ->getDropList()
                                 ->getWidth()
                                 .asAbsolute(static_cast<float>(game_res_w)) *
                             global_downscalex;
      window_setting_height = static_cast<CEGUI::Combobox*>(window_setting)
                                  ->getEditbox()
                                  ->getHeight()
                                  .asAbsolute(static_cast<float>(game_res_h)) *
                              global_downscaley;
    }
    // get default dimension
    else {
      window_setting_width = window_setting->getWidth().asAbsolute(
                                 static_cast<float>(game_res_w)) *
                             global_downscalex;
      window_setting_height = window_setting->getHeight().asAbsolute(
                                  static_cast<float>(game_res_h)) *
                              global_downscaley;
    }

    // update row height
    if (window_setting_height > row_height) {
      row_height = window_setting_height;
    }
    if (window_name_height > row_height) {
      row_height = window_name_height;
    }

    // current position
    float const object_final_pos_x =
        m_start_pos_x + m_rect.m_w + 5.0f + obj_posx - pActive_Camera->m_x;
    float const object_final_pos_y =
        m_start_pos_y + 5.0f + obj_posy - pActive_Camera->m_y;

    // set name position
    window_name->setXPosition(
        CEGUI::UDim(0, object_final_pos_x * global_upscalex));
    window_name->setYPosition(
        CEGUI::UDim(0, object_final_pos_y * global_upscaley));
    // set setting position
    window_setting->setXPosition(CEGUI::UDim(
        0, (window_name_width + object_final_pos_x) * global_upscalex));
    window_setting->setYPosition(
        CEGUI::UDim(0, object_final_pos_y * global_upscaley));

    // set new position x
    obj_posx += window_name_width + window_setting_width + 0.05f;
  }
}

bool cSprite ::Editor_Image_Text_Changed(const CEGUI::EventArgs& event) {
  const CEGUI::WindowEventArgs& windowEventArgs =
      static_cast<const CEGUI::WindowEventArgs&>(event);
  std::string const str_text =
      static_cast<CEGUI::Editbox*>(windowEventArgs.window)->getText().c_str();

  Set_Image(pVideo->Get_Surface(str_text), true);

  return true;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
