/*
 * This file is part of LSST cRIOcpp test suite. Tests firmware loading.
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
#include <cRIO/ILC.h>
#include <cRIO/PrintILC.h>
#include <cRIO/SimulatedILC.h>

#include <iostream>
#include <iomanip>

using namespace LSST::cRIO;

// global variables
uint16_t currentMode = 0;  // current address 8 mod
double currentTimestamp = 0;

class TestILC : public PrintILC {
public:
    TestILC(uint8_t bus) : PrintILC(bus) {}
};

class TestFPGA : public FPGA {
public:
    TestFPGA() : FPGA(SS), _call(0) {}
    void initialize() override {}
    void open() override {}
    void close() override {}
    void finalize() override {}
    uint16_t getTxCommand(uint8_t bus) override { return bus + 24; }
    uint16_t getRxCommand(uint8_t bus) override { return bus + 14; }
    uint32_t getIrq(uint8_t bus) override { return 1; }

    void writeMPUFIFO(MPU& mpu) override {}
    void readMPUFIFO(MPU& mpu) override {}

    void writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void writeRequestFIFO(uint16_t* data, size_t length, uint32_t timeout) override {}
    void readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void waitOnIrqs(uint32_t irqs, uint32_t timeout, uint32_t* triggered = NULL) {}
    void ackIrqs(uint32_t irqs) {}

private:
    uint8_t _call;
};

void _printBuffer(uint16_t* data, size_t length, const char* prefix) {
    std::cerr << prefix;
    for (size_t i = 0; i < length; i++) {
        std::cerr << " " << std::hex << std::setfill('0') << std::setw(4) << data[i];
    }
    std::cerr << std::endl;
}

void TestFPGA::writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) {
    _printBuffer(data, length, "C>");
    _call = 0xff & (data[4] >> 1);
}

void TestFPGA::readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) {
    if (length == 1) {
        switch (_call) {
            case 65:
                data[0] = 18;
                break;
            case 103:
                data[0] = 19;
            default:
                data[0] = 17;
        }
    } else {
        SimulatedILC buf;

        buf.writeFPGATimestamp(0);
        buf.write<uint8_t>(18);
        buf.write<uint8_t>(_call);

        switch (_call) {
            case 65:
                buf.write<uint16_t>(0);
                break;
            case 100:
            case 101:
            case 102:
                break;
            case 103:
                buf.write<uint16_t>(0);
                break;
            default:
                std::cerr << "Unknown function " << std::dec << static_cast<int>(_call) << " (0x" << std::hex
                          << _call << ")" << std::endl;
        }

        buf.writeCRC();
        buf.writeRxTimestamp(1000);
        buf.writeEndOfFrame();

        memcpy(data, buf.getBuffer(), buf.getLength() * sizeof(uint16_t));
    }

    _printBuffer(data, length, "R<");
}

TEST_CASE("Test load ILC", "[FirmwareLoad]") {
    IntelHex hex;
    hex.load("data/ILC-3.hex");

    TestILC tILC(1);
    TestFPGA testFPGA;
    REQUIRE_NOTHROW(tILC.programILC(&testFPGA, 18, hex));
}
