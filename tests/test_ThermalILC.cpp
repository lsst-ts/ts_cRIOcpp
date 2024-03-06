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

#include <memory>
#include <cmath>

#include <catch2/catch_test_macros.hpp>

#include <cRIO/ThermalILC.h>

using namespace LSST::cRIO;

class TestThermalILC : public ThermalILC {
public:
    TestThermalILC() : ILC::ILCBusList(1), ThermalILC(1) {
        responseStatus = 0;
        responseDifferentialTemperature = NAN;
        responseFanRPM = 0;
        responseAbsoluteTemperature = NAN;

        responseProportionalGain = NAN;
        responseIntegralGain = NAN;
    }

    uint8_t responseStatus;
    float responseDifferentialTemperature;
    uint8_t responseFanRPM;
    float responseAbsoluteTemperature;

    float responseProportionalGain;
    float responseIntegralGain;

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override {}

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override {}

    void processChangeILCMode(uint8_t address, uint16_t mode) override {}

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override {}

    void processResetServer(uint8_t address) override {}

    void processThermalStatus(uint8_t address, uint8_t status, float differentialTemperature, uint8_t fanRPM,
                              float absoluteTemperature) override {
        responseStatus = status;
        responseDifferentialTemperature = differentialTemperature;
        responseFanRPM = fanRPM;
        responseAbsoluteTemperature = absoluteTemperature;
    }

    void processReHeaterGains(uint8_t address, float proportionalGain, float integralGain) override {
        responseProportionalGain = proportionalGain;
        responseIntegralGain = integralGain;
    }
};

TEST_CASE("Test setThermalStatus", "[ThermalILC]") {
    TestThermalILC ilc;

    ilc.setThermalDemand(211, 222, 111);
    REQUIRE(ilc.size() == 1);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 211);
    CHECK(parser.func() == 88);
    CHECK(parser.read<uint8_t>() == 222);
    CHECK(parser.read<uint8_t>() == 111);
    REQUIRE_NOTHROW(parser.checkCRC());
}

TEST_CASE("Broadcast set heater & fan target values", "[ThermalILC]") {
    uint8_t values[NUM_TS_ILC];
    for (int i = 0; i < NUM_TS_ILC; i++) {
        values[i] = i;
    }

    TestThermalILC ilc;

    ilc.broadcastThermalDemand(values, values);
    REQUIRE(ilc.size() == 1);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 250);
    CHECK(parser.func() == 88);
    CHECK(parser.read<uint8_t>() == 1);
    for (int i = 0; i < NUM_TS_ILC; i++) {
        CHECK(parser.read<uint8_t>() == i);
        CHECK(parser.read<uint8_t>() == i);
    }
    REQUIRE_NOTHROW(parser.checkCRC());
}

TEST_CASE("Test parsing of thermal status response", "[ThermalILC]") {
    TestThermalILC ilc;

    ilc.reportThermalStatus(77);
    REQUIRE(ilc.size() == 1);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 77);
    CHECK(parser.func() == 89);
    REQUIRE_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(77);
    response.write<uint8_t>(89);
    response.write<uint8_t>(1);
    response.write<float>(34.567f);
    response.write<uint8_t>(0xff);
    response.write<float>(215.567f);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.parse(response));

    CHECK(ilc.responseStatus == 1);
    CHECK(ilc.responseDifferentialTemperature == 34.567f);
    CHECK(ilc.responseFanRPM == 0xff);
    CHECK(ilc.responseAbsoluteTemperature == 215.567f);
}

TEST_CASE("Test parsing of thermal re-heater gains response", "[ThermalILC]") {
    TestThermalILC ilc;

    ilc.reportReHeaterGains(78);
    REQUIRE(ilc.size() == 1);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 78);
    CHECK(parser.func() == 93);
    REQUIRE_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(78);
    response.write<uint8_t>(93);
    response.write<float>(31.355f);
    response.write<float>(678.234f);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.parse(response));

    CHECK(ilc.responseProportionalGain == 31.355f);
    CHECK(ilc.responseIntegralGain == 678.234f);
}
