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
#include <list>
#include <map>
#include <vector>

#include <spdlog/spdlog.h>

#include <Modbus/Buffer.h>
#include <Modbus/Parser.h>

namespace Modbus {

class MissingResponse : std::runtime_error {
public:
    MissingResponse(uint8_t address, uint8_t func)
            : std::runtime_error(fmt::format("Missing response for function {} from ILC with address {}",
                                             func, address)) {}
};

class UnexpectedResponse : std::runtime_error {
public:
    UnexpectedResponse(uint8_t address, uint8_t func)
            : std::runtime_error(
                      fmt::format("Unexpected response - received {} for address {}", address, func)) {}
};

class CommandRecord {
public:
    CommandRecord(Buffer _buffer, uint32_t _timming) : buffer(_buffer), timming(_timming) {}

    Buffer buffer;
    uint32_t timming;
};

/**
 * Holds callbacks for supported functions.
 */
class ResponseRecord {
public:
    ResponseRecord(uint8_t _func, std::function<void(Parser)> _action, uint8_t _error_reply,
                   std::function<void(uint8_t, uint8_t)> _error_action)
            : func(_func), action(_action), error_reply(_error_reply), error_action(_error_action) {}
    const uint8_t func;
    std::function<void(Parser)> action;
    const uint8_t error_reply;
    std::function<void(uint8_t, uint8_t)> error_action;
};

/**
 * Manages communication with ILCs. Provides methods to call function,
 * construct a buffer to send to FPGA, and handles FPGA response. Should be
 * sub-classed to form a specialized classes for a devices and purposes (e.g. a
 * different ILC types).
 */
class BusList : public std::vector<CommandRecord> {
public:
    BusList(uint8_t bus);

    const uint8_t getBus() { return _bus; }

    /**
     * Reset bus list parsing processing.
     */
    virtual void reset() { _parsed_index = 0; }

    void callFunction(uint8_t address, uint8_t func, uint32_t timming) {
        emplace(end(), CommandRecord(Buffer(address, func), timming));
    }

    template <typename... dt>
    void callFunction(uint8_t address, uint8_t func, uint32_t timming, const dt &...params) {
        emplace(end(), CommandRecord(Buffer(address, func, params...), timming));
    }

    void parse(Parser parser);

    void parse(uint8_t *data, size_t len) { parse(std::vector<uint8_t>(data, data + len)); }

    /**
     * Add response callbacks. Both function code and error response code shall
     * be specified.
     *
     * @param func callback for this function code
     * @param action action to call when the response is found. Passed address
     * as sole parameter. Should read response (as length of the response data
     * is specified by function) and check CRC (see ModbusBuffer::read and
     * ModbusBuffer::checkCRC)
     * @param error_reply error response code
     * @param error_action action to call when error is found. If no action is
     * specified, raises ModbusBuffer::Exception. Th action receives two parameters,
     * address and error code. CRC checking is done in processResponse. This
     * method shall not manipulate the buffer (e.g. shall not call
     * ModbusBuffer::read or ModbusBuffer::checkCRC).
     *
     * @see checkCached
     */
    void addResponse(uint8_t func, std::function<void(Parser)> action, uint8_t error_reply,
                     std::function<void(uint8_t, uint8_t)> error_action = nullptr);

    uint8_t requestBuffer(uint8_t index) { return at(_parsed_index).buffer[index]; }
    uint16_t requestBufferU16(uint8_t index) {
        return Parser::u8tou16(requestBuffer(index + 1), requestBuffer(index));
    }

protected:
    size_t _parsed_index = 0;

private:
    std::list<ResponseRecord> _functions;

    const uint8_t _bus;
};

}  // namespace Modbus

#endif /* !__Modbus_BusList__ */
