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

ILC::ILC(uint8_t bus) {
    _data_prefix = FIFO::TX_MASK;
    _bus = bus;
    _broadcastCounter = 0;
    _alwaysTrigger = false;

    addResponse(
            17,
            [this](uint8_t address) {
                recordChanges();
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
                if (responseMatchCached(address, 17) == false) {
                    processServerID(address, uniqueID, ilcAppType, networkNodeType, ilcSelectedOptions,
                                    networkNodeOptions, majorRev, minorRev, fwName);
                }
            },
            145);

    addResponse(
            18,
            [this](uint8_t address) {
                recordChanges();
                uint8_t mode = read<uint8_t>();
                uint16_t status = read<uint16_t>();
                uint16_t faults = read<uint16_t>();
                checkCRC();
                if (responseMatchCached(address, 18) == false) {
                    processServerStatus(address, mode, status, faults);
                }
            },
            146);

    addResponse(
            65,
            [this](uint8_t address) {
                recordChanges();
                uint16_t mode = read<uint16_t>();
                checkCRC();
                if (responseMatchCached(address, 65) == false) {
                    processChangeILCMode(address, mode);
                }
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

void ILC::simulateResponse(bool simulate) {
    if (simulate) {
        _data_prefix = FIFO::RX_MASK;
    } else {
        _data_prefix = FIFO::TX_MASK;
    }
}

void ILC::addResponse(uint8_t func, std::function<void(uint8_t)> action, uint8_t errorResponse,
                      std::function<void(uint8_t, uint8_t)> errorAction) {
    _actions[func] = action;
    _errorActions[errorResponse] =
            std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>(func, errorAction);
}

void ILC::processResponse(uint16_t *response, size_t length) {
    preProcess();

    setBuffer(response, length);

    while (endOfBuffer() == false) {
        uint8_t address = read<uint8_t>();
        uint8_t func = read<uint8_t>();

        // either function response was received, or error response. For error
        // response, check if the function for which it is used was called.
        if (_errorActions.find(func) == _errorActions.end()) {
            checkCommanded(address, func);
        } else {
            checkCommanded(address, _errorActions[func].first);
        }

        try {
            _actions.at(func)(address);
        } catch (std::out_of_range &_ex) {
            try {
                auto errorAction = _errorActions.at(func);
                uint8_t exception = read<uint8_t>();
                checkCRC();
                if (errorAction.second) {
                    errorAction.second(address, exception);
                } else {
                    throw Exception(address, func, exception);
                }
            } catch (std::out_of_range &_ex2) {
                throw UnknownResponse(address, func);
            }
        }
    }

    postProcess();
}

ILC::UnknownResponse::UnknownResponse(uint8_t address, uint8_t func)
        : std::runtime_error(fmt::format("Unknown function {1} (0x{1:02x}) in ILC response for address {0}",
                                         address, func)) {}

ILC::Exception::Exception(uint8_t address, uint8_t func, uint8_t exception)
        : std::runtime_error(
                  fmt::format("ILC Exception {2} (ILC address {0}, ILC response function {1} (0x{1:02x}))",
                              address, func, exception)) {}

uint8_t ILC::nextBroadcastCounter() {
    _broadcastCounter++;
    if (_broadcastCounter > 15) {
        _broadcastCounter = 0;
    }
    return _broadcastCounter;
}

uint16_t ILC::getByteInstruction(uint8_t data) {
    processDataCRC(data);
    return _data_prefix | ((static_cast<uint16_t>(data)) << 1);
}

uint8_t ILC::readInstructionByte() {
    if (endOfBuffer()) {
        throw EndOfBuffer();
    }
    return (uint8_t)((_buffer[_index++] >> 1) & 0xFF);
}

bool ILC::responseMatchCached(uint8_t address, uint8_t func) {
    try {
        std::map<uint8_t, std::vector<uint8_t>> &fc = _cachedResponse.at(address);
        try {
            return checkRecording(fc[func]) && !_alwaysTrigger;
        } catch (std::out_of_range &ex1) {
            _cachedResponse[address].emplace(func, std::vector<uint8_t>());
        }
    } catch (std::out_of_range &ex2) {
        _cachedResponse.emplace(std::make_pair(
                address,
                std::map<uint8_t, std::vector<uint8_t>>({std::make_pair(func, std::vector<uint8_t>())})));
    }
    return checkRecording(_cachedResponse[address][func]) && !_alwaysTrigger;
}

}  // namespace cRIO
}  // namespace LSST
