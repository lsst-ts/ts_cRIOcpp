/*
 * Implements generic Modbus Buffer functions.
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

#include <Modbus/Buffer.h>
#include <Modbus/CRC.h>

using namespace Modbus;

Exception::Exception(uint8_t address, uint8_t func, uint8_t exception)
        : std::runtime_error(fmt::format(
                  "ModBus Exception {2} (ModBus address {0}, ModBus response function {1} (0x{1:02x})).",
                  address, func, exception)) {}

Buffer::Buffer() : std::vector<uint8_t>() {}

Buffer::~Buffer() {}

uint16_t Buffer::getCalcCrc() {
    CRC crc(*this);
    return crc.get();
}

void Buffer::callFunction(uint8_t address, uint8_t func) {
    write(address);
    write(func);
    writeCRC();
}

void Buffer::writeI24(int32_t data) {
    pushBuffer(data >> 16);
    pushBuffer(data >> 8);
    pushBuffer(data);
}

void Buffer::writeCRC() {
    uint16_t crc = getCalcCrc();
    pushBuffer(crc & 0xFF);
    pushBuffer((crc >> 8) & 0xFF);
}
