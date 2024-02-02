/*
 * Bus List handling communication (receiving and
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

#ifndef __Modbus_BusList__
#define __Modbus_BusList__

#include <vector>

#include <Modbus/Buffer.h>

namespace Modbus {

/**
 * Manages communication with ILCs. Provides methods to call function,
 * construct a buffer to send to FPGA, and handles FPGA response. Should be
 * sub-classed to form a specialized classes for a devices and purposes (e.g. a
 * different ILC types).
 */
class BusList : public std::vector<Buffer> {
public:
    BusList();

    template <typename... dt>
    void callFunction(uint8_t address, uint8_t func, const dt&... params) {
        emplace(end(), Buffer(address, func, params...));
    }
};

}  // namespace Modbus

#endif /* !__Modbus_BusList__ */
