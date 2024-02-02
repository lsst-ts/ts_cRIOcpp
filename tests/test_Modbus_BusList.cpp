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

#include <Modbus/BusList.h>

using namespace Modbus;

TEST_CASE("Call functions", "[Calls]") {
    BusList buslist;

    buslist.callFunction(123, 17, static_cast<uint8_t>(0xfe), static_cast<uint16_t>(0xffcc),
                         static_cast<float>(22.33));

    buslist.callFunction(43, 86, static_cast<float>(0.1122), static_cast<int16_t>(-1234),
                         static_cast<uint64_t>(0x0123456789abcdef));

    REQUIRE(buslist.size() == 2);

    REQUIRE(buslist[0].size() == 11);
    CHECK(buslist[0][0] == 0x7b);
    CHECK(buslist[0][1] == 0x11);
    CHECK(buslist[0][2] == 0xfe);
    CHECK(buslist[0][3] == 0xff);
    CHECK(buslist[0][4] == 0xcc);
    CHECK(buslist[0][5] == 0x41);
    CHECK(buslist[0][6] == 0xb2);
    CHECK(buslist[0][7] == 0xa3);
    CHECK(buslist[0][8] == 0xd7);
    CHECK(buslist[0][9] == 0x4b);
    CHECK(buslist[0][10] == 0xa7);

    REQUIRE(buslist[1].size() == 18);
    CHECK(buslist[1][0] == 0x2b);
    CHECK(buslist[1][1] == 0x56);
    CHECK(buslist[1][2] == 0x3d);
    CHECK(buslist[1][3] == 0xe5);
    CHECK(buslist[1][4] == 0xc9);
    CHECK(buslist[1][5] == 0x1d);
    CHECK(buslist[1][6] == 0xfb);
    CHECK(buslist[1][7] == 0x2e);
    CHECK(buslist[1][8] == 0x01);
    CHECK(buslist[1][9] == 0x23);
    CHECK(buslist[1][10] == 0x45);
    CHECK(+buslist[1][11] == 0x67);
    CHECK(+buslist[1][12] == 0x89);
    CHECK(+buslist[1][13] == 0xab);
    CHECK(+buslist[1][14] == 0xcd);
    CHECK(+buslist[1][15] == 0xef);
    CHECK(+buslist[1][16] == 0x0f);
    CHECK(+buslist[1][17] == 0xfd);
}
