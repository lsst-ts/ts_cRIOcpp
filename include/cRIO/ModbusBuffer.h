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
#include <string>
#include <vector>
#include <stdexcept>

#include <arpa/inet.h>
#include <endian.h>

namespace LSST {
namespace cRIO {

/**
 * Exception thrown when calculated CRC doesn't match received CRC.
 */
class CRCError : public std::runtime_error {
public:
    CRCError(uint16_t calculated, uint16_t received);
};

/**
 * Utility class for Modbus buffer management. Provides function to write and
 * read cRIO FIFO (FPGA) Modbus buffers.
 *
 * 8bit data are stored as 16bit values. Real data are left shifted by 1. Last
 * bit (0, transmitted first) is start bit, always 1 for ILC communication.
 *
 * Doesn't handle subnet. Doesn't handle FPGA FIFO read/writes - that's
 * responsibility of FPGA. ModbusBuffer handles only serialization & de
 * serialization of FPGA's FIFO data.
 *
 * Functions throws std::runtime_error (or its subclass) on any error.
 */
class ModbusBuffer {
public:
    ModbusBuffer();
    virtual ~ModbusBuffer();

    int32_t getIndex() { return _index; }
    uint16_t* getBuffer() { return &_buffer[0]; }
    size_t getLength() { return _buffer.size(); }

    void skipToNextFrame();

    /**
     * Set empty buffer.
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

    void readBuffer(void* buf, size_t len);

    template <typename dt>
    dt read();

    uint64_t readU48();
    std::string readString(size_t length);
    double readTimestamp();

    /**
     * Check that accumulated data CRC matches readed CRC.
     *
     * @throw CRCError if CRC doesn't match
     */
    void checkCRC();

    /**
     * Reads instruction byte from FPGA FIFO.
     *
     * @param instruction instruction from Response FIFO.
     *
     * @return byte written by the instruction. Start bit is removed.
     */
    static inline uint8_t readInstructionByte(uint16_t instruction) {
        return (uint8_t)((instruction >> 1) & 0xFF);
    }

    void readEndOfFrame();

    void writeBuffer(uint8_t* data, size_t len);

    template <typename dt>
    void write(dt data);

    void writeI24(int32_t data);

    /**
     * Writes CRC for all data already written. Reset internal CRC counter, so
     * buffer can accept more commands.
     */
    void writeCRC();

    void writeDelay(uint32_t delayMicros);
    void writeEndOfFrame();
    void writeSoftwareTrigger();
    void writeTimestamp();
    void writeTriggerIRQ();
    void writeWaitForRx(uint32_t timeoutMicros);

    /**
     * Fills buffer with data from response, returns start and end timestamps.
     * Checks for CRC.
     *
     * Throws exception on error.
     */
    void pullModbusResponse(uint16_t request, uint64_t& beginTs, uint64_t& endTs, std::vector<uint8_t>& data);

    void skipRead() { _index++; }

protected:
    /**
     * Return data item to write to buffer. Updates CRC counter.
     *
     * @param data data to write.
     *
     * @return 16bit for command queue.
     */
    uint16_t getByteInstruction(uint8_t data);

    void processDataCRC(uint8_t data);

    /**
     * Add to buffer Modbus function. Assumes subnet, data lengths and triggers are
     * send by FPGA class.
     *
     * @param address ILC address on subnet
     * @param function ILC function to call
     * @param timeout function call timeout (excluding transfer times) in us (microseconds)
     */
    void callFunction(uint8_t address, uint8_t function, uint32_t timeout);

    /**
     * Call function with single parameter.
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
    }

private:
    std::vector<uint16_t> _buffer;
    uint32_t _index;
    uint16_t _crcCounter;

    /**
     * Reset internal CRC counter.
     */
    void _resetCRC() { _crcCounter = 0xFFFF; }
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
