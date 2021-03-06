/*
 * Thermal ILC functions.
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

#ifndef _cRIO_ThermalILC_h
#define _cRIO_ThermalILC_h

#include <cRIO/ILC.h>

namespace LSST {
namespace cRIO {

/**
 * Number of Thermal System ILC.
 */
constexpr int NUM_TS_ILC = 96;

/**
 * Class for communication with Thermal ILCs.
 *
 * Replies received from ILCs shall be processed with ILC::processResponse method.
 */
class ThermalILC : public virtual ILC {
public:
    /**
     * Populate responses for known ILC functions.
     *
     * @param bus ILC bus number (1..). Defaults to 1.
     */
    ThermalILC(uint8_t bus = 1);

    /**
     * Unicast heater PWM and fan RPM. ILC command code 88 (0x58)
     *
     * @param address ILC address
     * @param heaterPWM commanded heater PWM values (0-100% = 0-255)
     * @param fanRPM commanded fan RPM values (0-2550)
     */
    void setThermalDemand(uint8_t address, uint8_t heaterPWM, uint8_t fanRPM) {
        callFunction(address, 88, 500, heaterPWM, fanRPM);
    }

    /**
     * Report thermal ILC settings and values. Command code 89 (0x59).
     *
     * @param address ILC address to query.
     */
    void reportThermalStatus(uint8_t address) { callFunction(address, 89, 300); }

    /**
     * Broadcast heater PWM and fan RPM. ILC command code 88 (0x58).
     *
     * @param heaterPWM[NUM_TS_ILC]
     * @param fanRPM[NUM_TS_ILC]
     */
    void broadcastThermalDemand(uint8_t heaterPWM[NUM_TS_ILC], uint8_t fanRPM[NUM_TS_ILC]);

protected:
    /**
     * Called when response from call to command 89 (0x59) is read.
     *
     * @param address status returned from this ILC
     * @param status ILC status. See LTS-646 for details.
     * @param differentialTemperature differential temperature (degC)
     * @param fanRPM measure fan RPM - 0 to 255 = 0 to 2550 RPM in 10 RPM increments
     * @param absoluteTemperature absolute temperature (degC)
     */
    virtual void processThermalStatus(uint8_t address, uint8_t status, float differentialTemperature,
                                      uint8_t fanRPM, float absoluteTemperature) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! _cRIO_ThermalILC_h
