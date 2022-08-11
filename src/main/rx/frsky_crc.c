/*
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "rx/frsky_crc.h"

uint8_t frskyCheckSum(uint8_t *data, uint8_t length) {
    uint16_t checksum = 0;
    for (unsigned i = 0; i < length; i++) {
        checksum += *data++;
    }
    while (checksum > 0xFF) {
        checksum = (checksum & 0xFF) + (checksum >> 8);
    }
    checksum = 0xFF - checksum;
    return checksum;
}
