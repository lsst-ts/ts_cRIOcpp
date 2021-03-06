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

#ifndef _cRIO_ILC_
#define _cRIO_ILC_

#include <cRIO/ModbusBuffer.h>

#include <functional>
#include <map>

namespace LSST {
namespace cRIO {

/**
 * Class filling ModbusBuffer with commands. Should serve single subnet, so
 * allows sending messages with different node addresses. Subnet=network=bus is
 * provided in constructor and accessible via getBus() method.
 *
 * Functions timeouts (for writing on command line as RxWait) is specified in
 * calls to callFunction.
 *
 * Replies received from ILCs shall be processed with ILC::processResponse()
 * method. Virtual processXX methods are called to process received data.
 * Relations between functions and processing methods are established with
 * ILC::addResponse().
 *
 * When processing data, the class guarantee that every non-broadcast call
 * generates reply at correct order - see ILC::processResponse() for details.
 */
class ILC : public ModbusBuffer {
public:
    /**
     * Populate responses for know ILC functions.
     *
     * @param bus ILC bus number (1..). Defaults to 1.
     */
    ILC(uint8_t bus = 1);

    /**
     * Returns bus number. 1 based (1-5). 1=A,2=B,..5=E bus.
     */
    uint8_t getBus() { return _bus; }

    /**
     * Set whenever all received data will trigger callback calls.
     */
    void setAlwaysTrigger(bool newAlwaysTrigger) { _alwaysTrigger = newAlwaysTrigger; }

    /**
     * Calls function 17 (0x11), ask for ILC identity.
     *
     * @param address ILC address
     */
    void reportServerID(uint8_t address) { callFunction(address, 17, 835); }

    /**
     * Calls function 18 (0x12), ask for ILC status.
     *
     * @param address ILC address
     */
    void reportServerStatus(uint8_t address) { callFunction(address, 18, 270); }

    /**
     * Change ILC mode. Calls function 65 (0x41). Supported ILC modes are:
     *
     * Mode | Supported by | Description
     * ---- | ------------ | ------------------------------------
     * 0    | all ILCs     | Standby (No Motions or Acquisitions)
     * 1.   | no HM        | Disabled mode (Acquire only)
     * 2.   | all ILCs     | Enabled mode (Acquire and Motion)
     * 3.   | all ILCs     | Firmware update
     * 4.   | all ILCs     | Fault
     *
     * where all is Electromechanical (Hard-Point), Pneumatic, Thermal and
     * Hadpoint Monitoring ILC. HM is Hardpoint Monitoring.
     *
     * @param address ILC address
     * @param mode new ILC mode - see above
     */
    void changeILCMode(uint8_t address, uint16_t mode) { callFunction(address, 65, 335, mode); }

    /**
     * Set temporary ILC address. ILC must be address-less (attached to address
     * 255). Can be used only if one ILC on a bus failed to read its address
     * from its network connection and therefore adopts the failure address
     * 255.
     *
     * @param temporaryAddress new ILC address
     */
    void setTempILCAddress(uint8_t temporaryAddress) { callFunction(255, 72, 250, temporaryAddress); }

    /**
     * Reset ILC. Calls function 107 (0x6b).
     *
     * @param address ILC address
     */
    void resetServer(uint8_t address) { callFunction(address, 107, 86840); }

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
     * specified, raises ILC::Exception. Th action receives two parameters,
     * address and error code. CRC checking is done in processResponse. This
     * method shall not manipulate the buffer (e.g. shall not call
     * ModbusBuffer::read or ModbusBuffer::checkCRC).
     *
     * @see checkCached
     */
    void addResponse(uint8_t func, std::function<void(uint8_t)> action, uint8_t errorResponse,
                     std::function<void(uint8_t, uint8_t)> errorAction = nullptr);

    /**
     * Process received data. Reads function code, check CRC, check that the
     * function was called in request (using _commanded buffer) and calls
     * method to process data. Repeat until all data are processed.
     *
     * @note Can be called multiple times. Please call ModbusBuffer::checkCommandedEmpty
     * after all data are processed.
     *
     * @param response response includes response code (0x9) and start bit (need to >> 1 && 0xFF to get the
     * Modbus data)
     * @param length data length
     *
     * @throw std::runtime_error subclass on any detected error
     *
     * @see ModbusBuffer::checkCommandedEmpty()
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
         * @param func ILC function. Response for this function is unknown at the moment.
         */
        UnknownResponse(uint8_t address, uint8_t func);
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
         * @param func ILC (error) function received
         * @param exception exception code
         */
        Exception(uint8_t address, uint8_t func, uint8_t exception);
    };

protected:
    /**
     * Called before responses are processed (before processXX methods are
     * called).
     */
    virtual void preProcess(){};

    /**
     * Called after responses are processed (after processXX methods are
     * called).
     */
    virtual void postProcess(){};

    /**
     * Callback for reponse to ServerID request. See LTS-646 Code 17 (0x11) for
     * details.
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

    /**
     * Callback for server status reply.
     *
     * @param address ILC address
     * @param mode ILC mode
     * @param status ILC status
     * @param faults ILC faults
     */
    virtual void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) = 0;

    /**
     * Callback for change ILC mode reply.
     *
     * @param address ILC address
     * @param mode new (current) ILC mode
     */
    virtual void processChangeILCMode(uint8_t address, uint16_t mode) = 0;

    /**
     * Callback for temporary address assignment (function code 72).
     *
     * @param address
     * @param newAddress
     */
    virtual void processSetTempILCAddress(uint8_t address, uint8_t newAddress) = 0;

    /**
     * Callback for reply from server reset.
     *
     * @param address ILC address
     */
    virtual void processResetServer(uint8_t address) = 0;

    /**
     * Return counter for broadcast commands.
     *
     * @return broadcast counter (shall be in range 0-15)
     */
    uint8_t getBroadcastCounter() { return _broadcastCounter; }

    /**
     * Return next broadcast counter.
     */
    uint8_t nextBroadcastCounter();

    /**
     * Cache management function. Search for cached response. If none is
     * found, create entry in cache. Use cache entry for
     * ModbusBuffer::checkRecording call.
     *
     * Example code in test_ILC.cpp hopefully explain how to use the function.
     *
     * @param address called ILC address
     * @param func called ILC function code
     *
     * @return true if cached response matches last parsed response
     *
     * @see ModbusBuffer::recordChanges
     * @see ModbusBuffer::pauseRecordChanges
     * @see ModbusBuffer::checkRecording
     */
    bool responseMatchCached(uint8_t address, uint8_t func);

private:
    uint8_t _bus;

    std::map<uint8_t, std::function<void(uint8_t)>> _actions;
    std::map<uint8_t, std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>> _errorActions;

    uint8_t _broadcastCounter;
    unsigned int _timestampShift;

    bool _alwaysTrigger;
    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t>>> _cachedResponse;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !_cRIO_ILC_
