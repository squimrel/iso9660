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

#include "./example/persistent-storage.h"

#include <fstream>
#include <ios>
#include <iostream>

#include "./include/iso9660.h"

#include "./example/file-manipulation.h"

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
  constexpr const char* const configfiles[] = {"isolinux.cfg", "grub.cfg",
                                               "grub.conf"};
  for (const char* const configfile : configfiles) {
    add_overlay(&isoimage, configfile, insert_overlay_switch);
  }
  add_overlay(&isoimage, "efiboot.img",
              add_overlay_switch_to_grub_on_fat_image);
  add_overlay(&isoimage, "macboot.img",
              add_overlay_switch_to_grub_on_hfsplus_image);
  isoimage.write();
}
