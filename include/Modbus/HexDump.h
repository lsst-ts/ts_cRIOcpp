/*
 * Dumping hex strings.
 *
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

#ifndef __Modbus__HexDump__
#define __Modbus__HexDump__

#include <iomanip>
#include <sstream>

namespace Modbus {
/**
 * Dumps hex data to ostring stream.
 *
 * @param dt buffer to print
 * @param len length of the buffer
 */
template <typename dt>
static const std::string hexDump(const dt* buf, size_t len) {
    std::ostringstream os;
    os << std::setfill('0') << std::hex;
    for (size_t i = 0; i < len; i++) {
        if (i > 0) {
            os << " ";
        }
        os << std::setw(sizeof(dt) * 2) << +(buf[i]);
    }
    os << std::dec;
    return os.str();
}

template <typename dt>
static const std::string hexDump(const std::vector<dt>& data) {
    return hexDump<dt>(data.data(), data.size());
}

}  // namespace Modbus

#endif  /// !__Modbus__HexDump__
