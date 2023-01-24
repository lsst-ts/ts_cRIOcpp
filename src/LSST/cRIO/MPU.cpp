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

#include <spdlog/spdlog.h>

#include <cRIO/MPU.h>

using namespace LSST::cRIO;

MPU::MPU(uint8_t bus, uint8_t mpu_address) : _bus(bus), _mpu_address(mpu_address), _contains_read(false) {
    addResponse(
            2,
            [this](uint8_t address) {
                if (_readInputStatus.empty()) {
                    throw std::runtime_error("Empty read input status");
                }
                auto req = _readInputStatus.front();
                if (address != _mpu_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid ModBus address {}, expected {}", address, _mpu_address));
                }
                uint8_t len = read<uint8_t>();
                if (len != ceil(req.second / 8.0)) {
                    throw std::runtime_error(fmt::format(
                            "Invalid length for inputs starting at {:04x} - received {}, expected {}",
                            req.first, len, req.second));
                }
                uint16_t start_address = req.first;
                uint16_t end_address = start_address + req.second;
                uint8_t data = 0x00;
                for (uint16_t a = start_address; a < end_address; a++) {
                    if ((a - start_address) % 8 == 0) {
                        data = read<uint8_t>();
                    }
                    _inputStatus[a] = data & 0x01;
                    data >>= 1;
                }
                _readInputStatus.pop_front();
                checkCRC();
            },
            0x82);

    addResponse(
            3,
            [this](uint8_t address) {
                if (address != _mpu_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid ModBus address {}, expected {}", address, _mpu_address));
                }
                uint8_t len = read<uint8_t>();

                {
                    std::lock_guard<std::mutex> lg(_registerMutex);
                    for (size_t i = 0; i < len; i += 2) {
                        if (_readRegisters.empty()) {
                            throw std::runtime_error("Too big response");
                        }
                        uint16_t reg = _readRegisters.front();
                        uint16_t val = read<uint16_t>();
                        _registers[reg] = val;
                        _readRegisters.pop_front();
                    }
                }
                checkCRC();
            },
            0x83);

    addResponse(
            6,
            [this](uint8_t address) {
                if (address != _mpu_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid ModBus address {}, expected {}", address, _mpu_address));
                }
                uint16_t reg = read<uint16_t>();
                uint16_t value = read<uint16_t>();
                auto preset = _presetRegister.front();
                _presetRegister.pop_front();
                if (reg != preset.first) {
                    throw std::runtime_error(
                            fmt::format("Invalid register {:04x}, expected {:04x}", reg, preset.first));
                }
                if (value != preset.second) {
                    throw std::runtime_error(fmt::format("Invalid value {} for address {:04x}, expected {}",
                                                         value, reg, preset.second));
                }
                checkCRC();
            },
            0x86);

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
            0x90);
}

void MPU::clearCommanded() {
    clear();
    _commands.clear();

    _readInputStatus.clear();
    _readRegisters.clear();
    _presetRegister.clear();
    _presetRegisters.clear();
}

void MPU::readInputStatus(uint16_t address, uint16_t count, uint8_t timeout) {
    callFunction(_mpu_address, 2, 0, address, count);

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
    _contains_read = true;
    // extras: device address, function, length (all 1 byte), CRC (2 bytes) = 5 total
    _commands.push_back(5 + ceil(count / 8.0));
    _commands.push_back(MPUCommands::CHECK_CRC);

    _readInputStatus.push_back(std::pair<uint16_t, uint16_t>(address, count));
}

void MPU::readHoldingRegisters(uint16_t address, uint16_t count, uint8_t timeout) {
    // remove EXIT if commands were already pushed to buffer
    if (!_commands.empty()) {
        _commands.pop_back();
    }

    clear(true);
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
    _contains_read = true;
    // extras: device address, function, length (all 1 byte), CRC (2 bytes) = 5 total
    _commands.push_back(5 + count * 2);

    _commands.push_back(MPUCommands::OUTPUT);

    _commands.push_back(MPUCommands::EXIT);

    for (uint16_t add = address; add < address + count; add++) {
        _readRegisters.push_back(add);
    }
}

void MPU::presetHoldingRegister(uint16_t address, uint16_t value, uint8_t timeout) {
    write(_mpu_address);
    write<uint8_t>(6);
    write(address);
    write<uint16_t>(value);

    writeCRC();
    writeEndOfFrame();
    writeWaitForRx(0);

    pushCommanded(_mpu_address, 6);

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
    _contains_read = true;
    // extras: device address, function (all 1 byte), address, number of registers written, CRC (2 bytes)
    _commands.push_back(8);

    _commands.push_back(MPUCommands::OUTPUT);

    _commands.push_back(MPUCommands::CHECK_CRC);
    _commands.push_back(MPUCommands::EXIT);

    _presetRegister.push_back(std::pair<uint16_t, uint16_t>(address, value));
}

void MPU::presetHoldingRegisters(uint16_t address, uint16_t *values, uint8_t count, uint8_t timeout) {
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

    _presetRegisters.push_back(std::pair<uint16_t, uint16_t>(address, count));
}
