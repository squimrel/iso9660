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

#include <iostream>
#include <string>

#include "./include/iso9660.h"

#include "./example/file-manipulation.h"

template <class F>
static void add_overlay(iso9660::Image* const isoimage,
                        const std::string& filename, F func) {
  auto file = isoimage->find(filename);
  if (file == nullptr) {
    std::cout << "Can't find " + filename + ". Skipping..\n" << std::flush;
    return;
  }
  isoimage->modify_file(*file, func);
}
