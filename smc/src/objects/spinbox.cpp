/***************************************************************************
 * spinbox.cpp  -  spinning box
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

#include "../objects/spinbox.h"
#include "../core/framerate.h"
#include "../core/game_core.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

cSpinBox ::cSpinBox(cSprite_Manager* sprite_manager)
    : cBaseBox(sprite_manager) {
  cSpinBox::Init();
}

cSpinBox ::cSpinBox(CEGUI::XMLAttributes& attributes,
                    cSprite_Manager* sprite_manager)
    : cBaseBox(sprite_manager) {
  cSpinBox::Init();
  cSpinBox::Load_From_XML(attributes);
}

cSpinBox ::~cSpinBox(void) = default;

void cSpinBox ::Init(void) {
  m_type = TYPE_SPIN_BOX;
  box_type = m_type;
  m_camera_range = 5000;
  m_can_be_on_ground = false;

  m_spin_counter = 0.0f;
  m_spin = false;

  // default is infinite times activate-able
  Set_Useable_Count(-1, true);
  // Spinbox Animation
  Set_Animation_Type("Spin");

  // editor image
  m_item_image = pVideo->Get_Surface("game/arrow/small/white/up.png");

  Create_Name();
}

std::unique_ptr<cSprite> cSpinBox ::Copy(void) const {
  auto spinbox = std::make_unique<cSpinBox>(m_sprite_manager);
  spinbox->Set_Pos(m_start_pos_x, m_start_pos_y, true);
  spinbox->Set_Invisible(m_box_invisible);
  spinbox->Set_Useable_Count(m_start_useable_count, true);
  return spinbox;
}

void cSpinBox ::Load_From_XML(CEGUI::XMLAttributes& attributes) {
  cBaseBox::Load_From_XML(attributes);
}

void cSpinBox ::Save_To_XML(CEGUI::XMLSerializer& stream) {
  // begin
  stream.openTag(m_type_name);

  cBaseBox::Save_To_XML(stream);

  // end
  stream.closeTag();
}

void cSpinBox ::Activate(void) {
  // already spinning
  if (m_spin) {
    return;
  }

  m_spin = true;
  Update_Valid_Update();
  // passive box for spinning
  m_massive_type = MASS_PASSIVE;

  m_spin_counter = 0.0f;
  // enable animation
  Set_Animation(true);
  Reset_Animation();
}

void cSpinBox ::Stop(void) {
  // already stopped spinning
  if (!m_spin) {
    return;
  }

  // disabled image
  if (!m_useable_count) {
    Set_Image_Num(0);
  }
  // default image
  else {
    Set_Image_Num(1);
  }
  // reset
  m_spin = false;
  Update_Valid_Update();
  m_spin_counter = 0.0f;

  // back to a massive box
  m_massive_type = MASS_MASSIVE;
  // disable animation
  Set_Animation(false);
  Reset_Animation();
}

void cSpinBox ::Update(void) {
  if (!m_valid_update || !Is_In_Range()) {
    return;
  }

  cBaseBox::Update();

  if (m_spin) {
    m_spin_counter += pFramerate->m_speed_factor;

    // spinning animation finished
    if (m_curr_img == 1) {
      // spinning time finished
      if (m_spin_counter > speedfactor_fps * 5) {
        // reset spin counter
        m_spin_counter = 0.0f;
        // set to massive for collision check
        m_massive_type = MASS_MASSIVE;
        // collision data
        std::unique_ptr<cObjectCollisionType> col_list =
            Collision_Check(&m_col_rect, COLLIDE_ONLY_BLOCKING);

        // check if spinning should continue
        bool spin_again = false;

        // colliding with player or enemy
        if (col_list->Is_Included(TYPE_PLAYER) ||
            col_list->Is_Included(ARRAY_ENEMY)) {
          spin_again = true;
        }
        // colliding with an active object
        else if (col_list->Is_Included(ARRAY_ACTIVE)) {
          cSprite* col_obj = col_list->Find_First(ARRAY_ACTIVE)->m_obj;

          // check for items
          if (col_obj->m_type == TYPE_MUSHROOM_LIVE_1 ||
              col_obj->m_type == TYPE_MUSHROOM_DEFAULT ||
              col_obj->m_type == TYPE_MUSHROOM_POISON ||
              col_obj->m_type == TYPE_MUSHROOM_BLUE ||
              col_obj->m_type == TYPE_MUSHROOM_GHOST ||
              col_obj->m_type == TYPE_FIREPLANT ||
              col_obj->m_type == TYPE_STAR ||
              col_obj->m_type == TYPE_FALLING_GOLDPIECE) {
            // found blocking active object
            spin_again = true;
          }
        }

        // continue spinning
        if (spin_again) {
          // spin some time again
          m_spin_counter = speedfactor_fps * 2;
          // passive for spinning
          m_massive_type = MASS_PASSIVE;
        }
        // finished spinning
        else {
          Stop();
        }
      }
    }
  }
}

bool cSpinBox ::Is_Update_Valid(void) {
  if (m_spin) {
    return true;
  }

  return cBaseBox::Is_Update_Valid();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
