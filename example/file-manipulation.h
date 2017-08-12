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

#include <fstream>
#include <ios>

#include "./include/iso9660.h"

/**
 * Note: A better name for this would be much appreciated.
 */
std::streamsize add_overlay_switch_to_grub_on_fat_image(
    std::fstream* iofile, const iso9660::File& fileinfo);

std::streamsize insert_overlay_switch(std::fstream* iofile,
                                      const iso9660::File& fileinfo);
