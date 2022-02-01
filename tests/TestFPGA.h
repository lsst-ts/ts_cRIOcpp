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

#ifndef __TEST_FPGA__
#define __TEST_FPGA__

#include <cRIO/FPGA.h>
#include <cRIO/PrintILC.h>
#include <cRIO/SimulatedILC.h>

enum FPGAAddress { MODBUS_A_RX = 21, MODBUS_A_TX = 25, HEARTBEAT = 62 };  // namespace FPGAAddress

class TestILC : public LSST::cRIO::PrintILC {
public:
    TestILC(uint8_t bus) : PrintILC(bus) {}

protected:
    void processChangeILCMode(uint8_t address, uint16_t mode) override;
};

class TestFPGA : public LSST::cRIO::FPGA, public LSST::cRIO::PrintILC {
public:
    TestFPGA();

    void initialize() override {}
    void open() override {}
    void close() override {}
    void finalize() override {}
    uint16_t getTxCommand(uint8_t bus) override { return FPGAAddress::MODBUS_A_TX; }
    uint16_t getRxCommand(uint8_t bus) override { return FPGAAddress::MODBUS_A_RX; }
    uint32_t getIrq(uint8_t bus) override { return 1; }
    void writeMPUFIFO(LSST::cRIO::MPU&) override {}
    void readMPUFIFO(LSST::cRIO::MPU&) override {}
    void writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void writeRequestFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) override;
    void waitOnIrqs(uint32_t irqs, uint32_t timeout, uint32_t* triggered = NULL) override {}
    void ackIrqs(uint32_t irqs) override {}

    void setPages(uint8_t* pages) { _pages = pages; }

protected:
    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override;
    void processChangeILCMode(uint8_t address, uint16_t mode) override;
    void processWriteApplicationPage(uint8_t address) override { _ackFunction(address, 102); }
    void processVerifyUserApplication(uint8_t address, uint16_t status) override;

private:
    LSST::cRIO::SimulatedILC _response;

    void _simulateModbus(uint16_t* data, size_t length);
    void _ackFunction(uint8_t address, uint8_t func);

    enum { IDLE, LEN, DATA } _U16ResponseStatus;

    uint8_t* _pages;

    double _currentTimestamp;
};

#endif  //!__TEST_FPGA__
