/*
 * This file is part of LSST M1M3 SS test suite. Tests Modbus buffer class.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

    REQUIRE(buf[2] == (0x1200 | (0xe3 << 1)));
    REQUIRE(buf[3] == (0x1200 | (0x4c << 1)));
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

    REQUIRE(buf[19] == (0x1200 | (0xA7 << 1)));
    REQUIRE(buf[20] == (0x1200 | (0x9F << 1)));
}

TEST_CASE("WriteUxx", "[ModbusBuffer]") {
    ModbusBuffer mbuf;
    mbuf.write<uint8_t>(0x12);
    mbuf.write<uint16_t>(0x3456);
    mbuf.write<uint32_t>(0x7890abcd);

    mbuf.writeCRC();

    uint16_t* buf = mbuf.getBuffer();

    // bytes are written left shifted by 1, and masked with 0x12

    REQUIRE(buf[0] == 0x1224);
    REQUIRE(buf[1] == 0x1268);
    REQUIRE(buf[2] == 0x12ac);
    REQUIRE(buf[3] == 0x12f0);
    REQUIRE(buf[4] == 0x1320);
    REQUIRE(buf[5] == 0x1356);
    REQUIRE(buf[6] == 0x139a);

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 0x12);
    REQUIRE(mbuf.read<uint16_t>() == 0x3456);
    REQUIRE(mbuf.read<uint32_t>() == 0x7890abcd);

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

    REQUIRE(buf[0] == 0x1224);
    REQUIRE(buf[1] == 0x1268);
    REQUIRE(buf[2] == 0x12ac);
    REQUIRE(buf[3] == 0x12f0);
    REQUIRE(buf[4] == 0x1320);
    REQUIRE(buf[5] == 0x1356);
    REQUIRE(buf[6] == 0x139a);
    REQUIRE(buf[7] == 0x13f0);
    REQUIRE(buf[8] == 0x1320);
    REQUIRE(buf[9] == 0x1356);
    REQUIRE(buf[10] == 0x139a);

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

    REQUIRE(buf[0] == 0x127a);
    REQUIRE(buf[1] == 0x13f6);
    REQUIRE(buf[2] == 0x13ce);
    REQUIRE(buf[3] == 0x12da);

    REQUIRE(buf[4] == 0x138a);
    REQUIRE(buf[5] == 0x13a6);
    REQUIRE(buf[6] == 0x1260);
    REQUIRE(buf[7] == 0x13fa);

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
    REQUIRE(buf[7] == (0x1200 | (0x05 << 1)));
    REQUIRE(buf[8] == (0x1200 | (0xad << 1)));

    mbuf.reset();

    REQUIRE(mbuf.read<uint8_t>() == 140);
    REQUIRE(mbuf.read<uint8_t>() == 18);
    REQUIRE(mbuf.read<uint8_t>() == 4);
    REQUIRE(mbuf.read<uint16_t>() == (0x0040 | 0x0002));
    REQUIRE(mbuf.read<uint16_t>() == 0x0004);
    REQUIRE_NOTHROW(mbuf.checkCRC());
}

// wrapper class to test protected variadic template
class TestBuffer : public ModbusBuffer {
public:
    template <typename... dt>
    void testFunction(uint8_t address, uint8_t function, uint32_t timeout, const dt&... params) {
        callFunction(address, function, timeout, params...);
    }
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
