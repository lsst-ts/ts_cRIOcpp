/*
 * This file is part of LSST M1M3 SS test suite. Tests ILC generic functions.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <memory>
#include <cmath>
#include <iostream>

#include <cRIO/ILC.h>

using namespace LSST::cRIO;

class TestILC : public ILC {
public:
    TestILC() {
        responseUniqueID = 0;
        responseILCAppType = 0;
        responseNetworkNodeType = 0;
        responseILCSelectedOptions = 0;
        responseNetworkNodeOptions = 0;
        responseMajorRev = 0;
        responseMinorRev = 0;
    }

    uint64_t responseUniqueID;

    uint8_t responseILCAppType, responseNetworkNodeType, responseILCSelectedOptions,
            responseNetworkNodeOptions, responseMajorRev, responseMinorRev;

    std::string responseFirmwareName;

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override {
        responseUniqueID = uniqueID;
        responseILCAppType = ilcAppType;
        responseNetworkNodeType = networkNodeType;
        responseILCSelectedOptions = ilcSelectedOptions;
        responseNetworkNodeOptions = networkNodeOptions;
        responseMajorRev = majorRev;
        responseMinorRev = minorRev;
        responseFirmwareName = firmwareName;
    }
};

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
    REQUIRE_NOTHROW(ilc.skipRead());

    REQUIRE(ilc.read<uint8_t>() == 31);
    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE_NOTHROW(ilc.skipRead());

    REQUIRE(ilc.read<uint8_t>() == 134);
    REQUIRE(ilc.read<uint8_t>() == 107);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE_NOTHROW(ilc.skipRead());
}

TEST_CASE("Parse response", "[ILC]") {
    TestILC ilc1;
    TestILC ilc2;

    ilc1.reportServerID(132);

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

    REQUIRE_NOTHROW(ilc1.processResponse(buf, 20));

    REQUIRE(ilc1.responseUniqueID == 0x010203040506);
    REQUIRE(ilc1.responseILCAppType == 7);
    REQUIRE(ilc1.responseNetworkNodeType == 8);
    REQUIRE(ilc1.responseILCSelectedOptions == 9);
    REQUIRE(ilc1.responseNetworkNodeOptions == 10);
    REQUIRE(ilc1.responseMajorRev == 11);
    REQUIRE(ilc1.responseMinorRev == 12);
    REQUIRE(ilc1.responseFirmwareName == "AbC");

    // invalid length
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), 19), ModbusBuffer::EndOfBuffer);
    ilc2.write<uint8_t>(0xff);
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), 21), ModbusBuffer::EndOfBuffer);

    // invalid CRC
    buf[18] = 0x1200 | (0xe8 << 1);
    REQUIRE_THROWS_AS(ilc1.processResponse(ilc2.getBuffer(), 20), ModbusBuffer::CRCError);
}
