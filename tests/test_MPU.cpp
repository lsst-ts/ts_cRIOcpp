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

#include <cRIO/MPU.h>

using namespace LSST::cRIO;

TEST_CASE("Test MPU read input status", "[MPU]") {
    MPU mpu(1, 0x11);
    mpu.readInputStatus(0x00C4, 0x0016, 108);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 15);

    REQUIRE(commands[0] == MPUCommands::WRITE);
    REQUIRE(commands[1] == 8);
    REQUIRE(commands[2] == 0x11);
    REQUIRE(commands[3] == 0x02);
    REQUIRE(commands[4] == 0x00);
    REQUIRE(commands[5] == 0xC4);
    REQUIRE(commands[6] == 0x00);
    REQUIRE(commands[7] == 0x16);
    REQUIRE(commands[8] == 0xBA);
    REQUIRE(commands[9] == 0xA9);
    REQUIRE(commands[10] == MPUCommands::WAIT_MS);
    REQUIRE(commands[11] == 108);
    REQUIRE(commands[12] == MPUCommands::READ);
    REQUIRE(commands[13] == 8);
    REQUIRE(commands[14] == MPUCommands::CHECK_CRC);

    std::vector<uint16_t> res = {0x11, 0x02, 0x03, 0xAC, 0xDB, 0x35, 0x20, 0x18};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));

    // input status is not 10001 offseted
    REQUIRE_THROWS(mpu.getInputStatus(195));

    // first byte
    REQUIRE(mpu.getInputStatus(196) == false);
    REQUIRE(mpu.getInputStatus(197) == false);
    REQUIRE(mpu.getInputStatus(198) == true);
    REQUIRE(mpu.getInputStatus(199) == true);

    REQUIRE(mpu.getInputStatus(200) == false);
    REQUIRE(mpu.getInputStatus(201) == true);
    REQUIRE(mpu.getInputStatus(202) == false);
    REQUIRE(mpu.getInputStatus(203) == true);

    // second byte
    REQUIRE(mpu.getInputStatus(204) == true);
    REQUIRE(mpu.getInputStatus(205) == true);
    REQUIRE(mpu.getInputStatus(206) == false);
    REQUIRE(mpu.getInputStatus(207) == true);

    REQUIRE(mpu.getInputStatus(208) == true);
    REQUIRE(mpu.getInputStatus(209) == false);
    REQUIRE(mpu.getInputStatus(210) == true);
    REQUIRE(mpu.getInputStatus(211) == true);

    // third byte
    REQUIRE(mpu.getInputStatus(212) == true);
    REQUIRE(mpu.getInputStatus(213) == false);
    REQUIRE(mpu.getInputStatus(214) == true);
    REQUIRE(mpu.getInputStatus(215) == false);

    REQUIRE(mpu.getInputStatus(216) == true);
    REQUIRE(mpu.getInputStatus(217) == true);
    REQUIRE_THROWS(mpu.getInputStatus(218));
    REQUIRE_THROWS(mpu.getInputStatus(219));
}

TEST_CASE("Test MPU read holding registers", "[MPU]") {
    MPU mpu(1, 12);
    mpu.readHoldingRegisters(3, 10, 101);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 16);

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
    REQUIRE(commands[14] == MPUCommands::OUTPUT);
    REQUIRE(commands[15] == MPUCommands::EXIT);

    std::vector<uint16_t> res = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));

    REQUIRE_THROWS(mpu.getRegister(1));
    REQUIRE_THROWS(mpu.getRegister(2));

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

    REQUIRE_THROWS(mpu.getRegister(13));
    REQUIRE_THROWS(mpu.getRegister(14));
}

TEST_CASE("Test MPU reading multiple registers - failed response", "[MPU]") {
    MPU mpu(1, 12);
    mpu.readHoldingRegisters(3, 10, 101);
    mpu.readHoldingRegisters(103, 10, 101);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 31);

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
    REQUIRE(commands[14] == MPUCommands::OUTPUT);

    REQUIRE(commands[15] == MPUCommands::WRITE);
    REQUIRE(commands[16] == 8);
    REQUIRE(commands[17] == 12);
    REQUIRE(commands[18] == 3);
    REQUIRE(commands[19] == 0);
    REQUIRE(commands[20] == 103);
    REQUIRE(commands[21] == 0);
    REQUIRE(commands[22] == 10);
    REQUIRE(commands[23] == 0x75);
    REQUIRE(commands[24] == 0x0F);
    REQUIRE(commands[25] == MPUCommands::WAIT_MS);
    REQUIRE(commands[26] == 101);
    REQUIRE(commands[27] == MPUCommands::READ);
    REQUIRE(commands[28] == 25);
    REQUIRE(commands[29] == MPUCommands::OUTPUT);

    REQUIRE(commands[30] == MPUCommands::EXIT);

    std::vector<uint16_t> res1 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.processResponse(res1.data(), res1.size()));

    REQUIRE_THROWS(mpu.getRegister(1));
    REQUIRE_THROWS(mpu.getRegister(2));

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

    REQUIRE_THROWS(mpu.getRegister(13));
    REQUIRE_THROWS(mpu.getRegister(14));

    REQUIRE_THROWS(mpu.getRegister(103));
    REQUIRE_THROWS(mpu.getRegister(104));

    REQUIRE_THROWS(mpu.checkCommandedEmpty());
}

TEST_CASE("Test MPU reading multiple registers - successful response", "[MPU]") {
    MPU mpu(1, 12);
    mpu.readHoldingRegisters(3, 10, 101);
    mpu.readHoldingRegisters(103, 10, 101);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 31);

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
    REQUIRE(commands[14] == MPUCommands::OUTPUT);

    REQUIRE(commands[15] == MPUCommands::WRITE);
    REQUIRE(commands[16] == 8);
    REQUIRE(commands[17] == 12);
    REQUIRE(commands[18] == 3);
    REQUIRE(commands[19] == 0);
    REQUIRE(commands[20] == 103);
    REQUIRE(commands[21] == 0);
    REQUIRE(commands[22] == 10);
    REQUIRE(commands[23] == 0x75);
    REQUIRE(commands[24] == 0x0F);
    REQUIRE(commands[25] == MPUCommands::WAIT_MS);
    REQUIRE(commands[26] == 101);
    REQUIRE(commands[27] == MPUCommands::READ);
    REQUIRE(commands[28] == 25);
    REQUIRE(commands[29] == MPUCommands::OUTPUT);

    REQUIRE(commands[30] == MPUCommands::EXIT);

    std::vector<uint16_t> res1 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.processResponse(res1.data(), res1.size()));

    REQUIRE_THROWS(mpu.getRegister(1));
    REQUIRE_THROWS(mpu.getRegister(2));

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

    REQUIRE_THROWS(mpu.getRegister(13));
    REQUIRE_THROWS(mpu.getRegister(14));

    REQUIRE_THROWS(mpu.getRegister(103));
    REQUIRE_THROWS(mpu.getRegister(104));

    std::vector<uint16_t> res2 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.processResponse(res2.data(), res2.size()));

    REQUIRE_THROWS(mpu.getRegister(1));
    REQUIRE_THROWS(mpu.getRegister(2));

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

    REQUIRE_THROWS(mpu.getRegister(13));
    REQUIRE_THROWS(mpu.getRegister(14));

    REQUIRE(mpu.getRegister(103) == 0x0102);
    REQUIRE(mpu.getRegister(104) == 0x0304);
    REQUIRE(mpu.getRegister(105) == 0x0506);
    REQUIRE(mpu.getRegister(106) == 0x0708);
    REQUIRE(mpu.getRegister(107) == 0x090a);
    REQUIRE(mpu.getRegister(108) == 0x0b0c);
    REQUIRE(mpu.getRegister(109) == 0x0d0e);
    REQUIRE(mpu.getRegister(110) == 0x0f10);
    REQUIRE(mpu.getRegister(111) == 0x1112);
    REQUIRE(mpu.getRegister(112) == 0x1314);

    REQUIRE_THROWS(mpu.getRegister(113));
    REQUIRE_THROWS(mpu.getRegister(114));

    REQUIRE_NOTHROW(mpu.checkCommandedEmpty());
}

TEST_CASE("Test MPU preset holding register", "[MPU]") {
    MPU mpu(5, 0x11);

    mpu.presetHoldingRegister(0x0001, 0x0003, 102);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 17);

    REQUIRE(commands[0] == MPUCommands::WRITE);
    REQUIRE(commands[1] == 8);
    REQUIRE(commands[2] == 0x11);
    REQUIRE(commands[3] == 0x06);
    REQUIRE(commands[4] == 0x00);
    REQUIRE(commands[5] == 0x01);
    REQUIRE(commands[6] == 0x00);
    REQUIRE(commands[7] == 0x03);
    REQUIRE(commands[8] == 0x9A);
    REQUIRE(commands[9] == 0x9B);
    REQUIRE(commands[10] == MPUCommands::WAIT_MS);
    REQUIRE(commands[11] == 102);
    REQUIRE(commands[12] == MPUCommands::READ);
    REQUIRE(commands[13] == 8);
    REQUIRE(commands[14] == MPUCommands::OUTPUT);
    REQUIRE(commands[15] == MPUCommands::CHECK_CRC);
    REQUIRE(commands[16] == MPUCommands::EXIT);

    std::vector<uint16_t> res = {0x11, 0x06, 0x00, 0x01, 0x00, 0x03, 0x9A, 0x9B};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));
}

TEST_CASE("Test MPU preset holding registers", "[MPU]") {
    MPU mpu(5, 17);

    std::vector<uint16_t> regs = {0x0102, 0x0304};
    mpu.presetHoldingRegisters(0x1718, regs.data(), regs.size(), 102);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 20);

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

    std::vector<uint16_t> res = {17, 16, 0x17, 0x18, 0, 2, 0xC6, 0xEB};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));
}

TEST_CASE("Test MPU preset holding registers by simplymodbus.ca", "[MPU]") {
    MPU mpu(5, 0x11);

    std::vector<uint16_t> regs = {0x000A, 0x0102};
    mpu.presetHoldingRegisters(0x0001, regs.data(), regs.size(), 102);

    auto commands = mpu.getCommandVector();

    REQUIRE(commands.size() == 20);

    REQUIRE(commands[0] == MPUCommands::WRITE);
    REQUIRE(commands[1] == 9 + 2 * regs.size());
    REQUIRE(commands[2] == 0x11);
    REQUIRE(commands[3] == 0x10);
    REQUIRE(commands[4] == 0x00);
    REQUIRE(commands[5] == 0x01);
    REQUIRE(commands[6] == 0x00);
    REQUIRE(commands[7] == 0x02);
    REQUIRE(commands[8] == 0x04);
    REQUIRE(commands[9] == 0x00);
    REQUIRE(commands[10] == 0x0A);
    REQUIRE(commands[11] == 0x01);
    REQUIRE(commands[12] == 0x02);
    REQUIRE(commands[13] == 0xC6);
    REQUIRE(commands[14] == 0xF0);
    REQUIRE(commands[15] == MPUCommands::WAIT_MS);
    REQUIRE(commands[16] == 102);
    REQUIRE(commands[17] == MPUCommands::READ);
    REQUIRE(commands[18] == 8);
    REQUIRE(commands[19] == MPUCommands::CHECK_CRC);

    std::vector<uint16_t> res = {0x11, 0x10, 0x00, 0x01, 0x00, 0x02, 0x12, 0x98};

    REQUIRE_NOTHROW(mpu.processResponse(res.data(), res.size()));
}
