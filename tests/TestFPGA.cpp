/*
 * This file is part of LSST cRIOcpp test suite. Tests FPGA Cli App.
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

#include <catch2/catch.hpp>
#include <cstring>

#include <Modbus/CRC.h>

#include <TestFPGA.h>

using namespace LSST::cRIO;

uint16_t currentMode = 0;  // current address 8 mod

void TestILC::processChangeILCMode(uint8_t address, uint16_t mode) {
    REQUIRE(address == 8);
    REQUIRE(mode == currentMode);
}

TestFPGA::TestFPGA()
        : ILC::ILCBusList(1),
          FPGA(fpgaType::SS),
          PrintILC(1),
          _response(),
          _U16ResponseStatus(IDLE),
          _pages(nullptr),
          _currentTimestamp(0) {}

void TestFPGA::writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) {
    uint16_t* d = data;
    while (d < data + length) {
        size_t dl;
        switch (*d) {
            case FPGAAddress::MODBUS_A_TX:
                d++;
                dl = *d;
                d++;
                _simulateModbus(d, dl);
                d += dl;
                break;
            case FPGAAddress::HEARTBEAT:
                d += 2;
                break;
            // modbus software trigger
            case 252:
                d++;
                break;
            default:
                throw std::runtime_error(
                        "SimulatedFPGA::writeCommandFIFO unknown/unimplemented instruction: " +
                        std::to_string(*d));
                d++;
                break;
        }
    }
}

void TestFPGA::writeRequestFIFO(uint16_t* data, size_t length, uint32_t timeout) { _U16ResponseStatus = LEN; }

void TestFPGA::readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) {
    switch (_U16ResponseStatus) {
        case IDLE:
            throw std::runtime_error("readU16ResponseFIFO called out of order");
        case LEN:
            REQUIRE(length == 1);
            *data = _response.getLength();
            _U16ResponseStatus = DATA;
            break;
        case DATA:
            REQUIRE(length == _response.getLength());
            memcpy(data, _response.getBuffer(), _response.getLength() * 2);
            _response.clear();
            _U16ResponseStatus = IDLE;
            break;
    }
}

void TestFPGA::waitOnIrqs(uint32_t irqs, uint32_t timeout, bool& timedout, uint32_t* triggered) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (triggered != NULL) {
        *triggered = _simulatedIRQs;
    }
    timedout = false;
}

void TestFPGA::ackIrqs(uint32_t irqs) {}

void TestFPGA::processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) {
    _response.write(address);
    _response.write<uint8_t>(18);
    _response.write(mode);
    _response.write(status);
    _response.write(faults);

    _response.writeCRC();
}

void TestFPGA::processChangeILCMode(uint8_t address, uint16_t mode) {
    _response.write(address);
    _response.write<uint8_t>(65);
    _response.write(mode);

    _response.writeCRC();
}

void TestFPGA::processVerifyUserApplication(uint8_t address, uint16_t status) {
    _response.write(address);
    _response.write<uint8_t>(103);
    _response.write(status);

    _response.writeCRC();
}

void TestFPGA::_simulateModbus(uint16_t* data, size_t length) {
    // reply format:
    // 4 bytes (forming uint64_t in low endian) beginning timestamp
    // data received from ILCs (& FIFO::TX_WAIT_LONG_RX)
    // end of frame (FIFO::RX_ENDFRAME)
    // 8 bytes of end timestap (& FIFO::RX_TIMESTAMP)
    _response.writeFPGATimestamp(_currentTimestamp++);

    uint16_t* d = data;

    auto read_data = [](uint16_t*& v, std::vector<uint8_t>& buf) -> uint8_t {
        uint8_t d = static_cast<uint8_t>((*v >> 1) & 0xFF);
        v++;
        buf.push_back(d);
        return d;
    };

    auto read_u16 = [read_data](uint16_t*& v, std::vector<uint8_t>& buf) -> uint16_t {
        uint16_t ret = read_data(v, buf);
        ret <<= 8;
        ret |= read_data(v, buf);
        return ret;
    };

    auto check_CRC = [read_data, read_u16](std::vector<uint8_t>& buf, uint16_t*& v) {
        Modbus::CRC crc(buf);
        uint16_t received = be16toh(read_u16(v, buf));
        CHECK(crc.get() == received);
    };

    while (static_cast<size_t>(d - data) < length) {
        if ((*d & FIFO::CMD_MASK) != FIFO::WRITE) {
            d++;
            continue;
        }

        std::vector<uint8_t> buf;

        uint8_t address = read_data(d, buf);
        CHECK(address == 8);
        uint8_t func = read_data(d, buf);
        switch (func) {
            case 18:
                check_CRC(buf, d);
                processServerStatus(address, ILC::Mode::Standby, 0, 0);
                break;
            case 65:
                currentMode = read_u16(d, buf);
                check_CRC(buf, d);
                processChangeILCMode(address, currentMode);
                break;
            case 100: {
                uint16_t dataCRC = read_u16(d, buf);
                CHECK(dataCRC == 0x0495);
                uint16_t startAddress = read_u16(d, buf);
                CHECK(startAddress == 0);
                uint16_t length = read_u16(d, buf);
                CHECK(length == 67);
                uint16_t crc = read_u16(d, buf);
                CHECK(crc == 0x3BAB);
                check_CRC(buf, d);
                _ackFunction(address, 100);
                break;
            }

            case 101: {
                check_CRC(buf, d);
                _ackFunction(address, func);
                break;
            }

            case 102: {
                uint16_t startAddress = read_u16(d, buf);
                CHECK(startAddress == 0);
                uint16_t length = read_u16(d, buf);
                CHECK(length == 192);
                uint8_t fw[192];
                for (size_t i = 0; i < 192; i++) {
                    fw[i] = read_data(d, buf);
                    CHECK(fw[i] == _pages[i]);
                }
                check_CRC(buf, d);
                processWriteApplicationPage(address);
                break;
            }
            case 103: {
                check_CRC(buf, d);
                processVerifyUserApplication(address, 0);
                break;
            }
            default:
                throw std::runtime_error("Invalid function number " + std::to_string(func));
        }
        _response.writeRxTimestamp(_currentTimestamp++);

        _response.writeRxEndFrame();
    }
}

void TestFPGA::_ackFunction(uint8_t address, uint8_t func) {
    _response.write(address);
    _response.write(func);

    _response.writeCRC();
}
