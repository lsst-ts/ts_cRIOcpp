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

void MPUBuffer::readHoldingRegisters(uint8_t mpu_address, uint16_t address, uint16_t count) {
    callFunction(mpu_address, 3, 0, address, count);
}

void MPUBuffer::presetHoldingRegisters(uint8_t mpu_address, uint16_t address, uint16_t *values,
                                       uint8_t count) {
    write(mpu_address);
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

    pushCommanded(mpu_address, 16);
}

MPU::MPU(uint8_t mpu_address) : _mpu_address(mpu_address) {}

void MPU::readHoldingRegisters(uint16_t address, uint16_t count, uint8_t timeout) {
    MPUBuffer buffer;
    buffer.readHoldingRegisters(_mpu_address, address, count);

    // write request
    _commands.push_back(MPUCommands::WRITE);
    _commands.push_back(buffer.getLength());
    for (auto b : buffer.getBufferVector()) {
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
    MPUBuffer buffer;
    buffer.presetHoldingRegisters(_mpu_address, address, values, count);

    // write request
    _commands.push_back(MPUCommands::WRITE);
    _commands.push_back(buffer.getLength());
    for (auto b : buffer.getBufferVector()) {
        _commands.push_back(b);
    }

    _commands.push_back(MPUCommands::WAIT_MS);
    _commands.push_back(timeout);

    // read response
    _commands.push_back(MPUCommands::READ);
    // extras: device address, function (all 1 byte), address, number of registers written, CRC (2 bytes)
    _commands.push_back(8);
    _commands.push_back(MPUCommands::CHECK_CRC);
}

void MPU::processResponse(uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf[i] != _mpu_address) {
            throw std::runtime_error(
                    fmt::format("Invalid ModBus address {}, expected {}", buf[i], _mpu_address));
        }
        i++;
        if (i >= len) {
            throw std::runtime_error("Empty response");
        }

        switch (buf[i]) {
            case 3:
                if (buf[i + 1] >= len - i) {
                    throw std::runtime_error(fmt::format(
                            "Insufficient response length: {}, expected at least {}", len - i, buf[i + 1]));
                }
                _processRegisters(buf + i + 1);
                i += 1 + buf[i + 1];
                break;
            default:
                throw std::runtime_error(fmt::format("Unsupported function: {}", buf[i]));
        }
    }
}

void MPU::_processRegisters(uint8_t *buf) {
    for (size_t i = 0; i < buf[0]; i += 2) {
        if (_readRegisters.empty()) {
            throw std::runtime_error("Too big response");
        }
        _registers[_readRegisters.front()] = be16toh(*(reinterpret_cast<uint16_t *>(buf + 1 + i)));
        _readRegisters.pop_front();
    }
}
