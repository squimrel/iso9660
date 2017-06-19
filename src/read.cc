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

#include "./include/buffer.h"
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
