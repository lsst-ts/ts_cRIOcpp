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

using namespace LSST::cRIO;

MPU::IRQTimeout::IRQTimeout(std::vector<uint8_t> _data)
        : std::runtime_error(fmt::format("Din't receive IRQ, reseting MPU. Read data: " +
                                         ModbusBuffer::hexDump(_data.data(), _data.size()))),
          data(_data) {}

MPU::MPU(uint8_t bus) : Modbus::BusList(bus) {
    _loop_state = loop_state_t::WAIT_WRITE;

    addResponse(
            2,
            [this](Modbus::Parser parser) {
                uint8_t len = parser.read<uint8_t>();
                uint16_t inputs_address = requestBufferU16(2);
                uint16_t inputs_len = requestBufferU16(4);
                if (len > ceil(inputs_len / 8.0)) {
                    throw std::runtime_error(fmt::format(
                            "Invalid length for inputs starting at {:04x} - received {}, expected {}",
                            inputs_address, len, inputs_len));
                }
                uint16_t end_address = inputs_address + inputs_len;
                uint8_t data = 0x00;
                for (uint16_t a = inputs_address; a < end_address; a++) {
                    if ((a - inputs_address) % 8 == 0) {
                        data = parser.read<uint8_t>();
                    }
                    _inputStatus[std::pair(parser.address(), a)] = data & 0x01;
                    data >>= 1;
                }
                parser.checkCRC();
            },
            0x82);

    addResponse(
            3,
            [this](Modbus::Parser parser) {
                uint8_t len = parser.read<uint8_t>();

                uint16_t request_address = requestBufferU16(2);
                uint16_t request_len = requestBufferU16(4);
                if (len != (request_len * 2)) {
                    throw std::runtime_error(
                            fmt::format("Invalid respond length for address {} - expected {}, received {}",
                                        parser.address(), len, request_len * 2));
                }
                {
                    std::lock_guard<std::mutex> lg(_registerMutex);
                    for (size_t i = 0; i < len; i += 2) {
                        uint16_t val = parser.read<uint16_t>();
                        _registers[std::pair(parser.address(), request_address)] = val;
                        request_address++;
                    }
                }
                parser.checkCRC();
            },
            0x83);

    addResponse(
            6,
            [this](Modbus::Parser parser) {
                uint16_t register_address = parser.read<uint16_t>();
                uint16_t value = parser.read<uint16_t>();

                uint16_t request_address = requestBufferU16(2);
                uint16_t request_value = requestBufferU16(4);

                if (register_address != request_address) {
                    throw std::runtime_error(fmt::format(
                            "Invalid register address for address {} - expected {:04x}, received {:04x}",
                            parser.address(), request_address, register_address));
                }

                if (value != request_value) {
                    throw std::runtime_error(fmt::format(
                            "Invalid value for address {} register {:04x} - expected {}, received {}",
                            parser.address(), register_address, request_value, value));
                }
                parser.checkCRC();
            },
            0x86);

    addResponse(
            16,
            [this](Modbus::Parser parser) {
                uint16_t register_address = parser.read<uint16_t>();
                uint16_t len = parser.read<uint16_t>();

                uint16_t request_address = requestBufferU16(2);
                uint16_t request_len = requestBufferU16(4);
                if (register_address != request_address) {
                    throw std::runtime_error(
                            fmt::format("Invalid register for address {} - expected {:04x}, received {:04x}",
                                        parser.address(), request_address, register_address));
                }
                if (len != request_len) {
                    throw std::runtime_error(
                            fmt::format("Invalid lenght for address {} register - expected {}, received {}",
                                        parser.address(), register_address, request_len, len));
                }
                parser.checkCRC();
            },
            0x90);
}

void MPU::reset() {
    _loop_state = loop_state_t::WAIT_WRITE;

    _inputStatus.clear();
    _registers.clear();
}

void MPU::presetHoldingRegisters(uint8_t address, uint16_t register_address, uint16_t* values, uint8_t count,
                                 uint16_t timeout) {
    Modbus::Buffer mbus;
    mbus.write<uint8_t>(address);
    mbus.write<uint8_t>(16);
    mbus.write<uint16_t>(register_address);
    mbus.write<uint16_t>(count);
    mbus.write<uint8_t>(count * 2);

    for (uint8_t i = 0; i < count; i++) {
        mbus.write<uint16_t>(values[i]);
    }

    mbus.writeCRC();
    emplace(end(), Modbus::CommandRecord(mbus, timeout));
}
