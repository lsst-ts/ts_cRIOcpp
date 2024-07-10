/*
 * Electromechanical and Pneumatic ILC functions.
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

#include <spdlog/fmt/fmt.h>

#include <cRIO/ElectromechanicalPneumaticILC.h>

using namespace LSST::cRIO;

ElectromechanicalPneumaticILC::ElectromechanicalPneumaticILC(uint8_t bus) : ILC::ILCBusList(bus) {
    auto stepperForceStatus = [this](Modbus::Parser parser) {
        uint8_t status = parser.read<uint8_t>();
        int32_t encoderPosition = parser.read<int32_t>();
        float loadCellForce = parser.read<float>();
        parser.checkCRC();
        processStepperForceStatus(parser.address(), status, encoderPosition, loadCellForce);
    };

    auto dcaGain = [this](Modbus::Parser parser) {
        float primaryGain = parser.read<float>();
        float secondaryGain = parser.read<float>();
        parser.checkCRC();
        processDCAGain(parser.address(), primaryGain, secondaryGain);
    };

    auto hardpointLVDT = [this](Modbus::Parser parser) {
        float breakwayLVDT = parser.read<float>();
        float displacementLVDT = parser.read<float>();
        parser.checkCRC();
        processHardpointLVDT(parser.address(), breakwayLVDT, displacementLVDT);
    };

    auto forceActuatorForceStatus = [this](Modbus::Parser parser) {
        uint8_t status = parser.read<uint8_t>();
        float primary = parser.read<float>();
        switch (parser.size()) {
            case 9: {
                parser.checkCRC();
                processSAAForceStatus(parser.address(), status, primary);
                break;
            }
            case 13: {
                float secondary = parser.read<float>();
                parser.checkCRC();
                processDAAForceStatus(parser.address(), status, primary, secondary);
                break;
            }
            default:
                throw std::runtime_error(
                        fmt::format("Invalid reply length - {}, expected 9 or 13", parser.size()));
        }
    };

    auto calibrationData = [this](Modbus::Parser parser) {
        float mainADCK[4], mainOffset[4], mainSensitivity[4], backupADCK[4], backupOffset[4],
                backupSensitivity[4];
        auto read4 = [this, &parser](float a[4]) {
            for (int n = 0; n < 4; n++) {
                a[n] = parser.read<float>();
            }
        };
        read4(mainADCK);
        read4(mainOffset);
        read4(mainSensitivity);
        read4(backupADCK);
        read4(backupOffset);
        read4(backupSensitivity);
        parser.checkCRC();
        processCalibrationData(parser.address(), mainADCK, mainOffset, mainSensitivity, backupADCK,
                               backupOffset, backupSensitivity);
    };

    auto pressureData = [this](Modbus::Parser parser) {
        float primaryPush, primaryPull, secondaryPush, secondaryPull;
        primaryPush = parser.read<float>();
        primaryPull = parser.read<float>();
        secondaryPull = parser.read<float>();
        secondaryPush = parser.read<float>();
        parser.checkCRC();
        processMezzaninePressure(parser.address(), primaryPush, primaryPull, secondaryPush, secondaryPull);
    };

    addResponse(ILC_EM_CMD::SET_STEPPER_STEPS, stepperForceStatus, 194);

    addResponse(ILC_EM_CMD::STEPPER_FORCE_STATUS, stepperForceStatus, 195);

    addResponse(ILC_EM_CMD::SET_DCA_GAIN, [this](Modbus::Parser parser) { parser.checkCRC(); }, 201);

    addResponse(ILC_EM_CMD::REPORT_DCA_GAIN, dcaGain, 202);

    addResponse(ILC_EM_CMD::SET_FORCE_OFFSET, forceActuatorForceStatus, 210);

    addResponse(ILC_EM_CMD::REPORT_FA_FORCE_STATUS, forceActuatorForceStatus, 220);

    addResponse(
            ILC_EM_CMD::SET_OFFSET_AND_SENSITIVITY, [this](Modbus::Parser parser) { parser.checkCRC(); },
            235);

    addResponse(ILC_EM_CMD::REPORT_CALIBRATION_DATA, calibrationData, 238);

    addResponse(ILC_EM_CMD::REPORT_MEZZANINE_PRESSURE, pressureData, 247);

    addResponse(ILC_EM_CMD::REPORT_HARDPOINT_LVDT, hardpointLVDT, 250);
}
