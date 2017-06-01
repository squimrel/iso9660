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

#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "./include/iso9660.h"

std::streamsize insert_overlay_switch(std::fstream* iofile,
                                      const iso9660::File& fileinfo) {
  auto& file = *iofile;
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
      throw std::runtime_error("Unexpected end of file at byte " +
                               std::to_string(read_bytes) + " of " +
                               std::to_string(fileinfo.size) + ".");
    }
    const auto position = line.find(needle);
    if (position != std::string::npos) {
      growth += overlay_switch.size();
      if (growth > max_growth) {
        throw std::runtime_error(
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

void add_overlay(iso9660::Image* const isoimage, const std::string& filename) {
  auto file = isoimage->find(filename);
  if (file == nullptr) {
    throw std::runtime_error("Can't find " + filename);
  }
  isoimage->modify_file(*file, insert_overlay_switch);
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <boot.iso>\n" << std::flush;
    return 1;
  }
  std::fstream isofile(argv[1],
                       std::ios::binary | std::ios::in | std::ios::out);
  if (!isofile.is_open()) {
    return 1;
  }
  iso9660::Image isoimage(&isofile);
  isoimage.read();
  add_overlay(&isoimage, "isolinux.cfg");
  add_overlay(&isoimage, "grub.cfg");
  add_overlay(&isoimage, "grub.conf");
}
