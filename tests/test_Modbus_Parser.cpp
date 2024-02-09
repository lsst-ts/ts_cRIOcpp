/*
 * This file is part of LSST cRIOcpp test suite. Tests Modbus buffer class.
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

#include <Modbus/Parser.h>

using namespace Modbus;

TEST_CASE("Parser buffer", "[Parsing]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF, 0xBB,
                                 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72, 0xA7, 0x9F};

    Parser parser(data);

    CHECK(parser.address() == 0x81);
    CHECK(parser.func() == 0x11);
    CHECK(parser.read<uint8_t>() == 0x10);
    CHECK(parser.read<uint16_t>() == 0x1234);
    CHECK(parser.read<uint32_t>() == 0x567890AA);
    CHECK(parser.read<uint64_t>() == 0xFFBBCCDDEE115374);
    CHECK(parser.readString(2) == "ar");
    REQUIRE_NOTHROW(parser.checkCRC());
}

TEST_CASE("Small buffer", "[Parsing]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34};

    Parser parser(data);

    CHECK(parser.address() == 0x81);
    CHECK(parser.func() == 0x11);
    CHECK(parser.read<uint8_t>() == 0x10);
    CHECK(parser.read<uint16_t>() == 0x1234);
    REQUIRE_THROWS_AS(parser.read<uint8_t>(), std::out_of_range);
}

TEST_CASE("Invalid CRC", "[Parsing]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF, 0xBB,
                                 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72, 0xA7, 0x9e};

    Parser parser(data);

    CHECK(parser.address() == 0x81);
    CHECK(parser.func() == 0x11);
    CHECK(parser.read<uint8_t>() == 0x10);
    CHECK(parser.read<uint16_t>() == 0x1234);
    CHECK(parser.read<uint32_t>() == 0x567890AA);
    CHECK(parser.read<uint64_t>() == 0xFFBBCCDDEE115374);
    CHECK(parser.readString(2) == "ar");
    REQUIRE_THROWS_AS(parser.checkCRC(), CRCError);
}

TEST_CASE("Small buffer - no CRC", "[Parsing]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72, 0xA7};

    Parser parser(data);

    CHECK(parser.address() == 0x81);
    CHECK(parser.func() == 0x11);
    CHECK(parser.read<uint8_t>() == 0x10);
    CHECK(parser.read<uint16_t>() == 0x1234);
    CHECK(parser.read<uint32_t>() == 0x567890AA);
    CHECK(parser.read<uint64_t>() == 0xFFBBCCDDEE115374);
    CHECK(parser.readString(2) == "ar");
    REQUIRE_THROWS_AS(parser.checkCRC(), std::out_of_range);
}
