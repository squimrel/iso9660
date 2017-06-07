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

#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "./include/file.h"
#include "./include/read.h"
#include "./include/volume-descriptor.h"
#include "./include/write.h"

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

std::size_t iso9660::sector_align(std::size_t size) {
  return (size + (iso9660::SECTOR_SIZE - 1)) & -iso9660::SECTOR_SIZE;
}
