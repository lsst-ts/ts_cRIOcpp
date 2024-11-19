/*
 * Transport base class for communication.
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

#ifndef __Transports_Transport__
#define __Transports_Transport__

#include <vector>

#include <cRIO/Thread.h>
#include <Modbus/BusList.h>

namespace Transports {

/**
 * Base transport class. Provides abstract interface to write and read bytes to
 * serial devices. The methods can throw errors on failures.
 */
class Transport {
public:
    /**
     * Opens transport connection.
     *
     * @throw std::runtime_error on failure
     */
    virtual void open() {}

    /**
     * Closes transport connection.
     *
     * @throw std::runtime_error on failure
     */
    virtual void close() {}

    /**
     * Send data to the other end of the transport.
     *
     * @param buf buffer to send
     * @param size its size
     *
     * @throw std::runtime_error on failure
     */
    virtual void write(const unsigned char* buf, size_t len) = 0;

    /**
     * Reads data from transport. Returns data available until timeout expires. Can be interrupted by notifing
     * interrupt_condition.
     *
     * @param len expected len to read. If timeout expires and data aren't available, returns what was read
     * @param timeout maximal time to wait for data
     * @param calling_thread thread calling the read. Used for waits.
     *
     * @throw std::runtime_error on failure
     */
    virtual std::vector<uint8_t> read(size_t len, std::chrono::microseconds timeout,
                                      LSST::cRIO::Thread* calling_thread = NULL) = 0;

    /**
     * Sends MPU commands to command FIFO. MPU command buffer must be filled
     * before calling this method. Read returns for commands, ask MPU object to process those.
     *
     * @note This function shall live in transport, as this seems to be the
     * only way to combine traditional serial port approach with cRIO DIO based
     * mass processing.
     *
     * @param bus_list Modbus bus list containing the commands and processing
     * @param timeout timeout to sleep before reading
     * @param calling_thread thread calling the read. Used for waits.
     */
    virtual void commands(Modbus::BusList& bus_list, std::chrono::microseconds timeout,
                          LSST::cRIO::Thread* calling_thread = NULL) = 0;

    /**
     * Flush transport buffers.
     */
    virtual void flush() = 0;

    /**
     * Retrieve transport telemetry.
     *
     * @param write_bytes number of bytes written to serial device
     * @param read_bytes number of bytes read from serial device
     */
    virtual void telemetry(uint64_t& write_bytes, uint64_t& read_bytes) = 0;

protected:
    void execute_command(std::vector<uint8_t> command, Modbus::BusList& bus_list,
                         std::chrono::time_point<std::chrono::steady_clock> end,
                         LSST::cRIO::Thread* calling_thread);
};

}  // namespace Transports

#endif  // !__Transports_Transport__
