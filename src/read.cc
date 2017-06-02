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

#include "./include/read.h"

#include <cstdint>
#include <ctime>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

#include "./include/iso9660.h"
#include "./include/utility.h"

/**
 * @return Datetime in milliseconds since epoch with up-to centisecond
 * precision. Therefore the timezone offset will be discarded but taken into
 * account.
 */
std::int64_t iso9660::read::long_datetime(
    iso9660::Buffer::const_iterator first,
    iso9660::Buffer::const_iterator last) {
  auto toint = [first, last](int pos, int count) {
    /**
     * The potential overhead of copying to string could be avoided by
     * implementing an stoi function that accepts an iterator range.
     */
    return std::stoi(utility::substr(first, last, pos, count));
  };
  // Year from 1 to 9999.
  int year = toint(0, 4);
  /*
   * The specification says that the date and time are not specified if all
   * digits and the offset byte are zero.
   * This implementation quickly aborts if it the year is zero since the
   * specification requires the year to be in the range from 1 to 9999 this
   * seems reasonable.
   */
  if (year == 0) return 0;
  struct tm datetime;
  datetime.tm_year = year - 1900;
  // Month of the year from 1 to 12.
  datetime.tm_mon = toint(4, 2) - 1;
  // Day of the month from 1 to 31.
  datetime.tm_mday = toint(6, 2);
  // Hour of the day from 0 to 23.
  datetime.tm_hour = toint(8, 2);
  // Minute of the hour from 0 to 59.
  datetime.tm_min = toint(10, 2);
  // Second of the minute from 0 to 59.
  datetime.tm_sec = toint(12, 2);
  datetime.tm_isdst = 0;
  // Hundredths of a second.
  std::int64_t milliseconds = toint(14, 2) * 10;
  // Offset from UTC in number of 15 minute intervals from -48 (West) to +52
  // (East).
  int offset = static_cast<int>(
                   static_cast<std::uint8_t>(utility::at(first, last, 16))) *
               15;
  std::int64_t seconds = mktime(&datetime) - offset * 60;
  milliseconds += seconds * 1000;
  return milliseconds;
}

std::int64_t iso9660::read::long_datetime(iso9660::Buffer::const_iterator first,
                                          std::size_t size) {
  return iso9660::read::long_datetime(first, first + size);
}

/**
 * The datetime that will be parsed is supposed to be in the following format:
 * 1. byte: Number of years since 1900.
 * 2. byte: Month of the year from 1 to 12.
 * 3. byte: Day of the month from 1 to 31.
 * 4. byte: Hour of the day from 0 to 23.
 * 5. byte: Minute of hour from 0 to 59.
 * 6. byte: Second of minute from 0 to 59.
 * 7. byte: Offset from Greenwich mean time in number for 15 minute intervales
 * from -48 (West) to +52 (East).
 *
 * @return Datetime in seconds since epoch. Therefore the timezone offset will
 * be discarded but taken into * account.
 */
int iso9660::read::short_datetime(iso9660::Buffer::const_iterator first,
                                  iso9660::Buffer::const_iterator last) {
  struct tm datetime;
  utility::at(first, last, &datetime.tm_year, 0);
  if (datetime.tm_year == 0) return 0;
  utility::at(first, last, &datetime.tm_mon, 1);
  datetime.tm_mon -= 1;
  utility::at(first, last, &datetime.tm_mday, 2);
  utility::at(first, last, &datetime.tm_hour, 3);
  utility::at(first, last, &datetime.tm_min, 4);
  utility::at(first, last, &datetime.tm_sec, 5);
  datetime.tm_isdst = 0;
  int offset =
      static_cast<int>(static_cast<std::uint8_t>(utility::at(first, last, 6))) *
      15;
  return mktime(&datetime) - offset * 60;
}

int iso9660::read::short_datetime(iso9660::Buffer::const_iterator first,
                                  std::size_t size) {
  return iso9660::read::short_datetime(first, first + size);
}

/**
 * Read directory record according to ECMA-119. Note that in ECMA-167
 * information control block tags are used instead.
 */
iso9660::File iso9660::read::directory_record(
    iso9660::Buffer::const_iterator first,
    iso9660::Buffer::const_iterator last) {
  using utility::integer;
  iso9660::File file;
  utility::at(first, last, &file.length, 0);
  // + length + (length % 2 == 0 ? 0 : 1);
  utility::at(first, last, &file.extended_length, 1);
  file.extended_length = std::move(iso9660::sector_align(file.extended_length));
  integer(&file.location, first + 2, first + 6, 4);
  integer(&file.size, first + 10, first + 14, 4);
  file.datetime =
      iso9660::read::short_datetime(first + 18, SHORT_DATETIME_SIZE);
  utility::at(first, last, &file.flags, 25);
  utility::at(first, last, &file.file_unit_size, 26);
  utility::at(first, last, &file.interleave_gap_size, 27);
  integer(&file.volume_sequence_number, first + 28, first + 30, 2);
  std::size_t length = utility::at(first, last, 32);
  file.name = utility::substr(first, last, 33, length);
  // TODO(squimrel): Read rock ridge attributes.
  return file;
}

iso9660::File iso9660::read::directory_record(
    iso9660::Buffer::const_iterator first, std::size_t size) {
  return iso9660::read::directory_record(first, first + size);
}

/**
 * Read path table according to ECMA-119. Note that the path table is not used
 * anymore in ECMA-167.
 */
iso9660::Directory iso9660::read::directory(
    iso9660::Buffer::const_iterator first,
    iso9660::Buffer::const_iterator last) {
  using utility::integer;
  iso9660::Directory directory;
  const auto length = static_cast<int>(utility::at(first, last, 0));
  // 8 byte + length of name + padding.
  directory.size = 8 + length + (length % 2 == 0 ? 0 : 1);
  utility::at(first, last, &directory.extended_length, 1);
  integer(&directory.location, first + 2, 4);
  integer(&directory.parent, first + 6, 2);
  directory.name = utility::substr(first, last, 8, length);
  return directory;
}

/**
 * Read path table record according to ECMA-119. Note that the path table is
 * not used anymore in ECMA-167.
 */
std::vector<iso9660::Directory> iso9660::read::path_table(
    iso9660::Buffer::const_iterator first,
    iso9660::Buffer::const_iterator last) {
  std::vector<iso9660::Directory> records;
  std::size_t size = std::distance(first, last);
  for (std::size_t record_position = 0; record_position < size;) {
    auto directory = iso9660::read::directory(first + record_position, last);
    record_position += directory.size;
    records.emplace_back(std::move(directory));
  }
  return records;
}

/**
 * Post-process supplementary volume descriptor that uses joliet.
 */
void iso9660::read::joliet(std::vector<iso9660::Directory>* path_table) {
  for (auto& directory : *path_table) {
    directory.name = utility::from_ucs2(std::move(directory.name));
    for (auto& file : directory.files) {
      if (!file.isdir()) {
        file.name = utility::from_ucs2(std::move(file.name));
      }
    }
  }
}

/**
 * Read any volume descriptor.
 */
iso9660::SectorType iso9660::read::volume_descriptor(
    iso9660::Buffer::const_iterator first, iso9660::Buffer::const_iterator last,
    iso9660::VolumeDescriptors* volume_descriptors) {
  using iso9660::SectorType;
  auto type = static_cast<iso9660::SectorType>(utility::at(first, last, 0));
  iso9660::VolumeDescriptorHeader header;
  header.type = type;
  header.identifier = utility::substr(first, last, 1, 5);
  utility::at(first, last, &header.version, 6);
  auto identifier = iso9660::identifier_of(header.identifier);
  // Currently only ECMA 119 is understood.
  if (identifier != iso9660::Identifier::ECMA_119) {
    throw std::runtime_error("Unknown identifier: " + header.identifier);
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
      auto tmp =
          iso9660::read::volume_descriptor(first, last, std::move(header));
      if (type == SectorType::PRIMARY) {
        volume_descriptors->primary = std::move(tmp);
      } else if (tmp->joliet_level() > 0) {
        volume_descriptors->supplementary = std::move(tmp);
      } else {
        std::cout << "Warning: Skipping non-joliet supplementary volume "
                  << "descriptor record." << std::endl;
      }
    } break;
    // ECMA 119 - 9.3
    case SectorType::PARTITION:
    default:
      std::cout << "Unknown type: " << static_cast<int>(header.type)
                << std::endl;
      break;
  }
  return type;
}

/**
 * Read primary or supplementary volume descriptor according to ECMA-119.
 */
std::unique_ptr<iso9660::VolumeDescriptor> iso9660::read::volume_descriptor(
    iso9660::Buffer::const_iterator first, iso9660::Buffer::const_iterator last,
    iso9660::VolumeDescriptorHeader header) {
  using utility::integer;
  namespace read = iso9660::read;

  /*
   * FIXME: Use C++14. By the way there's no reason to have this managed by a
   * unique_ptr. std::optional could be used but there's no C++17 anyways.
   */
  std::unique_ptr<iso9660::VolumeDescriptor> vd(new iso9660::VolumeDescriptor);
  vd->header = std::move(header);
  if (vd->header.type == iso9660::SectorType::SUPPLEMENTARY)
    utility::at(first, last, &vd->flags, 7);
  vd->system_identifier = utility::substr(first, last, 8, 32);
  vd->volume_identifier = utility::substr(first, last, 40, 32);
  if (vd->header.type == iso9660::SectorType::SUPPLEMENTARY)
    vd->escape_sequences = utility::substr(first, last, 88, 32);

  integer(&vd->volume_space_size, first + 80, first + 84, 4);
  integer(&vd->volume_sequence_number, first + 124, first + 126, 2);
  integer(&vd->logical_block_size, first + 128, first + 130, 2);
  integer(&vd->path_table_size, first + 132, first + 136, 4);
  /*
   * As specified in 6.9.2 there're actually two path tables which store the
   * same information but using different endianness.
   */
  integer(&vd->path_table_location, first + 140, first + 148, 4);
  integer(&vd->optional_path_table_location, first + 144, first + 152, 4);

  vd->root_directory =
      read::directory_record(first + 156, iso9660::DIRECTORY_RECORD_SIZE);
  vd->volume_set_identifier =
      utility::substr(first, last, 190, iso9660::IDENTIFIER_SIZE);
  vd->publisher_identifier =
      utility::substr(first, last, 318, iso9660::IDENTIFIER_SIZE);
  vd->data_preparer_identifier =
      utility::substr(first, last, 446, iso9660::IDENTIFIER_SIZE);
  vd->application_identifier =
      utility::substr(first, last, 574, iso9660::IDENTIFIER_SIZE);
  vd->copyright_file_identifier =
      utility::substr(first, last, 702, iso9660::FILE_IDENTIFIER_SIZE);
  vd->abstract_file_identifier =
      utility::substr(first, last, 739, iso9660::FILE_IDENTIFIER_SIZE);
  vd->bibliographic_file_identifier =
      utility::substr(first, last, 776, iso9660::FILE_IDENTIFIER_SIZE);
  vd->volume_create_datetime =
      read::long_datetime(first + 813, iso9660::LONG_DATETIME_SIZE);
  vd->volume_modify_datetime =
      read::long_datetime(first + 830, iso9660::LONG_DATETIME_SIZE);
  vd->volume_expiration_datetime =
      read::long_datetime(first + 847, iso9660::LONG_DATETIME_SIZE);
  vd->volume_effective_datetime =
      read::long_datetime(first + 864, iso9660::LONG_DATETIME_SIZE);
  utility::at(first, last, &vd->file_structure_version, 881);
  vd->application_use =
      utility::substr(first, last, 883, iso9660::APPLICATION_USE_SIZE);
  return vd;
}
