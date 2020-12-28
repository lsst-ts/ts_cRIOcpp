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

namespace LSST {
namespace cRIO {

/**
 * Utility class for Modbus buffer management. Provides function to write and
 * read cRIO FIFO (FPGA) Modbus buffers.
 *
 * 8bit data are stored as 16bit values. Real data are left shifted by 1. Last
 * bit (0, transmitted first) is start bit, always 1 for ILC communication.
 */
class ModbusBuffer {
public:
    ModbusBuffer();
    virtual ~ModbusBuffer();

    int32_t getIndex();
    void setIndex(int32_t index);
    void incIndex(int32_t inc);
    void skipToNextFrame();
    uint16_t* getBuffer();
    void set(int32_t index, uint16_t data);
    void setLength(int32_t length);
    int32_t getLength();

    /**
     * Set empty buffer.
     */
    void reset();
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
     * Calculate Modbus CRC16
     *
     * @param data data for CRC16 calculation
     *
     * @return calculated Modbus CRC16
     */
    static uint16_t calculateCRC(std::vector<uint8_t> data);

    /**
     * Calculate Modbus from written data
     *
     * @param length buffer length
     *
     * @return calculated Modbus CRC16
     */
    uint16_t calculateCRC(int32_t length);

    void readBuffer(void* buf, size_t len);

    uint16_t readLength();
    int32_t readI32();
    uint8_t readU8();
    uint16_t readU16();
    uint32_t readU32();
    uint64_t readU48();
    float readSGL();
    std::string readString(int32_t length);
    uint16_t readCRC();
    double readTimestamp();

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

    void writeBuffer(void* data, size_t len);

    void writeSubnet(uint8_t data);
    void writeLength(uint16_t data);
    void writeI8(int8_t data);
    void writeI16(int16_t data);
    void writeI24(int32_t data);
    void writeI32(int32_t data);
    void writeU8(uint8_t data);
    void writeU16(uint16_t data);
    void writeU32(uint32_t data);
    void writeSGL(float data);
    void writeCRC(int32_t length);

    /**
     * Return data item to write to buffer.
     *
     * @param data data to write.
     *
     * @return 16bit for command queue.
     */
    static inline uint16_t writeByteInstruction(uint8_t data) { return (((uint16_t)data) << 1) | 0x1200; };

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

private:
    uint16_t _buffer[5120];
    uint8_t _floatPointBuffer[8];
    uint8_t _stringBuffer[256];
    int32_t _index;
    int32_t _length;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* MODBUSBUFFER_H_ */
