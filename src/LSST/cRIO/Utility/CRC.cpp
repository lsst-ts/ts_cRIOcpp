/*
 * This file is part of LSST M1M3 support system package.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cRIO/CRC.h>
#include <spdlog/spdlog.h>

namespace LSST {
namespace cRIO {

uint16_t CRC::modbus(uint8_t* buffer, int32_t startIndex, int32_t length) {
    uint16_t crc = 0xFFFF;
    for (int i = startIndex; i < startIndex + length; i++) {
        crc = crc ^ ((uint16_t)buffer[i]);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = crc >> 1;
                crc = crc ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

uint16_t CRC::modbus(uint16_t* buffer, int32_t startIndex, int32_t length) {
    uint16_t crc = 0xFFFF;
    for (int i = startIndex; i < startIndex + length; i++) {
        crc = crc ^ buffer[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = crc >> 1;
                crc = crc ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

}  // namespace cRIO
}  // namespace LSST
