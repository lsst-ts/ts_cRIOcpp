/*
 * Sensors handling. Adds handling of the function 84 (0x54), readout of the
 * sensors values.
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

#ifndef __ILC_SensorMonitor__
#define __ILC_SensorMonitor__

#include <ILC/ILCBusList.h>

namespace ILC {

/**
 * Class handling sensor monitoring. Intended for ILCs running application
 * fimrware of type 4, 5 and 6 - Temperature, Displacement and Inclinometer
 * monitors.
 */
class SensorMonitor : public virtual ILCBusList {
public:
    SensorMonitor(uint8_t bus);

    /**
     * Calls function 84 (0x54), requestion measured sensor values.
     *
     * @param address @glos{ILC} address
     */
    void reportSensorValues(uint8_t address) { callFunction(address, SENSOR_VALUES, 400); }

    enum SENSOR_MONITOR_CMD { SENSOR_VALUES = 84 };

protected:
    /***
     * Process response containing sensor values.
     *
     * @param address @glos{ILC} address
     * @param values retrieved values
     */
    virtual void processSensorValues(uint8_t address, std::vector<float> values) = 0;
};

}  // namespace ILC

#endif  //* !__ILC_SensorMonitor__
