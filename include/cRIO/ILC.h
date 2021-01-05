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

#include <functional>
#include <map>

namespace LSST {
namespace cRIO {

/**
 * Class filling ModbusBuffer with commands. Should serve single subnet, so
 * allows sending messages with different node addresses.
 *
 * Functions timeouts (for writing on command line as RxWait) is specified in
 * calls to callFunction.
 */
class ILC : public ModbusBuffer {
public:
    /**
     * Populate responses for know ILC functions.
     */
    ILC();

    void reportServerID(uint8_t address) { callFunction(address, 17, 835); }
    void reportServerStatus(uint8_t address) { callFunction(address, 18, 270); }
    void changeILCMode(uint8_t address, uint16_t mode) { callFunction(address, 65, 335, mode); }
    void setTempILCAddress(uint8_t temporaryAddress) { callFunction(255, 72, 250, temporaryAddress); }
    void resetServer(uint8_t address) { callFunction(address, 107, 86840); }

    void addResponse(uint8_t function, std::function<void(uint8_t)> action, uint8_t errorResponse,
                     std::function<void(uint8_t, uint8_t)> errorAction = nullptr);

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

    /**
     * Thrown when an unknown response function is received. As unknown function
     * response means unknown message length and hence unknown CRC position and
     * start of a new frame, the preferred handling when such error is seen is to
     * flush response buffer and send ILC's queries again.
     */
    class UnknownResponse : public std::runtime_error {
    public:
        /**
         * Constructed with data available during response.
         *
         * @param address ILC address
         * @param function ILC function. Response for this function is unknown at the moment.
         */
        UnknownResponse(uint8_t address, uint8_t function);
    };

    /**
     * Thrown when ILC error response is received.
     */
    class Exception : public std::runtime_error {
    public:
        /**
         * The class is constructed when an ILC's error response is received.
         *
         * @param address ILC address
         * @param function ILC (error) function received
         * @param exception exception code
         */
        Exception(uint8_t address, uint8_t function, uint8_t exception);
    };

protected:
    /**
     * Callback when reponse to ServerID request is received. See LTS-646 Code
     * 17 (0x11) for details.
     *
     * ### Types
     *
     * Value | Description
     * ----- | -----------
     * 1     | Electromechanical Actuator
     * 2     | Pneumatic Actuator
     * 3     | Thermal
     * 7     | Hardpoint Monitoring
     * 10    | Bootloader
     *
     * ### Selected options
     *
     * <table>
     *   <tr>
     *     <th>ILC type</th>
     *     <th>Value</th>
     *     <th>Description</th>
     *   </tr>
     *   <tr>
     *     <td rowspan=2><b>Electromechanical Actuator ILC</b></td>
     *     <td>0x00</td>
     *     <td>Gray Code SSI</td>
     *   </tr>
     *   <tr>
     *     <td>0x01</td>
     *     <td>Binary SSI</td>
     *   </tr>
     *   <tr>
     *     <td rowspan=2><b>Electromechanical Actuator ILC</b></td>
     *     <td>0x00</td>
     *     <td>Gray Code SSI Encoder</td>
     *   </tr>
     *   <tr>
     *     <td>0x01</td>
     *     <td>Binary SSI Encoder</td>
     *   </tr>
     *   <tr>
     *     <td rowspan=2><b>Pneumatic Actuator ILC</b></td>
     *     <td>0x00</td>
     *     <td>Single Load Cell/Axis</td>
     *   </tr>
     *   <tr>
     *     <td>0x02</td>
     *     <td>Two Load Cells/Axes</td>
     *   </tr>
     *   <tr>
     *     <td><b>Thermal ILC</b></td>
     *     <td colspan=2>None</td>
     *   </tr>
     *   <tr>
     *     <td><b>Hardpoint Monitoring ILC</b></td>
     *     <td colspan=2>None</td>
     *   </tr>
     *   <tr>
     *     <td><b>Bootloader</b></td>
     *     <td colspan=2>None</td>
     *   </tr>
     * </table>
     *
     * @param address  ILC address
     * @param uniqueID  ILC unigue ID
     * @param ilcAppType ILC App (Firmware) Type. See Types table for values
     * @param networkNodeType ILC Network Node (TEDS) Type. See Types table for values
     * @param ilcSelectedOptions ILC Selected Options. See Selected Options table for values
     * @param networkNodeOptions ILC Network Node (TEDS) Options. See Selected Options table for values
     * @param majorRev Firmware major revision number
     * @param minorRev Firmware minor revision number
     * @param firmwareName ASCII name string for ILC firmware
     */
    virtual void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType,
                                 uint8_t networkNodeType, uint8_t ilcSelectedOptions,
                                 uint8_t networkNodeOptions, uint8_t majorRev, uint8_t minorRev,
                                 std::string firmwareName) = 0;

    virtual void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) = 0;

private:
    std::map<uint8_t, std::function<void(uint8_t)>> _actions;
    std::map<uint8_t, std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>> _errorActions;
};

}  // namespace cRIO
}  // namespace LSST
