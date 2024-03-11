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
#include <stdexcept>
#include <vector>

namespace Modbus {

/**
 * 24 bit (3 bytes) integer. This is used to pass some parameters to ILC calls.
 */
class int24_t {
public:
    int32_t value;  ///< value stored in the class

    /**
     * Construct int24_t value.
     *
     * @param v integer value
     */
    int24_t(int32_t v) { value = v; }
};

/**
 * Thrown when ModBus error response is received.
 */
class Exception : public std::runtime_error {
public:
    /**
     * The class is constructed when an Modbus's error response is received.
     *
     * @param address Modbus address
     * @param func Modbus (error) function received
     * @param exception exception code
     */
    Exception(uint8_t address, uint8_t func, uint8_t exception);
};

/**
 * Represents a single Modbus message. Use BusList to organize a set of Modbus
 * messages or callbacks on various functions.
 */
class Buffer : public std::vector<uint8_t> {
public:
    /**
     * Construct an empty buffer.
     */
    Buffer();

    /**
     * Fill newly constructed buffer with data.
     *
     * @param data Buffer's data
     */
    Buffer(std::vector<uint8_t> data) : std::vector<uint8_t>(data) {}

    /**
     * Construct buffer for a given Modbus function. Assumes subnet, data
     * lengths and triggers are send by FPGA class.
     *
     * @param address ModBus address on subnet
     * @param func ModBus function to call
     * @param params ModBus command arguments.
     */
    template <typename... dt>
    Buffer(uint8_t address, uint8_t func, const dt&... params) {
        callFunction(address, func, params...);
    }

    /**
     * Destruct buffer instance.
     */
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
     * Writes 24bit signed integer.
     *
     * @param data 24bit signed integer
     *
     * @see write
     */
    void writeI24(int32_t data);

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
     * Return address stored in the Modbus buffer. Essentially returns the
     * first byte.
     *
     * @return Buffer command address
     */
    uint8_t address() { return at(0); }

    /**
     * Return function call stored in the Modbus buffer. Essentially returns
     * the second byte.
     *
     * @return Function call code
     */
    uint8_t func() { return at(1); }

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

/**
 * Write 8 bit signed integer to the end of the buffer.
 *
 * @param data 8 bit signed integer to write
 */
template <>
inline void Buffer::write(int8_t data) {
    pushBuffer(reinterpret_cast<uint8_t*>(&data), 1);
}

/**
 * Write 16 bit signed integer to the end of the buffer.
 *
 * @param data 16 bit signed integer to write
 */
template <>
inline void Buffer::write(int16_t data) {
    int16_t d = htons(data);
    pushBuffer(reinterpret_cast<uint8_t*>(&d), 2);
}

template <>
inline void Buffer::write(int24_t data) {
    writeI24(data.value);
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

template <>
inline void Buffer::write(std::vector<uint8_t> data) {
    for (auto d : data) {
        write<uint8_t>(d);
    }
}

}  // namespace Modbus

#endif /* !__Modbus_Buffer__ */
