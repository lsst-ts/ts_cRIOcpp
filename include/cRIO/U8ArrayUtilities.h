/*
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the Vera C.Rubin Observatory Project
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

#ifndef U8ARRAYUTILITIES_H_
#define U8ARRAYUTILITIES_H_

#include <cRIO/DataTypes.h>
#include <string>
#include <cstring>

namespace LSST {
namespace cRIO {

class U8ArrayUtilities {
public:
    inline static int8_t I8(uint8_t* buffer, int32_t index) { return (int8_t)buffer[index]; }

    inline static int16_t I16(uint8_t* buffer, int32_t index) {
        return ((int16_t)buffer[index] << 8) | (int16_t)buffer[index + 1];
    }

    inline static int32_t I32(uint8_t* buffer, int32_t index) {
        return ((int32_t)buffer[index] << 24) | ((int32_t)buffer[index + 1] << 16) |
               ((int32_t)buffer[index + 2] << 8) | (int32_t)buffer[index + 3];
    }

    inline static int64_t I64(uint8_t* buffer, int32_t index) {
        return ((int64_t)buffer[index] << 54) | ((int64_t)buffer[index + 1] << 48) |
               ((int64_t)buffer[index + 2] << 40) | ((int64_t)buffer[index + 3] << 32) |
               ((int64_t)buffer[index + 4] << 24) | ((int64_t)buffer[index + 5] << 16) |
               ((int64_t)buffer[index + 6] << 8) | (int64_t)buffer[index + 7];
    }

    inline static uint8_t U8(uint8_t* buffer, int32_t index) { return buffer[index]; }

    inline static uint16_t U16(uint8_t* buffer, int32_t index) {
        return ((uint16_t)buffer[index] << 8) | (uint16_t)buffer[index + 1];
    }

    inline static uint32_t U32(uint8_t* buffer, int32_t index) {
        return ((uint32_t)buffer[index] << 24) | ((uint32_t)buffer[index + 1] << 16) |
               ((uint32_t)buffer[index + 2] << 8) | (uint32_t)buffer[index + 3];
    }

    inline static uint64_t U64(uint8_t* buffer, int32_t index) {
        return ((uint64_t)buffer[index] << 56) | ((uint64_t)buffer[index + 1] << 48) |
               ((uint64_t)buffer[index + 2] << 40) | ((uint64_t)buffer[index + 3] << 32) |
               ((uint64_t)buffer[index + 4] << 24) | ((uint64_t)buffer[index + 5] << 16) |
               ((uint64_t)buffer[index + 6] << 8) | ((uint64_t)buffer[index + 7]);
    }

    inline static float SGL(uint8_t* buffer, int32_t index) {
        uint8_t tempBuffer[4] = {buffer[index + 3], buffer[index + 2], buffer[index + 1], buffer[index]};
        float value = 0;
        memcpy(&value, tempBuffer, 4);
        return value;
    }

    static std::string toString(uint8_t* buffer, int32_t index, int32_t length) {
        char tmp[256];
        memset(tmp, 0, 256);
        memcpy(tmp, buffer + index, length);
        return std::string(tmp, length);
    }
};

}  // namespace cRIO
}  // namespace LSST

#endif /* U8ARRAYUTILITIES_H_ */
