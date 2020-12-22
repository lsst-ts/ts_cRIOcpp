/*
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org). See the COPYRIGHT file at the top-level directory of
 * this distribution for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CRC_H_
#define CRC_H_

#include <cRIO/DataTypes.h>

namespace LSST {
namespace cRIO {

/**
 * CRC utility functions.
 */
class CRC {
public:
    /**
     * Calculates 16 bit Modbus CRC. See (CRC calculator)[https://crccalc.com]
     * for checks.
     *
     * @param buffer data buffer
     * @param startIndex start index
     * @param length data lenght
     *
     * @return 16 bits Modbus CRC
     */
    static uint16_t modbus(uint8_t* buffer, int32_t startIndex, int32_t length);

    /**
     * Calculates 16 bit Modbus CRC. See (CRC calculator)[https://crccalc.com]
     * for checks.
     *
     * @param buffer data buffer
     * @param startIndex start index
     * @param length data lenght
     *
     * @return 16 bits Modbus CRC
     */
    static uint16_t modbus(uint16_t* buffer, int32_t startIndex, int32_t length);
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRC_H_ */
