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

#ifndef ISO9660_PATH_TABLE_H_
#define ISO9660_PATH_TABLE_H_

#include <string>
#include <utility>
#include <vector>

#include "./include/file.h"
#include "./include/iso9660.h"

namespace iso9660 {

/**
 * ECMA-119 calls this a path table record which is basically a directory.
 */
class Directory {
 public:
  std::size_t size;
  std::size_t extended_length;
  std::size_t location;
  int parent;
  std::string name;
  std::vector<iso9660::File> files;

  Directory(iso9660::Buffer::const_iterator first,
            iso9660::Buffer::const_iterator last);
};

class PathTable {
 public:
  std::vector<Directory> directories;

  PathTable(iso9660::Buffer::const_iterator first,
            iso9660::Buffer::const_iterator last);
  void joliet();
};

}  // namespace iso9660

#endif  // ISO9660_PATH_TABLE_H_
