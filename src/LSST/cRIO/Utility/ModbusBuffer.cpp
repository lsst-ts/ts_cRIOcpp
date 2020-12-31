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

#include <cRIO/ModbusBuffer.h>
#include <cRIO/Timestamp.h>
#include <string.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

using namespace std;

// masks for FPGA FIFO commands
const static uint16_t FIFO_TX_FRAMEEND = 0x20DA;
const static uint16_t FIFO_TX_TIMESTAMP = 0x3000;
const static uint16_t FIFO_TX_WAIT_RX = 0x6000;
const static uint16_t FIFO_TX_IRQTRIGGER = 0x7000;
const static uint16_t FIFO_TX_WAIT_TRIGGER = 0x8000;
const static uint16_t FIFO_TX_WAIT_LONG_RX = 0x9000;
const static uint16_t FIFO_RX_ENDFRAME = 0xA000;

namespace LSST {
namespace cRIO {

ModbusBuffer::ModbusBuffer() { clear(); }

ModbusBuffer::~ModbusBuffer() {}

void ModbusBuffer::skipToNextFrame() {
    // Scan for the end of frame marker
    while (!endOfFrame() && !endOfBuffer()) {
        _index++;
    }
    // Increment to the address of the next message in the buffer
    _index++;
}

void ModbusBuffer::reset() {
    _index = 0;
    _resetCRC();
}

void ModbusBuffer::clear() {
    _buffer.clear();
    reset();
}

bool ModbusBuffer::endOfBuffer() { return _index >= _buffer.size(); }

bool ModbusBuffer::endOfFrame() { return _buffer[_index] == FIFO_RX_ENDFRAME; }

std::vector<uint8_t> ModbusBuffer::getReadData(int32_t length) {
    std::vector<uint8_t> data;
    for (size_t i = _index - length; i < _index; i++) {
        data.push_back(readInstructionByte(_buffer[i]));
    }
    return data;
}

void ModbusBuffer::readBuffer(void* buf, size_t len) {
    for (size_t i = 0; i < len; i++, _index++) {
        uint8_t d = readInstructionByte(_buffer[_index]);
        processDataCRC(d);
        (reinterpret_cast<uint8_t*>(buf))[i] = d;
    }
}

int32_t ModbusBuffer::readI32() {
    int32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

uint8_t ModbusBuffer::readU8() {
    uint8_t ret;
    readBuffer(&ret, 1);
    return ret;
}

uint16_t ModbusBuffer::readU16() {
    uint16_t db;
    readBuffer(&db, 2);
    return ntohs(db);
}

uint32_t ModbusBuffer::readU32() {
    uint32_t db;
    readBuffer(&db, 4);
    return ntohl(db);
}

uint64_t ModbusBuffer::readU48() {
    _index += 6;
    return ((uint64_t)readInstructionByte(_buffer[_index - 6]) << 40) |
           ((uint64_t)readInstructionByte(_buffer[_index - 5]) << 32) |
           ((uint64_t)readInstructionByte(_buffer[_index - 4]) << 24) |
           ((uint64_t)readInstructionByte(_buffer[_index - 3]) << 16) |
           ((uint64_t)readInstructionByte(_buffer[_index - 2]) << 8) |
           ((uint64_t)readInstructionByte(_buffer[_index - 1]));
}

float ModbusBuffer::readSGL() {
    uint32_t d = readU32();
    float* db = reinterpret_cast<float*>(&d);
    return *db;
}

std::string ModbusBuffer::readString(size_t length) {
    uint8_t buf[length];
    readBuffer(buf, length);
    return std::string((const char*)buf, length);
}

double ModbusBuffer::readTimestamp() {
    uint64_t ret;
    readBuffer(&ret, 8);
    ret = le64toh(ret);
    return Timestamp::fromRaw(ret);
}

bool ModbusBuffer::checkCRC() {
    uint16_t crc;
    uint16_t calCrc = _crcCounter;
    readBuffer(&crc, 2);
    return le32toh(crc) == calCrc;
}

void ModbusBuffer::readEndOfFrame() { _index++; }

void ModbusBuffer::writeBuffer(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        _buffer.push_back(getByteInstruction(data[i]));
    }
}

void ModbusBuffer::writeI24(int32_t data) {
    _buffer.push_back(getByteInstruction((uint8_t)(data >> 16)));
    _buffer.push_back(getByteInstruction((uint8_t)(data >> 8)));
    _buffer.push_back(getByteInstruction((uint8_t)data));
}

void ModbusBuffer::writeCRC() {
    _buffer.push_back(0x1200 | ((_crcCounter & 0xFF) << 1));
    _buffer.push_back(0x1200 | (((_crcCounter >> 8) & 0xFF) << 1));
    _resetCRC();
}

void ModbusBuffer::writeDelay(uint32_t delayMicros) {
    _buffer.push_back(delayMicros > 4095 ? (((delayMicros / 1000) + 1) | 0x5000) : (delayMicros | 0x4000));
}

void ModbusBuffer::writeEndOfFrame() { _buffer.push_back(FIFO_TX_FRAMEEND); }

void ModbusBuffer::writeSoftwareTrigger() { _buffer.push_back(FIFO_TX_WAIT_TRIGGER); }

void ModbusBuffer::writeTimestamp() { _buffer.push_back(FIFO_TX_TIMESTAMP); }

void ModbusBuffer::writeTriggerIRQ() { _buffer.push_back(FIFO_TX_IRQTRIGGER); }

void ModbusBuffer::writeWaitForRx(uint32_t timeoutMicros) {
    _buffer.push_back(timeoutMicros > 4095 ? (((timeoutMicros / 1000) + 1) | FIFO_TX_WAIT_LONG_RX)
                                           : (timeoutMicros | FIFO_TX_WAIT_RX));
}

uint16_t ModbusBuffer::getByteInstruction(uint8_t data) {
    processDataCRC(data);
    return 0x1200 | ((static_cast<uint16_t>(data)) << 1);
}

void ModbusBuffer::processDataCRC(uint8_t data) {
    _crcCounter = _crcCounter ^ (uint16_t(data));
    for (int j = 0; j < 8; j++) {
        if (_crcCounter & 0x0001) {
            _crcCounter = _crcCounter >> 1;
            _crcCounter = _crcCounter ^ 0xA001;
        } else {
            _crcCounter = _crcCounter >> 1;
        }
    }
}

}  // namespace cRIO
}  // namespace LSST
