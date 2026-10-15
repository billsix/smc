/***************************************************************************
 * hud.h
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

#ifndef SMC_HUD_H
#define SMC_HUD_H

#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"

namespace SMC {

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

class cHudSprite : public cSprite {
 public:
  cHudSprite(cSprite_Manager* sprite_manager);
  ~cHudSprite(void) override;

  // copy this sprite
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;
};

/* *** *** *** *** *** *** *** cHud_Manager *** *** *** *** *** *** *** *** *** *** */

class cHud_Manager : public cObject_Manager<cHudSprite> {
 public:
  cHud_Manager(cSprite_Manager* sprite_manager);
  ~cHud_Manager(void) override;

  // Load the complete HUD
  void Load(void);
  // Unload the complete HUD
  void Unload(void);

  // Update and reload text
  void Update_Text(void);
  // Update the objects
  void Update(void);
  // Draw the objects
  void Draw(void);

  // Set the parent sprite manager
  void Set_Sprite_Manager(cSprite_Manager* sprite_manager);

  // the parent sprite manager
  cSprite_Manager* m_sprite_manager;

  using HudSpriteList = vector<std::unique_ptr<cHudSprite>>;

  // true if loaded
  bool m_loaded;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD Manager
extern cHud_Manager* pHud_Manager;

/* *** *** *** *** *** *** *** *** PointsText *** *** *** *** *** *** *** *** *** */

struct PointsText : public cHudSprite {
 public:
  PointsText(cSprite_Manager* sprite_manager);
  ~PointsText(void) override;

  float m_vely;
  unsigned int m_points;
};

/* *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** *** */

class cMenuBackground : public cHudSprite {
 public:
  cMenuBackground(cSprite_Manager* sprite_manager);
  ~cMenuBackground(void) override;

  void Draw(cSurface_Request* request = nullptr) override;

  cGL_Surface* m_maryo_head;
  cGL_Surface* m_goldpiece;

  GL_point m_rect_maryo_head;
  GL_point m_rect_goldpiece;
};

/* *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** *** */

class cStatusText : public cHudSprite {
 public:
  cStatusText(cSprite_Manager* sprite_manager);
  ~cStatusText(void) override;

  void Draw(cSurface_Request* request = nullptr) override;
};

/* *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** *** */

class cPlayerPoints : public cStatusText {
 public:
  cPlayerPoints(cSprite_Manager* sprite_manager);
  ~cPlayerPoints(void) override;

  void Set_Points(long points);
  void Add_Points(unsigned int points, float x = 0.0f, float y = 0.0f,
                  std::string strtext = "",
                  const Color& color = static_cast<Uint8>(255),
                  bool allow_multiplier = false);

  // removes all point texts
  void Clear(void);

  void Draw(cSurface_Request* request = nullptr) override;

  using PointsTextList = vector<std::unique_ptr<PointsText>>;
  PointsTextList m_points_objects;
};

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldDisplay : public cStatusText {
 public:
  cGoldDisplay(cSprite_Manager* sprite_manager);
  ~cGoldDisplay(void) override;

  void Set_Gold(int gold);
  void Add_Gold(int gold);

  void Draw(cSurface_Request* request = nullptr) override;
};

/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cLiveDisplay : public cStatusText {
 public:
  cLiveDisplay(cSprite_Manager* sprite_manager);
  ~cLiveDisplay(void) override;

  void Set_Lives(int lives);
  void Add_Lives(int lives);

  void Draw(cSurface_Request* request = nullptr) override;
};

/* *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cTimeDisplay : public cStatusText {
 public:
  cTimeDisplay(cSprite_Manager* sprite_manager);
  ~cTimeDisplay(void) override;

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  // Set time
  void Set_Time(Uint32 milliseconds);

  // reset
  void Reset(void);

  char m_text[50];
  Uint32 m_last_update_seconds;
  Uint32 m_milliseconds;
};

/* *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** *** *** */

class cItemBox : public cStatusText {
 public:
  cItemBox(cSprite_Manager* sprite_manager);
  ~cItemBox(void) override;

  // Set the parent sprite manager
  void Set_Sprite_Manager(cSprite_Manager* sprite_manager) override;

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  /* Set the item
	* sound : if set the box sound is played
	*/
  void Set_Item(SpriteType item_type, bool sound = true);
  // Activates the itembox
  void Request_Item(void);
  // push the item back to the itembox
  void Push_back(void);

  void Reset(void);

  /* The current Item
	 * uses the Item defines
	 */
  SpriteType m_item_id;

  // alpha effect
  float m_item_counter;
  // alpha effect mod
  bool m_item_counter_mod;

  // itembox color
  Color m_box_color;

  // stored item (owned value)
  cMovingSprite m_item;
};

/* *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cDebugDisplay : public cStatusText {
 public:
  cDebugDisplay(cSprite_Manager* sprite_manager);
  ~cDebugDisplay(void) override;

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request = nullptr) override;
  // draw the frames per second info
  void Draw_fps(void);
  // draw the debug mode info
  void Draw_Debug_Mode(void);
  // draw the performance debug mode info
  void Draw_Performance_Debug_Mode(void);

  // set the debug text to display
  void Set_Text(const std::string& ntext,
                float display_time = speedfactor_fps * 2.0f);

  // display text
  std::string m_text, m_text_old;
  // text counter
  float m_counter;

  // CEGUI debug text
  CEGUI::Window *m_window_debug_text, *m_text_debug_text;

  // last game mode
  GameMode m_game_mode_last;
  // last level text
  std::string m_level_old;
  // last object counters
  int m_obj_counter, m_pass_counter, m_mass_counter, m_enemy_counter,
      m_active_counter;
  // sprites
  using HudSpriteList = vector<std::unique_ptr<cHudSprite>>;
  HudSpriteList m_sprites;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD
extern cPlayerPoints* pHud_Points;
extern cDebugDisplay* pHud_Debug;
extern cGoldDisplay* pHud_Goldpieces;
extern cLiveDisplay* pHud_Lives;
extern cTimeDisplay* pHud_Time;
extern cItemBox* pHud_Itembox;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
