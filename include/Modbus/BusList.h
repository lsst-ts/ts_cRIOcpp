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

/**
 * Error thrown when a response is missing. This is mostly caused by an @glos{ILC} on
 * the bus being dead/not reacting to the command send.
 */
class MissingResponse : std::runtime_error {
public:
    /**
     * Construct missing response exception.
     *
     * @param address Expected @glos{ILC} address, for which response wasn't received
     * @param func Expected function which wasn't responded by the @glos{ILC}
     */
    MissingResponse(uint8_t address, uint8_t func)
            : std::runtime_error(fmt::format("Missing response for function {} from ILC with address {}",
                                             func, address)) {}
};

/**
 * Error thrown when response action for the function received in response is
 * undefined.
 *
 * @see BusList::addResponse
 */
class UnexpectedResponse : std::runtime_error {
public:
    /**
     * Construct unexpected response.
     *
     * @param address Modbus address for which the response action was undefined
     * @param func Function code for which the response action was undefined
     */
    UnexpectedResponse(uint8_t address, uint8_t func)
            : std::runtime_error(
                      fmt::format("Unexpected response - received {} for address {}", address, func)) {}
};

/**
 * Utility class to hold command buffer with timinig data. BusList is a collection of those
 */
class CommandRecord {
public:
    /**
     * Construct record for the command send.
     *
     * @param _buffer Modbus buffer with command (address/function with possible arguments)
     * @param _timming Maximal response time in microseconds
     */
    CommandRecord(Buffer _buffer, uint32_t _timming) : buffer(_buffer), timming(_timming) {}

    Buffer buffer;     ///< Buffer send to the bus
    uint32_t timming;  ///< Timing value in microseconds. An error shall be thrown when the response isn't
                       ///< received in the specified time.
};

/**
 * Holds callbacks for supported functions.
 */
class ResponseRecord {
public:
    /**
     * Construct callback entry.
     *
     * @param _func Function code
     * @param _action Action to execute when function code is encountered
     * @param _error_reply Error reply. That's usually 0x80 | _func
     * @param _error_action Error action
     */
    ResponseRecord(uint8_t _func, std::function<void(Parser)> _action, uint8_t _error_reply,
                   std::function<void(uint8_t, uint8_t)> _error_action)
            : func(_func), action(_action), error_reply(_error_reply), error_action(_error_action) {}

    const uint8_t func;                                  ///< sucessfull response function code
    std::function<void(Parser)> action;                  ///< action to call on sucessfull response
    const uint8_t error_reply;                           ///< error response code
    std::function<void(uint8_t, uint8_t)> error_action;  ///< action to call on the error response
};

/**
 * Manages communication with devices on the ModBus bus. Provides methods to
 * call function, construct a buffer to send to the @glos{FPGA}, and handles
 * the @glos{FPGA} response. Should be sub-classed to form a specialized
 * classes for a devices and purposes (e.g. a different @glos{ILC} types).
 *
 * When functions are scheduled to be called, the record of the call is stored
 * in the classs. When response buffer is parsed, the class check if the passed
 * response address and function code matches expected values.
 *
 * The BusList is ideally suited to send a repated message patterns. Take for
 * example the M1M3. The loop to operate the mirror sends to @glos{ILC} those
 * commands:
 *
 * 1. Broadcast command to set actuators target values. Use of the broadcast,
 * which doesn't require any response from the @glos{ILC}s, significantly decreases
 * time needed to send the target values to @glos{ILC}s.
 *
 * 2. Query all @glos{ILC}s in unicast fashion (adressing all @glos{ILC}s on the bus). The
 * responses are checked against target and safe values.
 *
 * As the second set of commands is the same (query is 4 bytes, address,
 * function code and 2 byte CRC, and those values are fixed), those can be
 * stored and hence no CRC computation is executed in the loop. Only the
 * broadcast command is constructed every time the loop executes.
 *
 * Usually the commands are send on the @glos{FIFO} along with communication-specific
 * data, e.g. time the port communication logic shall wait for the reply and
 * expected length of the reply. So the list is formed in the loop, passed in
 * FIFO to the @glos{FPGA}. The @glos{FPGA} executes commands, and store
 * responses in a FIFO.  This approach distrubutes communication logic to the
 * @glos{FPGA}, leaving CPU free to perform other tasks.
 *
 * When parsing responses, the class check if responses's addresses matches
 * expected addresses.
 */
class BusList : public std::vector<CommandRecord> {
public:
    BusList();

    /**
     * Reset bus list parsing processing. Move index of the parsed responses
     * back to beggining, indicating a new responses might be parsed.
     */
    void reset() { _parsed_index = 0; }

    /**
     * Calls Modbus function. Timing parameter is passed to the @glos{FPGA},
     * which is responsible to properly use it. Timing parameter is passed to
     * the @glos{FPGA}, which is responsible to pass it to the hardware
     * controller.
     *
     * @param address ModBus/@glos{ILC} address @param func ModBus/@glos{ILC}
     * function code @param timing function call timing in microseconds
     * (1/10^-6 second)
     */
    void callFunction(uint8_t address, uint8_t func, uint32_t timming) {
        emplace(end(), CommandRecord(Buffer(address, func), timming));
    }

    /**
     * Template to call Modbus function with variable arguments. Argument types
     * are used to encode data as Modbus command, including possible endian
     * conversion. Timing parameter is passed to the @glos{FPGA}, which is responsible to
     * pass it to the hardware controller.
     *
     * @param address ModBus/@glos{ILC} address
     * @param func ModBus/@glos{ILC} function code
     * @param timming function call timing in microseconds (1/10^-6 second)
     * @param ...params
     *
     * Intended usage:
     *
     * @code{cpp}
     * BusList busList;
     *
     * uint16_t arg1 = 1;
     * float arg2 = 2.34;
     * uint8_t arg3 = 5;
     *
     * busList.callFunction(10, 11, 500, arg1, arg2, arg3);
     * @endcode
     */
    template <typename... dt>
    void callFunction(uint8_t address, uint8_t func, uint32_t timming, const dt &...params) {
        emplace(end(), CommandRecord(Buffer(address, func, params...), timming));
    }

    /**
     * Process @glos{ILC} response. Address and function of the response are checked if
     *
     * @throw MissingResponse when response for the address is missing. The
     * calling code can catch the exception, pass the same response again, and
     * if it pass, then the @glos{ILC} did not respond to command and can (if that's
     * safe) disable the failing @glos{ILC}.
     *
     * @throw UnexpectedResponse Throwed when the bus list action for the
     * address/function (set with the addResponse call) wasn't set.
     */
    void parse(Parser parser);

    /**
     * Construct buffer from passed data and parse the response.
     *
     * @see ::parse(Parser)
     */
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

private:
    std::list<ResponseRecord> _functions;

    size_t _parsed_index = 0;
};

}  // namespace Modbus

#endif /* !__Modbus_BusList__ */
