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

#ifndef ISO9660_UTILITY_H_
#define ISO9660_UTILITY_H_

#include <iterator>
#include <numeric>
#include <string>
#include <utility>

#include "./include/buffer.h"

namespace utility {

enum class Endian { BIG = 0, LITTLE = 1 };

/*
 * No matter how it tries to detect endieness it'll not be conform to the
 * standard anyways so this should be just fine.
 */
#undef ENDIANNESS
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN ||                 \
    defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) ||       \
    defined(__MIBSEB__)
#define ENDIANNESS utility::Endian::BIG
#else
#define ENDIANNESS utility::Endian::LITTLE
#endif

void big_endian(std::string* const raw);
std::string from_ucs2(const std::u16string& from);
std::string from_ucs2(std::string&& raw);

std::string substr(iso9660::Buffer::const_iterator first,
                   iso9660::Buffer::const_iterator last, std::size_t at,
                   std::size_t size = 0);

template <class T>
T integer(iso9660::Buffer::const_iterator first,
          iso9660::Buffer::const_iterator last) {
  constexpr std::size_t BITS_IN_BYTE = 8;
  const std::size_t bytes = std::distance(first, last);
  const std::size_t bits = (bytes - 1) * BITS_IN_BYTE;

  std::size_t i = 0;
  return std::accumulate(
      first, last, static_cast<T>(0),
      [&i, bits](T result, iso9660::Buffer::value_type byte) {
        result |= static_cast<T>(byte) << (bits - i * BITS_IN_BYTE);
        ++i;
        return result;
      });
}

template <std::size_t SIZE, Endian ENDIAN, class T>
std::array<char, SIZE> integer(const T number) {
  constexpr std::size_t BITS_IN_BYTE = 8;
  constexpr std::size_t bits = (SIZE - 1) * BITS_IN_BYTE;
  std::array<char, SIZE> result;
  for (std::size_t i = 0; i < SIZE; ++i) {
    if (ENDIAN == ENDIANNESS) {
      result[i] = (number >> (bits - i * BITS_IN_BYTE)) & 0xff;
    } else {
      result[i] = (number >> (bits - (SIZE - i - 1) * BITS_IN_BYTE)) & 0xff;
    }
  }
  return result;
}

template <class T>
void integer(T* const number, iso9660::Buffer::const_iterator big_endian,
             iso9660::Buffer::const_iterator little_endian, std::size_t size) {
  if (ENDIANNESS == utility::Endian::LITTLE) {
    *number = integer<T>(little_endian, little_endian + size);
  } else {
    *number = integer<T>(big_endian, big_endian + size);
  }
}

/**
 * The caller has to guarantee that the passed characters store the number in
 * the users system endian.
 */
template <class T>
void integer(T* const number, iso9660::Buffer::const_iterator first,
             std::size_t size) {
  *number = integer<T>(first, first + size);
}

iso9660::Buffer::value_type at(iso9660::Buffer::const_iterator first,
                               iso9660::Buffer::const_iterator last,
                               std::size_t index);

template <class T>
void at(iso9660::Buffer::const_iterator first,
        iso9660::Buffer::const_iterator last, T* const result,
        std::size_t index) {
  *result = static_cast<T>(at(first, last, index));
}

}  // namespace utility

#endif  // ISO9660_UTILITY_H_
