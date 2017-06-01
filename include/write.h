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

#ifndef ISO9660_WRITE_H_
#define ISO9660_WRITE_H_

#include <cstdlib>

#include <algorithm>
#include <ostream>
#include <utility>

#include "./include/utility.h"

namespace iso9660 {
namespace write {

template <class ForwardIt>
void resize_file(std::ostream* const file, ForwardIt first, ForwardIt last,
                 std::size_t size) {
  constexpr std::size_t DIRECTORY_RECORD_SIZE_OFFSET = 10;
  const auto big_endian_size = utility::integer<4, utility::Endian::BIG>(size);
  const auto little_endian_size =
      utility::integer<4, utility::Endian::LITTLE>(size);
  std::for_each(
      first, last,
      [file, &little_endian_size, &big_endian_size](std::size_t position) {
        file->clear();
        file->seekp(position + DIRECTORY_RECORD_SIZE_OFFSET);
        file->write(big_endian_size.data(), big_endian_size.size());
        file->write(little_endian_size.data(), little_endian_size.size());
      });
}

}  // namespace write
}  // namespace iso9660

#endif  // ISO9660_WRITE_H_
