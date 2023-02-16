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

#include <cRIO/ThermalILC.h>

namespace LSST {
namespace cRIO {

ThermalILC::ThermalILC(uint8_t bus) : ILC(bus) {
    auto thermalStatus = [this](uint8_t address) {
        uint8_t status = read<uint8_t>();
        float differentialTemperature = read<float>();
        uint8_t fanRPM = read<uint8_t>();
        float absoluteTemperature = read<float>();
        checkCRC();
        processThermalStatus(address, status, differentialTemperature, fanRPM, absoluteTemperature);
    };

    auto reheaterGains = [this](uint8_t address) {
        float proportionalGain = read<float>();
        float integralGain = read<float>();
        checkCRC();
        processReHeaterGains(address, proportionalGain, integralGain);
    };

    addResponse(88, thermalStatus, 216);

    addResponse(89, thermalStatus, 217);

    addResponse(
            92, [this](uint8_t address) { checkCRC(); }, 220);

    addResponse(93, reheaterGains, 221);
}

void ThermalILC::broadcastThermalDemand(uint8_t heaterPWM[NUM_TS_ILC], uint8_t fanRPM[NUM_TS_ILC]) {
    uint8_t params[NUM_TS_ILC * 2];
    for (int i = 0, o = 0; i < NUM_TS_ILC; i++, o++) {
        params[o] = heaterPWM[i];
        o++;
        params[o] = fanRPM[i];
    }

    broadcastFunction(250, 88, nextBroadcastCounter(), 450, params, NUM_TS_ILC * 2);
}

}  // namespace cRIO
}  // namespace LSST
