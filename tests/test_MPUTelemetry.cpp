/*
 * This file is part of LSST cRIOcpp test suite. Tests MPU class.
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

#include <cRIO/MPUTelemetry.h>

using namespace LSST::cRIO;

TEST_CASE("Test MPU telemetry class", "[MPUTelemetry]") {
    uint8_t data[45] = {
            // current IP
            0x00,
            0x01,
            // outputCounter
            0x01,
            0x02,
            0x03,
            0x04,
            0x05,
            0x06,
            0x07,
            0xff,
            // inputCounter
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x02,
            // outputTimeouts
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x03,
            // inputTimeouts
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x04,
            // instructionPointerOnError
            0x00,
            0x05,
            // writeTimeout
            0x00,
            0x06,
            // readTimeout
            0x00,
            0x07,
            // errorStatus
            0x00,
            // errorCode
            0x00,
            0x08,
            // modbusCRC
            0xB6,
            0x35,
    };

    MPUTelemetry mpuTel(data);

    REQUIRE(mpuTel.instructionPointer == 0x01);
    REQUIRE(mpuTel.outputCounter == 0x01020304050607ff);
    REQUIRE(mpuTel.inputCounter == 0x02);
    REQUIRE(mpuTel.outputTimeouts == 0x03);
    REQUIRE(mpuTel.inputTimeouts == 0x04);

    REQUIRE_NOTHROW(mpuTel.checkCRC());

    data[43] = 0xB7;

    MPUTelemetry mpuTelFailed(data);
    REQUIRE_THROWS(mpuTelFailed.checkCRC());
}
