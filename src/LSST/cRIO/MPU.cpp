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

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#include <cRIO/CliApp.h>
#include <cRIO/MPU.h>
#include <Modbus/Parser.h>

using namespace LSST::cRIO;

MPU::MPU(uint8_t node_address) : _bus(0), _node_address(node_address) {
    addResponse(
            READ_INPUT_STATUS,
            [this](Modbus::Parser parser) {
                auto commanded = _commanded_info.front();
                _commanded_info.pop_front();

                if (commanded.address == 0 || commanded.length == 0) {
                    throw std::runtime_error("Empty read input status");
                }
                if (parser.address() != _node_address) {
                    throw std::runtime_error(fmt::format("Invalid ModBus address {}, expected {}",
                                                         parser.address(), _node_address));
                }
                uint8_t len = parser.read<uint8_t>();
                uint8_t data = 0x00;
                if (commanded.length / 8 + (commanded.length % 8 == 0 ? 0 : 1) != len) {
                    throw std::runtime_error(
                            fmt::format("Invalid reply length - received {}, ceiling from {} / 8", len,
                                        commanded.length));
                }
                for (uint16_t i = 0; i < commanded.length; i++) {
                    if (i % 8 == 0) {
                        data = parser.read<uint8_t>();
                    }
                    _inputStatus[commanded.address + i] = data & 0x01;
                    data >>= 1;
                }
                parser.checkCRC();
            },
            0x82);

    addResponse(
            READ_HOLDING_REGISTERS,
            [this](Modbus::Parser parser) {
                auto commanded = _commanded_info.front();
                _commanded_info.pop_front();

                if (parser.address() != _node_address) {
                    throw std::runtime_error(fmt::format("Invalid ModBus address {}, expected {}",
                                                         parser.address(), _node_address));
                }
                uint8_t len = parser.read<uint8_t>() / 2;
                {
                    std::lock_guard<std::mutex> lg(_registerMutex);
                    for (size_t i = 0; i < len; i++) {
                        uint16_t val = parser.read<uint16_t>();
                        _registers[commanded.address + i] = val;
                    }
                }
                parser.checkCRC();
            },
            0x83);

    addResponse(
            PRESET_HOLDING_REGISTER,
            [this](Modbus::Parser parser) {
                auto commanded = _commanded_info.front();
                _commanded_info.pop_front();

                if (parser.address() != _node_address) {
                    throw std::runtime_error(fmt::format("Invalid ModBus address {}, expected {}",
                                                         parser.address(), _node_address));
                }
                uint16_t reg = parser.read<uint16_t>();
                uint16_t value = parser.read<uint16_t>();
                if (reg != commanded.address) {
                    throw std::runtime_error(
                            fmt::format("Invalid register {:04x}, expected {:04x}", reg, commanded.address));
                }
                {
                    std::lock_guard<std::mutex> lg(_registerMutex);
                    _registers[commanded.address] = value;
                }
                parser.checkCRC();
            },
            0x86);

    addResponse(
            PRESET_HOLDING_REGISTERS,
            [this](Modbus::Parser parser) {
                auto commanded = _commanded_info.front();
                _commanded_info.pop_front();

                if (parser.address() != _node_address) {
                    throw std::runtime_error(fmt::format("Invalid ModBus address {}, expected {}",
                                                         parser.address(), _node_address));
                }
                uint16_t reg = parser.read<uint16_t>();
                uint16_t len = parser.read<uint16_t>();
                if (reg != commanded.address) {
                    throw std::runtime_error(
                            fmt::format("Invalid register {:04x}, expected {:04x}", reg, commanded.address));
                }
                if (len != commanded.length) {
                    throw std::runtime_error(
                            fmt::format("Invalid length - register {:04x}, length {}, expected {}", reg, len,
                                        commanded.length));
                }
                parser.checkCRC();
            },
            0x90);
}

int MPU::responseLength(const std::vector<uint8_t> &response) {
    if (response.size() < 2) {
        return -1;
    }

    // Modbus error response. 5 bytes total - address, function, error code, CRC checksum
    switch (response[1]) {
        case READ_INPUT_STATUS:
        case READ_HOLDING_REGISTERS:
            if (response.size() < 3) {
                return -1;
            }
            return 5 + response[2];

        case PRESET_HOLDING_REGISTER:
        case PRESET_HOLDING_REGISTERS:
            return 8;

        default:
            if ((response[1] & 0x80) == 0x80) {
                return 5;
            }
            throw std::runtime_error(fmt::format("Invalid function for responseLength: {}", response[1]));
    }
}

void MPU::readInputStatus(uint16_t start_register_address, uint16_t count, uint32_t timing) {
    callFunction(_node_address, READ_INPUT_STATUS, timing, start_register_address, count);
    _commanded_info.emplace_back(start_register_address, count);
}

void MPU::readHoldingRegisters(uint16_t start_register_address, uint16_t count, uint32_t timing) {
    callFunction(_node_address, READ_HOLDING_REGISTERS, timing, start_register_address, count);
    _commanded_info.emplace_back(start_register_address, count);
}

void MPU::presetHoldingRegister(uint16_t register_address, uint16_t value, uint32_t timing) {
    callFunction(_node_address, PRESET_HOLDING_REGISTER, timing, register_address, value);
    _commanded_info.emplace_back(register_address, 0);
}

void MPU::presetHoldingRegisters(uint16_t start_register_address, const std::vector<uint16_t> &values,
                                 uint32_t timing) {
    callFunction(_node_address, PRESET_HOLDING_REGISTERS, timing, start_register_address,
                 static_cast<uint16_t>(values.size()), static_cast<uint8_t>(values.size() * 2), values);
    _commanded_info.emplace_back(start_register_address, values.size());
}

bool MPU::getInputStatus(uint16_t input_address) { return _inputStatus.at(input_address); }

uint16_t MPU::getRegister(uint16_t address) {
    std::lock_guard<std::mutex> lg(_registerMutex);
    try {
        return _registers.at(address);
    } catch (std::out_of_range &e) {
        throw std::runtime_error(
                fmt::format("Cannot retrieve holding register {} (0x{:04x})", address, address));
    }
}
