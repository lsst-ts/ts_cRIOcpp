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

namespace LSST {
namespace cRIO {

ElectromechanicalPneumaticILC::ElectromechanicalPneumaticILC(uint8_t bus) : ILC(bus) {
    auto hardpointForceStatus = [this](uint8_t address) {
        uint8_t status = read<uint8_t>();
        int32_t encoderPosition = read<int32_t>();
        float loadCellForce = read<float>();
        checkCRC();
        processHardpointForceStatus(address, status, encoderPosition, loadCellForce);
    };

    auto forceActuatorForceStatus = [this](uint8_t address) {
        uint8_t status = read<uint8_t>();
        float primary = read<float>();
        switch (getLength()) {
            case 9: {
                checkCRC();
                processSAAForceStatus(address, status, primary);
                break;
            }
            case 13: {
                float secondary = read<float>();
                checkCRC();
                processDAAForceStatus(address, status, primary, secondary);
                break;
            }
            default:
                throw std::runtime_error(
                        fmt::format("Invalid reply length - {}, expected 9 or 13", getLength()));
        }
    };

    auto calibrationData = [this](uint8_t address) {
        float mainADCK[4], mainOffset[4], mainSensitivity[4], backupADCK[4], backupOffset[4],
                backupSensitivity[4];
        auto read4 = [this](float a[4]) {
            for (int n = 0; n < 4; n++) {
                a[n] = read<float>();
            }
        };
        read4(mainADCK);
        read4(mainOffset);
        read4(mainSensitivity);
        read4(backupADCK);
        read4(backupOffset);
        read4(backupSensitivity);
        checkCRC();
        processCalibrationData(address, mainADCK, mainOffset, mainSensitivity, backupADCK, backupOffset,
                               backupSensitivity);
    };

    auto pressureData = [this](uint8_t address) {
        float primaryPush, primaryPull, secondaryPush, secondaryPull;
        primaryPush = read<float>();
        primaryPull = read<float>();
        secondaryPull = read<float>();
        secondaryPush = read<float>();
        checkCRC();
        processMezzaninePressure(address, primaryPush, primaryPull, secondaryPush, secondaryPull);
    };

    addResponse(67, hardpointForceStatus, 200);

    addResponse(75, forceActuatorForceStatus, 210);

    addResponse(76, forceActuatorForceStatus, 220);

    addResponse(
            81, [this](uint8_t address) { checkCRC(); }, 235);

    addResponse(110, calibrationData, 238);

    addResponse(119, pressureData, 247);
}

}  // namespace cRIO
}  // namespace LSST
