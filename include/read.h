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

#ifndef ISO9660_READ_H_
#define ISO9660_READ_H_

#include <cstdint>
#include <cstdlib>

#include <memory>

#include "./include/iso9660.h"

namespace iso9660 {
namespace read {

std::int64_t long_datetime(iso9660::Buffer::const_iterator first,
                           iso9660::Buffer::const_iterator last);
std::int64_t long_datetime(iso9660::Buffer::const_iterator first,
                           std::size_t size);
int short_datetime(iso9660::Buffer::const_iterator first,
                   iso9660::Buffer::const_iterator last);
int short_datetime(iso9660::Buffer::const_iterator first, std::size_t size);

iso9660::SectorType volume_descriptor(
    iso9660::Buffer::const_iterator first, iso9660::Buffer::const_iterator last,
    iso9660::VolumeDescriptors* volume_descriptors);
std::unique_ptr<iso9660::VolumeDescriptor> volume_descriptor(
    iso9660::Buffer::const_iterator first, iso9660::Buffer::const_iterator last,
    iso9660::VolumeDescriptorHeader header);

iso9660::File directory_record(iso9660::Buffer::const_iterator first,
                               iso9660::Buffer::const_iterator last);
iso9660::File directory_record(iso9660::Buffer::const_iterator first,
                               std::size_t size);
iso9660::Directory directory(iso9660::Buffer::const_iterator first,
                             iso9660::Buffer::const_iterator last);
std::vector<iso9660::Directory> path_table(
    iso9660::Buffer::const_iterator first,
    iso9660::Buffer::const_iterator last);
void joliet(std::vector<iso9660::Directory>* path_table);

}  // namespace read
}  // namespace iso9660

#endif  // ISO9660_READ_H_
