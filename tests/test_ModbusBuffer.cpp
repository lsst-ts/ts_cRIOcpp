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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <memory>
#include <cmath>
#include <iostream>

#include <cRIO/ModbusBuffer.h>

using namespace LSST::cRIO;

TEST_CASE("CalculateCRC", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    // address
    mbuf.write<uint8_t>(123);
    mbuf.write<uint8_t>(17);
    mbuf.writeCRC();
    mbuf.writeEndOfFrame();

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(buf[2] == 0xe3);
    REQUIRE(buf[3] == 0x4c);
}

TEST_CASE("CalculateLongCRC", "[ModbusBuffer]") {
    std::vector<uint8_t> data = {0x81, 0x11, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xFF,
                                 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x53, 0x74, 0x61, 0x72};

    ModbusBuffer mbuf;
    for (auto d : data) mbuf.write(d);

    mbuf.writeCRC();
    mbuf.writeEndOfFrame();

    REQUIRE(mbuf.getLength() == 22);

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(buf[19] == 0xA7);
    REQUIRE(buf[20] == 0x9F);
}

TEST_CASE("WriteUxx", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.write<uint8_t>(0x12);
    mbuf.write<uint16_t>(0x3456);
    mbuf.write<uint32_t>(0x7890abcd);
    mbuf.write<uint64_t>(0xAAbbCCddEEff00);

    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x12);
    REQUIRE(buf[1] == 0x34);
    REQUIRE(buf[2] == 0x56);
    REQUIRE(buf[3] == 0x78);
    REQUIRE(buf[4] == 0x90);
    REQUIRE(buf[5] == 0xab);
    REQUIRE(buf[6] == 0xcd);

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 0x12);
    REQUIRE(mbuf.read<uint16_t>() == 0x3456);
    REQUIRE(mbuf.read<uint32_t>() == 0x7890abcd);
    REQUIRE(mbuf.read<uint64_t>() == 0xAAbbCCddEEff00);

    REQUIRE_NOTHROW(mbuf.checkCRC());
}

TEST_CASE("WriteIxx", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.write<int8_t>(0x12);
    mbuf.write<int16_t>(0x3456);
    mbuf.write<int32_t>(0x7890abcd);
    mbuf.write<int32_t>(0xf890abcd);
    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x12);
    REQUIRE(buf[1] == 0x34);
    REQUIRE(buf[2] == 0x56);
    REQUIRE(buf[3] == 0x78);
    REQUIRE(buf[4] == 0x90);
    REQUIRE(buf[5] == 0xab);
    REQUIRE(buf[6] == 0xcd);
    REQUIRE(buf[7] == 0xf8);
    REQUIRE(buf[8] == 0x90);
    REQUIRE(buf[9] == 0xab);
    REQUIRE(buf[10] == 0xcd);

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 0x12);
    REQUIRE(mbuf.read<uint16_t>() == 0x3456);
    REQUIRE(mbuf.read<uint32_t>() == 0x7890abcd);
    REQUIRE(mbuf.read<uint32_t>() == 0xf890abcd);

    REQUIRE_NOTHROW(mbuf.checkCRC());
}

TEST_CASE("WriteSGL", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.write<float>(0.123);
    mbuf.write(-6758.1234f);
    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

    REQUIRE(buf[0] == 0x3d);
    REQUIRE(buf[1] == 0xfb);
    REQUIRE(buf[2] == 0xe7);
    REQUIRE(buf[3] == 0x6d);

    REQUIRE(buf[4] == 0xc5);
    REQUIRE(buf[5] == 0xd3);
    REQUIRE(buf[6] == 0x30);
    REQUIRE(buf[7] == 0xfd);

    mbuf.reset();

    REQUIRE(mbuf.read<float>() == 0.123f);
    REQUIRE(mbuf.read<float>() == -6758.1234f);
    REQUIRE_NOTHROW(mbuf.checkCRC());
}

TEST_CASE("Calculate function response CRC", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.write<uint8_t>(140);
    mbuf.write<uint8_t>(18);
    mbuf.write<uint8_t>(4);
    mbuf.write<uint16_t>(0x0040 | 0x0002);
    mbuf.write<uint16_t>(0x0004);
    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();
    REQUIRE(buf[7] == 0x05);
    REQUIRE(buf[8] == 0xad);

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 140);
    REQUIRE(mbuf.read<uint8_t>() == 18);
    REQUIRE(mbuf.read<uint8_t>() == 4);
    REQUIRE(mbuf.read<uint16_t>() == (0x0040 | 0x0002));
    REQUIRE(mbuf.read<uint16_t>() == 0x0004);
    REQUIRE_NOTHROW(mbuf.checkCRC());
}

// wrapper class to test protected variadic templates and changes
class TestBuffer : public ModbusBuffer {
public:
    template <typename... dt>
    void testFunction(uint8_t address, uint8_t function, uint32_t timeout, const dt&... params) {
        callFunction(address, function, timeout, params...);
    }
    void testBroadcast(uint8_t address, uint8_t function, uint8_t counter, uint32_t delay, uint8_t* data,
                       size_t dataLen) {
        broadcastFunction(address, function, counter, delay, data, dataLen);
    }

    void recordChanges() { ModbusBuffer::recordChanges(); }
    void pauseRecordChanges() { ModbusBuffer::pauseRecordChanges(); }
    bool checkRecording(std::vector<uint8_t>& changed) { return ModbusBuffer::checkRecording(changed); }
};

TEST_CASE("Call function with arguments", "[ModbusBuffer]") {
    TestBuffer mbuf;
    mbuf.testFunction(123, 17, 23, static_cast<uint8_t>(0xfe), static_cast<uint16_t>(0xffcc),
                      static_cast<float>(M_PI));

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 123);
    REQUIRE(mbuf.read<uint8_t>() == 17);
    REQUIRE(mbuf.read<uint8_t>() == 0xfe);
    REQUIRE(mbuf.read<uint16_t>() == 0xffcc);
    REQUIRE(mbuf.read<float>() == static_cast<float>(M_PI));
    REQUIRE_NOTHROW(mbuf.checkCRC());
    REQUIRE_NOTHROW(mbuf.readEndOfFrame());
    REQUIRE(mbuf.readWaitForRx() == 23);
}

TEST_CASE("Test broadcast", "[ModbusBuffer]") {
    TestBuffer mbuf;

    uint8_t data[22] = {0x01, 0xff, 0xfe, 0xce, 0xdf, 0xac, 0xef, 0x12, 0xAC, 0xee, 0x78,
                        0xf7, 0x3a, 0x8f, 0xbb, 0xcc, 0xf1, 0xdd, 0x1f, 0xDC, 0xcb, 0xca};

    mbuf.testBroadcast(250, 89, 2, 300, data, 22);

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 250);
    REQUIRE(mbuf.read<uint8_t>() == 89);
    REQUIRE(mbuf.read<uint8_t>() == 2);

    for (int i = 0; i < 22; i++) {
        REQUIRE(mbuf.read<uint8_t>() == data[i]);
    }
    REQUIRE_NOTHROW(mbuf.checkCRC());
    REQUIRE_NOTHROW(mbuf.readEndOfFrame());
    REQUIRE(mbuf.readDelay() == 300);
}

TEST_CASE("Test changed calculations", "[ModbusBuffer]") {
    TestBuffer mbuf;

    mbuf.testFunction(11, 23, 25, static_cast<uint8_t>(0x1a), static_cast<uint16_t>(0xffcc),
                      static_cast<int32_t>(-977453), static_cast<uint32_t>(0xffaaccdd),
                      static_cast<float>(M_PI * 211.12), static_cast<uint32_t>(87346));

    std::vector<uint8_t> changed;

    auto readAll = [&mbuf, &changed](int32_t nrp = -977453, uint32_t rp = 87346) {
        mbuf.reset();

        REQUIRE(mbuf.read<uint8_t>() == 11);
        REQUIRE(mbuf.read<uint8_t>() == 23);

        REQUIRE_NOTHROW(mbuf.recordChanges());
        REQUIRE(mbuf.read<uint8_t>() == 0x1a);
        REQUIRE(mbuf.read<uint16_t>() == 0xffcc);

        REQUIRE_NOTHROW(mbuf.pauseRecordChanges());
        REQUIRE(mbuf.read<int32_t>() == nrp);
        REQUIRE_NOTHROW(mbuf.recordChanges());

        REQUIRE(mbuf.read<uint32_t>() == 0xffaaccdd);
        REQUIRE(mbuf.read<float>() == static_cast<float>(M_PI * 211.12));
        REQUIRE(mbuf.read<uint32_t>() == rp);
        REQUIRE_NOTHROW(mbuf.checkCRC());
        REQUIRE_NOTHROW(mbuf.readEndOfFrame());
        REQUIRE(mbuf.readWaitForRx() == 25);
    };

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == false);

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == true);

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == true);

    changed[2] += 1;

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == false);

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == true);

    mbuf.clear();
    mbuf.testFunction(11, 23, 25, static_cast<uint8_t>(0x1a), static_cast<uint16_t>(0xffcc),
                      static_cast<int32_t>(-977453), static_cast<uint32_t>(0xffaaccdd),
                      static_cast<float>(M_PI * 211.12), static_cast<uint32_t>(87346));

    readAll();
    REQUIRE(mbuf.checkRecording(changed) == true);

    mbuf.clear();
    mbuf.testFunction(11, 23, 25, static_cast<uint8_t>(0x1a), static_cast<uint16_t>(0xffcc),
                      static_cast<int32_t>(2956), static_cast<uint32_t>(0xffaaccdd),
                      static_cast<float>(M_PI * 211.12), static_cast<uint32_t>(87346));

    readAll(2956);
    REQUIRE(mbuf.checkRecording(changed) == true);

    mbuf.clear();
    mbuf.testFunction(11, 23, 25, static_cast<uint8_t>(0x1a), static_cast<uint16_t>(0xffcc),
                      static_cast<int32_t>(2956), static_cast<uint32_t>(0xffaaccdd),
                      static_cast<float>(M_PI * 211.12), static_cast<uint32_t>(48342));

    readAll(2956, 48342);
    REQUIRE(mbuf.checkRecording(changed) == false);

    readAll(2956, 48342);
    REQUIRE(mbuf.checkRecording(changed) == true);
}

TEST_CASE("CRC class", "[ModbusBuffer::CRC]") {
    ModbusBuffer::CRC crc;

    for (uint8_t d = 0; d < 0xFF; d++) {
        crc.add(d);
    }

    REQUIRE(crc.get() == 0xADD6);

    crc.reset();

    for (auto d : std::string("This is Modbus CRC!")) {
        crc.add(d);
    }

    REQUIRE(crc.get() == 0xAEDA);

    crc.reset();

    for (auto d : std::string("Calculating CRC is as easy as answering 42.")) {
        crc.add(d);
    }

    REQUIRE(crc.get() == 0x2879);

    crc.reset();

    uint8_t data[5] = {0x12, 0x34, 0x56, 0x78, 0xff};

    for (auto d : data) {
        crc.add(d);
    }

    REQUIRE(crc.get() == 0x6310);
}
