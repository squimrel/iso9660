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

#include <cstdint>
#include <cstdlib>

#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "./include/file.h"
#include "./include/iso9660.h"
#include "./include/path-table.h"
#include "./include/volume-descriptor.h"

#ifndef ISO9660_IMAGE_H_
#define ISO9660_IMAGE_H_

namespace iso9660 {

class Image {
 private:
  void read_directories(iso9660::PathTable* const path_table);
  std::vector<iso9660::File> read_directory(std::size_t location);
  void read_path_table(iso9660::VolumeDescriptor* const volume_descriptor);
  iso9660::SectorType read_volume_descriptor();

 public:
  explicit Image(std::fstream* file);
  void read();
  const iso9660::File* find(const std::string& filename);
  void modify_file(
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
