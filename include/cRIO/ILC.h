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

#include <map>

#include <cRIO/ModbusBuffer.h>
#include <cRIO/IntelHex.h>

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
 * Provides function to write and read cRIO FIFO (FPGA) Modbus buffers. Modbus
 * serial bus is serviced inside FPGA with
 * [Common_FPGA_Modbus](https://github.com/lsst-ts/Common_FPGA_Modbus) module.
 *
 * 8bit data are stored as 16bit values. Real data are left shifted by 1. Last
 * bit (0, transmitted first) is start bit, always 0 for ILC communication.
 * First data bit (transmitted last) is stop bit, shall be 1. So uint8_t data d
 * needs to be written as:
 *
 * (0x1200 | (d << 1))
 * (TX_MASK | (d << 1))
 *
 * and response from FPGA ResponseFIFOs is coming with 0x9200 prefix, so:
 *
 * (0x9200 | (d << 1))
 * (RX_MASK | (d << 1))
 *
 * Please see ModbusBuffer::simulateReponse() for details of how to change
 * prefix.
 */
class ILC : public ModbusBuffer {
public:
    /**
     * Populate responses for know ILC functions.
     *
     * @param bus ILC bus number (1..). Defaults to 1.
     */
    ILC(uint8_t bus = 1);

    void writeEndOfFrame() override;

    void writeWaitForRx(uint32_t timeoutMicros) override;

    void writeRxEndFrame() override;

    void readEndOfFrame() override;

    /**
     * Returns wait for receive timeout.
     *
     * @return timeout in us (microseconds)
     *
     * @throw std::runtime_error if wait for rx delay command isn't present
     */
    uint32_t readWaitForRx();

    /**
     * Sets simulate mode.
     *
     * @param simulate true if buffer shall product simulated replies
     */
    void simulateResponse(bool simulate);

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

    enum ILCMode { Standby = 0, Disabled = 1, Enabled = 2, FirmwareUpdate = 3, Fault = 4, ClearFaults = 5 };

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
     * 5.   | all ILCs     | Clear faults
     *
     * where all is Electromechanical (Hard-Point), Pneumatic, Thermal and
     * Hardpoint Monitoring ILC. HM is Hardpoint Monitoring.
     *
     * @param address ILC address
     * @param mode new ILC mode - see above
     */
    void changeILCMode(uint8_t address, uint16_t mode);

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

protected:
    uint16_t getByteInstruction(uint8_t data) override;

    uint8_t readInstructionByte() override;

    uint8_t getLastMode(uint8_t address) { return _lastMode.at(address); }

    const char *getModeStr(uint8_t mode);

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

    uint8_t _broadcastCounter;
    unsigned int _timestampShift;

    bool _alwaysTrigger;
    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t>>> _cachedResponse;

    // last know ILC mode
    std::map<uint8_t, uint8_t> _lastMode;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !_cRIO_ILC_
