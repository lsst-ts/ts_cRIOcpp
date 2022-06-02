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

#include <catch2/catch_test_macros.hpp>

#include <cRIO/Timestamp.h>

using namespace LSST::cRIO;

TEST_CASE("fromRaw", "[Timestamp]") {
    REQUIRE(Timestamp::fromRaw(1200000000) == 1.2);
    REQUIRE(Timestamp::fromRaw(98) == 0.000000098);
}

TEST_CASE("toRaw", "[Timestamp]") {
    REQUIRE(Timestamp::toRaw(1.234567891) == 1234567891);
    REQUIRE(Timestamp::toRaw(0.000000001) == 1);
    REQUIRE(Timestamp::toRaw(999999999) == (uint64_t)999999999000000000);
}

TEST_CASE("fromRaw<->toRaw", "[Timestamp]") {
    REQUIRE(Timestamp::toRaw(Timestamp::fromRaw(1)) == 1);
    REQUIRE(Timestamp::toRaw(Timestamp::fromRaw(33567335)) == 33567335);

    REQUIRE(Timestamp::fromRaw(Timestamp::toRaw(1)) == 1);
    REQUIRE(Timestamp::fromRaw(Timestamp::toRaw(33567335.12)) == 33567335.12);
}

TEST_CASE("fromFPGA", "[Timestamp]") {
    REQUIRE(Timestamp::fromFPGA(1200000000) == 1.2);
    REQUIRE(Timestamp::fromFPGA(98) == 0.000000098);
}

TEST_CASE("toFPGA", "[Timestamp]") {
    REQUIRE(Timestamp::toFPGA(1.234567891) == 1234567891);
    REQUIRE(Timestamp::toFPGA(0.000000001) == 1);
    REQUIRE(Timestamp::toFPGA(999999999) == (uint64_t)999999999000000000);
}

TEST_CASE("fromFPGABuffer", "[Timestamp]") {
    uint16_t buf[4];
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x3b9a;
    buf[3] = 0xca00;
    REQUIRE(Timestamp::fromFPGABuffer(buf) == 1.0);
}

TEST_CASE("fromFPGA<->toFPGA", "[Timestamp]") {
    REQUIRE(Timestamp::toFPGA(Timestamp::fromFPGA(1)) == 1);
    REQUIRE(Timestamp::toFPGA(Timestamp::fromFPGA(33567335)) == 33567335);

    REQUIRE(Timestamp::fromFPGA(Timestamp::toFPGA(1)) == 1);
    REQUIRE(Timestamp::fromFPGA(Timestamp::toFPGA(33567335.12)) == 33567335.12);
}
