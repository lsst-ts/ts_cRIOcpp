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

#include <memory>
#include <cmath>
#include <iostream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <Modbus/Buffer.h>

using namespace Modbus;

TEST_CASE("CalculateCRC", "[Write]") {
    Buffer mbuf;
    // address
    mbuf.write<uint8_t>(123);
    mbuf.write<uint8_t>(17);
    mbuf.writeCRC();

    REQUIRE(mbuf.size() == 4);

    REQUIRE(mbuf[2] == 0xe3);
    REQUIRE(mbuf[3] == 0x4c);
}

TEST_CASE("CalculateLongCRC", "[Write]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72};

    Buffer mbuf;
    for (auto d : data) mbuf.write(d);

    mbuf.writeCRC();

    REQUIRE(mbuf.size() == 21);

    CHECK(mbuf[19] == 0xA7);
    CHECK(mbuf[20] == 0x9F);
}

TEST_CASE("WriteUxx", "[Write]") {
    Buffer mbuf;
    mbuf.write<uint8_t>(0x12);
    mbuf.write<uint16_t>(0x3456);
    mbuf.write<uint32_t>(0x7890abcd);
    mbuf.write<uint64_t>(0xAAbbCCddEEff00);

    mbuf.writeCRC();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(mbuf.size() == 17);
    CHECK(mbuf[0] == 0x12);
    CHECK(mbuf[1] == 0x34);
    CHECK(mbuf[2] == 0x56);
    CHECK(mbuf[3] == 0x78);
    CHECK(mbuf[4] == 0x90);
    CHECK(mbuf[5] == 0xab);
    CHECK(mbuf[6] == 0xcd);
}

TEST_CASE("Call function with arguments", "[Call]") {
    Buffer mbuf;
    mbuf.callFunction(123, 17, static_cast<uint8_t>(0xfe), static_cast<uint16_t>(0xffcc),
                      static_cast<float>(M_PI));

    REQUIRE(mbuf.size() == 11);
    CHECK(mbuf[0] == 123);
    CHECK(mbuf[1] == 17);
    CHECK(mbuf[2] == 0xfe);
    CHECK(mbuf[3] == 0xff);
    CHECK(mbuf[4] == 0xcc);
    CHECK(mbuf[5] == 64);
    CHECK(mbuf[6] == 73);
    CHECK(mbuf[7] == 15);
    CHECK(mbuf[8] == 219);
    CHECK(mbuf[9] == 70);
    CHECK(mbuf[10] == 175);
}

TEST_CASE("Call function with arguments (constructor)", "[Call]") {
    Buffer mbuf(123, 17, static_cast<uint8_t>(0xef), static_cast<uint16_t>(0xffdd), static_cast<float>(M_PI));

    REQUIRE(mbuf.size() == 11);
    CHECK(mbuf[0] == 0x7b);
    CHECK(mbuf[1] == 0x11);
    CHECK(mbuf[2] == 0xef);
    CHECK(mbuf[3] == 0xff);
    CHECK(mbuf[4] == 0xdd);
    CHECK(mbuf[5] == 0x40);
    CHECK(mbuf[6] == 0x49);
    CHECK(mbuf[7] == 0x0f);
    CHECK(mbuf[8] == 0xdb);
    CHECK(mbuf[9] == 0xbb);
    CHECK(mbuf[10] == 0xad);
}
