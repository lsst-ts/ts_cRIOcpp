/*
 * Modbus Processing Unit class.
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

#include <cRIO/MPU.h>
#include <spdlog/spdlog.h>

using namespace LSST::cRIO;

MPU::MPU(uint8_t mpu_address) : _mpu_address(mpu_address) {
    addResponse(
            3,
            [this](uint8_t address) {
                if (address != _mpu_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid ModBus address {}, expected {}", address, _mpu_address));
                }
                uint8_t len = read<uint8_t>();
                for (size_t i = 0; i < len; i += 2) {
                    if (_readRegisters.empty()) {
                        throw std::runtime_error("Too big response");
                    }
                    _registers[_readRegisters.front()] = read<uint16_t>();
                    _readRegisters.pop_front();
                }
                checkCRC();
            },
            0);

    addResponse(
            16,
            [this](uint8_t address) {
                if (address != _mpu_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid ModBus address {}, expected {}", address, _mpu_address));
                }
                uint16_t reg = read<uint16_t>();
                uint16_t len = read<uint16_t>();
                auto preset = _presetRegisters.front();
                _presetRegisters.pop_front();
                if (reg != preset.first) {
                    throw std::runtime_error(
                            fmt::format("Invalid register {:04x}, expected {:04x}", reg, preset.first));
                }
                if (len != preset.second) {
                    throw std::runtime_error(
                            fmt::format("Invalid lenght {}, expected {}", len, preset.second));
                }
                checkCRC();
            },
            0);
}

void MPU::readHoldingRegisters(uint16_t address, uint16_t count, uint8_t timeout) {
    callFunction(_mpu_address, 3, 0, address, count);

    // write request
    _commands.push_back(MPUCommands::WRITE);
    _commands.push_back(getLength());
    for (auto b : getBufferVector()) {
        _commands.push_back(b);
    }

    _commands.push_back(MPUCommands::WAIT_MS);
    _commands.push_back(timeout);

    // read response
    _commands.push_back(MPUCommands::READ);
    // extras: device address, function, length (all 1 byte), CRC (2 bytes) = 5 total
    _commands.push_back(5 + count * 2);
    _commands.push_back(MPUCommands::CHECK_CRC);

    for (uint16_t add = address; add < address + count; add++) {
        _readRegisters.push_back(add);
    }
}

void MPU::presetHoldingRegisters(uint16_t address, uint16_t *values, uint8_t count, uint16_t timeout) {
    write(_mpu_address);
    write<uint8_t>(16);
    write(address);
    write<uint16_t>(count);
    write<uint8_t>(count * 2);

    for (uint8_t i = 0; i < count; i++) {
        write<uint16_t>(values[i]);
    }

    writeCRC();
    writeEndOfFrame();
    writeWaitForRx(0);

    pushCommanded(_mpu_address, 16);

    // write request
    _commands.push_back(MPUCommands::WRITE);
    _commands.push_back(getLength());
    for (auto b : getBufferVector()) {
        _commands.push_back(b);
    }

    _commands.push_back(MPUCommands::WAIT_MS);
    _commands.push_back(timeout);

    // read response
    _commands.push_back(MPUCommands::READ);
    // extras: device address, function (all 1 byte), address, number of registers written, CRC (2 bytes)
    _commands.push_back(8);
    _commands.push_back(MPUCommands::CHECK_CRC);

    _presetRegisters.push_back(std::pair<uint16_t, uint8_t>(address, count));
}
