/*
 * This file is part of LSST cRIOcpp test suite. Tests ILC generic functions.
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

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <cmath>

#include <cRIO/ILC.h>

using namespace LSST::cRIO;

class TestILC : public ILC {
public:
    TestILC() {
        serverIDCallCounter = 0;

        responseUniqueID = 0;
        responseILCAppType = 0;
        responseNetworkNodeType = 0;
        responseILCSelectedOptions = 0;
        responseNetworkNodeOptions = 0;
        responseMajorRev = 0;
        responseMinorRev = 0;

        responseMode = 0;
        responseStatus = 0;
        responseFaults = 0;

        newMode = 0;

        responseNewAddress = 0;

        lastReset = 0;
    }

    unsigned int serverIDCallCounter;

    uint64_t responseUniqueID;

    uint8_t responseILCAppType, responseNetworkNodeType, responseILCSelectedOptions,
            responseNetworkNodeOptions, responseMajorRev, responseMinorRev;

    std::string responseFirmwareName;

    uint8_t responseMode;

    uint16_t responseStatus, responseFaults;

    uint16_t newMode;

    uint8_t responseNewAddress;

    uint8_t lastReset;

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override {
        serverIDCallCounter++;
        responseUniqueID = uniqueID;
        responseILCAppType = ilcAppType;
        responseNetworkNodeType = networkNodeType;
        responseILCSelectedOptions = ilcSelectedOptions;
        responseNetworkNodeOptions = networkNodeOptions;
        responseMajorRev = majorRev;
        responseMinorRev = minorRev;
        responseFirmwareName = firmwareName;
    }

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override {
        responseMode = mode;
        responseStatus = status;
        responseFaults = faults;
    }

    void processChangeILCMode(uint8_t address, uint16_t mode) override { newMode = mode; }

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override {
        responseNewAddress = newAddress;
    }

    void processResetServer(uint8_t address) override { lastReset = address; }
};

TEST_CASE("Construct filled buffer", "[ILC]") {
    uint16_t buf[3] = {0x1202, 0x1204, 0x13fe};
    TestILC ilc;
    ilc.setBuffer(buf, 3);

    REQUIRE(ilc.read<uint8_t>() == 0x01);
    REQUIRE(ilc.read<uint8_t>() == 0x02);
    REQUIRE(ilc.read<uint8_t>() == 0xff);
    REQUIRE_THROWS_AS(ilc.read<uint8_t>(), ModbusBuffer::EndOfBuffer);
}

TEST_CASE("Generic functions", "[ILC]") {
    TestILC ilc;

    ilc.reportServerID(125);
    ilc.reportServerStatus(31);
    ilc.resetServer(134);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 125);
    REQUIRE(ilc.read<uint8_t>() == 17);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 835);

    REQUIRE(ilc.read<uint8_t>() == 31);
    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 270);

    REQUIRE(ilc.read<uint8_t>() == 134);
    REQUIRE(ilc.read<uint8_t>() == 107);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 87000);
}

TEST_CASE("CalculateCRC", "[ILC]") {
    TestILC ilc;
    // address
    ilc.write<uint8_t>(123);
    ilc.write<uint8_t>(17);
    ilc.writeCRC();
    ilc.writeEndOfFrame();

    uint16_t* buf = ilc.getBuffer();

    REQUIRE(buf[2] == (0x1200 | (0xe3 << 1)));
    REQUIRE(buf[3] == (0x1200 | (0x4c << 1)));
}

TEST_CASE("CalculateLongCRC", "[ILC]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72};

    TestILC ilc;
    for (auto d : data) ilc.write(d);

    ilc.writeCRC();
    ilc.writeEndOfFrame();

    REQUIRE(ilc.getLength() == 22);

    uint16_t* buf = ilc.getBuffer();

    REQUIRE(buf[19] == (0x1200 | (0xA7 << 1)));
    REQUIRE(buf[20] == (0x1200 | (0x9F << 1)));
}

TEST_CASE("WriteUxx", "[ILC]") {
    TestILC ilc;
    ilc.write<uint8_t>(0x12);
    ilc.write<uint16_t>(0x3456);
    ilc.write<uint32_t>(0x7890abcd);
    ilc.write<uint64_t>(0xAAbbCCddEEff00);

    ilc.writeCRC();

    uint16_t* buf = ilc.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x1224);
    REQUIRE(buf[1] == 0x1268);
    REQUIRE(buf[2] == 0x12ac);
    REQUIRE(buf[3] == 0x12f0);
    REQUIRE(buf[4] == 0x1320);
    REQUIRE(buf[5] == 0x1356);
    REQUIRE(buf[6] == 0x139a);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 0x12);
    REQUIRE(ilc.read<uint16_t>() == 0x3456);
    REQUIRE(ilc.read<uint32_t>() == 0x7890abcd);
    REQUIRE(ilc.read<uint64_t>() == 0xAAbbCCddEEff00);

    REQUIRE_NOTHROW(ilc.checkCRC());
}

TEST_CASE("WriteIxx", "[ILC]") {
    TestILC ilc;
    ilc.write<int8_t>(0x12);
    ilc.write<int16_t>(0x3456);
    ilc.write<int32_t>(0x7890abcd);
    ilc.write<int32_t>(0xf890abcd);
    ilc.writeCRC();

    uint16_t* buf = ilc.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x1224);
    REQUIRE(buf[1] == 0x1268);
    REQUIRE(buf[2] == 0x12ac);
    REQUIRE(buf[3] == 0x12f0);
    REQUIRE(buf[4] == 0x1320);
    REQUIRE(buf[5] == 0x1356);
    REQUIRE(buf[6] == 0x139a);
    REQUIRE(buf[7] == 0x13f0);
    REQUIRE(buf[8] == 0x1320);
    REQUIRE(buf[9] == 0x1356);
    REQUIRE(buf[10] == 0x139a);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 0x12);
    REQUIRE(ilc.read<uint16_t>() == 0x3456);
    REQUIRE(ilc.read<uint32_t>() == 0x7890abcd);
    REQUIRE(ilc.read<uint32_t>() == 0xf890abcd);

    REQUIRE_NOTHROW(ilc.checkCRC());
}

TEST_CASE("WriteSGL", "[ModbusBuffer]") {
    TestILC ilc;
    ilc.write<float>(0.123);
    ilc.write(-6758.1234f);
    ilc.writeCRC();

    uint16_t* buf = ilc.getBuffer();

    REQUIRE(buf[0] == 0x127a);
    REQUIRE(buf[1] == 0x13f6);
    REQUIRE(buf[2] == 0x13ce);
    REQUIRE(buf[3] == 0x12da);

    REQUIRE(buf[4] == 0x138a);
    REQUIRE(buf[5] == 0x13a6);
    REQUIRE(buf[6] == 0x1260);
    REQUIRE(buf[7] == 0x13fa);

    ilc.reset();

    REQUIRE(ilc.read<float>() == 0.123f);
    REQUIRE(ilc.read<float>() == -6758.1234f);
    REQUIRE_NOTHROW(ilc.checkCRC());
}

TEST_CASE("Parse response", "[ILC]") {
    TestILC ilc1;
    TestILC ilc2;

    auto constructCommands = [&ilc1]() {
        ilc1.clear();
        ilc1.reportServerID(132);
    };

    constructCommands();

    // construct response in ILC
    ilc2.write<uint8_t>(132);
    ilc2.write<uint8_t>(17);
    ilc2.write<uint8_t>(15);

    // uniqueID
    ilc2.write<uint8_t>(1);
    ilc2.write<uint8_t>(2);
    ilc2.write<uint8_t>(3);
    ilc2.write<uint8_t>(4);
    ilc2.write<uint8_t>(5);
    ilc2.write<uint8_t>(6);

    ilc2.write<uint8_t>(7);
    ilc2.write<uint8_t>(8);
    ilc2.write<uint8_t>(9);
    ilc2.write<uint8_t>(10);
    ilc2.write<uint8_t>(11);
    ilc2.write<uint8_t>(12);

    ilc2.write<uint8_t>('A');
    ilc2.write<uint8_t>('b');
    ilc2.write<uint8_t>('C');
    ilc2.writeCRC();

    uint16_t* buf = ilc2.getBuffer();

    REQUIRE(buf[18] == (0x1200 | (0xe7 << 1)));
    REQUIRE(buf[19] == (0x1200 | (0xa9 << 1)));

    REQUIRE_NOTHROW(ilc1.processResponse(buf, ilc2.getLength()));

    REQUIRE(ilc1.responseUniqueID == 0x010203040506);
    REQUIRE(ilc1.responseILCAppType == 7);
    REQUIRE(ilc1.responseNetworkNodeType == 8);
    REQUIRE(ilc1.responseILCSelectedOptions == 9);
    REQUIRE(ilc1.responseNetworkNodeOptions == 10);
    REQUIRE(ilc1.responseMajorRev == 11);
    REQUIRE(ilc1.responseMinorRev == 12);
    REQUIRE(ilc1.responseFirmwareName == "AbC");

    // invalid length
    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), 19), ModbusBuffer::EndOfBuffer);

    ilc2.write<uint8_t>(0xff);

    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()), ModbusBuffer::EndOfBuffer);

    // invalid CRC
    ilc2.getBuffer()[18] = 0x1200 | (0xe8 << 1);

    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()), ModbusBuffer::CRCError);
}

TEST_CASE("Change ILC mode response", "[ILC]") {
    TestILC ilc1;
    TestILC ilc2;

    ilc1.changeILCMode(17, 3);

    // construct response in ILC
    ilc2.write<uint8_t>(17);
    ilc2.write<uint8_t>(65);
    ilc2.write<uint16_t>(4);
    ilc2.writeCRC();

    REQUIRE(ilc1.newMode == 0);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.newMode == 4);
}

TEST_CASE("Set Temp ILC Address", "[ILC]") {
    TestILC ilc1;
    TestILC ilc2;

    ilc1.setTempILCAddress(22);

    // construct response in ILC
    ilc2.write<uint8_t>(255);
    ilc2.write<uint8_t>(72);
    ilc2.write<uint8_t>(22);
    ilc2.writeCRC();

    REQUIRE(ilc1.responseNewAddress == 0);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.responseNewAddress == 22);
}

TEST_CASE("Unmatched response", "[ILC]") {
    TestILC ilc1, ilc2;

    auto constructCommands = [&ilc1]() {
        ilc1.clear();
        ilc1.reportServerID(132);
        ilc1.reportServerStatus(140);
    };

    constructCommands();

    // construct response in ILC
    ilc2.write<uint8_t>(132);
    ilc2.write<uint8_t>(17);
    ilc2.write<uint8_t>(15);

    // uniqueID
    ilc2.write<uint8_t>(1);
    ilc2.write<uint8_t>(2);
    ilc2.write<uint8_t>(3);
    ilc2.write<uint8_t>(4);
    ilc2.write<uint8_t>(5);
    ilc2.write<uint8_t>(6);

    ilc2.write<uint8_t>(7);
    ilc2.write<uint8_t>(8);
    ilc2.write<uint8_t>(9);
    ilc2.write<uint8_t>(10);
    ilc2.write<uint8_t>(11);
    ilc2.write<uint8_t>(12);

    ilc2.write<uint8_t>('A');
    ilc2.write<uint8_t>('b');
    ilc2.write<uint8_t>('C');
    ilc2.writeCRC();

    // server status
    ilc2.write<uint8_t>(140);
    ilc2.write<uint8_t>(18);

    ilc2.write<uint8_t>(4);
    ilc2.write<uint16_t>(0x0040 | 0x0002);
    ilc2.write<uint16_t>(0x0004);
    ilc2.writeCRC();

    uint16_t* buf = ilc2.getBuffer();

    REQUIRE(buf[18] == (0x1200 | (0xe7 << 1)));
    REQUIRE(buf[19] == (0x1200 | (0xa9 << 1)));

    REQUIRE(buf[27] == (0x1200 | (0x05 << 1)));
    REQUIRE(buf[28] == (0x1200 | (0xad << 1)));

    REQUIRE_NOTHROW(ilc1.processResponse(buf, ilc2.getLength()));

    REQUIRE(ilc1.responseUniqueID == 0x010203040506);
    REQUIRE(ilc1.responseILCAppType == 7);
    REQUIRE(ilc1.responseNetworkNodeType == 8);
    REQUIRE(ilc1.responseILCSelectedOptions == 9);
    REQUIRE(ilc1.responseNetworkNodeOptions == 10);
    REQUIRE(ilc1.responseMajorRev == 11);
    REQUIRE(ilc1.responseMinorRev == 12);
    REQUIRE(ilc1.responseFirmwareName == "AbC");

    REQUIRE(ilc1.responseMode == 4);
    REQUIRE(ilc1.responseStatus == 0x0042);
    REQUIRE(ilc1.responseFaults == 0x0004);

    TestILC ilc3;
    ilc3.setBuffer(ilc2.getBuffer(), ilc2.getLength());

    // invalid length
    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength() - 1),
                      ModbusBuffer::EndOfBuffer);

    ilc2.write<uint8_t>(0xff);

    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()), ModbusBuffer::EndOfBuffer);

    // missing command
    ilc1.clear();
    ilc1.reportServerID(132);
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()),
                      ModbusBuffer::UnmatchedFunction);

    // invalid address
    ilc1.clear();
    ilc1.reportServerID(132);
    ilc1.reportServerStatus(141);
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()),
                      ModbusBuffer::UnmatchedFunction);

    // missing reply
    constructCommands();
    ilc1.resetServer(121);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength() - 1));
    REQUIRE_THROWS(ilc1.checkCommandedEmpty());

    // recheck
    constructCommands();
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength() - 1));

    // invalid CRC
    ilc2.getBuffer()[18] = 0x1200 | (0xe8 << 1);

    constructCommands();
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()), ModbusBuffer::CRCError);

    // invalid function
    ilc2.getBuffer()[1] = 0x1200 | (1 << 1);
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()),
                      ModbusBuffer::UnmatchedFunction);

    // reset function
    ilc3.write<uint8_t>(17);
    ilc3.write<uint8_t>(107);
    ilc3.writeCRC();

    constructCommands();
    ilc1.resetServer(17);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc3.getBuffer(), ilc3.getLength()));
    REQUIRE(ilc1.lastReset == 17);
}

TEST_CASE("Error response", "[ILC]") {
    TestILC ilc1, ilc2;

    ilc1.reportServerID(103);

    uint8_t buf[3] = {103, 145, 3};
    ilc2.writeBuffer(buf, 3);
    ilc2.writeCRC();

    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()), ILC::Exception);
    REQUIRE(ilc1.responseUniqueID == 0);
}

TEST_CASE("Multiple calls to processResponse", "[ILC]") {
    TestILC ilc1;
    TestILC ilc2;

    ilc1.changeILCMode(17, 3);
    ilc1.reportServerID(18);

    // construct response in ILC
    ilc2.write<uint8_t>(17);
    ilc2.write<uint8_t>(65);
    ilc2.write<uint16_t>(4);
    ilc2.writeCRC();

    REQUIRE(ilc1.newMode == 0);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.newMode == 4);

    ilc2.clear();
    ilc2.write<uint8_t>(18);
    ilc2.write<uint8_t>(17);
    ilc2.write<uint8_t>(15);

    // uniqueID
    ilc2.write<uint8_t>(1);
    ilc2.write<uint8_t>(2);
    ilc2.write<uint8_t>(3);
    ilc2.write<uint8_t>(4);
    ilc2.write<uint8_t>(5);
    ilc2.write<uint8_t>(6);

    ilc2.write<uint8_t>(7);
    ilc2.write<uint8_t>(8);
    ilc2.write<uint8_t>(9);
    ilc2.write<uint8_t>(10);
    ilc2.write<uint8_t>(11);
    ilc2.write<uint8_t>(12);

    ilc2.write<uint8_t>('A');
    ilc2.write<uint8_t>('b');
    ilc2.write<uint8_t>('C');
    ilc2.writeCRC();

    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));

    REQUIRE(ilc1.responseUniqueID == 0x010203040506);
    REQUIRE(ilc1.responseILCAppType == 7);
    REQUIRE(ilc1.responseNetworkNodeType == 8);
    REQUIRE(ilc1.responseILCSelectedOptions == 9);
    REQUIRE(ilc1.responseNetworkNodeOptions == 10);
    REQUIRE(ilc1.responseMajorRev == 11);
    REQUIRE(ilc1.responseMinorRev == 12);
    REQUIRE(ilc1.responseFirmwareName == "AbC");

    REQUIRE_NOTHROW(ilc1.checkCommandedEmpty());
}

TEST_CASE("Response cache management", "[ILC]") {
    TestILC ilc1, ilc2;

    ilc1.reportServerID(18);

    auto constructResponse = [&ilc2](uint8_t address, uint8_t id1) {
        ilc2.clear();
        ilc2.write<uint8_t>(address);
        ilc2.write<uint8_t>(17);
        ilc2.write<uint8_t>(15);

        // uniqueID
        ilc2.write<uint8_t>(1);
        ilc2.write<uint8_t>(2);
        ilc2.write<uint8_t>(3);
        ilc2.write<uint8_t>(4);
        ilc2.write<uint8_t>(5);
        ilc2.write<uint8_t>(6);

        ilc2.write<uint8_t>(7);
        ilc2.write<uint8_t>(8);
        ilc2.write<uint8_t>(9);
        ilc2.write<uint8_t>(10);
        ilc2.write<uint8_t>(11);
        ilc2.write<uint8_t>(12);

        ilc2.write<uint8_t>(id1);
        ilc2.write<uint8_t>('b');
        ilc2.write<uint8_t>('C');
        ilc2.writeCRC();
    };

    constructResponse(18, 'A');

    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 1);

    ilc1.reportServerID(18);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 1);

    constructResponse(11, 'A');

    ilc1.reportServerID(11);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 2);

    ilc1.reportServerID(11);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 2);

    constructResponse(18, 'a');

    ilc1.reportServerID(18);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 3);

    ilc1.reportServerID(18);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 3);

    ilc1.reportServerID(18);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 3);

    constructResponse(11, 'a');

    ilc1.reportServerID(11);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 4);

    ilc1.setAlwaysTrigger(true);

    constructResponse(18, 'a');

    ilc1.reportServerID(18);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 5);

    constructResponse(11, 'a');

    ilc1.reportServerID(11);
    REQUIRE_NOTHROW(ilc1.processResponse(ilc2.getBuffer(), ilc2.getLength()));
    REQUIRE(ilc1.serverIDCallCounter == 6);
}
