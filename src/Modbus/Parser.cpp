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

void Parser::parse(std::vector<uint8_t> buffer) {
    if (buffer.size() < 4) {
        throw std::runtime_error(
                fmt::format("Cannot parse small buffer (size {}) - minimal Modbus buffer length is 4 bytes "
                            "(address, function and "
                            "2 bytes CRC)",
                            buffer.size()));
    }
    assign(buffer.begin(), buffer.end());
    _data = 2;
}

void Parser::checkCRC() {
    CRC crc(data(), _data);
    uint16_t calculated = crc.get();
    uint16_t received = be16toh(read<uint16_t>());
    if (calculated != received) {
        throw CRCError(calculated, received);
    }
    if (_data < size()) {
        throw LongResponse(data() + _data, size() - _data);
    }
}

uint64_t Parser::readU48() {
    uint64_t ret = 0;
    readBuffer(reinterpret_cast<uint8_t *>(&ret) + 2, 6);
    return be64toh(ret);
}

std::string Parser::readString(size_t length) {
    uint8_t buf[length];
    readBuffer(buf, length);
    return std::string((const char *)buf, length);
}
