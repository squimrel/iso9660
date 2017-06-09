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

#include "./include/file.h"

#include <algorithm>
#include <utility>

#include "./include/buffer.h"
#include "./include/read.h"
#include "./include/utility.h"

/**
 * Read directory record according to ECMA-119. Note that in ECMA-167
 * information control block tags are used instead.
 */
iso9660::File::File(iso9660::Buffer::const_iterator first,
                    iso9660::Buffer::const_iterator last) {
  constexpr std::size_t SHORT_DATETIME_SIZE = 7;
  using utility::integer;
  utility::at(first, last, &length, 0);
  // + length + (length % 2 == 0 ? 0 : 1);
  utility::at(first, last, &extended_length, 1);
  extended_length = std::move(sector_align(extended_length));
  integer(&location, first + 2, first + 6, 4);
  integer(&size, first + 10, first + 14, 4);
  datetime = iso9660::read::short_datetime(first + 18, SHORT_DATETIME_SIZE);
  utility::at(first, last, &flags, 25);
  utility::at(first, last, &file_unit_size, 26);
  utility::at(first, last, &interleave_gap_size, 27);
  integer(&volume_sequence_number, first + 28, first + 30, 2);
  std::size_t length = utility::at(first, last, 32);
  name = utility::substr(first, last, 33, length);
  // TODO(squimrel): Read rock ridge attributes.
}

iso9660::File::File(iso9660::Buffer::const_iterator first, std::size_t size) {
  File(first, first + size);
}

bool iso9660::File::has(iso9660::File::Flag flag) const {
  return (flags & static_cast<int>(flag)) != 0;
}

/**
 * Check if file is a directory. There's an extra function for this since this
 * is most likely the most common operation.
 */
bool iso9660::File::isdir() const {
  return has(iso9660::File::Flag::DIRECTORY);
}

std::size_t iso9660::File::max_growth() const {
  return sector_align(size) - size - extended_length;
}

std::size_t iso9660::File::sector_align(std::size_t size) {
  return (size + (iso9660::SECTOR_SIZE - 1)) & -iso9660::SECTOR_SIZE;
}
