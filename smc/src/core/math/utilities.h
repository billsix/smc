/***************************************************************************
 * utilities.h
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

#ifndef SMC_UTILITIES_H
#define SMC_UTILITIES_H

#include "../../core/global_basic.h"
#include "../../core/global_game.h"
#include <concepts>
#include <random>

namespace SMC {

/* *** *** *** *** *** *** *** *** Math utility functions *** *** *** *** *** *** *** *** *** */

template <class T>
inline T Clamp(const T& v, const T& min, const T& max) {
  if (v > max) {
    return max;
  } else if (v < min) {
    return min;
  }

  return v;
}

// The game's random engine: one Mersenne Twister, seeded once from
// std::random_device on first use.
inline std::mt19937& Random_Engine() {
  static std::mt19937 engine{std::random_device{}()};
  return engine;
}

// A uniformly distributed integer in [0, n), of n's own type; 0 when n <= 0.
// The draw goes through long long because std::uniform_int_distribution may not
// be instantiated with char-sized types such as Uint8.
template <std::integral T>
inline T Random_Below(T n) {
  if (n <= T{0}) {
    return T{0};
  }
  std::uniform_int_distribution<long long> distribution(
      0, static_cast<long long>(n) - 1);
  return static_cast<T>(distribution(Random_Engine()));
}

// return a random floating point value between the given values (either order)
inline float Get_Random_Float(float min, float max) {
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  return min + (max - min) * unit(Random_Engine());
}

// Checks if number is power of 2 and if not returns the next power of two size
inline unsigned int Get_Power_of_2(unsigned int size) {
  unsigned int value = 1;

  while (value < size) {
    value <<= 1;
  }

  return value;
};

/* Returns true if the number is valid
 * accept_floating_point: if is set also accept floating point values
*/
bool Is_Valid_Number(std::string num, bool accept_floating_point = true);

// Returns true if the floats are equal with the given tolerance
inline bool Is_Float_Equal(float a, float b, float tolerance = 0.0001f) {
  if (fabs(b - a) <= tolerance) {
    return true;
  }

  return false;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
