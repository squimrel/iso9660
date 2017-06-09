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

#include "./include/volume-descriptor.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>

#include "./include/file.h"
#include "./include/buffer.h"
#include "./include/read.h"
#include "./include/utility.h"

/**
 * Read primary or supplementary volume descriptor according to ECMA-119.
 */
iso9660::VolumeDescriptor::VolumeDescriptor(
    iso9660::Buffer::const_iterator first, iso9660::Buffer::const_iterator last,
    iso9660::VolumeDescriptorHeader generic_header) {
  constexpr std::size_t APPLICATION_USE_SIZE = 512;
  constexpr std::size_t DIRECTORY_RECORD_SIZE = 34;
  constexpr std::size_t FILE_IDENTIFIER_SIZE = 37;
  constexpr std::size_t IDENTIFIER_SIZE = 128;
  constexpr std::size_t LONG_DATETIME_SIZE = 17;
  using utility::integer;
  namespace read = iso9660::read;

  header = std::move(generic_header);
  if (header.type == iso9660::SectorType::SUPPLEMENTARY)
    utility::at(first, last, &flags, 7);
  system_identifier = utility::substr(first, last, 8, 32);
  volume_identifier = utility::substr(first, last, 40, 32);
  if (header.type == iso9660::SectorType::SUPPLEMENTARY)
    escape_sequences = utility::substr(first, last, 88, 32);

  integer(&volume_space_size, first + 80, first + 84, 4);
  integer(&volume_sequence_number, first + 124, first + 126, 2);
  integer(&logical_block_size, first + 128, first + 130, 2);
  integer(&path_table_size, first + 132, first + 136, 4);
  /*
   * As specified in 6.9.2 there're actually two path tables which store the
   * same information but using different endianness.
   */
  integer(&path_table_location, first + 140, first + 148, 4);
  integer(&optional_path_table_location, first + 144, first + 152, 4);

  root_directory = std::unique_ptr<iso9660::File>(
      new iso9660::File(first + 156, DIRECTORY_RECORD_SIZE));
  volume_set_identifier = utility::substr(first, last, 190, IDENTIFIER_SIZE);
  publisher_identifier = utility::substr(first, last, 318, IDENTIFIER_SIZE);
  data_preparer_identifier = utility::substr(first, last, 446, IDENTIFIER_SIZE);
  application_identifier = utility::substr(first, last, 574, IDENTIFIER_SIZE);
  copyright_file_identifier =
      utility::substr(first, last, 702, FILE_IDENTIFIER_SIZE);
  abstract_file_identifier =
      utility::substr(first, last, 739, FILE_IDENTIFIER_SIZE);
  bibliographic_file_identifier =
      utility::substr(first, last, 776, FILE_IDENTIFIER_SIZE);
  volume_create_datetime = read::long_datetime(first + 813, LONG_DATETIME_SIZE);
  volume_modify_datetime = read::long_datetime(first + 830, LONG_DATETIME_SIZE);
  volume_expiration_datetime =
      read::long_datetime(first + 847, LONG_DATETIME_SIZE);
  volume_effective_datetime =
      read::long_datetime(first + 864, LONG_DATETIME_SIZE);
  utility::at(first, last, &file_structure_version, 881);
  application_use = utility::substr(first, last, 883, APPLICATION_USE_SIZE);
}

/**
 * Initialize lookup table so that files can be quickly found by name.
 */
void iso9660::VolumeDescriptor::build_file_lookup() {
  for (const auto& directory : path_table->directories) {
    for (const auto& file : directory.files) {
      if (!file.isdir()) {
        filenames.emplace(file.name, &file);
      }
    }
  }
}

int iso9660::VolumeDescriptor::joliet_level() const {
  /*
   * The Joliet specification does not specify what the difference between the
   * different levels is. Level 1-3 all specifiy the use of UCS-2 stored in the
   * big endian format.
   */
  static std::unordered_map<char, int> joliet_level{
      {'@', 1}, {'C', 2}, {'E', 3}};
  if (escape_sequences[0] == '%' && escape_sequences[1] == '/' &&
      escape_sequences[3] == '\0') {
    auto level = joliet_level.find(escape_sequences[2]);
    if (level != joliet_level.end()) return level->second;
  }
  return 0;
}
