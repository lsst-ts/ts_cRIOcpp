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

#ifndef __Modbus_Buffer__
#define __Modbus_Buffer__

#include <arpa/inet.h>
#include <cstdint>
#include <vector>

namespace Modbus {

/**
 * Represents a single Modbus message. Use BusList to organize a set of Modbus
 * messages or callbacks on various functions.
 */
class Buffer : public std::vector<uint8_t> {
public:
    Buffer();

    /**
     * Construct buffer for a given Modbus function. Assumes subnet, data
     * lengths and triggers are send by FPGA class.
     *
     * @param address ModBus address on subnet
     * @param func ModBus function to call
     */
    template <typename... dt>
    Buffer(uint8_t address, uint8_t func, const dt&... params) {
        callFunction(address, func, params...);
    }

    virtual ~Buffer();

    /**
     * Write a value to buffer.
     *
     * @tparam dt value data type. Supported are uint8_t, uint16_t, uint32_t,
     * int8_t, int16_t, int32_t and float.
     *
     * @param data value to write to buffer
     */
    template <typename dt>
    void write(dt data);

    /**
     * Returns current calculcated CRC.
     *
     * @return calculated CRC
     */
    uint16_t getCalcCrc();

    /**
     * Writes CRC for all data already written. Reset internal CRC counter, so
     * buffer can accept more commands.
     */
    void writeCRC();

    /**
     * Add to buffer Modbus function. Assumes subnet, data lengths and triggers are
     * send by FPGA class.
     *
     * @param address ModBus address on subnet
     * @param func ModBus function to call
     */
    void callFunction(uint8_t address, uint8_t func);

    /**
     * Call Modbus function with one or more parameter.
     *
     * @see callFunction(uint8_t, uint8_t, uint32_t)
     *
     * @tparam dt parameter type
     * @param address ModBus address on subnet
     * @param func ModBus function to call
     * @param params function parameters
     */
    template <typename... dt>
    void callFunction(uint8_t address, uint8_t func, const dt&... params) {
        write(address);
        write(func);
        _functionArguments(params...);
        writeCRC();
    }

protected:
    virtual void pushBuffer(uint8_t data) { push_back(data); }

    inline void pushBuffer(uint8_t* data, std::size_t length) {
        for (std::size_t i = 0; i < length; i++) {
            push_back(data[i]);
        }
    }

private:
    void _functionArguments() {}

    template <typename dp1, typename... dt>
    void _functionArguments(const dp1& p1, const dt&... args) {
        write<dp1>(p1);
        _functionArguments(args...);
    }
};

template <>
inline void Buffer::write(int8_t data) {
    pushBuffer(reinterpret_cast<uint8_t*>(&data), 1);
}

template <>
inline void Buffer::write(int16_t data) {
    int16_t d = htons(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 2);
}

template <>
inline void Buffer::write(int32_t data) {
    int32_t d = htonl(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 4);
}

template <>
inline void Buffer::write(uint8_t data) {
    pushBuffer(&data, 1);
}

template <>
inline void Buffer::write(uint16_t data) {
    uint16_t d = htons(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 2);
}

template <>
inline void Buffer::write(uint32_t data) {
    uint32_t d = htonl(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 4);
}

template <>
inline void Buffer::write(uint64_t data) {
    uint64_t d = htobe64(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 8);
}

template <>
inline void Buffer::write(float data) {
    uint32_t* db = reinterpret_cast<uint32_t*>(&data);
    write<uint32_t>(*db);
}

}  // namespace Modbus

#endif /* !__Modbus_Buffer__ */
