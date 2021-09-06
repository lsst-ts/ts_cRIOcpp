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
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

using namespace std;

namespace LSST {
namespace cRIO {

ModbusBuffer::ModbusBuffer() { clear(); }

ModbusBuffer::~ModbusBuffer() {}

void ModbusBuffer::reset() {
    _index = 0;
    _crc.reset();
    _recordChanges = false;
    _records.clear();
}

void ModbusBuffer::clear() {
    _buffer.clear();
    std::queue<std::pair<uint8_t, uint8_t>> emptyQ;
    _commanded.swap(emptyQ);
    reset();
}

bool ModbusBuffer::endOfBuffer() { return _index >= _buffer.size(); }

bool ModbusBuffer::endOfFrame() { return _buffer[_index] == FIFO::RX_ENDFRAME; }

std::vector<uint8_t> ModbusBuffer::getReadData(int32_t length) {
    std::vector<uint8_t> data;
    for (size_t i = _index - length; i < _index; i++) {
        data.push_back(readInstructionByte());
    }
    return data;
}

void ModbusBuffer::readBuffer(void* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t d = readInstructionByte();
        processDataCRC(d);
        (reinterpret_cast<uint8_t*>(buf))[i] = d;
    }
}

uint64_t ModbusBuffer::readU48() {
    uint64_t ret = 0;
    readBuffer(reinterpret_cast<uint8_t*>(&ret) + 2, 6);
    return be64toh(ret);
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

void ModbusBuffer::checkCRC() {
    uint16_t crc;
    uint16_t calCrc = _crc.get();
    _recordChanges = false;
    readBuffer(&crc, 2);
    crc = le32toh(crc);
    if (crc != calCrc) {
        throw CRCError(calCrc, crc);
    }
    _crc.reset();
}

uint32_t ModbusBuffer::readDelay() {
    uint16_t c = _buffer[_index] & 0xF000;
    uint32_t ret = 0;
    switch (c) {
        case FIFO::DELAY:
            ret = 0x0FFF & _buffer[_index];
            break;
        case FIFO::LONG_DELAY:
            ret = (0x0FFF & _buffer[_index]) * 1000;
            break;
        default:
            throw std::runtime_error(
                    fmt::format("Expected delay, finds {:04x} (@ offset {})", _buffer[_index], _index));
    }
    _index++;
    return ret;
}

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
    uint16_t crc = _crc.get();
    _buffer.push_back(getByteInstruction(crc & 0xFF));
    _buffer.push_back(getByteInstruction((crc >> 8) & 0xFF));
    _crc.reset();
}

void ModbusBuffer::writeDelay(uint32_t delayMicros) {
    _buffer.push_back(delayMicros > 0x0FFF ? ((0x0FFF & ((delayMicros / 1000) + 1)) | FIFO::LONG_DELAY)
                                           : (delayMicros | FIFO::DELAY));
}

void ModbusBuffer::writeWaitForRx(uint32_t timeoutMicros) {
    _buffer.push_back(timeoutMicros > 0x0FFF
                              ? ((0x0FFF & ((timeoutMicros / 1000) + 1)) | FIFO::TX_WAIT_LONG_RX)
                              : (timeoutMicros | FIFO::TX_WAIT_RX));
}

void ModbusBuffer::setBuffer(uint16_t* buffer, size_t length) {
    _buffer.clear();

    _index = 0;
    _crc.reset();
    _buffer.resize(length);
    memcpy(_buffer.data(), buffer, length * sizeof(uint16_t));
}

void ModbusBuffer::checkCommandedEmpty() {
    if (_commanded.empty()) {
        return;
    }
    std::ostringstream os;
    while (!_commanded.empty()) {
        if (os.str().length() > 0) {
            os << ",";
        }
        auto c = _commanded.front();
        os << static_cast<int>(c.first) << ":" << static_cast<int>(c.second);
        _commanded.pop();
    }
    throw std::runtime_error("Responses for those <address:function> pairs weren't received: " + os.str());
}

void ModbusBuffer::CRC::add(uint8_t data) {
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

void ModbusBuffer::CRC::add(uint8_t data) {
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

ModbusBuffer::CRCError::CRCError(uint16_t calculated, uint16_t received)
        : std::runtime_error(fmt::format("checkCRC invalid CRC - expected 0x{:04x}, got 0x{:04x}", calculated,
                                         received)) {}

ModbusBuffer::EndOfBuffer::EndOfBuffer() : std::runtime_error("End of buffer while reading response") {}

ModbusBuffer::UnmatchedFunction::UnmatchedFunction(uint8_t address, uint8_t function)
        : std::runtime_error(
                  fmt::format("Received response {1} with address {0} without matching send function.",
                              address, function)) {}

ModbusBuffer::UnmatchedFunction::UnmatchedFunction(uint8_t address, uint8_t function, uint8_t expectedAddress,
                                                   uint8_t expectedFunction)
        : std::runtime_error(fmt::format("Invalid response received - expected {2} (0x{2:02x}) from {3}, got "
                                         "{1} (0x{1:02x}) from {0}",
                                         address, function, expectedAddress, expectedFunction)) {}

uint16_t ModbusBuffer::getByteInstruction(uint8_t data) {
    processDataCRC(data);
    return data;
}

void ModbusBuffer::processDataCRC(uint8_t data) {
    if (_recordChanges) {
        _records.push_back(data);
    }

    _crc.add(data);
}

uint8_t ModbusBuffer::readInstructionByte() {
    if (endOfBuffer()) {
        throw EndOfBuffer();
    }
    return (uint8_t)(_buffer[_index++]);
}

void ModbusBuffer::callFunction(uint8_t address, uint8_t function, uint32_t timeout) {
    write(address);
    write(function);
    writeCRC();
    writeEndOfFrame();
    writeWaitForRx(timeout);

    pushCommanded(address, function);
}

void ModbusBuffer::broadcastFunction(uint8_t address, uint8_t function, uint8_t counter, uint32_t delay,
                                     uint8_t* data, size_t dataLen) {
    write(address);
    write(function);
    write(counter);
    for (size_t i = 0; i < dataLen; i++) {
        write(data[i]);
    }
    writeCRC();
    writeEndOfFrame();
    writeDelay(delay);
}

void ModbusBuffer::checkCommanded(uint8_t address, uint8_t function) {
    if (_commanded.empty()) {
        throw UnmatchedFunction(address, function);
    }
    std::pair<uint8_t, uint8_t> last = _commanded.front();
    _commanded.pop();
    if (last.first != address || last.second != function) {
        throw UnmatchedFunction(address, function, last.first, last.second);
    }
}

bool ModbusBuffer::checkRecording(std::vector<uint8_t>& cached) {
    _recordChanges = false;
    if (cached == _records) {
        _records.clear();
        return true;
    }
    std::swap(cached, _records);
    _records.clear();
    return false;
}

void ModbusBuffer::pushCommanded(uint8_t address, uint8_t function) {
    if ((address > 0 && address < 248) || (address == 255)) {
        _commanded.push(std::pair<uint8_t, uint8_t>(address, function));
    }
}

}  // namespace cRIO
}  // namespace LSST
