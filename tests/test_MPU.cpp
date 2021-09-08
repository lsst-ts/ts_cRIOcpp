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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <cRIO/MPU.h>

using namespace LSST::cRIO;

TEST_CASE("Test MPU read holding registers", "[MPU]") {
    MPU mpu(12);
    mpu.readHoldingRegisters(3, 10, 101);

    uint16_t* commands = mpu.getCommands();

    REQUIRE(commands[0] == MPUCommands::WRITE);
    REQUIRE(commands[1] == 8);
    REQUIRE(commands[2] == 12);
    REQUIRE(commands[3] == 3);
    REQUIRE(commands[4] == 0);
    REQUIRE(commands[5] == 3);
    REQUIRE(commands[6] == 0);
    REQUIRE(commands[7] == 10);
    REQUIRE(commands[8] == 0x34);
    REQUIRE(commands[9] == 0xD0);
    REQUIRE(commands[10] == MPUCommands::WAIT_MS);
    REQUIRE(commands[11] == 101);
    REQUIRE(commands[12] == MPUCommands::READ);
    REQUIRE(commands[13] == 25);
}
