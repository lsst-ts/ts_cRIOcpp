/*
 * ILC Bus List handling communication (receiving and
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

#ifndef __ILC_BusList__
#define __ILC_BusList__

#include <string>

#include <Modbus/BusList.h>

namespace ILC {

enum Mode { Standby = 0, Disabled = 1, Enabled = 2, FirmwareUpdate = 3, Fault = 4, ClearFaults = 5 };

/**
 * Handles basic ILC communication. Provides methods to run ILC functions and
 * callback for responses.
 */
class ILCBusList : public Modbus::BusList {
public:
    ILCBusList(uint8_t bus);
    virtual ~ILCBusList();

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
    /**
     * Call broadcast function.
     *
     * @param address broadcast address. Shall be 0, 148, 149 or 250. Not checked if in correct range
     * @param func function to call
     * @param delay delay in us (microseconds) for broadcast processing. Bus will remain silence for this
     * number of us to allow ModBus process the broadcast function
     * @param counter broadcast counter. ModBus provides method to retrieve this
     * in unicast function to verify the broadcast was received and processed
     * @param data function parameters. Usually device's bus ID indexed array
     * of values to pass to the devices
     */
    void broadcastFunction(uint8_t address, uint8_t func, uint32_t delay, uint8_t counter,
                           std::vector<uint8_t> data);

    /**
     * Return next bus broadcast counter.
     *
     * @return next broadcast counter value
     */
    uint8_t nextBroadcastCounter();

    /**
     * Return current broadcast counter value.
     *
     * @return current broadcast counter value
     */
    uint8_t getBroadcastCounter() { return _broadcastCounter; }

    /**
     * Return string with current mode description.
     *
     * @param mode ILC mode, as returned by function 18.
     *
     * @return status description (enabled, standby,..).
     */
    const char *getModeStr(uint8_t mode);

    /**
     * Return string with short text describing all code 18 status response.
     *
     * @param status status returned from function 18 (and other functions).
     *
     * @return vector of strings with status description
     */
    virtual std::vector<const char *> getStatusString(uint16_t status);

    enum ILCStatus { MajorFault = 0x0001, MinorFault = 0x0002, FaultOverride = 0x0008 };

    /**
     * Returns last know mode (state) of the ILC at the address.
     *
     * @param address ILC node address
     *
     * @return last know ILC state
     *
     * @throw std::out_of_range when ILC mode is not known
     */
    uint8_t getLastMode(uint8_t address) { return _lastMode.at(address); }

    /**
     * Return ILC fault textual description.
     *
     * @param fault ILC faults returned by function 18.
     *
     * @return vector of strings with fault description
     */
    virtual std::vector<const char *> getFaultString(uint16_t fault);

    enum ILCFault {
        UniqueIRC = 0x0001,
        AppType = 0x0002,
        NoILC = 0x0004,
        ILCAppCRC = 0x0008,
        NoTEDS = 0x0010,
        TEDS1 = 0x0020,
        TEDS2 = 0x0040,
        WatchdogReset = 0x0100,
        BrownOut = 0x0200,
        EventTrap = 0x0400,
        SSR = 0x1000,
        AUX = 0x2000
    };

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

private:
    // last know ILC mode
    std::map<uint8_t, uint8_t> _lastMode;
    uint8_t _broadcastCounter = 0;
};

}  // namespace ILC

#endif /* !__ILC_BusList__ */
