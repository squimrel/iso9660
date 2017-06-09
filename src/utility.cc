/*
 * Copyright (C) 2017 squimrel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include "./include/utility.h"

#include <algorithm>
#include <codecvt>
#include <iterator>
#include <locale>
#include <string>

#include "./include/buffer.h"

/**
 * Convert from UCS-2 to UTF-8 string.
 */
std::string utility::from_ucs2(const std::u16string& from) {
  static std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
  return convert.to_bytes(from.c_str());
}

/**
 * Convert a raw string that actually uses UCS-2 big endian to UTF-8 string.
 */
std::string utility::from_ucs2(std::string&& raw) {
  utility::big_endian(&raw);
  raw += '\0';
  std::u16string result(reinterpret_cast<const char16_t*>(raw.c_str()),
                        raw.size());
  return utility::from_ucs2(std::move(result));
}

/**
 * Correct byte order for our target system assuming that the current byte
 * order is big endian.
 */
void utility::big_endian(std::string* const raw) {
  if (ENDIANNESS == utility::Endian::BIG) return;
  for (auto first = raw->begin(); first < raw->end(); first += 2) {
    std::swap(*first, *(first + 1));
  }
}

/**
 * Move a part of an array to a string.
 *
 * Since the ISO 9660 standard sometimes refers to values higher than 127 an
 * unsigned char is used to work directly with the read data so that the values
 * provided by ISO 9660 do not have to be converted to its signed 8 bit
 * equivalent by the user.
 *
 * When human readable data is stored we need to convert it back to char and
 * we'll copy it into a string so that it won't change if the data in the view
 * changes.
 */
std::string utility::substr(iso9660::Buffer::const_iterator first,
                            iso9660::Buffer::const_iterator last,
                            std::size_t at, std::size_t size) {
  return std::string(
      std::make_move_iterator(first + at),
      std::make_move_iterator(size == 0 ? last : (first + at + size)));
}

iso9660::Buffer::value_type utility::at(iso9660::Buffer::const_iterator first,
                                        iso9660::Buffer::const_iterator last,
                                        std::size_t index) {
  // Could do out of bounds check here.
  return std::move(*(first + index));
}
