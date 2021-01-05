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

#include <cRIO/ILC.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace LSST {
namespace cRIO {

ILC::ILC() {
    addResponse(
            17,
            [this](uint8_t address) {
                uint8_t fnLen = read<uint8_t>();
                if (fnLen < 12) {
                    throw std::runtime_error(fmt::format(
                            "invalid ILC function 17 response length - except at least 12, got {}", fnLen));
                }
                fnLen -= 12;

                uint64_t uniqueID = readU48();
                uint8_t ilcAppType = read<uint8_t>();
                uint8_t networkNodeType = read<uint8_t>();
                uint8_t ilcSelectedOptions = read<uint8_t>();
                uint8_t networkNodeOptions = read<uint8_t>();
                uint8_t majorRev = read<uint8_t>();
                uint8_t minorRev = read<uint8_t>();
                std::string fwName = readString(fnLen);
                checkCRC();
                processServerID(address, uniqueID, ilcAppType, networkNodeType, ilcSelectedOptions,
                                networkNodeOptions, majorRev, minorRev, fwName);
            },
            145);

    addResponse(
            18,
            [this](uint8_t address) {
                uint8_t mode = read<uint8_t>();
                uint16_t status = read<uint16_t>();
                uint16_t faults = read<uint16_t>();
                checkCRC();
                processServerStatus(address, mode, status, faults);
            },
            146);

    addResponse(
            65,
            [this](uint8_t address) {
                uint16_t mode = read<uint16_t>();
                checkCRC();
                processChangeILCMode(address, mode);
            },
            193);

    addResponse(
            72,
            [this](uint8_t address) {
                uint8_t newAddress = read<uint8_t>();
                checkCRC();
                processSetTempILCAddress(address, newAddress);
            },
            200);

    addResponse(
            107,
            [this](uint8_t address) {
                checkCRC();
                processResetServer(address);
            },
            235);
}

void ILC::addResponse(uint8_t function, std::function<void(uint8_t)> action, uint8_t errorResponse,
                      std::function<void(uint8_t, uint8_t)> errorAction) {
    _actions[function] = action;
    _errorActions[errorResponse] =
            std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>(function, errorAction);
}

void ILC::processResponse(uint16_t *response, size_t length) {
    setBuffer(response, length);

    while (endOfBuffer() == false) {
        uint8_t address = read<uint8_t>();
        uint8_t function = read<uint8_t>();
        checkCommanded(address, function);

        try {
            _actions.at(function)(address);
        } catch (std::out_of_range &_ex) {
            try {
                auto errorAction = _errorActions.at(function);
                uint8_t exception = read<uint8_t>();
                checkCRC();
                if (errorAction.second) {
                    errorAction.second(address, exception);
                } else {
                    throw Exception(address, function, exception);
                }
            } catch (std::out_of_range &_ex2) {
                throw UnknownResponse(address, function);
            }
        }
    }

    checkCommanded(0, 0);
}

ILC::UnknownResponse::UnknownResponse(uint8_t address, uint8_t function)
        : std::runtime_error(fmt::format("Unknown function {1} (0x{1:02x}) in ILC response for address {0}",
                                         address, function)) {}

ILC::Exception::Exception(uint8_t address, uint8_t function, uint8_t exception)
        : std::runtime_error(
                  fmt::format("ILC Exception {2} (ILC address {0}, ILC response function {1} (0x{1:02x}))",
                              address, function, exception)) {}

}  // namespace cRIO
}  // namespace LSST
