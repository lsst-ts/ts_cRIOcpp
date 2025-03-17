/*
 * This file is part of LSST cRIOcpp test suite. Tests ILC generic functions.
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

#include <vector>

#include <catch2/catch_all.hpp>

#include <cRIO/MPU.h>
#include <cRIO/CliApp.h>
#include <Modbus/Parser.h>
#include <Transports/SimulatedTransport.h>

using namespace Transports;
using namespace LSST::cRIO;

class TestDevice : public MPU {
public:
    TestDevice();
};

TestDevice::TestDevice() : MPU(1) {}

class TestTransport : public SimulatedTransport {
public:
    TestTransport();
    void generate_response(const unsigned char* buf, size_t len) override;

private:
    BytesValue<uint64_t> _test_val;
};

TestTransport::TestTransport() { _test_val.value = 10; }

void TestTransport::generate_response(const unsigned char* buf, size_t len) {
    Modbus::Parser parser(std::vector<uint8_t>(buf, buf + len));
    _response.push_back(parser.address());
    switch (parser.func()) {
        case MPU::READ_HOLDING_REGISTERS: {
            uint16_t reg = parser.read<uint16_t>();
            uint16_t reg_len = parser.read<uint16_t>() * 2;

            _response.push_back(parser.func());
            _response.push_back(reg_len);

            for (size_t i = 0; i < reg_len; i += 2, reg++) {
                switch (reg) {
                    case 1000:
                        _test_val.value += 2;
                    case 1001:
                    case 1002:
                    case 1003:
                        _response.push_back(_test_val.bytes[7 - 2 * (reg - 1000)]);
                        _response.push_back(_test_val.bytes[6 - 2 * (reg - 1000)]);
                        break;
                    default:
                        _response.push_back(1);
                        _response.push_back(2);
                        break;
                }
            }
            break;
        }
        default:
            _response.push_back(Modbus::BusList::MODBUS_ERROR_MASK | parser.func());
            _response.push_back(1);
    }

    _response.writeCRC();
}

TEST_CASE("Test SimulatedTransport", "[SimulatedTransport]") {
    TestTransport transport;
    TestDevice test_device;

    test_device.readHoldingRegisters(1000, 4);
    transport.commands(test_device, std::chrono::seconds(1));

    CHECK(test_device.getRegister(1000) == 0);
    CHECK(test_device.getRegister(1001) == 0);
    CHECK(test_device.getRegister(1002) == 0);
    CHECK(test_device.getRegister(1003) == 12);

    test_device.reset();

    test_device.readHoldingRegisters(1000, 4);
    transport.commands(test_device, std::chrono::seconds(1));

    CHECK(test_device.getRegister(1000) == 0);
    CHECK(test_device.getRegister(1001) == 0);
    CHECK(test_device.getRegister(1002) == 0);
    CHECK(test_device.getRegister(1003) == 14);
}
