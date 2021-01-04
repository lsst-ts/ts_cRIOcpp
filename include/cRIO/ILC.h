/*
 * Implements generic ILC functions.
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

#include <cRIO/ModbusBuffer.h>

namespace LSST {
namespace cRIO {

/**
 * Thrown when ILC error response is received.
 */
class ILCException : public std::runtime_error {
public:
    /**
     * The class is constructed when an error is received. Contains
     *
     * @param address ILC address
     * @param function ILC (error) function received
     * @param exception exception code
     */
    ILCException(uint8_t address, uint8_t function, uint8_t exception);
};

/**
 * Class filling ModbusBuffer with commands. Should serve single subnet, so
 * allows sending messages with different node addresses.
 *
 * Functions timeouts (for writing on command line as RxWait) is specified in
 * calls to callFunction.
 */
class ILC : public ModbusBuffer {
public:
    void reportServerID(uint8_t address) { callFunction(address, 17, 835); }
    void reportServerStatus(uint8_t address) { callFunction(address, 18, 270); }
    void changeILCMode(uint8_t address, uint16_t mode) { callFunction(address, 65, 335, mode); }
    void setTempILCAddress(uint8_t temporaryAddress) { callFunction(255, 72, 250, temporaryAddress); }
    void resetServer(uint8_t address) { callFunction(address, 107, 86840); }

    /**
     * Process received data. Reads function code, check CRC, check that the
     * function was called in request (using _commanded buffer) and calls
     * method to process data. Repeat until all data are processed.
     *
     * @param response response includes response code (0x9) and start bit (need to >> 1 && 0xFF to get the
     * Modbus data)
     * @param length data length
     *
     * @throw std::runtime_error subclass on any detected error.
     */
    void processResponse(uint16_t* response, size_t length);
};

}  // namespace cRIO
}  // namespace LSST
