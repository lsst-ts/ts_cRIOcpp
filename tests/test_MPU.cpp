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

#include <TestFPGA.h>

using namespace LSST::cRIO;

class TestMPU : public MPU {
public:
    TestMPU(uint8_t bus) : MPU(bus) {}

    void loopWrite() override {}
    void loopRead(bool timedout) override {}
};

TEST_CASE("Test MPU read input status", "[MPU]") {
    TestMPU mpu(1);
    TestFPGA MPUFPGA;

    mpu.readInputStatus(0x11, 0x00C4, 0x0016, 108);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 15);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 8);
    CHECK(commands[2] == 0x11);
    CHECK(commands[3] == 0x02);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0xC4);
    CHECK(commands[6] == 0x00);
    CHECK(commands[7] == 0x16);
    CHECK(commands[8] == 0xBA);
    CHECK(commands[9] == 0xA9);
    CHECK(commands[10] == MPUCommands::MPU_READ_MS);
    CHECK(commands[11] == 8);
    CHECK(commands[12] == 0);
    CHECK(commands[13] == 108);
    CHECK(commands[14] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res = {0x11, 0x02, 0x03, 0xAC, 0xDB, 0x35, 0x20, 0x18};

    REQUIRE_NOTHROW(mpu.parse(res));

    // input status is not 10001 offseted
    CHECK_THROWS(mpu.getInputStatus(0x11, 195));

    // first byte
    CHECK(mpu.getInputStatus(0x11, 196) == false);
    CHECK(mpu.getInputStatus(0x11, 197) == false);
    CHECK(mpu.getInputStatus(0x11, 198) == true);
    CHECK(mpu.getInputStatus(0x11, 199) == true);

    CHECK(mpu.getInputStatus(0x11, 200) == false);
    CHECK(mpu.getInputStatus(0x11, 201) == true);
    CHECK(mpu.getInputStatus(0x11, 202) == false);
    CHECK(mpu.getInputStatus(0x11, 203) == true);

    // second byte
    CHECK(mpu.getInputStatus(0x11, 204) == true);
    CHECK(mpu.getInputStatus(0x11, 205) == true);
    CHECK(mpu.getInputStatus(0x11, 206) == false);
    CHECK(mpu.getInputStatus(0x11, 207) == true);

    CHECK(mpu.getInputStatus(0x11, 208) == true);
    CHECK(mpu.getInputStatus(0x11, 209) == false);
    CHECK(mpu.getInputStatus(0x11, 210) == true);
    CHECK(mpu.getInputStatus(0x11, 211) == true);

    // third byte
    CHECK(mpu.getInputStatus(0x11, 212) == true);
    CHECK(mpu.getInputStatus(0x11, 213) == false);
    CHECK(mpu.getInputStatus(0x11, 214) == true);
    CHECK(mpu.getInputStatus(0x11, 215) == false);

    CHECK(mpu.getInputStatus(0x11, 216) == true);
    CHECK(mpu.getInputStatus(0x11, 217) == true);
    CHECK_THROWS(mpu.getInputStatus(0x11, 218));
    CHECK_THROWS(mpu.getInputStatus(0x11, 219));
}

TEST_CASE("Test MPU read holding registers", "[MPU]") {
    TestMPU mpu(1);
    TestFPGA MPUFPGA;

    mpu.readHoldingRegisters(12, 3, 10, 101);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 15);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 8);
    CHECK(commands[2] == 12);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 3);
    CHECK(commands[6] == 0);
    CHECK(commands[7] == 10);
    CHECK(commands[8] == 0x34);
    CHECK(commands[9] == 0xD0);
    CHECK(commands[10] == MPUCommands::MPU_READ_MS);
    CHECK(commands[11] == 25);
    CHECK(commands[12] == 0);
    CHECK(commands[13] == 101);
    CHECK(commands[14] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.parse(res));

    CHECK_THROWS(mpu.getRegister(12, 1));
    CHECK_THROWS(mpu.getRegister(12, 2));

    CHECK(mpu.getRegister(12, 3) == 0x0102);
    CHECK(mpu.getRegister(12, 4) == 0x0304);
    CHECK(mpu.getRegister(12, 5) == 0x0506);
    CHECK(mpu.getRegister(12, 6) == 0x0708);
    CHECK(mpu.getRegister(12, 7) == 0x090a);
    CHECK(mpu.getRegister(12, 8) == 0x0b0c);
    CHECK(mpu.getRegister(12, 9) == 0x0d0e);
    CHECK(mpu.getRegister(12, 10) == 0x0f10);
    CHECK(mpu.getRegister(12, 11) == 0x1112);
    CHECK(mpu.getRegister(12, 12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(12, 13));
    CHECK_THROWS(mpu.getRegister(12, 14));
}

TEST_CASE("Test MPU reading multiple registers - failed response", "[MPU]") {
    TestMPU mpu(1);
    TestFPGA MPUFPGA;

    mpu.readHoldingRegisters(12, 3, 10, 101);
    mpu.readHoldingRegisters(12, 103, 10, 101);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 29);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 8);
    CHECK(commands[2] == 12);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 3);
    CHECK(commands[6] == 0);
    CHECK(commands[7] == 10);
    CHECK(commands[8] == 0x34);
    CHECK(commands[9] == 0xD0);
    CHECK(commands[10] == MPUCommands::MPU_READ_MS);
    CHECK(commands[11] == 25);
    CHECK(commands[12] == 0);
    CHECK(commands[13] == 101);

    CHECK(commands[14] == MPUCommands::MPU_WRITE);
    CHECK(commands[15] == 8);
    CHECK(commands[16] == 12);
    CHECK(commands[17] == 3);
    CHECK(commands[18] == 0);
    CHECK(commands[19] == 103);
    CHECK(commands[20] == 0);
    CHECK(commands[21] == 10);
    CHECK(commands[22] == 0x75);
    CHECK(commands[23] == 0x0F);
    CHECK(commands[24] == MPUCommands::MPU_READ_MS);
    CHECK(commands[25] == 25);
    CHECK(commands[26] == 0);
    CHECK(commands[27] == 101);

    CHECK(commands[28] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res1 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.parse(res1));

    CHECK_THROWS(mpu.getRegister(12, 1));
    CHECK_THROWS(mpu.getRegister(12, 2));

    CHECK(mpu.getRegister(12, 3) == 0x0102);
    CHECK(mpu.getRegister(12, 4) == 0x0304);
    CHECK(mpu.getRegister(12, 5) == 0x0506);
    CHECK(mpu.getRegister(12, 6) == 0x0708);
    CHECK(mpu.getRegister(12, 7) == 0x090a);
    CHECK(mpu.getRegister(12, 8) == 0x0b0c);
    CHECK(mpu.getRegister(12, 9) == 0x0d0e);
    CHECK(mpu.getRegister(12, 10) == 0x0f10);
    CHECK(mpu.getRegister(12, 11) == 0x1112);
    CHECK(mpu.getRegister(12, 12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(12, 13));
    CHECK_THROWS(mpu.getRegister(12, 14));

    CHECK_THROWS(mpu.getRegister(12, 103));
    CHECK_THROWS(mpu.getRegister(12, 104));
}

TEST_CASE("Test MPU reading multiple registers - successful response", "[MPU]") {
    TestMPU mpu(1);
    TestFPGA MPUFPGA;

    mpu.readHoldingRegisters(12, 3, 10, 101);
    mpu.readHoldingRegisters(12, 103, 10, 101);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 29);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 8);
    CHECK(commands[2] == 12);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 3);
    CHECK(commands[6] == 0);
    CHECK(commands[7] == 10);
    CHECK(commands[8] == 0x34);
    CHECK(commands[9] == 0xD0);
    CHECK(commands[10] == MPUCommands::MPU_READ_MS);
    CHECK(commands[11] == 25);
    CHECK(commands[12] == 0);
    CHECK(commands[13] == 101);

    CHECK(commands[14] == MPUCommands::MPU_WRITE);
    CHECK(commands[15] == 8);
    CHECK(commands[16] == 12);
    CHECK(commands[17] == 3);
    CHECK(commands[18] == 0);
    CHECK(commands[19] == 103);
    CHECK(commands[20] == 0);
    CHECK(commands[21] == 10);
    CHECK(commands[22] == 0x75);
    CHECK(commands[23] == 0x0F);
    CHECK(commands[24] == MPUCommands::MPU_READ_MS);
    CHECK(commands[25] == 25);
    CHECK(commands[26] == 0);
    CHECK(commands[27] == 101);

    CHECK(commands[28] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res1 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.parse(res1));

    CHECK_THROWS(mpu.getRegister(12, 1));
    CHECK_THROWS(mpu.getRegister(12, 2));

    CHECK(mpu.getRegister(12, 3) == 0x0102);
    CHECK(mpu.getRegister(12, 4) == 0x0304);
    CHECK(mpu.getRegister(12, 5) == 0x0506);
    CHECK(mpu.getRegister(12, 6) == 0x0708);
    CHECK(mpu.getRegister(12, 7) == 0x090a);
    CHECK(mpu.getRegister(12, 8) == 0x0b0c);
    CHECK(mpu.getRegister(12, 9) == 0x0d0e);
    CHECK(mpu.getRegister(12, 10) == 0x0f10);
    CHECK(mpu.getRegister(12, 11) == 0x1112);
    CHECK(mpu.getRegister(12, 12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(12, 13));
    CHECK_THROWS(mpu.getRegister(12, 14));

    CHECK_THROWS(mpu.getRegister(12, 103));
    CHECK_THROWS(mpu.getRegister(12, 104));

    std::vector<uint8_t> res2 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    REQUIRE_NOTHROW(mpu.parse(res2));

    CHECK_THROWS(mpu.getRegister(12, 1));
    CHECK_THROWS(mpu.getRegister(12, 2));

    CHECK(mpu.getRegister(12, 3) == 0x0102);
    CHECK(mpu.getRegister(12, 4) == 0x0304);
    CHECK(mpu.getRegister(12, 5) == 0x0506);
    CHECK(mpu.getRegister(12, 6) == 0x0708);
    CHECK(mpu.getRegister(12, 7) == 0x090a);
    CHECK(mpu.getRegister(12, 8) == 0x0b0c);
    CHECK(mpu.getRegister(12, 9) == 0x0d0e);
    CHECK(mpu.getRegister(12, 10) == 0x0f10);
    CHECK(mpu.getRegister(12, 11) == 0x1112);
    CHECK(mpu.getRegister(12, 12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(12, 13));
    CHECK_THROWS(mpu.getRegister(12, 14));

    CHECK(mpu.getRegister(12, 103) == 0x0102);
    CHECK(mpu.getRegister(12, 104) == 0x0304);
    CHECK(mpu.getRegister(12, 105) == 0x0506);
    CHECK(mpu.getRegister(12, 106) == 0x0708);
    CHECK(mpu.getRegister(12, 107) == 0x090a);
    CHECK(mpu.getRegister(12, 108) == 0x0b0c);
    CHECK(mpu.getRegister(12, 109) == 0x0d0e);
    CHECK(mpu.getRegister(12, 110) == 0x0f10);
    CHECK(mpu.getRegister(12, 111) == 0x1112);
    CHECK(mpu.getRegister(12, 112) == 0x1314);

    CHECK_THROWS(mpu.getRegister(12, 113));
    CHECK_THROWS(mpu.getRegister(12, 114));
}

TEST_CASE("Test MPU preset holding register", "[MPU]") {
    TestMPU mpu(5);
    TestFPGA MPUFPGA;

    mpu.presetHoldingRegister(0x11, 0x0001, 0x0003, 102);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 15);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 8);
    CHECK(commands[2] == 0x11);
    CHECK(commands[3] == 0x06);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0x01);
    CHECK(commands[6] == 0x00);
    CHECK(commands[7] == 0x03);
    CHECK(commands[8] == 0x9A);
    CHECK(commands[9] == 0x9B);
    CHECK(commands[10] == MPUCommands::MPU_READ_MS);
    CHECK(commands[11] == 8);
    CHECK(commands[12] == 0);
    CHECK(commands[13] == 102);
    CHECK(commands[14] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res = {0x11, 0x06, 0x00, 0x01, 0x00, 0x03, 0x9A, 0x9B};

    REQUIRE_NOTHROW(mpu.parse(res));
}

TEST_CASE("Test MPU preset holding registers", "[MPU]") {
    TestMPU mpu(5);
    TestFPGA MPUFPGA;

    std::vector<uint16_t> regs = {0x0102, 0x0304};
    mpu.presetHoldingRegisters(17, 0x1718, regs.data(), regs.size(), 102);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 20);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 9 + 2 * regs.size());
    CHECK(commands[2] == 17);
    CHECK(commands[3] == 16);
    CHECK(commands[4] == 0x17);
    CHECK(commands[5] == 0x18);
    CHECK(commands[6] == 0);
    CHECK(commands[7] == regs.size());
    CHECK(commands[8] == regs.size() * 2);
    CHECK(commands[9] == 0x01);
    CHECK(commands[10] == 0x02);
    CHECK(commands[11] == 0x03);
    CHECK(commands[12] == 0x04);
    CHECK(commands[13] == 0xED);
    CHECK(commands[14] == 0x3A);
    CHECK(commands[15] == MPUCommands::MPU_READ_MS);
    CHECK(commands[16] == 8);
    CHECK(commands[17] == 0);
    CHECK(commands[18] == 102);
    CHECK(commands[19] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res = {17, 16, 0x17, 0x18, 0, 2, 0xC6, 0xEB};

    REQUIRE_NOTHROW(mpu.parse(res));
}

TEST_CASE("Test MPU preset holding registers by simplymodbus.ca", "[MPU]") {
    TestMPU mpu(5);
    TestFPGA MPUFPGA;

    std::vector<uint16_t> regs = {0x000A, 0x0102};
    mpu.presetHoldingRegisters(0x11, 0x0001, regs.data(), regs.size(), 102);

    MPUFPGA.mpuCommands(mpu);
    auto commands = MPUFPGA.last_mpu;

    REQUIRE(commands.size() == 20);

    CHECK(commands[0] == MPUCommands::MPU_WRITE);
    CHECK(commands[1] == 9 + 2 * regs.size());
    CHECK(commands[2] == 0x11);
    CHECK(commands[3] == 0x10);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0x01);
    CHECK(commands[6] == 0x00);
    CHECK(commands[7] == 0x02);
    CHECK(commands[8] == 0x04);
    CHECK(commands[9] == 0x00);
    CHECK(commands[10] == 0x0A);
    CHECK(commands[11] == 0x01);
    CHECK(commands[12] == 0x02);
    CHECK(commands[13] == 0xC6);
    CHECK(commands[14] == 0xF0);
    CHECK(commands[15] == MPUCommands::MPU_READ_MS);
    CHECK(commands[16] == 8);
    CHECK(commands[17] == 0);
    CHECK(commands[18] == 102);
    CHECK(commands[19] == MPUCommands::MPU_IRQ);

    std::vector<uint8_t> res = {0x11, 0x10, 0x00, 0x01, 0x00, 0x02, 0x12, 0x98};

    REQUIRE_NOTHROW(mpu.parse(res));
}
