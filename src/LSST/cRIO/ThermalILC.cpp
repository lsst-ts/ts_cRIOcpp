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

using namespace LSST::cRIO;

ThermalILC::ThermalILC(uint8_t bus) : ILC::ILCBusList(bus) {
    auto thermalStatus = [this](Modbus::Parser parser) {
        uint8_t status = parser.read<uint8_t>();
        float differentialTemperature = parser.read<float>();
        uint8_t fanRPM = parser.read<uint8_t>();
        float absoluteTemperature = parser.read<float>();
        parser.checkCRC();
        processThermalStatus(parser.address(), status, differentialTemperature, fanRPM, absoluteTemperature);
    };

    auto reheaterGains = [this](Modbus::Parser parser) {
        float proportionalGain = parser.read<float>();
        float integralGain = parser.read<float>();
        parser.checkCRC();
        processReHeaterGains(parser.address(), proportionalGain, integralGain);
    };

    addResponse(ILC_THERMAL_CMD::SET_THERMAL_DEMAND, thermalStatus, 216);

    addResponse(ILC_THERMAL_CMD::REPORT_THERMAL_STATUS, thermalStatus, 217);

    addResponse(
            ILC_THERMAL_CMD::SET_REHEATER_GAINS, [this](Modbus::Parser parser) { parser.checkCRC(); }, 220);

    addResponse(ILC_THERMAL_CMD::REPORT_REHEATER_GAINS, reheaterGains, 221);
}

std::vector<const char*> ThermalILC::getStatusString(uint16_t status) {
    std::vector<const char*> ret = ILC::ILCBusList::getStatusString(status);

    // 0x0010 NA
    // 0x0020 NA
    if (status & ThermalILCStatus::RefResistor) {
        ret.push_back("Ref Resistor Error");
    }
    if (status & ThermalILCStatus::RTDError) {
        ret.push_back("RTD Error");
    }
    // 0x0100 NA
    // 0x0200 NA
    if (status & ThermalILCStatus::HeaterBreaker) {
        ret.push_back("Heater Breaker Failed");
    }
    if (status & ThermalILCStatus::FanBreaker) {
        ret.push_back("Fan Breaker Failed");
    }
    // 0x1000 NA
    // 0x2000 NA
    // 0x4000 NA
    // 0x8000 reserved

    return ret;
}

std::vector<const char*> ThermalILC::getThermalStatusString(uint8_t status) {
    std::vector<const char*> ret;

    if (status & ThermalStatus::ILCFault) {
        ret.push_back("ILC Fault");
    }
    if (status & ThermalStatus::HeaterDisabled) {
        ret.push_back("Heater Disabled");
    }
    if (status & ThermalStatus::HeaterBreakerOpen) {
        ret.push_back("Heater Breaker Open");
    }
    if (status & ThermalStatus::FanBreakerOpen) {
        ret.push_back("Fan Breaker Open");
    }

    return ret;
}

void ThermalILC::broadcastThermalDemand(uint8_t heaterPWM[NUM_TS_ILC], uint8_t fanRPM[NUM_TS_ILC]) {
    std::vector<uint8_t> params(NUM_TS_ILC * 2);
    for (int i = 0, o = 0; i < NUM_TS_ILC; i++, o++) {
        params[o] = heaterPWM[i];
        o++;
        params[o] = fanRPM[i];
    }

    broadcastFunction(250, 88, 450, nextBroadcastCounter(), params);
}
