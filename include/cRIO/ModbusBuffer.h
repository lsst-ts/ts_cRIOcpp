/*
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

#ifndef MODBUSBUFFER_H_
#define MODBUSBUFFER_H_

#include <cRIO/DataTypes.h>
#include <queue>
#include <string>
#include <stdexcept>
#include <vector>

#include <arpa/inet.h>
#include <endian.h>

namespace LSST {
namespace cRIO {

/**
 * Utility class for Modbus buffer management. Provides function to write and
 * read cRIO FIFO (FPGA) Modbus buffers. Modbus serial bus is serviced inside
 * FPGA with
 * [Common_FPGA_Modbus](https://github.com/lsst-ts/Common_FPGA_Modbus) module.
 *
 * 8bit data are stored as 16bit values. Real data are left shifted by 1. Last
 * bit (0, transmitted first) is start bit, always 0 for ILC communication.
 * First data bit (transmitted last) is stop bit, shall be 1. So uint8_t data d
 * needs to be written as:
 *
 * (0x1200 | (d << 1))
 *
 * This class doesn't handle subnet. Doesn't handle FPGA FIFO read/writes -
 * that's responsibility of FPGA. ModbusBuffer handles only serialization & de
 * serialization of FPGA's FIFO data. Also this class handles CRC calculation,
 * writes (in output buffers) and checks (for input buffers).
 *
 * Functions throws std::runtime_error (or its subclass) on any error.
 */
class ModbusBuffer {
public:
    /**
     * Constructs empty ModbusBuffer.
     */
    ModbusBuffer();
    virtual ~ModbusBuffer();

    /**
     * Returns buffer memory.  Calls to write* methods can result in
     * reallocation of the buffer memory, trying to read from getBuffer if that
     * happens results in undefined behaviour.
     *
     * @return uint16_t FIFO buffer
     */
    uint16_t* getBuffer() { return _buffer.data(); }

    /**
     * Returns buffer length.
     *
     * @return buffer length
     */
    size_t getLength() { return _buffer.size(); }

    /**
     * Resets internal offset to 0 to start reading message again. Clears
     * internal CRC counter, so CRC will be recalculated during subsequent
     * reads.
     */
    void reset();

    /**
     * Clears modbus buffer.
     */
    void clear();

    bool endOfBuffer();
    bool endOfFrame();

    /**
     * Returns read data from buffer.
     *
     * @param length data length
     *
     * @return read data
     */
    std::vector<uint8_t> getReadData(int32_t length);

    /**
     * Reads data from buffer. Updates CRC as it reads the data. Size of
     * receiving buffer is assumed to be equal or greater than len.
     *
     * @param buf target buffer to read the data
     * @param len how many bytes shall be read
     */
    void readBuffer(void* buf, size_t len);

    /**
     * Template to read next data from message. Data length is specified with
     * template type. Handles conversion from ILC's big endian (network order).
     *
     * Intended usage:
     *
     * @code
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
    double readTimestamp();

    /**
     * Check that accumulated data CRC matches readed CRC.
     *
     * @throw CRCError if CRC doesn't match
     */
    void checkCRC();

    /**
     * @throw std::runtime_error if end of frame isn't in buffer
     */
    void readEndOfFrame();

    /**
     * @throw std::runtime_error if wait for rx delay command isn't present
     */
    uint32_t readWaitForRx();

    void writeBuffer(uint8_t* data, size_t len);

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
     * Writes CRC for all data already written. Reset internal CRC counter, so
     * buffer can accept more commands.
     */
    void writeCRC();

    /**
     * Write FPGA delay command. Delays processing of read buffer by given
     * number of microseconds.
     *
     * @param delayMicros delay in microseconds
     */
    void writeDelay(uint32_t delayMicros);

    /**
     * Writes end of the frame. Causes silence on transmitting bus, so
     * commanded ILC can check CRC of the incomming message and starts to
     * execute the commanded action.
     */
    void writeEndOfFrame();

    /**
     * Write FPGA Modbus command to wait for ILC response. If no response is received within timeout,
     *
     * @param timeoutMicros
     */
    void writeWaitForRx(uint32_t timeoutMicros);

    /**
     * Sets current read buffer
     *
     * @param buffer
     * @param length
     */
    void setBuffer(uint16_t* buffer, size_t length);

    /**
     * Exception thrown when calculated CRC doesn't match received CRC.
     */
    class CRCError : public std::runtime_error {
    public:
        CRCError(uint16_t calculated, uint16_t received);
    };

    class EndOfBuffer : public std::runtime_error {
    public:
        EndOfBuffer();
    };

    class UnmatchedFunction : public std::runtime_error {
    public:
        UnmatchedFunction(uint8_t address, uint8_t function);
        UnmatchedFunction(uint8_t address, uint8_t function, uint8_t expectedAddress,
                          uint8_t expectedFunction);
    };

protected:
    void processDataCRC(uint8_t data);

    /**
     * Add to buffer Modbus function. Assumes subnet, data lengths and triggers are
     * send by FPGA class. If non-broadcast address is passed, stores address
     * and function into _commanded buffer.
     *
     * @param address ILC address on subnet
     * @param function ILC function to call
     * @param timeout function call timeout (excluding transfer times) in us (microseconds)
     */
    void callFunction(uint8_t address, uint8_t function, uint32_t timeout);

    /**
     * Call Modbus function with single parameter.
     *
     * @see callFunction(uint8_t, uint8_t, uint32_t)
     *
     * @tparam dt parameter type
     * @param address ILC address on subnet
     * @param function ILC function to call
     * @param timeout function call timeout (excluding transfer time) in us (microseconds)
     * @param p1 function parameter
     */
    template <typename dt>
    void callFunction(uint8_t address, uint8_t function, uint32_t timeout, dt p1) {
        write(address);
        write(function);
        write<dt>(p1);
        writeCRC();
        writeEndOfFrame();
        writeWaitForRx(timeout);

        _pushCommanded(address, function);
    }

    /**
     * Checks that received response matches expected response.
     *
     * @param address ILC address on subnet
     * @param function ILC function code; if check is performed for error response, must equal to called
     * function
     *
     * @throw std::runtime_error or its subclass on error.
     * @throw ILCException when ILC function is received.
     */
    void checkCommanded(uint8_t address, uint8_t function);

private:
    std::vector<uint16_t> _buffer;
    uint32_t _index;
    uint16_t _crcCounter;

    std::queue<std::pair<uint8_t, uint8_t>> _commanded;

    /**
     * Reset internal CRC counter.
     */
    void _resetCRC() { _crcCounter = 0xFFFF; }

    void _pushCommanded(uint8_t address, uint8_t function) {
        if (address > 0 && address < 248) {
            _commanded.push(std::pair<uint8_t, uint8_t>(address, function));
        }
    }

    /**
     * Reads instruction byte from FPGA FIFO. Increases index after instruction is read.
     *
     * @throw EndOfBuffer if asking for instruction after end of the buffer
     *
     * @return byte written by the instruction. Start bit is removed.
     */
    uint8_t _readInstructionByte() {
        if (endOfBuffer()) {
            throw EndOfBuffer();
        }
        return (uint8_t)((_buffer[_index++] >> 1) & 0xFF);
    }

    /**
     * Return data item to write to buffer. Updates CRC counter.
     *
     * @param data data to write.
     *
     * @return 16bit for command queue.
     */
    uint16_t _getByteInstruction(uint8_t data);
};

template <>
inline int32_t ModbusBuffer::read() {
    int32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

template <>
inline uint8_t ModbusBuffer::read() {
    uint8_t ret;
    readBuffer(&ret, 1);
    return ret;
}

template <>
inline uint16_t ModbusBuffer::read() {
    uint16_t db;
    readBuffer(&db, 2);
    return ntohs(db);
}

template <>
inline uint32_t ModbusBuffer::read() {
    uint32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

template <>
inline uint64_t ModbusBuffer::read() {
    uint64_t db;
    readBuffer(&db, 8);
    return be64toh(db);
}

template <>
inline float ModbusBuffer::read() {
    uint32_t d = read<uint32_t>();
    float* db = reinterpret_cast<float*>(&d);
    return *db;
}

template <>
inline void ModbusBuffer::write(int8_t data) {
    writeBuffer(reinterpret_cast<uint8_t*>(&data), 1);
}

template <>
inline void ModbusBuffer::write(int16_t data) {
    int16_t d = htons(data);
    writeBuffer(reinterpret_cast<uint8_t*>(&d), 2);
}

template <>
inline void ModbusBuffer::write(int32_t data) {
    int32_t d = htonl(data);
    writeBuffer(reinterpret_cast<uint8_t*>(&d), 4);
}

template <>
inline void ModbusBuffer::write(uint8_t data) {
    writeBuffer(&data, 1);
}

template <>
inline void ModbusBuffer::write(uint16_t data) {
    uint16_t d = htons(data);
    writeBuffer(reinterpret_cast<uint8_t*>(&d), 2);
}

template <>
inline void ModbusBuffer::write(uint32_t data) {
    uint32_t d = htonl(data);
    writeBuffer(reinterpret_cast<uint8_t*>(&d), 4);
}

template <>
inline void ModbusBuffer::write(float data) {
    uint32_t* db = reinterpret_cast<uint32_t*>(&data);
    write<uint32_t>(*db);
}

}  // namespace cRIO
}  // namespace LSST

#endif /* MODBUSBUFFER_H_ */
