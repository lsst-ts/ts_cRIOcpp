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

uint8_t data[45] = {
        0x00, 0x01,                                      // current IP
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xff,  // outputCounter
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,  // inputCounter
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,  // outputTimeouts
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,  // inputTimeouts
        0x00, 0x05,                                      // instructionPointerOnError
        0x00, 0x06,                                      // writeTimeout
        0x00, 0x07,                                      // readTimeout
        0x00,                                            // errorStatus
        0x00, 0x08,                                      // errorCode
        0xB6, 0x35,                                      // modbusCRC

};

TEST_CASE("Test MPU telemetry class", "[MPUTelemetry]") {
    MPUTelemetry mpuTel(data);

    REQUIRE(mpuTel.instructionPointer == 0x01);
    REQUIRE(mpuTel.outputCounter == 0x01020304050607ff);
    REQUIRE(mpuTel.inputCounter == 0x02);
    REQUIRE(mpuTel.outputTimeouts == 0x03);
    REQUIRE(mpuTel.inputTimeouts == 0x04);
    REQUIRE(mpuTel.instructionPointerOnError == 0x05);
    REQUIRE(mpuTel.writeTimeout == 0x06);
    REQUIRE(mpuTel.readTimeout == 0x07);
    REQUIRE(mpuTel.errorStatus == 0x00);
    REQUIRE(mpuTel.errorCode == 0x08);
    REQUIRE(mpuTel.modbusCRC == mpuTel.calculatedCRC);

    REQUIRE_NOTHROW(mpuTel.checkCRC());

    data[43] = 0xB7;

    MPUTelemetry mpuTelFailed(data);
    REQUIRE_THROWS(mpuTelFailed.checkCRC());
}

struct testMsg_t {
    uint16_t instructionPointer;         /// Current MPU instruction pointer
    uint64_t outputCounter;              /// Current MPU output counter
    uint64_t inputCounter;               /// Current MPU input counter
    uint64_t outputTimeouts;             /// Current MPU output timeouts counter
    uint64_t inputTimeouts;              /// Current MPU input timeouts counter
    uint16_t instructionPointerOnError;  /// Instruction counter on MPU error
    uint16_t writeTimeout;               ///
    uint16_t readTimeout;
    uint8_t errorStatus;
    uint16_t errorCode;
};

TEST_CASE("Test MPU sending", "[MPUTelemetry]") {
    MPUTelemetry mpuTel(data);

    struct testMsg_t testMsg;
    memset(&testMsg, '0', sizeof(testMsg));

    REQUIRE(mpuTel.sendUpdates(&testMsg) == true);
    REQUIRE(mpuTel.sendUpdates(&testMsg) == false);

    testMsg.errorCode = 1;

    REQUIRE(mpuTel.sendUpdates(&testMsg) == true);
    REQUIRE(mpuTel.sendUpdates(&testMsg) == false);
}
