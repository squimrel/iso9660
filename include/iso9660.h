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

/**
 * Collection of utility functions that are deticated specifically to reading
 * an ISO 9660 image.
 */

#ifndef ISO9660_ISO9660_H_
#define ISO9660_ISO9660_H_

#include <cstdint>
#include <cstdlib>

#include <array>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace iso9660 {

enum class SectorType {
  BOOT_RECORD = 0,
  PRIMARY = 1,
  SUPPLEMENTARY = 2,
  PARTITION = 3,
  SET_TERMINATOR = 255
};

/**
 * ECMA-119 calls this a directory record but it's actually either a file or a
 * directory so it's a file.
 */
struct File {
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

  bool has(Flag flag) const;
  bool isdir() const;
};

/**
 * ECMA-119 calls this a path table record which is basically a directory.
 */
struct Directory {
  std::size_t size;
  std::size_t extended_length;
  std::size_t location;
  int parent;
  std::string name;
  std::vector<File> files;
};

struct VolumeDescriptorHeader {
  SectorType type;
  /*
   * Identifier specifies according to what specification the following data
   * was recorded.  The following information was taken from ECMA-167/3:
   * BEA01: 2/9.2 Beginning extended area descriptor.
   * BOOT2: 2/9.4 Boot descriptor.
   * CD001: ECMA-119
   * CDW02: ECMA-168
   * NSR02: ECMA-167/2
   * NSR03: ECMA-167/3
   * TEA01: 2/9.3 Terminating extended area descriptor.
   */
  std::string identifier;
  int version;
};

/**
 * The "generic" volume descriptor for the primary and supplementary volume
 * descriptor.
 */
struct VolumeDescriptor {
  struct VolumeDescriptorHeader header;
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
  File root_directory;
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
  std::vector<Directory> path_table;
};

struct VolumeDescriptors {
  VolumeDescriptors() : primary(nullptr), supplementary(nullptr) {}
  std::unique_ptr<VolumeDescriptor> primary;
  std::unique_ptr<VolumeDescriptor> supplementary;
};

constexpr std::size_t LONG_DATETIME_SIZE = 17;
constexpr std::size_t SHORT_DATETIME_SIZE = 7;
constexpr std::size_t DIRECTORY_RECORD_SIZE = 34;
constexpr std::size_t IDENTIFIER_SIZE = 128;
constexpr std::size_t APPLICATION_USE_SIZE = 512;
constexpr std::size_t FILE_IDENTIFIER_SIZE = 37;
constexpr std::size_t SECTOR_SIZE = 2048;
constexpr std::size_t NUM_SYSTEM_SECTORS = 16;
constexpr std::size_t SYSTEM_AREA_SIZE = NUM_SYSTEM_SECTORS * SECTOR_SIZE;

enum class Identifier {
  ECMA_119,
  ECMA_168,
  // ECMA-167 Edition 2
  ECMA_167_PREVIOUS,
  // ECMA-167 has a different identifiers for nearly each volume descriptor.
  ECMA_167,
  ECMA_167_EXTENDED,
  ECMA_167_BOOT,
  ECMO_167_TERMINATOR,
  UNKNOWN
};

Identifier identifier_of(const std::string& identifier);

std::size_t sector_align(std::size_t size);

using Buffer = std::array<unsigned char, iso9660::SECTOR_SIZE>;

class Image {
 private:
  void read_directories(std::vector<iso9660::Directory>* const directories);
  std::vector<iso9660::File> read_directory(std::size_t location);
  void read_path_table(VolumeDescriptor* const volume_descriptor);

 public:
  explicit Image(std::fstream* file);
  void read();

 private:
  std::fstream& file_;
  Buffer buffer_;
  VolumeDescriptors volume_descriptors_;
};

}  // namespace iso9660

#endif  // ISO9660_ISO9660_H_
