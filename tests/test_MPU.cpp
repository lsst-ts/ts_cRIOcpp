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

#include <catch2/catch.hpp>

#include <cRIO/MPU.h>

using namespace LSST::cRIO;

class TestMPU : public MPU {
public:
    TestMPU(uint8_t mpu_address) : MPU(mpu_address) {}
};

TEST_CASE("Test MPU read input status", "[MPU]") {
    TestMPU mpu(0x11);
    mpu.readInputStatus(0x00C4, 0x0016, 108);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 0x11);
    CHECK(commands[1] == 0x02);
    CHECK(commands[2] == 0x00);
    CHECK(commands[3] == 0xC4);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0x16);
    CHECK(commands[6] == 0xBA);
    CHECK(commands[7] == 0xA9);

    CHECK_NOTHROW(mpu.parse(std::vector<uint8_t>({0x11, 0x02, 0x03, 0xAC, 0xDB, 0x35, 0x20, 0x18})));

    // input status is not 10001 offseted
    CHECK_THROWS(mpu.getInputStatus(195));

    // first byte
    CHECK(mpu.getInputStatus(196) == false);
    CHECK(mpu.getInputStatus(197) == false);
    CHECK(mpu.getInputStatus(198) == true);
    CHECK(mpu.getInputStatus(199) == true);

    CHECK(mpu.getInputStatus(200) == false);
    CHECK(mpu.getInputStatus(201) == true);
    CHECK(mpu.getInputStatus(202) == false);
    CHECK(mpu.getInputStatus(203) == true);

    // second byte
    CHECK(mpu.getInputStatus(204) == true);
    CHECK(mpu.getInputStatus(205) == true);
    CHECK(mpu.getInputStatus(206) == false);
    CHECK(mpu.getInputStatus(207) == true);

    CHECK(mpu.getInputStatus(208) == true);
    CHECK(mpu.getInputStatus(209) == false);
    CHECK(mpu.getInputStatus(210) == true);
    CHECK(mpu.getInputStatus(211) == true);

    // third byte
    CHECK(mpu.getInputStatus(212) == true);
    CHECK(mpu.getInputStatus(213) == false);
    CHECK(mpu.getInputStatus(214) == true);
    CHECK(mpu.getInputStatus(215) == false);

    CHECK(mpu.getInputStatus(216) == true);
    CHECK(mpu.getInputStatus(217) == true);
    CHECK_THROWS(mpu.getInputStatus(218));
    CHECK_THROWS(mpu.getInputStatus(219));
}

TEST_CASE("Test MPU read holding registers", "[MPU]") {
    TestMPU mpu(12);
    mpu.readHoldingRegisters(3, 10, 101);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 12);
    CHECK(commands[1] == 3);
    CHECK(commands[2] == 0);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 10);
    CHECK(commands[6] == 0x34);
    CHECK(commands[7] == 0xD0);

    std::vector<uint8_t> res = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    CHECK_NOTHROW(mpu.parse(res));

    CHECK_THROWS(mpu.getRegister(1));
    CHECK_THROWS(mpu.getRegister(2));

    CHECK(mpu.getRegister(3) == 0x0102);
    CHECK(mpu.getRegister(4) == 0x0304);
    CHECK(mpu.getRegister(5) == 0x0506);
    CHECK(mpu.getRegister(6) == 0x0708);
    CHECK(mpu.getRegister(7) == 0x090a);
    CHECK(mpu.getRegister(8) == 0x0b0c);
    CHECK(mpu.getRegister(9) == 0x0d0e);
    CHECK(mpu.getRegister(10) == 0x0f10);
    CHECK(mpu.getRegister(11) == 0x1112);
    CHECK(mpu.getRegister(12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(13));
    CHECK_THROWS(mpu.getRegister(14));
}

TEST_CASE("Test MPU reading multiple registers - failed response", "[MPU]") {
    TestMPU mpu(12);
    mpu.readHoldingRegisters(3, 10, 101);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 12);
    CHECK(commands[1] == 3);
    CHECK(commands[2] == 0);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 10);
    CHECK(commands[6] == 0x34);
    CHECK(commands[7] == 0xD0);

    std::vector<uint8_t> res1 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    CHECK_NOTHROW(mpu.parse(res1));

    CHECK_THROWS(mpu.getRegister(1));
    CHECK_THROWS(mpu.getRegister(2));

    CHECK(mpu.getRegister(3) == 0x0102);
    CHECK(mpu.getRegister(4) == 0x0304);
    CHECK(mpu.getRegister(5) == 0x0506);
    CHECK(mpu.getRegister(6) == 0x0708);
    CHECK(mpu.getRegister(7) == 0x090a);
    CHECK(mpu.getRegister(8) == 0x0b0c);
    CHECK(mpu.getRegister(9) == 0x0d0e);
    CHECK(mpu.getRegister(10) == 0x0f10);
    CHECK(mpu.getRegister(11) == 0x1112);
    CHECK(mpu.getRegister(12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(13));
    CHECK_THROWS(mpu.getRegister(14));

    CHECK_THROWS(mpu.getRegister(103));
    CHECK_THROWS(mpu.getRegister(104));

    mpu.clear();
    mpu.readHoldingRegisters(103, 10, 101);

    CHECK(mpu.size() == 1);

    commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 12);
    CHECK(commands[1] == 3);
    CHECK(commands[2] == 0);
    CHECK(commands[3] == 103);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 10);
    CHECK(commands[6] == 0x75);
    CHECK(commands[7] == 0x0F);
}

TEST_CASE("Test MPU reading multiple registers - successful response", "[MPU]") {
    TestMPU mpu(12);
    mpu.readHoldingRegisters(3, 10, 101);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 12);
    CHECK(commands[1] == 3);
    CHECK(commands[2] == 0);
    CHECK(commands[3] == 3);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 10);
    CHECK(commands[6] == 0x34);
    CHECK(commands[7] == 0xD0);

    CHECK_NOTHROW(mpu.parse(std::vector<uint8_t>(
            {12, 3, 20, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde})));

    CHECK_THROWS(mpu.getRegister(1));
    CHECK_THROWS(mpu.getRegister(2));

    CHECK(mpu.getRegister(3) == 0x0102);
    CHECK(mpu.getRegister(4) == 0x0304);
    CHECK(mpu.getRegister(5) == 0x0506);
    CHECK(mpu.getRegister(6) == 0x0708);
    CHECK(mpu.getRegister(7) == 0x090a);
    CHECK(mpu.getRegister(8) == 0x0b0c);
    CHECK(mpu.getRegister(9) == 0x0d0e);
    CHECK(mpu.getRegister(10) == 0x0f10);
    CHECK(mpu.getRegister(11) == 0x1112);
    CHECK(mpu.getRegister(12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(13));
    CHECK_THROWS(mpu.getRegister(14));

    CHECK_THROWS(mpu.getRegister(103));
    CHECK_THROWS(mpu.getRegister(104));

    mpu.readHoldingRegisters(103, 10, 101);

    CHECK(mpu.size() == 2);

    commands = mpu[1].buffer;

    CHECK(commands[0] == 12);
    CHECK(commands[1] == 3);
    CHECK(commands[2] == 0);
    CHECK(commands[3] == 103);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == 10);
    CHECK(commands[6] == 0x75);
    CHECK(commands[7] == 0x0F);

    std::vector<uint8_t> res2 = {12, 3,  20, 1,  2,  3,  4,  5,  6,  7,  8,    9,   10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0xcf, 0xde};

    CHECK_NOTHROW(mpu.parse(res2));

    CHECK_THROWS(mpu.getRegister(1));
    CHECK_THROWS(mpu.getRegister(2));

    CHECK(mpu.getRegister(3) == 0x0102);
    CHECK(mpu.getRegister(4) == 0x0304);
    CHECK(mpu.getRegister(5) == 0x0506);
    CHECK(mpu.getRegister(6) == 0x0708);
    CHECK(mpu.getRegister(7) == 0x090a);
    CHECK(mpu.getRegister(8) == 0x0b0c);
    CHECK(mpu.getRegister(9) == 0x0d0e);
    CHECK(mpu.getRegister(10) == 0x0f10);
    CHECK(mpu.getRegister(11) == 0x1112);
    CHECK(mpu.getRegister(12) == 0x1314);

    CHECK_THROWS(mpu.getRegister(13));
    CHECK_THROWS(mpu.getRegister(14));

    CHECK(mpu.getRegister(103) == 0x0102);
    CHECK(mpu.getRegister(104) == 0x0304);
    CHECK(mpu.getRegister(105) == 0x0506);
    CHECK(mpu.getRegister(106) == 0x0708);
    CHECK(mpu.getRegister(107) == 0x090a);
    CHECK(mpu.getRegister(108) == 0x0b0c);
    CHECK(mpu.getRegister(109) == 0x0d0e);
    CHECK(mpu.getRegister(110) == 0x0f10);
    CHECK(mpu.getRegister(111) == 0x1112);
    CHECK(mpu.getRegister(112) == 0x1314);

    CHECK_THROWS(mpu.getRegister(113));
    CHECK_THROWS(mpu.getRegister(114));
}

TEST_CASE("Test MPU preset holding register", "[MPU]") {
    TestMPU mpu(0x11);

    mpu.presetHoldingRegister(0x0001, 0x0003, 102);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 8);

    CHECK(commands[0] == 0x11);
    CHECK(commands[1] == 0x06);
    CHECK(commands[2] == 0x00);
    CHECK(commands[3] == 0x01);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0x03);
    CHECK(commands[6] == 0x9A);
    CHECK(commands[7] == 0x9B);

    CHECK_NOTHROW(mpu.parse(std::vector<uint8_t>({0x11, 0x06, 0x00, 0x01, 0x00, 0x03, 0x9A, 0x9B})));
}

TEST_CASE("Test MPU preset holding registers", "[MPU]") {
    TestMPU mpu(17);

    std::vector<uint16_t> regs = {0x0102, 0x0304};
    mpu.presetHoldingRegisters(0x1718, regs, 102);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 9 + 2 * regs.size());

    CHECK(commands[0] == 17);
    CHECK(commands[1] == 16);
    CHECK(commands[2] == 0x17);
    CHECK(commands[3] == 0x18);
    CHECK(commands[4] == 0);
    CHECK(commands[5] == regs.size());
    CHECK(commands[6] == regs.size() * 2);
    CHECK(commands[7] == 0x01);
    CHECK(commands[8] == 0x02);
    CHECK(commands[9] == 0x03);
    CHECK(commands[10] == 0x04);
    CHECK(commands[11] == 0xED);
    CHECK(commands[12] == 0x3A);

    CHECK_NOTHROW(mpu.parse(std::vector<uint8_t>({17, 16, 0x17, 0x18, 0, 2, 0xC6, 0xEB})));
}

TEST_CASE("Test MPU preset holding registers by simplymodbus.ca", "[MPU]") {
    TestMPU mpu(0x11);

    std::vector<uint16_t> regs = {0x000A, 0x0102};
    mpu.presetHoldingRegisters(0x0001, regs, 102);

    CHECK(mpu.size() == 1);

    auto commands = mpu[0].buffer;

    CHECK(commands.size() == 9 + 2 * regs.size());

    CHECK(commands[0] == 0x11);
    CHECK(commands[1] == 0x10);
    CHECK(commands[2] == 0x00);
    CHECK(commands[3] == 0x01);
    CHECK(commands[4] == 0x00);
    CHECK(commands[5] == 0x02);
    CHECK(commands[6] == 0x04);
    CHECK(commands[7] == 0x00);
    CHECK(commands[8] == 0x0A);
    CHECK(commands[9] == 0x01);
    CHECK(commands[10] == 0x02);
    CHECK(commands[11] == 0xC6);
    CHECK(commands[12] == 0xF0);

    CHECK_NOTHROW(mpu.parse(std::vector<uint8_t>({0x11, 0x10, 0x00, 0x01, 0x00, 0x02, 0x12, 0x98})));
}

TEST_CASE("Test responseLength calculations", "[ResponseLength]") {
    TestMPU mpu(0x11);

    CHECK(mpu.responseLength({0x11, 0x83}) == 5);

    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::READ_INPUT_STATUS}) == -1);
    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::READ_INPUT_STATUS, 0x02}) == 7);

    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::READ_HOLDING_REGISTERS}) == -1);
    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::READ_HOLDING_REGISTERS, 0x08}) == 13);

    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::PRESET_HOLDING_REGISTER}) == 8);
    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::PRESET_HOLDING_REGISTER, 0x11, 0x22, 0x33, 0x44}) == 8);

    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::PRESET_HOLDING_REGISTERS}) == 8);
    CHECK(mpu.responseLength({0x11, MPU::MODBUS_CMD::PRESET_HOLDING_REGISTERS, 0x11, 0x22, 0x33, 0x44}) == 8);
}
