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

#include "./include/image.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#ifndef NDEBUG
#include <iostream>
#endif

#include "./include/exception.h"
#include "./include/file.h"
#include "./include/buffer.h"
#include "./include/path-table.h"
#include "./include/volume-descriptor.h"
#include "./include/write.h"

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
    iso9660::File file(buffer_.begin() + offset, buffer_.end());
    auto result = file_positions_.find(file.location);
    if (result == file_positions_.end()) {
      file_positions_.emplace(file.location,
                              std::vector<std::size_t>({position + offset}));
    } else {
      result->second.emplace_back(position + offset);
    }
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

void iso9660::Image::read_directories(iso9660::PathTable* const path_table) {
  for (auto& directory : path_table->directories) {
    directory.files = read_directory(directory.location);
  }
}

void iso9660::Image::read_path_table(
    iso9660::VolumeDescriptor* const volume_descriptor) {
  if (volume_descriptor == nullptr) return;
  auto& volume = *volume_descriptor;
  if (volume.path_table_size > iso9660::SECTOR_SIZE) {
    throw iso9660::NotImplementedException(
        "This implementation can't handle path tables that have a size greater "
        "than the size of one sector.");
  }
  std::size_t begin = volume.path_table_location * iso9660::SECTOR_SIZE;
  std::size_t end = begin + volume.path_table_size;
  std::size_t maxsize = std::min(iso9660::SECTOR_SIZE, end - begin);
  file_.seekg(begin);
  file_.read(reinterpret_cast<char*>(buffer_.data()), maxsize);
  volume.path_table = std::unique_ptr<iso9660::PathTable>(
      new iso9660::PathTable(buffer_.begin(), buffer_.begin() + maxsize));
  read_directories(volume.path_table.get());
}

/**
 * Read any volume descriptor.
 */
iso9660::SectorType iso9660::Image::read_volume_descriptor() {
  auto first = buffer_.begin();
  auto last = buffer_.end();
  using iso9660::SectorType;
  auto type = static_cast<iso9660::SectorType>(utility::at(first, last, 0));
  iso9660::VolumeDescriptorHeader header;
  header.type = type;
  header.identifier = utility::substr(first, last, 1, 5);
  utility::at(first, last, &header.version, 6);
  auto identifier = identifier_of(header.identifier);
  // Currently only ECMA 119 is understood.
  if (identifier != iso9660::Image::Identifier::ECMA_119) {
    throw iso9660::NotImplementedException("Unknown identifier: " +
                                           header.identifier);
  }
  switch (type) {
    // ECMA 119 - 8.2
    case SectorType::BOOT_RECORD:
      break;
    // ECMA 119 - 8.3
    case SectorType::SET_TERMINATOR:
      break;
    // ECMA 119 - 8.4
    case SectorType::PRIMARY:
    // ECMA 119 - 8.5
    case SectorType::SUPPLEMENTARY: {
      std::unique_ptr<iso9660::VolumeDescriptor> tmp(
          new iso9660::VolumeDescriptor(first, last, std::move(header)));
      if (type == SectorType::PRIMARY) {
        primary_ = std::move(tmp);
      } else if (tmp->joliet_level() > 0) {
        supplementary_ = std::move(tmp);
      } else {
#ifndef NDEBUG
        std::cout << "Warning: Skipping non-joliet supplementary volume "
                  << "descriptor record." << std::endl;
#endif
      }
    } break;
    // ECMA 119 - 9.3
    case SectorType::PARTITION:
    default:
#ifndef NDEBUG
      std::cout << "Unknown type: " << static_cast<int>(header.type)
                << std::endl;
#endif
      break;
  }
  return type;
}

void iso9660::Image::read() {
  // Skip system area.
  file_.seekg(iso9660::SYSTEM_AREA_SIZE);
  for (;;) {
    file_.read(reinterpret_cast<char*>(buffer_.data()), iso9660::SECTOR_SIZE);
    if (read_volume_descriptor() == iso9660::SectorType::SET_TERMINATOR) {
      break;
    }
  }
  /*
   * Joliet uses the supplementary volume descriptor. It's alright if there's
   * only one
   */
  if (primary_.get() == nullptr && supplementary_.get() == nullptr) {
    throw iso9660::CorruptFileException(
        "Couldn't find a primary or supplementary volume descriptor.");
  }
  read_path_table(primary_.get());
  read_path_table(supplementary_.get());
}

/**
 * After staging modifications one must always write.
 */
void iso9660::Image::write() {}

/**
 * Find the first file matching by name.
 */
const iso9660::File* iso9660::Image::find(const std::string& filename) {
  bool has_supplementary = supplementary_ != nullptr;
  auto& volume = has_supplementary ? *supplementary_ : *primary_;
  if (volume.filenames.empty()) {
    if (has_supplementary) {
      volume.path_table->joliet();
    }
    volume.build_file_lookup();
  }
  auto result = volume.filenames.find(filename);
  if (result == volume.filenames.end()) {
    return nullptr;
  }
  return result->second;
}

/**
 * An unsafe way to modify a file.
 * Unsafe because the fstream that is exposed to the user has power over the
 * entire ISO 9660 image at the moment even though it should only have power
 * over a tiny file section.
 *
 * TODO: Create an abstraction that provides a specially crafted fstream to the
 * user which can even grow an arbitrary amount.
 *
 * An easier solution would be to have a remove_file and add_file function.
 * Where the add_file function will be given an ifstream that will have to live
 * until write. Especially if the modified file will exceed the maximum allowed
 * growth such an solution would most likely be preferable.
 *
 * Note: Maybe a template should be used instead of std::function.
 */
void iso9660::Image::modify_file(
    const iso9660::File& file,
    std::function<std::streamsize(std::fstream*, const iso9660::File&)>
        modify) {
  file_.seekg(file.location * iso9660::SECTOR_SIZE + file.extended_length);
  std::streamsize growth = modify(&file_, file);
  if (growth == 0) return;
  auto result = file_positions_.find(file.location);
  if (result == file_positions_.end()) {
    throw iso9660::CorruptFileException("Could not find file location.");
  }
  /*
   * FIXME: This is messy. In the future the supplementary and primary files
   * should be summarized in a single File instance so that a lookup like this
   * does not need to be done.
   * When that has been done resize should be a member function of File.
   */
  iso9660::write::resize_file(&file_, result->second.begin(),
                              result->second.end(), file.size + growth);
}

iso9660::Image::Identifier iso9660::Image::identifier_of(
    const std::string& identifier) {
  static const std::unordered_map<std::string, iso9660::Image::Identifier>
      identifiers = {
          {"CD001", iso9660::Image::Identifier::ECMA_119},
          {"CDW02", iso9660::Image::Identifier::ECMA_168},
          {"NSR03", iso9660::Image::Identifier::ECMA_167},
          {"NSR02", iso9660::Image::Identifier::ECMA_167_PREVIOUS},
          {"BEA01", iso9660::Image::Identifier::ECMA_167_EXTENDED},
          {"BOOT2", iso9660::Image::Identifier::ECMA_167_BOOT},
          {"TEA01", iso9660::Image::Identifier::ECMO_167_TERMINATOR}};
  auto result = identifiers.find(identifier);
  if (result != identifiers.end()) {
    return result->second;
  }
  return iso9660::Image::Identifier::UNKNOWN;
}
