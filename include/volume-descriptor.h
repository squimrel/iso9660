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

#ifndef ISO9660_VOLUME_DESCRIPTOR_H_
#define ISO9660_VOLUME_DESCRIPTOR_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "./include/file.h"
#include "./include/iso9660.h"
#include "./include/path-table.h"

namespace iso9660 {

/**
 * The "generic" ECMA-119 volume descriptor for the primary and supplementary
 * volume descriptor.
 */
class VolumeDescriptor {
 public:
  VolumeDescriptorHeader header;
  // Only used in the supplementary volume descriptor.
  int flags;
  std::string system_identifier;
  std::string volume_identifier;
  std::size_t volume_space_size;
  // Only used in the supplementary volume descriptor.
  std::string escape_sequences;
  std::size_t volume_sequence_number;
  std::size_t logical_block_size;
  std::size_t path_table_size;
  std::size_t path_table_location;
  std::size_t optional_path_table_location;
  std::unique_ptr<iso9660::File> root_directory;
  std::string volume_set_identifier;
  std::string publisher_identifier;
  std::string data_preparer_identifier;
  std::string application_identifier;
  std::string copyright_file_identifier;
  std::string abstract_file_identifier;
  std::string bibliographic_file_identifier;
  std::int64_t volume_create_datetime;
  std::int64_t volume_modify_datetime;
  std::int64_t volume_expiration_datetime;
  std::int64_t volume_effective_datetime;
  int file_structure_version;
  std::string application_use;
  // The path table specifies the directory hierarchy.
  std::unique_ptr<iso9660::PathTable> path_table;
  // A lookup table that can be used to quickly find files.
  std::unordered_multimap<std::string, const iso9660::File*> filenames;

  VolumeDescriptor(iso9660::Buffer::const_iterator first,
                   iso9660::Buffer::const_iterator last,
                   iso9660::VolumeDescriptorHeader generic_header);
  void build_file_lookup();
  int joliet_level() const;
};

}  // namespace iso9660

#endif  // ISO9660_VOLUME_DESCRIPTOR_H_
