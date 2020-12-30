/*
 * This file is part of LSST M1M3 SS test suite. Tests Modbus buffer class.
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

#include <cRIO/ModbusBuffer.h>

using namespace LSST::cRIO;

TEST_CASE("CalculateCRC", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    // address
    mbuf.writeU8(123);
    mbuf.writeU8(17);
    mbuf.writeCRC();
    mbuf.writeEndOfFrame();

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(mbuf.readInstructionByte(buf[2]) == 0xe3);
    REQUIRE(mbuf.readInstructionByte(buf[3]) == 0x4c);
}

TEST_CASE("CalculateLongCRC", "[ModbusBuffer]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72};

    ModbusBuffer mbuf;
    for (auto d : data) mbuf.writeU8(d);

    mbuf.writeCRC();
    mbuf.writeEndOfFrame();

    REQUIRE(mbuf.getLength() == 22);

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(mbuf.readInstructionByte(buf[19]) == 0xA7);
    REQUIRE(mbuf.readInstructionByte(buf[20]) == 0x9F);
}

TEST_CASE("WriteUxx", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.writeU8(0x12);
    mbuf.writeU16(0x3456);
    mbuf.writeU32(0x7890abcd);

    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x1224);
    REQUIRE(buf[1] == 0x1268);
    REQUIRE(buf[2] == 0x12ac);
    REQUIRE(buf[3] == 0x12f0);
    REQUIRE(buf[4] == 0x1320);
    REQUIRE(buf[5] == 0x1356);
    REQUIRE(buf[6] == 0x139a);

    REQUIRE(mbuf.readInstructionByte(buf[0]) == 0x12);
    REQUIRE(mbuf.readInstructionByte(buf[1]) == 0x34);
    REQUIRE(mbuf.readInstructionByte(buf[2]) == 0x56);
    REQUIRE(mbuf.readInstructionByte(buf[3]) == 0x78);
    REQUIRE(mbuf.readInstructionByte(buf[4]) == 0x90);
    REQUIRE(mbuf.readInstructionByte(buf[5]) == 0xab);
    REQUIRE(mbuf.readInstructionByte(buf[6]) == 0xcd);

    mbuf.reset();

    REQUIRE(mbuf.readU8() == 0x12);
    REQUIRE(mbuf.readU16() == 0x3456);
    REQUIRE(mbuf.readU32() == 0x7890abcd);

    REQUIRE(mbuf.checkCRC() == true);
}

TEST_CASE("WriteIxx", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.writeI8(0x12);
    mbuf.writeI16(0x3456);
    mbuf.writeI32(0x7890abcd);
    mbuf.writeI32(0xf890abcd);
    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

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

    REQUIRE(mbuf.readInstructionByte(buf[0]) == 0x12);
    REQUIRE(mbuf.readInstructionByte(buf[1]) == 0x34);
    REQUIRE(mbuf.readInstructionByte(buf[2]) == 0x56);
    REQUIRE(mbuf.readInstructionByte(buf[3]) == 0x78);
    REQUIRE(mbuf.readInstructionByte(buf[4]) == 0x90);
    REQUIRE(mbuf.readInstructionByte(buf[5]) == 0xab);
    REQUIRE(mbuf.readInstructionByte(buf[6]) == 0xcd);
    REQUIRE(mbuf.readInstructionByte(buf[7]) == 0xf8);
    REQUIRE(mbuf.readInstructionByte(buf[8]) == 0x90);
    REQUIRE(mbuf.readInstructionByte(buf[9]) == 0xab);
    REQUIRE(mbuf.readInstructionByte(buf[10]) == 0xcd);

    mbuf.reset();

    REQUIRE(mbuf.readU8() == 0x12);
    REQUIRE(mbuf.readU16() == 0x3456);
    REQUIRE(mbuf.readU32() == 0x7890abcd);
    REQUIRE(mbuf.readU32() == 0xf890abcd);

    REQUIRE(mbuf.checkCRC() == true);
}

TEST_CASE("WriteSGL", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.writeSGL(0.123);
    mbuf.writeSGL(-6758.1234);

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(buf[0] == 0x127a);
    REQUIRE(buf[1] == 0x13f6);
    REQUIRE(buf[2] == 0x13ce);
    REQUIRE(buf[3] == 0x12da);

    REQUIRE(buf[4] == 0x138a);
    REQUIRE(buf[5] == 0x13a6);
    REQUIRE(buf[6] == 0x1260);
    REQUIRE(buf[7] == 0x13fa);

    REQUIRE(mbuf.readInstructionByte(buf[0]) == 0x3d);
    REQUIRE(mbuf.readInstructionByte(buf[1]) == 0xfb);
    REQUIRE(mbuf.readInstructionByte(buf[2]) == 0xe7);
    REQUIRE(mbuf.readInstructionByte(buf[3]) == 0x6d);

    REQUIRE(mbuf.readInstructionByte(buf[4]) == 0xc5);
    REQUIRE(mbuf.readInstructionByte(buf[5]) == 0xd3);
    REQUIRE(mbuf.readInstructionByte(buf[6]) == 0x30);
    REQUIRE(mbuf.readInstructionByte(buf[7]) == 0xfd);

    mbuf.reset();

    REQUIRE(mbuf.readSGL() == 0.123f);
    REQUIRE(mbuf.readSGL() == -6758.1234f);
}
