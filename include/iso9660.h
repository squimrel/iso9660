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

#include <array>
#include <string>
#include <utility>

namespace iso9660 {

enum class SectorType {
  BOOT_RECORD = 0,
  PRIMARY = 1,
  SUPPLEMENTARY = 2,
  PARTITION = 3,
  SET_TERMINATOR = 255
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

}  // namespace iso9660

#endif  // ISO9660_ISO9660_H_
