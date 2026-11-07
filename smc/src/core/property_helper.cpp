/***************************************************************************
 * property_helper.cpp  -  property helper
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

#include "../core/property_helper.h"
#include "../core/i18n.h"
#include "../video/color.h"
#include "../objects/sprite.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <limits>
#include <type_traits>
#include <string>

namespace SMC {

void string_replace_all(std::string& str, const std::string& search,
                        const std::string& format) {
  size_t pos = 0;

  while ((pos = str.find(search, pos)) != std::string::npos) {
    str.replace(pos, search.length(), format);
    pos += format.length();
  }
}

void cegui_string_replace_all(CEGUI::String& str, const CEGUI::String& search,
                              const CEGUI::String& format) {
  size_t pos = 0;

  while ((pos = str.find(search, pos)) != CEGUI::String::npos) {
    str.replace(pos, search.length(), format);
    pos += format.length();
  }
}

std::string string_trim_from_end(std::string str, const char search) {
  // find last position from end which is not the given character
  size_t const pos = str.find_last_not_of(search);

  // if all match or empty
  if (pos == std::string::npos) {
    return std::string();
  } else {
    return str.substr(0, pos + 1);
  }
}

std::string int_to_string(const int number) { return std::to_string(number); }

std::string int64_to_string(const Uint64 number) {
  return std::to_string(number);
}

std::string long_to_string(const long number) { return std::to_string(number); }

std::string float_to_string(double value, int prec /* = 6 */,
                            bool keep_zeros /* = 1 */) {
  // Precision is limited to 6 digits after the point: a float carries ~7
  // significant digits, so more would print noise.
  prec = std::clamp(prec, 0, 6);
  // Adding +0.0 turns -0.0 into +0.0, so a negative zero prints as "0".
  value += 0.0;

  // Fixed notation of DBL_MAX is 309 digits; 6 decimals, sign and point fit too.
  std::array<char, 330> buf;
  auto const [end, ec] = std::to_chars(buf.data(), buf.data() + buf.size(),
                                       value, std::chars_format::fixed, prec);
  std::string str(buf.data(), ec == std::errc() ? end : buf.data());

  // Without keep_zeros, "1.500" becomes "1.5" and "2.000" becomes "2".
  if (!keep_zeros && str.find('.') != std::string::npos) {
    str.erase(str.find_last_not_of('0') + 1);
    if (str.back() == '.') {
      str.pop_back();
    }
  }
  return str;
}

// Parse a number with std::from_chars (locale independent), keeping the
// std::istream semantics the level/save formats were written against: leading
// whitespace and a leading '+' are accepted; out-of-range input clamps to the
// type's limit; an unsigned type reads "-n" as its wrapped value (as strtoull
// does); unparseable input yields 0.
template <class T>
static T parse_number(const std::string& str) {
  const char* first = str.data();
  const char* const last = first + str.size();
  while (first != last && std::isspace(static_cast<unsigned char>(*first))) {
    ++first;
  }
  if (first != last && *first == '+') {
    ++first;
  }
  bool negate = false;
  if constexpr (std::is_unsigned_v<T>) {
    if (first != last && *first == '-') {
      negate = true;
      ++first;
    }
  }
  T num{};
  auto const [ptr, ec] = std::from_chars(first, last, num);
  if (ec == std::errc::result_out_of_range) {
    bool const below = !negate && first != last && *first == '-';
    num = below ? std::numeric_limits<T>::lowest()
                : std::numeric_limits<T>::max();
  } else if (ec != std::errc()) {
    num = T{};
  }
  return negate ? static_cast<T>(T{} - num) : num;
}

int string_to_int(const std::string& str) { return parse_number<int>(str); }

Uint64 string_to_int64(const std::string& str) {
  return parse_number<Uint64>(str);
}

long string_to_long(const std::string& str) { return parse_number<long>(str); }

float string_to_float(const std::string& str) {
  return parse_number<float>(str);
}

unsigned int string_to_version_number(std::string str) {
  if (str.empty()) {
    return 0;
  }

  std::string::size_type pos = str.find('.');

  // only major version
  if (pos == std::string::npos) {
    return static_cast<unsigned int>(string_to_int(str) * 10000);
  }

  unsigned int ver = 0;

  if (pos != 0) {
    // number before first '.' is major
    ver = static_cast<unsigned int>(string_to_int(str.substr(0, pos)) * 10000);
  }

  str.erase(0, pos + 1);
  pos = str.find('.');

  if (pos != 0) {
    // number after first '.' is minor
    ver += static_cast<unsigned int>(string_to_int(str.substr(0, pos)) * 100);
  }

  if (pos == std::string::npos) {
    return ver;
  }

  str.erase(0, pos + 1);

  if (!str.empty()) {
    // number after second '.' is patch
    ver += static_cast<unsigned int>(string_to_int(str));
  }

  return ver;
}

std::string xml_string_to_string(std::string str) {
  while (true) {
    std::string::size_type const pos = str.find("<br/>");

    if (pos == std::string::npos) {
      break;
    }

    str.replace(pos, 5, "\n");
  }

  return str;
}

#ifdef _WIN32
std::string ucs2_to_utf8(const std::wstring& utf16) {
  if (utf16.empty()) {
    return std::string();
  }

  const int utf8_length = WideCharToMultiByte(
      CP_UTF8, 0, utf16.data(), utf16.length(), NULL, 0, NULL, NULL);

  if (utf8_length == 0) {
    printf("Warning: ucs2_to_utf8 : WideCharToMultiByte returned zero length");
    return std::string();
  }

  std::string utf8(utf8_length, 0);

  if (!WideCharToMultiByte(CP_UTF8, 0, utf16.data(), utf16.length(), &utf8[0],
                           utf8.length(), NULL, NULL)) {
    printf("Warning: ucs2_to_utf8 : WideCharToMultiByte conversion failed");
  }

  return utf8;
}

std::wstring utf8_to_ucs2(const std::string& utf8) {
  if (utf8.empty()) {
    return std::wstring();
  }

  const int utf16_length =
      MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.length(), NULL, 0);

  if (utf16_length == 0) {
    printf("Warning: utf8_to_ucs2 : MultiByteToWideChar returned zero length");
    return std::wstring();
  }

  std::wstring utf16(utf16_length, 0);
  if (!MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.length(), &utf16[0],
                           utf16.length())) {
    printf("Warning: utf8_to_ucs2 : MultiByteToWideChar conversion failed");
  }

  return utf16;
}
#endif

std::string Time_to_String(time_t t, const char* format) {
  char str_time[60];
  // `format` is a controlled internal strftime pattern chosen by callers, not
  // user input, so the non-literal format string is safe here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  strftime(str_time, 60, format, localtime(&t));
#pragma GCC diagnostic pop

  return str_time;
}

ObjectDirection Get_Opposite_Direction(const ObjectDirection direction) {
  switch (direction) {
    case DIR_UP:
      return DIR_DOWN;
    case DIR_DOWN:
      return DIR_UP;
    case DIR_LEFT:
      return DIR_RIGHT;
    case DIR_RIGHT:
      return DIR_LEFT;
    case DIR_HORIZONTAL:
      return DIR_VERTICAL;
    case DIR_VERTICAL:
      return DIR_HORIZONTAL;
    default:
      break;
  }

  return DIR_UNDEFINED;
}

std::string Get_Direction_Name(const ObjectDirection dir) {
  switch (dir) {
    case DIR_UNDEFINED:
      return N_("undefined");
    case DIR_LEFT:
      return N_("left");
    case DIR_RIGHT:
      return N_("right");
    case DIR_UP:
      return N_("up");
    case DIR_DOWN:
      return N_("down");
    case DIR_TOP_LEFT:
      return N_("top_left");
    case DIR_TOP_RIGHT:
      return N_("top_right");
    case DIR_BOTTOM_LEFT:
      return N_("bottom_left");
    case DIR_BOTTOM_RIGHT:
      return N_("bottom_right");
    case DIR_LEFT_TOP:
      return N_("left_top");
    case DIR_LEFT_BOTTOM:
      return N_("left_bottom");
    case DIR_RIGHT_TOP:
      return N_("right_top");
    case DIR_RIGHT_BOTTOM:
      return N_("right_bottom");
    case DIR_HORIZONTAL:
      return N_("horizontal");
    case DIR_VERTICAL:
      return N_("vertical");
    case DIR_ALL:
      return N_("all");
    case DIR_FIRST:
      return N_("first");
    case DIR_LAST:
      return N_("last");
    default:
      break;
  }

  return "";
}

ObjectDirection Get_Direction_Id(const std::string& str_direction) {
  if (str_direction.compare("undefined") == 0) {
    return DIR_UNDEFINED;
  } else if (str_direction.compare("left") == 0) {
    return DIR_LEFT;
  } else if (str_direction.compare("right") == 0) {
    return DIR_RIGHT;
  } else if (str_direction.compare("up") == 0) {
    return DIR_UP;
  } else if (str_direction.compare("down") == 0) {
    return DIR_DOWN;
  } else if (str_direction.compare("top_left") == 0) {
    return DIR_TOP_LEFT;
  } else if (str_direction.compare("top_right") == 0) {
    return DIR_TOP_RIGHT;
  } else if (str_direction.compare("bottom_left") == 0) {
    return DIR_BOTTOM_LEFT;
  } else if (str_direction.compare("bottom_right") == 0) {
    return DIR_BOTTOM_RIGHT;
  } else if (str_direction.compare("left_top") == 0) {
    return DIR_LEFT_TOP;
  } else if (str_direction.compare("left_bottom") == 0) {
    return DIR_LEFT_BOTTOM;
  } else if (str_direction.compare("right_top") == 0) {
    return DIR_RIGHT_TOP;
  } else if (str_direction.compare("right_bottom") == 0) {
    return DIR_RIGHT_BOTTOM;
  } else if (str_direction.compare("horizontal") == 0) {
    return DIR_HORIZONTAL;
  } else if (str_direction.compare("vertical") == 0) {
    return DIR_VERTICAL;
  } else if (str_direction.compare("all") == 0) {
    return DIR_ALL;
  } else if (str_direction.compare("first") == 0) {
    return DIR_FIRST;
  } else if (str_direction.compare("last") == 0) {
    return DIR_LAST;
  }

  return DIR_UNDEFINED;
}

SpriteType Get_Sprite_Type_Id(const std::string& str_type) {
  if (str_type.compare("massive") == 0) {
    return TYPE_MASSIVE;
  } else if (str_type.compare("passive") == 0) {
    return TYPE_PASSIVE;
  } else if (str_type.compare("front_passive") == 0) {
    return TYPE_FRONT_PASSIVE;
  } else if (str_type.compare("halfmassive") == 0) {
    return TYPE_HALFMASSIVE;
  } else if (str_type.compare("climbable") == 0) {
    return TYPE_CLIMBABLE;
  } else {
    printf("Warning : Unknown Sprite Type String %s\n", str_type.c_str());
  }

  return TYPE_UNDEFINED;
}

Color Get_Sprite_Color(const cSprite* sprite) {
  switch (sprite->m_sprite_array) {
    case ARRAY_ENEMY: {
      return red;
    }
    case ARRAY_ACTIVE: {
      return blue;
    }
    case ARRAY_MASSIVE: {
      if (sprite->m_type == TYPE_PLAYER) {
        return lila;
      }

      return orange;
    }
    case ARRAY_PASSIVE: {
      if (sprite->m_type == TYPE_FRONT_PASSIVE) {
        return greenyellow;
      }

      return green;
    }
    case ARRAY_HUD: {
      return blackalpha128;
    }
    default: {
      break;
    }
  }

  return lightgreyalpha64;
}

std::string Get_Massive_Type_Name(const MassiveType mtype) {
  switch (mtype) {
    case MASS_PASSIVE:
      return "passive";
    case MASS_MASSIVE:
      return "massive";
    case MASS_HALFMASSIVE:
      return "halfmassive";
    case MASS_CLIMBABLE:
      return "climbable";
    default:
      break;
  }

  return "";
}

MassiveType Get_Massive_Type_Id(const std::string& str_massivetype) {
  if (str_massivetype.compare("passive") == 0) {
    return MASS_PASSIVE;
  } else if (str_massivetype.compare("massive") == 0) {
    return MASS_MASSIVE;
  } else if (str_massivetype.compare("halfmassive") == 0) {
    return MASS_HALFMASSIVE;
  } else if (str_massivetype.compare("climbable") == 0) {
    return MASS_CLIMBABLE;
  }

  return MASS_PASSIVE;
}

Color Get_Massive_Type_Color(MassiveType mtype) {
  switch (mtype) {
    case MASS_MASSIVE:
      return lightred;
    case MASS_HALFMASSIVE:
      return orange;
    case MASS_PASSIVE:
      return lightgreen;
    case MASS_CLIMBABLE:
      return lila;
    default:
      break;
  }

  return white;
}

GroundType Get_Ground_Type_Id(const std::string& str_groundtype) {
  if (str_groundtype.compare("normal") == 0) {
    return GROUND_NORMAL;
  } else if (str_groundtype.compare("earth") == 0) {
    return GROUND_EARTH;
  } else if (str_groundtype.compare("ice") == 0) {
    return GROUND_ICE;
  } else if (str_groundtype.compare("sand") == 0) {
    return GROUND_SAND;
  } else if (str_groundtype.compare("stone") == 0) {
    return GROUND_STONE;
  } else if (str_groundtype.compare("plastic") == 0) {
    return GROUND_PLASTIC;
  }

  return GROUND_NORMAL;
}

std::string Get_Level_Land_Type_Name(const LevelLandType land_type) {
  switch (land_type) {
    case LLT_UNDEFINED:
      return N_("undefined");
    case LLT_GREEN:
      return N_("green");
    case LLT_JUNGLE:
      return N_("jungle");
    case LLT_ICE:
      return N_("ice");
    case LLT_SNOW:
      return N_("snow");
    case LLT_WATER:
      return N_("water");
    case LLT_CANDY:
      return N_("candy");
    case LLT_DESERT:
      return N_("desert");
    case LLT_SAND:
      return N_("sand");
    case LLT_CASTLE:
      return N_("castle");
    case LLT_UNDERGROUND:
      return N_("underground");
    case LLT_CRYSTAL:
      return N_("crystal");
    case LLT_GHOST:
      return N_("ghost");
    case LLT_MUSHROOM:
      return N_("mushroom");
    case LLT_SKY:
      return N_("sky");
    case LLT_PLASTIC:
      return N_("plastic");
    default:
      break;
  }

  return "";
}

LevelLandType Get_Level_Land_Type_Id(const std::string& str_type) {
  if (str_type.compare("undefined") == 0) {
    return LLT_UNDEFINED;
  } else if (str_type.compare("green") == 0) {
    return LLT_GREEN;
  } else if (str_type.compare("jungle") == 0) {
    return LLT_JUNGLE;
  } else if (str_type.compare("ice") == 0) {
    return LLT_ICE;
  } else if (str_type.compare("snow") == 0) {
    return LLT_SNOW;
  } else if (str_type.compare("water") == 0) {
    return LLT_WATER;
  } else if (str_type.compare("candy") == 0) {
    return LLT_CANDY;
  } else if (str_type.compare("desert") == 0) {
    return LLT_DESERT;
  } else if (str_type.compare("sand") == 0) {
    return LLT_SAND;
  } else if (str_type.compare("castle") == 0) {
    return LLT_CASTLE;
  } else if (str_type.compare("underground") == 0) {
    return LLT_UNDERGROUND;
  } else if (str_type.compare("crystal") == 0) {
    return LLT_CRYSTAL;
  } else if (str_type.compare("ghost") == 0) {
    return LLT_GHOST;
  } else if (str_type.compare("mushroom") == 0) {
    return LLT_MUSHROOM;
  } else if (str_type.compare("sky") == 0) {
    return LLT_SKY;
  } else if (str_type.compare("plastic") == 0) {
    return LLT_PLASTIC;
  }

  return LLT_UNDEFINED;
}

std::string Get_Color_Name(const DefaultColor color) {
  switch (color) {
    case COL_DEFAULT:
      return N_("default");
    case COL_WHITE:
      return N_("white");
    case COL_BLACK:
      return N_("black");
    case COL_RED:
      return N_("red");
    case COL_ORANGE:
      return N_("orange");
    case COL_YELLOW:
      return N_("yellow");
    case COL_GREEN:
      return N_("green");
    case COL_BLUE:
      return N_("blue");
    case COL_BROWN:
      return N_("brown");
    case COL_GREY:
      return N_("grey");
    default:
      break;
  }

  return "";
}

DefaultColor Get_Color_Id(const std::string& str_color) {
  if (str_color.compare("white") == 0) {
    return COL_WHITE;
  } else if (str_color.compare("black") == 0) {
    return COL_BLACK;
  } else if (str_color.compare("red") == 0) {
    return COL_RED;
  } else if (str_color.compare("orange") == 0) {
    return COL_ORANGE;
  } else if (str_color.compare("yellow") == 0) {
    return COL_YELLOW;
  } else if (str_color.compare("green") == 0) {
    return COL_GREEN;
  } else if (str_color.compare("blue") == 0) {
    return COL_BLUE;
  } else if (str_color.compare("brown") == 0) {
    return COL_BROWN;
  } else if (str_color.compare("grey") == 0) {
    return COL_GREY;
  }

  return COL_DEFAULT;
}

std::string Get_Difficulty_Name(Uint8 difficulty) {
  if (difficulty == 0) {
    return "Undefined";
  } else if (difficulty < 10) {
    return "Easy-Peasy";
  } else if (difficulty < 20) {
    return "Very Easy";
  } else if (difficulty < 30) {
    return "Easy";
  } else if (difficulty < 40) {
    return "Easy-Medium";
  } else if (difficulty < 50) {
    return "Medium";
  } else if (difficulty < 60) {
    return "Medium-Hard";
  } else if (difficulty < 70) {
    return "Hard";
  } else if (difficulty < 80) {
    return "Very Hard";
  } else if (difficulty < 90) {
    return "Extreme";
  } else if (difficulty < 95) {
    return "Nightmare";
  }

  return "Ultimate";
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
