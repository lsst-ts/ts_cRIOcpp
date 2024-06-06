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
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <spdlog/fmt/fmt.h>

#include <Modbus/Buffer.h>

namespace Modbus {

/**
 * Dumps hex data to ostring stream.
 *
 * @param dt buffer to print
 * @param len length of the buffer
 */
template <typename dt>
static const std::string hexDump(const dt *buf, size_t len) {
    std::ostringstream os;
    os << std::setfill('0') << std::hex;
    for (size_t i = 0; i < len; i++) {
        if (i > 0) {
            os << " ";
        }
        os << std::setw(sizeof(dt) * 2) << +(buf[i]);
    }
    os << std::dec;
    return os.str();
}

template <typename dt>
static const std::string hexDump(const std::vector<dt> &data) {
    return hexDump<dt>(data.data(), data.size());
}

/**
 * Exception thrown when calculated CRC doesn't match received CRC.
 */
class CRCError : public std::runtime_error {
public:
    /**
     * Construct new CRC error. Signal either problem in the sending
     * electronics, or problems parsing the received replies.
     *
     * @param calculated CRC value calculated from the data (=expected)
     * @param received CRC value received in response (=produced by the other side).
     */
    CRCError(uint16_t calculated, uint16_t received);
};

/**
 * Thrown when response continue after CRC.
 */
class LongResponse : public std::runtime_error {
public:
    /**
     * Construct LongResponse from the given buffer.
     *
     * @param buf Buffer with data received
     * @param len Buffer length
     */
    LongResponse(uint8_t *buf, size_t len)
            : std::runtime_error(std::string("Too long response - received ") + hexDump(buf, len)) {}
};

/**
 * Class parsing a single Modbus response.
 */
class Parser : public std::vector<uint8_t> {
public:
    /**
     * Construct parser from given input data. Essentially only store the data
     * into buffer.
     *
     * @param buffer Buffer to parse
     */
    Parser(std::vector<uint8_t> buffer) { parse(buffer); }

    /**
     * Sets the given buffer as the one to be parsed.
     */
    void parse(std::vector<uint8_t> buffer);

    /**
     * Check that so far read data CRC matches calculated CRC.
     *
     * @throw CRCError if CRC doesn't match
     */
    void checkCRC();

    static const uint16_t u8tou16(uint8_t hb, uint8_t lb) {
        uint16_t ret = hb;
        return ntohs(ret << 8 | lb);
    }

    /**
     * Reads data from the buffer. Size of the receiving buffer is assumed to
     * be equal or greater than len argument.
     *
     * @param buf target buffer to read the data
     * @param len how many bytes shall be read
     */
    void readBuffer(void *buf, size_t len) {
        if (_data + len > size()) {
            throw std::out_of_range(fmt::format(
                    "Attempt to access data beyond buffer end (buffer index {}, but buffer length is {}).",
                    _data + len, size()));
        }
        memcpy(buf, data() + _data, len);
        _data += len;
    }

    /**
     * Template to read next data from message. Data length is specified with
     * template type. Handles conversion from ModBus/ILC's big endian (network order).
     *
     * Intended usage:
     *
     * @code{.cpp}
     * Parser p(std::vector<uint8_t>({0x0a, 0x0c, 0x0d)};
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

    /**
     * Returns address from the Modbus buffer. Actually returns the first byte.
     *
     * @return Modbus command address
     */
    const uint8_t address() { return at(0); }

    /**
     * Returns function code from the Modbus buffer. Actually returns the
     * second byte.
     *
     * @return Modbus function code
     */
    const uint8_t func() { return at(1); }

private:
    size_t _data = 0;  ///< Data pointer.
};

/**
 * Read 24 bites (3 bytes) integer value from the buffer.
 *
 * @return 24 bites value read from the buffer.
 */
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
