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

#ifndef U16ARRAYUTILITIES_H_
#define U16ARRAYUTILITIES_H_

#include <cRIO/DataTypes.h>

namespace LSST {
namespace cRIO {

class U16ArrayUtilities {
public:
    static uint64_t u64(uint16_t* buffer, int32_t index) {
        return ((uint64_t)buffer[index] << 48) | ((uint64_t)buffer[index + 1] << 32) |
               ((uint64_t)buffer[index + 2] << 16) | ((uint64_t)buffer[index + 3]);
    }
};

}  // namespace cRIO
}  // namespace LSST

#endif /* U16ARRAYUTILITIES_H_ */
