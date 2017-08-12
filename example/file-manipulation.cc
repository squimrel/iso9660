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
 * TODO(squimrel): Use a library that helps me with file manipulation and
 * delivers beautiful solutions.
 */

#include "./example/file-manipulation.h"

#include <cstring>

#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "./include/iso9660.h"
#include "./include/utility.h"

std::streamsize add_overlay_switch_to_grub_on_fat_image(
    std::fstream *iofile, const iso9660::File &fileinfo) {
  auto swap_byte_order = [](std::uint32_t num) {
    return (num >> 24) | ((num << 8) & 0x00ff0000) | ((num >> 8) & 0x0000ff00) |
           (num << 24);
  };
  constexpr char configfile[] = "GRUB    CFG";
  /*
   * FIXME(squimrel): Find the location of the grub.cfg file content more
   * reliably by reading the cluster number of its directory entry and
   * locating it with the help of fatlength, sectors per cluster and
   * number of reserved sectors which also have to be read.
   */
  constexpr char grub_signature[] = "set default";
  constexpr int SECTOR_SIZE = 512;
  constexpr int FILESIZE_OFFSET = 28;
  char buffer[SECTOR_SIZE];
  std::streamsize entrypos = -1;
  std::streamsize growth = 0;
  std::uint32_t filesize = 0;
  auto &file = *iofile;
  while (file.read(buffer, SECTOR_SIZE)) {
    // Figure out position of grub.cfg entry.
    char *entry = static_cast<char *>(
        memmem(buffer, SECTOR_SIZE, configfile,
               std::extent<decltype(configfile)>::value - 1));
    if (entry != nullptr) {
      entrypos = (file.tellg() - static_cast<std::streamoff>(SECTOR_SIZE)) +
                 (entry - buffer);
      std::memcpy(&filesize, entry + FILESIZE_OFFSET, sizeof(filesize));
      if (ENDIANNESS == utility::Endian::BIG) {
        filesize = swap_byte_order(filesize);
      }
      break;
    }
  }
  while (file.read(buffer, SECTOR_SIZE)) {
    // Modify overlay switches of grub.cfg.
    if (std::strncmp(buffer, grub_signature,
                     std::extent<decltype(grub_signature)>::value - 1) == 0) {
      file.seekg(-SECTOR_SIZE, std::ios::cur);
      iso9660::File fileinfo;
      fileinfo.size = filesize;
      growth = insert_overlay_switch(iofile, fileinfo);
      filesize += growth;
      break;
    }
  }
  if (entrypos >= 0) {
    file.seekg(entrypos + FILESIZE_OFFSET);
    if (ENDIANNESS == utility::Endian::BIG) {
      filesize = swap_byte_order(filesize);
    }
    file.write(reinterpret_cast<char *>(&filesize), sizeof(filesize));
  }
  return growth;
}

std::streamsize insert_overlay_switch(std::fstream *iofile,
                                      const iso9660::File &fileinfo) {
  auto &file = *iofile;
  const std::string needle = "rd.live.image";
  const std::string overlay_switch =
      " rd.live.overlay=LABEL=OVERLAY:/persistent-overlay.img";
  const std::size_t max_growth = fileinfo.max_growth();
  std::cout << "File has a size of " << fileinfo.size
            << " bytes and can grow by " << max_growth << " bytes.\n"
            << std::flush;
  /*
   * The size by which the file has grown due to added overlay switches.
   */
  std::size_t growth = 0;
  std::size_t insert_position = 0;
  std::size_t read_bytes = 0;
  /*
   * Simply keep the entire modified file section in memory. Could be done
   * smarter.
   */
  std::vector<char> file_content;
  for (std::string line; std::getline(file, line);) {
    line += '\n';
    read_bytes += line.size();
    if (read_bytes > fileinfo.size) {
      throw iso9660::CorruptFileException("Unexpected end of file at byte " +
                                          std::to_string(read_bytes) + " of " +
                                          std::to_string(fileinfo.size) + ".");
    }
    const auto position = line.find(needle);
    if (position != std::string::npos) {
      growth += overlay_switch.size();
      if (growth > max_growth) {
        throw iso9660::NotImplementedException(
            "Bad luck! The file has grown too much. It does not fit into the "
            "ISO 9660 image anymore without some serious modification.");
      }
      const std::size_t line_size = line.size();
      const std::size_t line_offset = position + needle.size();
      line.insert(line_offset, overlay_switch);
      if (insert_position == 0) {
        insert_position = file.tellg();
        insert_position -= line_size - line_offset;
        file_content.reserve((fileinfo.size + max_growth) - read_bytes);
        std::move(line.begin() + line_offset, line.end(),
                  std::back_inserter(file_content));
      } else {
        std::move(line.begin(), line.end(), std::back_inserter(file_content));
      }
    } else if (insert_position != 0) {
      std::move(line.begin(), line.end(), std::back_inserter(file_content));
    }
    if (read_bytes == fileinfo.size) break;
  }
  file.clear();
  file.seekp(insert_position);
  file.write(file_content.data(), file_content.size());
  std::cout << "Grown by " << growth << " of " << max_growth << " bytes.\n"
            << std::flush;
  return growth;
}
