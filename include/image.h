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

#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "./include/buffer.h"
#include "./include/file.h"
#include "./include/path-table.h"
#include "./include/volume-descriptor.h"

#ifndef ISO9660_IMAGE_H_
#define ISO9660_IMAGE_H_

namespace iso9660 {

class Image {
 private:
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
  static Identifier identifier_of(const std::string& identifier);
  void read_directories(iso9660::PathTable* const path_table);
  std::vector<iso9660::File> read_directory(std::size_t location);
  void read_path_table(iso9660::VolumeDescriptor* const volume_descriptor);
  iso9660::SectorType read_volume_descriptor();

 public:
  EXPORT explicit Image(std::fstream* file);
  EXPORT void read();
  EXPORT void write();
  EXPORT const iso9660::File* find(const std::string& filename);
  EXPORT bool modify_file(
      const iso9660::File& file,
      std::function<std::streamsize(std::fstream*, const iso9660::File&)>
          modify);

 private:
  std::fstream& file_;
  iso9660::Buffer buffer_;
  std::unique_ptr<iso9660::VolumeDescriptor> primary_;
  std::unique_ptr<iso9660::VolumeDescriptor> supplementary_;
  /**
   * In case only a minor change happened, i.e. the position of the file does
   * not need to be changed. It's convenient to know where the file information
   * is stored on the image.
   */
  std::unordered_map<std::size_t, std::vector<std::size_t>> file_positions_;
};

}  // namespace iso9660

#endif  // ISO9660_IMAGE_H_
