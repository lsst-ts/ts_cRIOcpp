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

using namespace LSST::cRIO;

void MPUBuffer::readHoldingRegisters(uint8_t mpu_address, uint16_t address, uint16_t count) {
    callFunction(mpu_address, 3, 0, address, count);
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
}
