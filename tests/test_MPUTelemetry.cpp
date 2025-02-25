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

#include <cstring>

#include <catch2/catch.hpp>

#include <cRIO/MPUTelemetry.h>

using namespace LSST::cRIO;

uint8_t data[16] = {
        0x01, 0x02, 0x03, 0x04, 0x10, 0x20, 0x30, 0x40,  // write bytes
        0x05, 0x06, 0x07, 0x08, 0x50, 0x60, 0x70, 0x80   // read HW bytes
};

TEST_CASE("Test MPU telemetry class", "[MPUTelemetry]") {
    MPUTelemetry mpuTel(data);

    CHECK(mpuTel.writeBytes == 0x0102030410203040);
    CHECK(mpuTel.readBytes == 0x0506070850607080);
}
