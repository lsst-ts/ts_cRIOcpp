/*
 * Class parsing modbus buffer device responses.
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

#include <Modbus/CRC.h>
#include <Modbus/Parser.h>

using namespace Modbus;

CRCError::CRCError(uint16_t calculated, uint16_t received)
        : std::runtime_error(fmt::format("checkCRC invalid CRC - expected 0x{:04x}, got 0x{:04x}.",
                                         calculated, received)) {}

void Parser::parse(const std::vector<uint8_t> &buffer) {
    _buffer = buffer;
    if (_buffer.size() < 4) {
        throw std::runtime_error(
                "Cannot parse small buffer - minimal Modbus buffer length is 4 bytes (address, function and "
                "2 bytes CRC)");
    }
    _data = 2;
    checkCRC();
}

void Parser::checkCRC() {
    CRC crc(_buffer.data(), _buffer.size() - 2);
    uint16_t calculated = crc.get();
    uint16_t received;
    memcpy(&received, _buffer.data() + _buffer.size() - 2, 2);
    received = le32toh(received);
    if (calculated != received) {
        throw CRCError(calculated, received);
    }
}

std::string Parser::readString(size_t length) {
    uint8_t buf[length];
    readBuffer(buf, length);
    return std::string((const char *)buf, length);
}
