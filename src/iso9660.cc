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

#include "./include/iso9660.h"

#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "./include/read.h"

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

iso9660::Identifier iso9660::identifier_of(const std::string& identifier) {
  static const std::unordered_map<std::string, iso9660::Identifier>
      identifiers = {{"CD001", iso9660::Identifier::ECMA_119},
                     {"CDW02", iso9660::Identifier::ECMA_168},
                     {"NSR03", iso9660::Identifier::ECMA_167},
                     {"NSR02", iso9660::Identifier::ECMA_167_PREVIOUS},
                     {"BEA01", iso9660::Identifier::ECMA_167_EXTENDED},
                     {"BOOT2", iso9660::Identifier::ECMA_167_BOOT},
                     {"TEA01", iso9660::Identifier::ECMO_167_TERMINATOR}};
  auto result = identifiers.find(identifier);
  if (result != identifiers.end()) {
    return result->second;
  }
  return iso9660::Identifier::UNKNOWN;
}

iso9660::Image::Image(std::fstream* file) : file_(*file) {}

std::vector<iso9660::File> iso9660::Image::read_directory(
    std::size_t location) {
  std::size_t position = location * iso9660::SECTOR_SIZE;
  std::size_t offset = 0;
  file_.seekg(position);
  file_.read(reinterpret_cast<char*>(buffer_.data()), iso9660::SECTOR_SIZE);
  auto record_length = static_cast<std::size_t>(buffer_[offset]);
  std::vector<iso9660::File> files;
  while (record_length > 0) {
    auto file = iso9660::read::directory_record(buffer_.begin() + offset,
                                                buffer_.end());
    offset += file.length;
    record_length = static_cast<std::size_t>(buffer_[offset]);
    /*
     * Currently this implementation does not make use of the extra information
     * that is stored in directory record. Extra in the sense that all
     * information this implementation needs to know about directories is
     * already provided by the path table.
     */
    if (!file.isdir()) {
      files.emplace_back(std::move(file));
    }
    if (offset + record_length >= iso9660::SECTOR_SIZE) {
      /*
       * Ignore issue silently. This implementation does not yet allow the sum
       * of the size of all directory records to be greater than the size of a
       * sector. Maybe this should throw or at least warn.
       */
      break;
    }
  }
  return files;
}

void iso9660::Image::read_directories(
    std::vector<iso9660::Directory>* const directories) {
  for (auto& directory : *directories) {
    directory.files = read_directory(directory.location);
  }
}

void iso9660::Image::read_path_table(
    VolumeDescriptor* const volume_descriptor) {
  if (volume_descriptor == nullptr) return;
  auto& volume = *volume_descriptor;
  if (volume.path_table_size > iso9660::SECTOR_SIZE) {
    throw std::runtime_error(
        "This implementation can't handle path tables that have a size greater "
        "than the size of one sector.");
  }
  std::size_t begin = volume.path_table_location * iso9660::SECTOR_SIZE;
  std::size_t end = begin + volume.path_table_size;
  std::size_t maxsize = std::min(iso9660::SECTOR_SIZE, end - begin);
  file_.seekg(begin);
  file_.read(reinterpret_cast<char*>(buffer_.data()), maxsize);
  volume.path_table =
      iso9660::read::path_table(buffer_.begin(), buffer_.begin() + maxsize);
  read_directories(&volume.path_table);
}

void iso9660::Image::read() {
  // Skip system area.
  file_.seekg(iso9660::SYSTEM_AREA_SIZE);
  for (;;) {
    file_.read(reinterpret_cast<char*>(buffer_.data()), iso9660::SECTOR_SIZE);
    if (iso9660::read::volume_descriptor(buffer_.begin(), buffer_.end(),
                                         &volume_descriptors_) ==
        iso9660::SectorType::SET_TERMINATOR) {
      break;
    }
  }
  /*
   * Joliet uses the supplementary volume descriptor. It's alright if there's
   * only one
   */
  if (volume_descriptors_.primary.get() == nullptr &&
      volume_descriptors_.supplementary.get() == nullptr) {
    throw std::runtime_error(
        "Couldn't find a primary or supplementary volume descriptor.");
  }
}
