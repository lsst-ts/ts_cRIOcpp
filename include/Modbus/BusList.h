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

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include <Modbus/Buffer.h>
#include <Modbus/Parser.h>

namespace Modbus {

class CalledFunction {
public:
    CalledFunction(uint8_t address, uint8_t func) : _address(address), _func(func) {}

private:
    uint8_t _address;
    uint8_t _func;
};

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
    void callFunction(uint8_t address, uint8_t func, const dt &...params) {
        called.emplace(called.end(), CalledFunction(address, func));
        emplace(end(), Buffer(address, func, params...));
    }

    void parse(uint8_t *data, size_t len);

    /**
     * Add response callbacks. Both function code and error response code shall
     * be specified.
     *
     * @param func callback for this function code
     * @param action action to call when the response is found. Passed address
     * as sole parameter. Should read response (as length of the response data
     * is specified by function) and check CRC (see ModbusBuffer::read and
     * ModbusBuffer::checkCRC)
     * @param errorResponse error response code
     * @param errorAction action to call when error is found. If no action is
     * specified, raises ModbusBuffer::Exception. Th action receives two parameters,
     * address and error code. CRC checking is done in processResponse. This
     * method shall not manipulate the buffer (e.g. shall not call
     * ModbusBuffer::read or ModbusBuffer::checkCRC).
     *
     * @see checkCached
     */
    void addResponse(uint8_t func, std::function<void(Parser)> action, uint8_t errorResponse,
                     std::function<void(uint8_t, uint8_t)> errorAction = nullptr);

protected:
    std::vector<CalledFunction> called;

private:
    std::map<uint8_t, std::function<void(Parser)>> _actions;
    std::map<uint8_t, std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>> _errorActions;
};

}  // namespace Modbus

#endif /* !__Modbus_BusList__ */
