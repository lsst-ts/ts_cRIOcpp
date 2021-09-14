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
    REQUIRE(commands[14] == MPUCommands::CHECK_CRC);

    std::vector<uint8_t> res = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8, 9,
                                10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));

    REQUIRE(mpu.getRegister(3) == 0x0102);
    REQUIRE(mpu.getRegister(4) == 0x0304);
    REQUIRE(mpu.getRegister(5) == 0x0506);
    REQUIRE(mpu.getRegister(6) == 0x0708);
    REQUIRE(mpu.getRegister(7) == 0x090a);
    REQUIRE(mpu.getRegister(8) == 0x0b0c);
    REQUIRE(mpu.getRegister(9) == 0x0d0e);
    REQUIRE(mpu.getRegister(10) == 0x0f10);
    REQUIRE(mpu.getRegister(11) == 0x1112);
    REQUIRE(mpu.getRegister(12) == 0x1314);
}

TEST_CASE("Test MPU preset holding registers", "[MPU]") {
    MPU mpu(17);

    std::vector<uint16_t> regs = {0x0102, 0x0304};
    mpu.presetHoldingRegisters(0x1718, regs.data(), regs.size(), 102);

    uint16_t* commands = mpu.getCommands();

    REQUIRE(commands[0] == MPUCommands::WRITE);
    REQUIRE(commands[1] == 9 + 2 * regs.size());
    REQUIRE(commands[2] == 17);
    REQUIRE(commands[3] == 16);
    REQUIRE(commands[4] == 0x17);
    REQUIRE(commands[5] == 0x18);
    REQUIRE(commands[6] == 0);
    REQUIRE(commands[7] == regs.size());
    REQUIRE(commands[8] == regs.size() * 2);
    REQUIRE(commands[9] == 0x01);
    REQUIRE(commands[10] == 0x02);
    REQUIRE(commands[11] == 0x03);
    REQUIRE(commands[12] == 0x04);
    REQUIRE(commands[13] == 0xED);
    REQUIRE(commands[14] == 0x3A);
    REQUIRE(commands[15] == MPUCommands::WAIT_MS);
    REQUIRE(commands[16] == 102);
    REQUIRE(commands[17] == MPUCommands::READ);
    REQUIRE(commands[18] == 8);
    REQUIRE(commands[19] == MPUCommands::CHECK_CRC);

    std::vector<uint8_t> res = {17, 16, 0x17, 0x18, 0, 2, 0xC6, 0xEB};

    // REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));
}
