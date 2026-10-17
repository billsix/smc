/***************************************************************************
 * img_manager.cpp  -  Image Handler/Manager
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

#include "../video/img_manager.h"
#include "../video/renderer.h"
#include "../core/i18n.h"
// CEGUI
#include "CEGUIWindowManager.h"
#include "elements/CEGUIProgressBar.h"

namespace SMC {

/* *** *** *** *** *** cSaved_Texture *** *** *** *** *** *** *** *** *** *** *** *** */

cSaved_Texture ::cSaved_Texture(void) {
  m_base = nullptr;
  m_pixels = nullptr;

  m_width = 0;
  m_height = 0;
  m_format = 0;

  m_min_filter = 0;
  m_mag_filter = 0;
  m_wrap_s = 0;
  m_wrap_t = 0;
}

cSaved_Texture ::~cSaved_Texture(void) {
  if (m_pixels) {
    delete[] m_pixels;
  }
}

/* *** *** *** *** *** *** cImage_Manager *** *** *** *** *** *** *** *** *** *** *** */

cImage_Manager ::cImage_Manager(void) : cObject_Manager<cGL_Surface>() {
  m_high_texture_id = 0;
}

cImage_Manager ::~cImage_Manager(void) { cImage_Manager::Delete_All(); }

cGL_Surface* cImage_Manager ::Add(std::unique_ptr<cGL_Surface> obj) {
  if (!obj) {
    return nullptr;
  }

  // it is now managed
  obj->m_managed = true;

  // Add
  return cObject_Manager<cGL_Surface>::Add(std::move(obj));
}

cGL_Surface* cImage_Manager ::Get_Pointer(const std::string& path) const {
  for (const auto& surface : objects) {
    // return first match
    if (surface->m_filename.compare(path) == 0) {
      return surface.get();
    }
  }

  // not found
  return nullptr;
}

void cImage_Manager ::Grab_Textures(bool from_file /* = 0 */,
                                    bool draw_gui /* = 0 */) {
  // progress bar
  CEGUI::ProgressBar* progress_bar = nullptr;

  if (draw_gui) {
    // get progress bar
    progress_bar = static_cast<CEGUI::ProgressBar*>(
        CEGUI::WindowManager::getSingleton().getWindow("progress_bar"));
    progress_bar->setProgress(0);
    // set loading screen text
    Loading_Screen_Draw_Text(_("Saving Textures"));
  }

  unsigned int loaded_files = 0;
  unsigned int const file_count = objects.size();

  // save all textures
  for (const auto& surface : objects) {
    // get surface
    // skip surfaces with an already deleted texture
    if (!glIsTexture(surface->m_image)) {
      continue;
    }

    // get software texture and save it to software memory
    m_saved_textures.push_back(surface->Get_Software_Texture(from_file));
    // delete hardware texture
    if (glIsTexture(surface->m_image)) {
      glDeleteTextures(1, &surface->m_image);
    }

    // count files
    loaded_files++;

    // draw
    if (draw_gui) {
      // update progress
      progress_bar->setProgress(static_cast<float>(loaded_files) /
                                static_cast<float>(file_count));

      Loading_Screen_Draw();
    }
  }
}

void cImage_Manager ::Restore_Textures(bool draw_gui /* = 0 */) {
  // progress bar
  CEGUI::ProgressBar* progress_bar = nullptr;

  if (draw_gui) {
    // get progress bar
    progress_bar = static_cast<CEGUI::ProgressBar*>(
        CEGUI::WindowManager::getSingleton().getWindow("progress_bar"));
    progress_bar->setProgress(0);
    // set loading screen text
    Loading_Screen_Draw_Text(_("Restoring Textures"));
  }

  unsigned int loaded_files = 0;
  unsigned int const file_count = m_saved_textures.size();

  // load back into hardware textures
  for (const auto& soft_tex : m_saved_textures) {
    // get saved texture
    // load it
    soft_tex->m_base->Load_Software_Texture(soft_tex.get());
    // delete

    // count files
    loaded_files++;

    // draw
    if (draw_gui) {
      // update progress
      progress_bar->setProgress(static_cast<float>(loaded_files) /
                                static_cast<float>(file_count));

      Loading_Screen_Draw();
    }
  }

  m_saved_textures.clear();
}

void cImage_Manager ::Delete_Image_Textures(void) {
  for (const auto& surface : objects) {
    // get object
    if (surface->m_auto_del_img && glIsTexture(surface->m_image)) {
      glDeleteTextures(1, &surface->m_image);
    }
  }
}

void cImage_Manager ::Delete_Hardware_Textures(void) {
  // delete all hardware surfaces (<= : the highest id, recorded by Create_Texture,
  // was never visited with a bare <)
  for (GLuint i = 0; i <= m_high_texture_id; i++) {
    if (glIsTexture(i)) {
      printf("ImageManager : deleting texture %d\n", i);
      glDeleteTextures(1, &i);
    }
  }

  m_high_texture_id = 0;
}

void cImage_Manager ::Delete_All(void) {
  // stops cGL_Surface destructor from checking if GL texture id still in use
  Delete_Image_Textures();
  cObject_Manager<cGL_Surface>::Delete_All();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cImage_Manager* pImage_Manager = nullptr;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
