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

#include <catch2/catch.hpp>

#include <Modbus/BusList.h>

using namespace Modbus;

class TestList : public BusList {
public:
    TestList(uint8_t _expectedAddress);

    void processReadRegister(uint8_t address, uint16_t reg1, uint16_t reg2, uint16_t reg3);

    uint8_t expectedAddress;
};

TestList::TestList(uint8_t _expectedAddress) : expectedAddress(_expectedAddress) {
    add_response(3, [this](Modbus::Parser parser) {
        CHECK(parser.func() == 3);
        CHECK(parser.read<uint8_t>() == 6);
        uint16_t reg1 = parser.read<uint16_t>();
        uint16_t reg2 = parser.read<uint16_t>();
        uint16_t reg3 = parser.read<uint16_t>();
        CHECK_NOTHROW(parser.checkCRC());
        processReadRegister(parser.address(), reg1, reg2, reg3);
    });
}

void TestList::processReadRegister(uint8_t address, uint16_t reg1, uint16_t reg2, uint16_t reg3) {
    CHECK(address == expectedAddress);
    CHECK(reg1 == 0xAE41);
    CHECK(reg2 == 0x5652);
    CHECK(reg3 == 0x4340);
}

TEST_CASE("Call functions", "[Calls]") {
    BusList buslist;

    buslist.callFunction(123, 17, 200, static_cast<uint8_t>(0xfe), static_cast<uint16_t>(0xffcc),
                         static_cast<float>(22.33));

    buslist.callFunction(43, 86, 200, static_cast<float>(0.1122), static_cast<int16_t>(-1234),
                         static_cast<uint64_t>(0x0123456789abcdef));

    CHECK(buslist.size() == 2);

    CHECK(buslist[0].buffer.size() == 11);
    CHECK(buslist[0].buffer[0] == 0x7b);
    CHECK(buslist[0].buffer[1] == 0x11);
    CHECK(buslist[0].buffer[2] == 0xfe);
    CHECK(buslist[0].buffer[3] == 0xff);
    CHECK(buslist[0].buffer[4] == 0xcc);
    CHECK(buslist[0].buffer[5] == 0x41);
    CHECK(buslist[0].buffer[6] == 0xb2);
    CHECK(buslist[0].buffer[7] == 0xa3);
    CHECK(buslist[0].buffer[8] == 0xd7);
    CHECK(buslist[0].buffer[9] == 0x4b);
    CHECK(buslist[0].buffer[10] == 0xa7);

    CHECK(buslist[1].buffer.size() == 18);
    CHECK(buslist[1].buffer[0] == 0x2b);
    CHECK(buslist[1].buffer[1] == 0x56);
    CHECK(buslist[1].buffer[2] == 0x3d);
    CHECK(buslist[1].buffer[3] == 0xe5);
    CHECK(buslist[1].buffer[4] == 0xc9);
    CHECK(buslist[1].buffer[5] == 0x1d);
    CHECK(buslist[1].buffer[6] == 0xfb);
    CHECK(buslist[1].buffer[7] == 0x2e);
    CHECK(buslist[1].buffer[8] == 0x01);
    CHECK(buslist[1].buffer[9] == 0x23);
    CHECK(buslist[1].buffer[10] == 0x45);
    CHECK(buslist[1].buffer[11] == 0x67);
    CHECK(buslist[1].buffer[12] == 0x89);
    CHECK(buslist[1].buffer[13] == 0xab);
    CHECK(buslist[1].buffer[14] == 0xcd);
    CHECK(buslist[1].buffer[15] == 0xef);
    CHECK(buslist[1].buffer[16] == 0x0f);
    CHECK(buslist[1].buffer[17] == 0xfd);
}

TEST_CASE("Call function, parser return", "[Parsing]") {
    TestList buslist(0x11);

    buslist.callFunction(0x11, 0x3, 200, static_cast<uint16_t>(0x1234), static_cast<uint16_t>(0x0003));

    std::vector<uint8_t> data({0x11, 0x03, 0x06, 0xAE, 0x41, 0x56, 0x52, 0x43, 0x40, 0x49, 0xAD});

    CHECK_NOTHROW(buslist.parse(data));
}

TEST_CASE("Call 10 functions, parser return", "[Parsing]") {
    TestList buslist(1);

    auto generate_reply = [](uint8_t address) -> std::vector<uint8_t> {
        Buffer mbuf(std::vector<uint8_t>({address, 0x03, 0x06, 0xAE, 0x41, 0x56, 0x52, 0x43, 0x40}));
        mbuf.writeCRC();
        return mbuf;
    };

    for (int address = 1; address < 10; address++) {
        buslist.callFunction(address, 3, 200, static_cast<uint16_t>(0x1234), static_cast<uint16_t>(0x0003));
    }

    for (int address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        CHECK_NOTHROW(buslist.parse(generate_reply(address)));
    }
}

TEST_CASE("Missing response", "[BusListErrors]") {
    TestList buslist(1);

    auto generate_reply = [](uint8_t address) -> std::vector<uint8_t> {
        Buffer mbuf(std::vector<uint8_t>({address, 0x03, 0x06, 0xAE, 0x41, 0x56, 0x52, 0x43, 0x40}));
        mbuf.writeCRC();
        return mbuf;
    };

    for (uint8_t address = 1; address < 10; address++) {
        buslist.callFunction(address, 3, 200, static_cast<uint16_t>(0x1234), static_cast<uint16_t>(0x0003));
    }

    for (uint8_t address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        CHECK_NOTHROW(buslist.parse(generate_reply(address)));
    }

    buslist.reset();

    for (uint8_t address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        CHECK_THROWS_AS(buslist.parse(generate_reply(address + 1)), WrongResponse);
    }

    buslist.reset();

    // test MissingResponse is thrown properly in expected processing sequences

    buslist.expectedAddress = 1;
    CHECK_THROWS_AS(buslist.parse(generate_reply(2)), WrongResponse);
    buslist.expectedAddress = 2;
    CHECK_NOTHROW(buslist.parse(generate_reply(2)));
    buslist.expectedAddress = 3;
    CHECK_NOTHROW(buslist.parse(generate_reply(3)));
    buslist.expectedAddress = 4;
    CHECK_NOTHROW(buslist.parse(generate_reply(4)));
    buslist.expectedAddress = 5;
    CHECK_THROWS_AS(buslist.parse(generate_reply(7)), WrongResponse);
    buslist.expectedAddress = 6;
    CHECK_THROWS_AS(buslist.parse(generate_reply(7)), WrongResponse);
    buslist.expectedAddress = 7;
    CHECK_NOTHROW(buslist.parse(generate_reply(7)));
    buslist.expectedAddress = 8;
    CHECK_NOTHROW(buslist.parse(generate_reply(8)));
    buslist.expectedAddress = 9;
    CHECK_NOTHROW(buslist.parse(generate_reply(9)));
    buslist.expectedAddress = 9;
    CHECK_THROWS_AS(buslist.parse(generate_reply(10)), std::out_of_range);
}

TEST_CASE("Modbus error response", "[ModbusError]") {
    TestList buslist(1);

    auto generate_reply = [](uint8_t address) -> std::vector<uint8_t> {
        Buffer mbuf(std::vector<uint8_t>({address, 0x03, 0x06, 0xAE, 0x41, 0x56, 0x52, 0x43, 0x40}));
        mbuf.writeCRC();
        return mbuf;
    };

    auto generate_error_reply = [](uint8_t address) -> std::vector<uint8_t> {
        Buffer mbuf(std::vector<uint8_t>({address, 0x83, 0x01}));
        mbuf.writeCRC();
        return mbuf;
    };

    for (uint8_t address = 1; address < 10; address++) {
        buslist.callFunction(address, 3, 200, static_cast<uint16_t>(0x1234), static_cast<uint16_t>(0x0003));
    }

    for (uint8_t address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        CHECK_THROWS_AS(buslist.parse(generate_error_reply(address)), ErrorResponse);
    }

    buslist.reset();

    for (uint8_t address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        if (address & 0x01) {
            CHECK_THROWS_AS(buslist.parse(generate_error_reply(address)), ErrorResponse);
        } else {
            CHECK_NOTHROW(buslist.parse(generate_reply(address)));
        }
    }

    uint8_t expected_address = 0;

    buslist.set_error_response(0x03, [&expected_address](uint8_t address, uint8_t called) -> void {
        CHECK(address == expected_address);
        CHECK(called == 0x83);
    });

    buslist.reset();

    for (uint8_t address = 1; address < 10; address++) {
        buslist.expectedAddress = address;
        expected_address = address;
        CHECK_NOTHROW(buslist.parse(generate_error_reply(address)));
    }
}

TEST_CASE("Response length calculations", "[ResponseLength]") {
    TestList buslist(1);

    CHECK(buslist.responseLength({}) == -1);
    CHECK(buslist.responseLength({0x01, 0x02}) == -1);
}
