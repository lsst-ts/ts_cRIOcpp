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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <memory>
#include <cmath>

#include <cRIO/ThermalILC.h>

using namespace LSST::cRIO;

class TestThermalILC : public ThermalILC {
public:
    TestThermalILC() {
        responseStatus = 0;
        responseDifferentialTemperature = std::nan("f");
        responseFanRPM = 0;
        responseAbsoluteTemperature = std::nan("f");
    }

    uint8_t responseStatus;
    float responseDifferentialTemperature;
    uint8_t responseFanRPM;
    float responseAbsoluteTemperature;

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
};

TEST_CASE("Test setThermalStatus", "[ThermalILC]") {
    TestThermalILC ilc;

    ilc.setThermalDemand(211, 222, 111);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 211);
    REQUIRE(ilc.read<uint8_t>() == 88);
    REQUIRE(ilc.read<uint8_t>() == 222);
    REQUIRE(ilc.read<uint8_t>() == 111);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 500);
}

TEST_CASE("Broadcast set heater & fan target values", "[ThermalILC]") {
    uint8_t values[NUM_TS_ILC];
    for (int i = 0; i < NUM_TS_ILC; i++) {
        values[i] = i;
    }

    TestThermalILC ilc;

    ilc.broadcastThermalDemand(values, values);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 250);
    REQUIRE(ilc.read<uint8_t>() == 88);
    REQUIRE(ilc.read<uint8_t>() == 1);
    for (int i = 0; i < NUM_TS_ILC; i++) {
        REQUIRE(ilc.read<uint8_t>() == i);
        REQUIRE(ilc.read<uint8_t>() == i);
    }
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readDelay() == 450);
}

TEST_CASE("Test parsing of thermal status response", "[ThermalILC]") {
    TestThermalILC ilc, response;

    ilc.reportThermalStatus(77);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 77);
    REQUIRE(ilc.read<uint8_t>() == 89);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 300);

    response.write<uint8_t>(77);
    response.write<uint8_t>(89);
    response.write<uint8_t>(1);
    response.write<float>(34.567f);
    response.write<uint8_t>(0xff);
    response.write<float>(215.567f);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    REQUIRE(ilc.responseStatus == 1);
    REQUIRE(ilc.responseDifferentialTemperature == 34.567f);
    REQUIRE(ilc.responseFanRPM == 0xff);
    REQUIRE(ilc.responseAbsoluteTemperature == 215.567f);
}
