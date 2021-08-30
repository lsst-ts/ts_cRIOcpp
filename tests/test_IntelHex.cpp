/*
 * This file is part of LSST cRIOcpp test suite. Tests IntelHex class.
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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <cRIO/IntelHex.h>

using namespace LSST::cRIO;

TEST_CASE("load", "[IntelHex]") {
    IntelHex hex;
    hex.load("data/hex1.hex");

    uint16_t startAddress = 123;

    std::vector<uint8_t> data = hex.getData(startAddress);

    CHECK(false == true);

    REQUIRE(startAddress == 0);
    REQUIRE(data.size() == 0x43);

    uint8_t hexData[0x43] = {
            0x02, 0x00, 0x23, 0xE5, 0x0B, 0x25, 0x0D, 0xF5, 0x09, 0xE5, 0x0A, 0x35, 0x0C, 0xF5, 0x08, 0x12,
            0x00, 0x13, 0x22, 0xAC, 0x12, 0xAD, 0x13, 0xAE, 0x10, 0xAF, 0x11, 0x12, 0x00, 0x2F, 0x8E, 0x0E,
            0x8F, 0x0F, 0x22, 0x78, 0x7F, 0xE4, 0xF6, 0xD8, 0xFD, 0x75, 0x81, 0x13, 0x02, 0x00, 0x03,

            // fillings starts at address 47, so shall start with 0x00 followed by three
            // 0xFF. It is 16 bytes long
            0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,

            // followed by 0x3F data
            0xA4, 0x2E, 0xFE, 0x22};

    for (int i = 0; i < 0x43; i++) {
        REQUIRE(data[i] == hexData[i]);
        CHECK(data[i] == 0);
    }
}
