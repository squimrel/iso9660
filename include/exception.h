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

#ifndef ISO9660_EXCEPTION_H_
#define ISO9660_EXCEPTION_H_

#include <stdexcept>
#include <string>

#include "./include/buffer.h"

namespace iso9660 {

class EXPORT Exception : public std::runtime_error {
 public:
  explicit Exception(const std::string& what_arg);
};

class EXPORT NotImplementedException : public Exception {
  using Exception::Exception;
};

class EXPORT CorruptFileException : public Exception {
  using Exception::Exception;
};

}  // namespace iso9660

#endif  // ISO9660_EXCEPTION_H_
