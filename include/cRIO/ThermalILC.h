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

#include <cRIO/ILC.h>

namespace LSST {
namespace cRIO {

/**
 * Class for communication with Thermal ILCs.
 *
 * Replies received from ILCs shall be processed with ILC::processResponse method.
 */
class ThermalILC : public ILC {
public:
    /**
     * Populate responses for know ILC functions.
     */
    ThermalILC();

    void setThermalDemand(uint8_t address, uint8_t heaterPWM, uint8_t fanRPM) { callFunction(address, 88, 500, heaterPWM, fanRPM); }
    void reportThermalStatus(uint8_t address) { callFunction(address, 89, 300); }

protected:
    virtual void processThermalStatus(uint8_t address, uint8_t status, float differentialTemperature, uint8_t fanRPM, float absoluteTemperature) = 0;
};

}  // namespace cRIO
}  // namespace LSST
