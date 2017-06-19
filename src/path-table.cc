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

#include "./include/path-table.h"

#include <algorithm>
#include <iterator>

#include "./include/buffer.h"
#include "./include/utility.h"

/**
 * Read path table record according to ECMA-119. Note that the path table is not
 * used anymore in ECMA-167.
 */
iso9660::Directory::Directory(iso9660::Buffer::const_iterator first,
                              iso9660::Buffer::const_iterator last) {
  using utility::integer;
  const auto length = static_cast<int>(utility::at(first, last, 0));
  // 8 byte + length of name + padding.
  size = 8 + length + (length % 2 == 0 ? 0 : 1);
  utility::at(first, last, &extended_length, 1);
  integer(&location, first + 2, 4);
  integer(&parent, first + 6, 2);
  name = utility::substr(first, last, 8, length);
}

/**
 * Read path table according to ECMA-119. Note that the path table is
 * not used anymore in ECMA-167.
 */
iso9660::PathTable::PathTable(iso9660::Buffer::const_iterator first,
                              iso9660::Buffer::const_iterator last) {
  std::size_t size = std::distance(first, last);
  for (std::size_t record_position = 0; record_position < size;) {
    iso9660::Directory directory(first + record_position, last);
    record_position += directory.size;
    directories.emplace_back(std::move(directory));
  }
}

/**
 * Post-process path table of supplementary volume descriptor that uses joliet.
 */
void iso9660::PathTable::joliet() {
  for (auto& directory : directories) {
    directory.name = utility::from_ucs2(std::move(directory.name));
    for (auto& file : directory.files) {
      if (!file.isdir()) {
        file.name = utility::from_ucs2(std::move(file.name));
      }
    }
  }
}
