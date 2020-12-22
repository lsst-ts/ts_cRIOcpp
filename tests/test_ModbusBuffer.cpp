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
    mbuf.writeCRC(2);
    mbuf.writeEndOfFrame();

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(mbuf.readInstructionByte(buf[2]) == 0xe3);
    REQUIRE(mbuf.readInstructionByte(buf[3]) == 0x4c);
}

TEST_CASE("CalculateLongCRC", "[ModbusBuffer]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72};

    REQUIRE(ModbusBuffer::calculateCRC(data) == 0x9FA7);

    ModbusBuffer mbuf;
    for (auto d : data) mbuf.writeU8(d);

    mbuf.writeCRC(19);
    mbuf.writeEndOfFrame();

    REQUIRE(mbuf.getIndex() == 22);

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(mbuf.readInstructionByte(buf[19]) == 0xA7);
    REQUIRE(mbuf.readInstructionByte(buf[20]) == 0x9F);
}
