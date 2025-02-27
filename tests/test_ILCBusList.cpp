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

#include <memory>
#include <cmath>

#include <catch2/catch_test_macros.hpp>

#include <ILC/ILCBusList.h>
#include <Modbus/Buffer.h>
#include <Modbus/Parser.h>

using namespace ILC;

class TestILC : public ILCBusList {
public:
    TestILC(uint8_t bus) : ILCBusList(bus) {
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

TEST_CASE("Generic functions", "[ILC]") {
    TestILC ilc(1);

    ilc.reportServerID(125);
    ilc.reportServerStatus(31);
    ilc.resetServer(134);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 125);
    CHECK(parser.func() == 17);
    CHECK_NOTHROW(parser.checkCRC());

    parser.parse(ilc[1].buffer);

    CHECK(parser.address() == 31);
    CHECK(parser.func() == 18);
    CHECK_NOTHROW(parser.checkCRC());

    parser.parse(ilc[2].buffer);

    CHECK(parser.address() == 134);
    CHECK(parser.func() == 107);
    CHECK_NOTHROW(parser.checkCRC());
}

TEST_CASE("Parse response", "[ILC]") {
    TestILC ilc(1);

    auto constructCommands = [&ilc]() {
        ilc.reset();
        ilc.reportServerID(132);
    };

    constructCommands();

    // construct response in ILC
    Modbus::Buffer mbuf;

    mbuf.write<uint8_t>(132);
    mbuf.write<uint8_t>(17);
    mbuf.write<uint8_t>(15);

    // uniqueID
    mbuf.write<uint8_t>(1);
    mbuf.write<uint8_t>(2);
    mbuf.write<uint8_t>(3);
    mbuf.write<uint8_t>(4);
    mbuf.write<uint8_t>(5);
    mbuf.write<uint8_t>(6);

    mbuf.write<uint8_t>(7);
    mbuf.write<uint8_t>(8);
    mbuf.write<uint8_t>(9);
    mbuf.write<uint8_t>(10);
    mbuf.write<uint8_t>(11);
    mbuf.write<uint8_t>(12);

    mbuf.write<uint8_t>('A');
    mbuf.write<uint8_t>('b');
    mbuf.write<uint8_t>('C');
    mbuf.writeCRC();

    CHECK(mbuf[18] == 0xe7);
    CHECK(mbuf[19] == 0xa9);

    CHECK_NOTHROW(ilc.parse(mbuf));

    CHECK(ilc.responseUniqueID == 0x010203040506);
    CHECK(ilc.responseILCAppType == 7);
    CHECK(ilc.responseNetworkNodeType == 8);
    CHECK(ilc.responseILCSelectedOptions == 9);
    CHECK(ilc.responseNetworkNodeOptions == 10);
    CHECK(ilc.responseMajorRev == 11);
    CHECK(ilc.responseMinorRev == 12);
    CHECK(ilc.responseFirmwareName == "AbC");

    // invalid length
    constructCommands();
    CHECK_THROWS_AS(ilc.parse(mbuf.data(), 10), std::out_of_range);

    mbuf.write<uint8_t>(0xff);

    ilc.reset();
    CHECK_THROWS_AS(ilc.parse(mbuf), Modbus::LongResponse);

    // invalid CRC
    mbuf[18] = 0xe8;

    constructCommands();
    CHECK_THROWS_AS(ilc.parse(mbuf), Modbus::CRCError);
}

TEST_CASE("Change ILC mode response", "[ILC]") {
    TestILC ilc(1);

    ilc.changeILCMode(17, 3);

    Modbus::Buffer mbuf;

    // construct response in ILC
    mbuf.write<uint8_t>(17);
    mbuf.write<uint8_t>(65);
    mbuf.write<uint16_t>(4);
    mbuf.writeCRC();

    CHECK(ilc.newMode == 0);
    CHECK_NOTHROW(ilc.parse(mbuf));
    CHECK(ilc.newMode == 4);
}

TEST_CASE("Set Temp ILC Address", "[ILC]") {
    TestILC ilc(1);

    ilc.setTempILCAddress(22);

    Modbus::Buffer mbuf;

    // construct response in ILC
    mbuf.write<uint8_t>(255);
    mbuf.write<uint8_t>(72);
    mbuf.write<uint8_t>(22);
    mbuf.writeCRC();

    CHECK(ilc.responseNewAddress == 0);
    CHECK_NOTHROW(ilc.parse(mbuf));
    CHECK(ilc.responseNewAddress == 22);
}

TEST_CASE("Unmatched response", "[ILC]") {
    TestILC ilc(1);

    auto constructCommands = [&ilc]() {
        ilc.reset();
        ilc.clear();
        ilc.reportServerID(132);
        ilc.reportServerStatus(140);
    };

    constructCommands();

    Modbus::Buffer mbuf1;

    // construct response in ILC
    mbuf1.write<uint8_t>(132);
    mbuf1.write<uint8_t>(17);
    mbuf1.write<uint8_t>(15);

    // uniqueID
    mbuf1.write<uint8_t>(1);
    mbuf1.write<uint8_t>(2);
    mbuf1.write<uint8_t>(3);
    mbuf1.write<uint8_t>(4);
    mbuf1.write<uint8_t>(5);
    mbuf1.write<uint8_t>(6);

    mbuf1.write<uint8_t>(7);
    mbuf1.write<uint8_t>(8);
    mbuf1.write<uint8_t>(9);
    mbuf1.write<uint8_t>(10);
    mbuf1.write<uint8_t>(11);
    mbuf1.write<uint8_t>(12);

    mbuf1.write<uint8_t>('A');
    mbuf1.write<uint8_t>('b');
    mbuf1.write<uint8_t>('C');
    mbuf1.writeCRC();

    Modbus::Buffer mbuf2;

    // server status
    mbuf2.write<uint8_t>(140);
    mbuf2.write<uint8_t>(18);

    mbuf2.write<uint8_t>(4);
    mbuf2.write<uint16_t>(0x0040 | 0x0002);
    mbuf2.write<uint16_t>(0x0004);
    mbuf2.writeCRC();

    CHECK(mbuf1[18] == 0xe7);
    CHECK(mbuf1[19] == 0xa9);

    CHECK(mbuf2[7] == 0x05);
    CHECK(mbuf2[8] == 0xad);

    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_NOTHROW(ilc.parse(mbuf2));

    CHECK(ilc.responseUniqueID == 0x010203040506);
    CHECK(ilc.responseILCAppType == 7);
    CHECK(ilc.responseNetworkNodeType == 8);
    CHECK(ilc.responseILCSelectedOptions == 9);
    CHECK(ilc.responseNetworkNodeOptions == 10);
    CHECK(ilc.responseMajorRev == 11);
    CHECK(ilc.responseMinorRev == 12);
    CHECK(ilc.responseFirmwareName == "AbC");

    CHECK(ilc.responseMode == 4);
    CHECK(ilc.responseStatus == 0x0042);
    CHECK(ilc.responseFaults == 0x0004);

    // invalid length
    constructCommands();
    CHECK_THROWS_AS(ilc.parse(mbuf1.data(), mbuf1.size() - 1), std::out_of_range);

    Modbus::Buffer mbuf3(mbuf1);
    mbuf3.write<uint8_t>(0xff);

    constructCommands();
    CHECK_THROWS_AS(ilc.parse(mbuf3), Modbus::LongResponse);

    // missing command
    ilc.reset();
    ilc.clear();
    ilc.reportServerID(132);

    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_THROWS_AS(ilc.parse(mbuf2), std::out_of_range);

    // invalid address
    ilc.reset();
    ilc.clear();
    ilc.reportServerID(132);
    ilc.reportServerStatus(141);
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_THROWS_AS(ilc.parse(mbuf2), Modbus::WrongResponse);

    // missing reply
    ilc.reset();
    ilc.clear();
    ilc.resetServer(121);
    CHECK_THROWS_AS(ilc.parse(mbuf1), Modbus::WrongResponse);
    CHECK_THROWS_AS(ilc.parse(mbuf2), std::out_of_range);

    // recheck correct reply are processed
    constructCommands();
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_NOTHROW(ilc.parse(mbuf2));

    // invalid CRC
    mbuf2[2] = 0xe8;

    constructCommands();
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_THROWS_AS(ilc.parse(mbuf2), Modbus::CRCError);

    // invalid function
    mbuf2[1] = 1;

    constructCommands();
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_THROWS_AS(ilc.parse(mbuf2), Modbus::WrongResponse);

    // reset function
    Modbus::Buffer mbuf4;
    mbuf4.write<uint8_t>(17);
    mbuf4.write<uint8_t>(107);
    mbuf4.writeCRC();

    mbuf2[1] = 18;
    mbuf2[2] = 4;

    constructCommands();
    ilc.resetServer(17);
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK_NOTHROW(ilc.parse(mbuf2));
    CHECK_NOTHROW(ilc.parse(mbuf4));

    CHECK(ilc.lastReset == 17);
}

TEST_CASE("Error response", "[ILC]") {
    TestILC ilc(1);

    ilc.reportServerID(103);

    Modbus::Buffer mbuf(std::vector<uint8_t>({103, 145, 3}));
    mbuf.writeCRC();

    CHECK_THROWS_AS(ilc.parse(mbuf), Modbus::ErrorResponse);
    CHECK(ilc.responseUniqueID == 0);

    Modbus::Buffer mbuf_ok;

    mbuf_ok.write<uint8_t>(103);
    mbuf_ok.write<uint8_t>(17);
    mbuf_ok.write<uint8_t>(15);

    // uniqueID
    mbuf_ok.write<uint8_t>(1);
    mbuf_ok.write<uint8_t>(2);
    mbuf_ok.write<uint8_t>(3);
    mbuf_ok.write<uint8_t>(4);
    mbuf_ok.write<uint8_t>(5);
    mbuf_ok.write<uint8_t>(6);

    mbuf_ok.write<uint8_t>(7);
    mbuf_ok.write<uint8_t>(8);
    mbuf_ok.write<uint8_t>(9);
    mbuf_ok.write<uint8_t>(10);
    mbuf_ok.write<uint8_t>(11);
    mbuf_ok.write<uint8_t>(12);

    mbuf_ok.write<uint8_t>('A');
    mbuf_ok.write<uint8_t>('b');
    mbuf_ok.write<uint8_t>('C');
    mbuf_ok.writeCRC();

    ilc.reset();

    CHECK_NOTHROW(ilc.parse(mbuf_ok));
    CHECK(ilc.responseUniqueID == 0x010203040506);
}

TEST_CASE("Multiple calls to processResponse", "[ILC]") {
    TestILC ilc(1);

    ilc.changeILCMode(17, 3);
    ilc.reportServerID(18);

    // construct response in ILC
    Modbus::Buffer mbuf1;

    mbuf1.write<uint8_t>(17);
    mbuf1.write<uint8_t>(65);
    mbuf1.write<uint16_t>(4);
    mbuf1.writeCRC();

    CHECK(ilc.newMode == 0);
    CHECK_NOTHROW(ilc.parse(mbuf1));
    CHECK(ilc.newMode == 4);

    Modbus::Buffer mbuf2;
    mbuf2.write<uint8_t>(18);
    mbuf2.write<uint8_t>(17);
    mbuf2.write<uint8_t>(15);

    // uniqueID
    mbuf2.write<uint8_t>(1);
    mbuf2.write<uint8_t>(2);
    mbuf2.write<uint8_t>(3);
    mbuf2.write<uint8_t>(4);
    mbuf2.write<uint8_t>(5);
    mbuf2.write<uint8_t>(6);

    mbuf2.write<uint8_t>(7);
    mbuf2.write<uint8_t>(8);
    mbuf2.write<uint8_t>(9);
    mbuf2.write<uint8_t>(10);
    mbuf2.write<uint8_t>(11);
    mbuf2.write<uint8_t>(12);

    mbuf2.write<uint8_t>('A');
    mbuf2.write<uint8_t>('b');
    mbuf2.write<uint8_t>('C');
    mbuf2.writeCRC();

    CHECK_NOTHROW(ilc.parse(mbuf2));

    CHECK(ilc.responseUniqueID == 0x010203040506);
    CHECK(ilc.responseILCAppType == 7);
    CHECK(ilc.responseNetworkNodeType == 8);
    CHECK(ilc.responseILCSelectedOptions == 9);
    CHECK(ilc.responseNetworkNodeOptions == 10);
    CHECK(ilc.responseMajorRev == 11);
    CHECK(ilc.responseMinorRev == 12);
    CHECK(ilc.responseFirmwareName == "AbC");
}

TEST_CASE("Response cache management", "[ILC]") {
    TestILC ilc(1);

    ilc.reportServerID(18);

    auto constructResponse = [](uint8_t address, uint8_t id1) -> std::vector<uint8_t> {
        Modbus::Buffer mbuf;

        mbuf.write<uint8_t>(address);
        mbuf.write<uint8_t>(17);
        mbuf.write<uint8_t>(15);

        // uniqueID
        mbuf.write<uint8_t>(1);
        mbuf.write<uint8_t>(2);
        mbuf.write<uint8_t>(3);
        mbuf.write<uint8_t>(4);
        mbuf.write<uint8_t>(5);
        mbuf.write<uint8_t>(6);

        mbuf.write<uint8_t>(7);
        mbuf.write<uint8_t>(8);
        mbuf.write<uint8_t>(9);
        mbuf.write<uint8_t>(10);
        mbuf.write<uint8_t>(11);
        mbuf.write<uint8_t>(12);

        mbuf.write<uint8_t>(id1);
        mbuf.write<uint8_t>('b');
        mbuf.write<uint8_t>('C');
        mbuf.writeCRC();

        return mbuf;
    };

    CHECK_NOTHROW(ilc.parse(constructResponse(18, 'A')));
    CHECK(ilc.serverIDCallCounter == 1);
    CHECK(ilc.responseFirmwareName == "AbC");

    ilc.reportServerID(19);
    CHECK_NOTHROW(ilc.parse(constructResponse(19, 'A')));
    CHECK(ilc.serverIDCallCounter == 2);
    CHECK(ilc.responseFirmwareName == "AbC");

    ilc.reportServerID(11);
    CHECK_NOTHROW(ilc.parse(constructResponse(11, 'A')));
    CHECK(ilc.serverIDCallCounter == 3);
    CHECK(ilc.responseFirmwareName == "AbC");

    ilc.reportServerID(12);
    CHECK_NOTHROW(ilc.parse(constructResponse(12, 'A')));
    CHECK(ilc.serverIDCallCounter == 4);
    CHECK(ilc.responseFirmwareName == "AbC");

    ilc.reportServerID(20);
    CHECK_NOTHROW(ilc.parse(constructResponse(20, 'a')));
    CHECK(ilc.serverIDCallCounter == 5);
    CHECK(ilc.responseFirmwareName == "abC");

    CHECK_THROWS_AS(ilc.parse(constructResponse(20, 'a')), std::out_of_range);

    ilc.reportServerID(21);
    CHECK_NOTHROW(ilc.parse(constructResponse(21, 'x')));
    CHECK(ilc.serverIDCallCounter == 6);
    CHECK(ilc.responseFirmwareName == "xbC");

    ilc.reportServerID(22);
    CHECK_NOTHROW(ilc.parse(constructResponse(22, 'h')));
    CHECK(ilc.serverIDCallCounter == 7);
    CHECK(ilc.responseFirmwareName == "hbC");

    ilc.reportServerID(22);
    CHECK_NOTHROW(ilc.parse(constructResponse(22, 'h')));
    CHECK(ilc.serverIDCallCounter == 8);
    CHECK(ilc.responseFirmwareName == "hbC");
}
