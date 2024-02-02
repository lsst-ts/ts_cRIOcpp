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

#ifndef __Modbus_Parser__
#define __Modbus_Parser__

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <Modbus/Buffer.h>

namespace Modbus {

/**
 * Exception thrown when calculated CRC doesn't match received CRC.
 */
class CRCError : public std::runtime_error {
public:
    CRCError(uint16_t calculated, uint16_t received);
};

class Parser : std::vector<uint8_t> {
public:
    Parser(const std::vector<uint8_t> &buffer) { parse(buffer); }

    void parse(const std::vector<uint8_t> &buffer);

    /**
     * Check that accumulated data CRC matches readed CRC. Also stops recording
     * of changes, as CRC shall be at the end of buffer.
     *
     * @see pauseRecordChanges
     *
     * @throw CRCError if CRC doesn't match
     */
    void checkCRC();

    /**
     * Reads data from buffer. Size of * receiving buffer is assumed to be
     * equal or greater than len.
     *
     * @param buf target buffer to read the data
     * @param len how many bytes shall be read
     */
    void readBuffer(void *buf, size_t len) {
        if (_data + len > _buffer.size()) {
            throw std::runtime_error("Trying to access data beyond buffer are.");
        }
        memcpy(buf, _buffer.data() + _data, len);
        _data += len;
    }
    /**
     * Template to read next data from message. Data length is specified with
     * template type. Handles conversion from ModBus/ILC's big endian (network order).
     *
     * Intended usage:
     *
     * @code{.cpp}
     * ModbusBuffer b;
     * b.setBuffer({(0x1200 | (0x0a << 1)), (0x1200 | (0x0c << 1)), (0x1200 | (0x0d << 1))}, 3);
     * uint8_t p1 = b.read<uint8_t>();
     * uint16_t p2 = b.read<uint16_t>();
     * @endcode
     *
     * @tparam dt variable data type. Supported are uint8_t, uint16_t,
     * uint32_t, uint64_t and int32_t and float.
     *
     * @return value of read response
     */
    template <typename dt>
    dt read();

    /**
     * Reads 6 bytes (48 bits) unsigned value.
     *
     * @return 6 bytes unsigned value read from the buffer
     */
    uint64_t readU48();

    /**
     * Reads string of given length from incoming buffer.
     *
     * @param length
     *
     * @return
     */
    std::string readString(size_t length);

    const uint8_t address() { return _buffer[0]; }

    const uint8_t func() { return _buffer[1]; }

private:
    std::vector<uint8_t> _buffer;

    /***
     * Data pointer.
     */
    size_t _data;
};

template <>
inline int24_t Parser::read() {
    int32_t db = 0;
    readBuffer(&db, 3);
    // handle properly negative values
    db = ntohl(db) >> 8;
    if (db & 0x800000) {
        return db | 0xFF000000;
    }
    return db;
}

template <>
inline int32_t Parser::read() {
    int32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

template <>
inline uint8_t Parser::read() {
    uint8_t ret;
    readBuffer(&ret, 1);
    return ret;
}

template <>
inline uint16_t Parser::read() {
    uint16_t db;
    readBuffer(&db, 2);
    return ntohs(db);
}

template <>
inline uint32_t Parser::read() {
    uint32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

template <>
inline uint64_t Parser::read() {
    uint64_t db;
    readBuffer(&db, 8);
    return be64toh(db);
}

template <>
inline float Parser::read() {
    uint32_t d = read<uint32_t>();
    float *db = reinterpret_cast<float *>(&d);
    return *db;
}

}  // namespace Modbus

#endif /* !__Modbus_Parser__ */
