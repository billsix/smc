/***************************************************************************
 * sound_manager.cpp  -  Sound Handler/Manager
 *
 * Copyright (C) 2006 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../audio/sound_manager.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** Sound *** *** *** *** *** *** *** *** *** */

cSound::cSound(void) { m_chunk = nullptr; }

cSound::~cSound(void) { Free(); }

bool cSound::Load(const std::string& filename) {
  Free();

  m_chunk = Mix_LoadWAV(filename.c_str());

  if (m_chunk) {
    m_filename = filename;
    return true;
  }

  return false;
}

void cSound::Free(void) {
  if (m_chunk) {
    Mix_FreeChunk(m_chunk);
    m_chunk = nullptr;
  }

  m_filename.clear();
}

/* *** *** *** *** *** *** cSound_Cache *** *** *** *** *** *** *** *** *** *** *** */

cSound_Cache::cSound_Cache(void) { m_load_count = 0; }

cSound_Cache::~cSound_Cache(void) { Delete_All(); }

cSound* cSound_Cache::Get_Pointer(const std::string& path) const {
  auto itr = m_by_filename.find(path);
  return itr == m_by_filename.end() ? nullptr : itr->second;
}

cSound* cSound_Cache::Add(std::unique_ptr<cSound> sound) {
  m_load_count++;
  cSound* added = m_objects.emplace_back(std::move(sound)).get();
  // the first sound added under a filename is the one Get_Pointer(path) finds
  m_by_filename.try_emplace(added->m_filename, added);
  return added;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cSound_Cache* pSound_Manager = nullptr;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
