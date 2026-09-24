/***************************************************************************
 * layer.h
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

#ifndef SMC_WORLD_LAYER_H
#define SMC_WORLD_LAYER_H

#include "../core/global_basic.h"
#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"
#include "../overworld/world_waypoint.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC {

/* *** *** *** *** *** *** Layer_Line *** *** *** *** *** *** *** *** *** *** *** */

// Layer line point class
// TODO : create a real basic/virtual sprite ?
class cLayer_Line_Point : public cSprite {
 public:
  // constructor
  cLayer_Line_Point(cSprite_Manager* sprite_manager, cOverworld* overworld,
                    SpriteType new_type);
  // destructor
  ~cLayer_Line_Point(void) override;

  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override {};

  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  /* set this sprite to destroyed and completely disable it
	 * sprite is still in the sprite manager but only to get possibly replaced
	*/
  void Destroy(void) override;

  // Get center position x
  float Get_Line_Pos_X(void) const;
  // Get center position y
  float Get_Line_Pos_Y(void) const;

  // parent overworld
  cOverworld* m_overworld;
  // the linked start/end point
  cLayer_Line_Point* m_linked_point;
};

// Layer line start point class
class cLayer_Line_Point_Start : public cLayer_Line_Point {
 public:
  // constructor
  cLayer_Line_Point_Start(cSprite_Manager* sprite_manager,
                          cOverworld* overworld);
  // destructor
  ~cLayer_Line_Point_Start(void) override;
  // create from stream
  cLayer_Line_Point_Start(CEGUI::XMLAttributes& attributes,
                          cSprite_Manager* sprite_manager,
                          cOverworld* overworld);

  // init defaults
  void Init(void) override;

  // copy (end point can not be copied)
  std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;

  // Set the parent sprite manager
  void Set_Sprite_Manager(cSprite_Manager* sprite_manager) override;

  // Draw
  void Draw(cSurface_Request* request = nullptr) override;

  // return a normal line
  GL_line Get_Line(void) const;

  /* Returns the Waypoint on the end of the line(s)
	 * if the line continues on another line it is followed to the end
	*/
  cWaypoint* Get_End_Waypoint(void) const;

  // editor activation
  void Editor_Activate(void) override;
  // editor origin text changed event
  bool Editor_Origin_Text_Changed(const CEGUI::EventArgs& event);

  /* animation type 
	 * 0 = normal walking, 1 = swimming
	 */
  unsigned int m_anim_type;
  // waypoint origin identifier
  unsigned int m_origin;
};

/* *** *** *** *** *** *** *** Layer *** *** *** *** *** *** *** *** *** *** */

// Layer Line Collision data
class cLine_collision {
 public:
  cLine_collision(void);

  // nearest line pointer
  cLayer_Line_Point_Start* m_line;
  // nearest line number
  int m_line_number;
  // position difference
  float m_difference;
};

using LayerLineList = vector<cLayer_Line_Point_Start*>;

// Layer class
// handles the line collision detection
// A cLayer does NOT own its line points: they are owned (and freed) by the overworld's
// cSprite_Manager, which cWorld_Sprite_Manager also feeds into this layer. The layer holds
// borrowed pointers, so it is a plain non-owning list, not a cObject_Manager.
class cLayer : public CEGUI::XMLHandler {
 public:
  cLayer(cOverworld* origin);
  ~cLayer(void) override;

  // Add a layer line
  void Add(cLayer_Line_Point_Start* line_point);

  // The layer's line points (borrowed; the sprite manager owns and frees them)
  LayerLineList objects;
  // Number of line points
  size_t size(void) const { return objects.size(); }
  // Return the line point's index, or -1 if not present
  int Get_Array_Num(const cLayer_Line_Point_Start* obj) const;
  /* Remove a line point from the layer
	 * delete_data : if set, free it -- but the sprite manager owns these, so callers pass false
	*/
  bool Delete(cLayer_Line_Point_Start* obj, bool delete_data = true);

  // Load from file
  void Load(const std::string& filename);

  // Save
  bool Save(const std::string& filename);

  // Delete all line points from this layer (does not free them -- the sprite manager does)
  void Delete_All(void);

  /* Returns the colliding Line start point
	 * if not found returns NULL
	*/
  cLayer_Line_Point_Start* Get_Line_Collision_Start(const GL_rect& line_rect);
  /* Returns the colliding Line from the given position and the added direction size
	 * if not found returns a NULL line in the class
	*/
  cLine_collision Get_Line_Collision_Direction(
      float x, float y, ObjectDirection dir, float dir_size = 15,
      unsigned int check_size = 10) const;

  /* Return the collision data between the nearest line and the given position
	 * check_size is maximum size for both direction checking lines
	 * if only_origin_id is set only checks lines with the given id
	*/
  cLine_collision Get_Nearest(float x, float y,
                              ObjectDirection dir = DIR_HORIZONTAL,
                              unsigned int check_size = 15,
                              int only_origin_id = -1) const;
  // Return the collision data between the given line and position
  cLine_collision Get_Nearest_Line(cLayer_Line_Point_Start* map_layer_line,
                                   float x, float y,
                                   ObjectDirection dir = DIR_HORIZONTAL,
                                   unsigned int check_size = 15) const;

  // parent overworld
  cOverworld* m_overworld;

 private:
  // XML element start
  void elementStart(const CEGUI::String& element,
                    const CEGUI::XMLAttributes& attributes) override;
  // XML element end
  void elementEnd(const CEGUI::String& element) override;

  // XML element property list
  CEGUI::XMLAttributes m_xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
