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

#ifndef ISO9660_FILE_H_
#define ISO9660_FILE_H_

#include <string>
#include <utility>

#include "./include/iso9660.h"

namespace iso9660 {

/**
 * ECMA-119 calls this a directory record but it's actually either a file or a
 * directory so it's a file.
 */
class File {
 public:
  enum class Flag {
    /*
     * Set if file is hidden.
     */
    HIDDEN = 1,
    /*
     * Set if entry is a directory.
     */
    DIRECTORY = 1 << 1,
    /*
     * Set if entry is an associated file.
     */
    ASSOCIATED = 1 << 2,
    /*
     * Set if information is structured according to the extended attribute
     * record.
     */
    EXTENDED_STRUCTURE = 1 << 3,
    /*
     * Set if owner, group and permissions are specified in the extended
     * attribute.
     */
    EXTENDED_PERMISSIONS = 1 << 4,
    /*
     * Set if file has more than one directory record.
     */
    MULTIPLE_RECORDS = 1 << 7
  };

  std::size_t length;
  std::size_t extended_length;
  std::size_t location;
  std::size_t size;
  int datetime;
  int flags;
  int file_unit_size;
  int interleave_gap_size;
  int volume_sequence_number;
  std::string name;

  File(iso9660::Buffer::const_iterator first,
       iso9660::Buffer::const_iterator last);
  File(iso9660::Buffer::const_iterator first, std::size_t size);
  bool has(Flag flag) const;
  bool isdir() const;
  std::size_t max_growth() const;
};

}  // namespace iso9660

#endif  // ISO9660_FILE_H_
