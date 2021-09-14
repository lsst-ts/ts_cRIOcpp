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

void ILC::writeEndOfFrame() { pushBuffer(FIFO::TX_FRAMEEND); }

void ILC::writeWaitForRx(uint32_t timeoutMicros) {
    pushBuffer(timeoutMicros > 0x0FFF ? ((0x0FFF & ((timeoutMicros / 1000) + 1)) | FIFO::TX_WAIT_LONG_RX)
                                      : (timeoutMicros | FIFO::TX_WAIT_RX));
}

void ILC::writeRxEndFrame() { pushBuffer(FIFO::RX_ENDFRAME); }

void ILC::writeFPGATimestamp(uint64_t timestamp) {
    for (int i = 0; i < 4; i++) {
        pushBuffer(timestamp & 0xFFFF);
        timestamp >>= 16;
    }
}

void ILC::writeRxTimestamp(uint64_t timestamp) {
    for (int i = 0; i < 8; i++) {
        pushBuffer(FIFO::RX_TIMESTAMP | (timestamp & 0xFF));
        timestamp >>= 8;
    }
}

void ILC::readEndOfFrame() {
    if (getCurrentBuffer() != FIFO::TX_FRAMEEND) {
        throw std::runtime_error(fmt::format("Expected end of frame, finds {:04x} (@ offset {})",
                                             getCurrentBuffer(), getCurrentIndex()));
    }
    incIndex();
    resetCRC();
}

uint32_t ILC::readWaitForRx() {
    uint16_t c = getCurrentBuffer() & 0xF000;
    uint32_t ret = 0;
    switch (c) {
        case FIFO::TX_WAIT_RX:
            ret = 0x0FFF & getCurrentBuffer();
            break;
        case FIFO::TX_WAIT_LONG_RX:
            ret = (0x0FFF & getCurrentBuffer()) * 1000;
            break;
        default:
            throw std::runtime_error(fmt::format("Expected wait for RX, finds {:04x} (@ offset {})",
                                                 getCurrentBuffer(), getCurrentIndex()));
    }
    incIndex();
    return ret;
}

void ILC::simulateResponse(bool simulate) {
    if (simulate) {
        _data_prefix = FIFO::RX_MASK;
    } else {
        _data_prefix = FIFO::TX_MASK;
    }
}

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
    return (uint8_t)((getCurrentBufferAndInc() >> 1) & 0xFF);
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
