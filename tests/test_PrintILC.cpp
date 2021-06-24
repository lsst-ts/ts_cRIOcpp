/*
 * This file is part of LSST cRIOcpp test suite. Tests IntelHex class.
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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <cRIO/FPGA.h>
#include <cRIO/IntelHex.h>
#include <cRIO/ModbusBuffer.h>
#include <cRIO/PrintILC.h>

using namespace LSST::cRIO;

// global variables
uint16_t currentMode = 0;  // current address 8 mod
double currentTimestamp = 0;

class TestILC : public PrintILC {
public:
    TestILC(uint8_t bus) : PrintILC(bus) {}

protected:
    void processChangeILCMode(uint8_t address, uint16_t mode) override;
};

void TestILC::processChangeILCMode(uint8_t address, uint16_t mode) {
    REQUIRE(address == 8);
    REQUIRE(mode == currentMode);
}

enum FPGAAddress { MODBUS_A_RX = 21, MODBUS_A_TX = 25, HEARTBEAT = 62 };  // namespace FPGAAddress

class TestFPGA : public FPGA, public PrintILC {
public:
    TestFPGA();

    void initialize() override{};
    void open() override{};
    void close() override{};
    void finalize() override{};
    void writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void writeRequestFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void waitOnIrqs(uint32_t irqs, uint32_t timeout, uint32_t* triggered = NULL) override {}
    void ackIrqs(uint32_t irqs) override {}

    uint16_t getTxCommand(uint8_t bus) override { return FPGAAddress::MODBUS_A_TX; }
    uint16_t getRxCommand(uint8_t bus) override { return FPGAAddress::MODBUS_A_RX; }
    uint32_t getIrq(uint8_t bus) override { return 1; }

protected:
    void processChangeILCMode(uint8_t address, uint16_t mode) override;

private:
    ModbusBuffer response;

    void _simulateModbus(uint16_t* data, size_t length);

    enum { IDLE, LEN, DATA } _U16ResponseStatus;
};

TestFPGA::TestFPGA() : FPGA(fpgaType::SS), PrintILC(1), _U16ResponseStatus(IDLE) {
    response.simulateResponse(true);
}

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
            break;
        case LEN:
            *data = response.getLength();
            _U16ResponseStatus = DATA;
            break;
        case DATA:
            memcpy(data, response.getBuffer(), response.getLength() * 2);
            response.clear();
            _U16ResponseStatus = IDLE;
            break;
    }
}

void TestFPGA::processChangeILCMode(uint8_t address, uint16_t mode) {
    response.write(address);
    response.write<uint8_t>(65);
    response.write(mode);

    response.writeCRC();
}

void TestFPGA::_simulateModbus(uint16_t* data, size_t length) {
    // reply format:
    // 4 bytes (forming uint64_t in low endian) beginning timestamp
    // data received from ILCs (& FIFO::TX_WAIT_LONG_RX)
    // end of frame (FIFO::RX_ENDFRAME)
    // 8 bytes of end timestap (& FIFO::RX_TIMESTAMP)

    response.writeFPGATimestamp(currentTimestamp++);

    ModbusBuffer buf(data, length);
    while (!buf.endOfBuffer()) {
        uint16_t p = buf.peek();
        if ((p & FIFO::CMD_MASK) != FIFO::WRITE) {
            buf.next();
            continue;
        }

        uint8_t address = buf.read<uint8_t>();
        uint8_t func = buf.read<uint8_t>();
        buf.checkCRC();
        switch (func) {
            case 65:
                processChangeILCMode(address, currentMode);
                break;
            // info
            default:
                throw std::runtime_error("Invalid function number " + std::to_string(func));
        }
        response.writeRxTimestamp(currentTimestamp++);

        response.writeRxEndFrame();
    }
}

TEST_CASE("Test program ILC", "[PrintILC]") {
    TestFPGA fpga;

    TestILC ilc(1);

    IntelHex hex;
    hex.load("data/hex1.hex");

    REQUIRE_NOTHROW(ilc.programILC(&fpga, 8, hex));
}
