/*
 * This file is part of LSST cRIOcpp test suite. Tests ILC Sensor Monitor functions.
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

#include <ILC/SensorMonitor.h>
#include <Modbus/Parser.h>

using namespace ILC;

class TestSensorMonitor : public SensorMonitor {
public:
    TestSensorMonitor() : ILC::ILCBusList(1), SensorMonitor(1) {}

    std::vector<float> values;

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override {}

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override {}

    void processChangeILCMode(uint8_t address, uint16_t mode) override {}

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override {}

    void processResetServer(uint8_t address) override {}

    void processSensorValues(uint8_t address, std::vector<float> _values) override { values = _values; }
};

TEST_CASE("Process response of the SensorValue", "[SensorValues]") {
    TestSensorMonitor ilc;

    ilc.reportSensorValues(83);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 83);
    CHECK(parser.func() == 84);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(83);
    response.write<uint8_t>(84);
    for (size_t i = 0; i < 4; i++) {
        response.write<float>(i + 0.01f * i);
    }
    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));

    for (size_t i = 0; i < 4; i++) {
        CHECK(ilc.values[i] == i + 0.01f * i);
    }
}
