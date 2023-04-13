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
#include <cstring>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cRIO/ElectromechanicalPneumaticILC.h>

using namespace LSST::cRIO;
using Catch::Approx;

class TestElectromechanicalPneumaticILC : public ElectromechanicalPneumaticILC {
public:
    TestElectromechanicalPneumaticILC() {
        auto set_nan = [](float a[4]) {
            for (int i = 0; i < 4; i++) {
                a[i] = std::nan("f");
            }
        };
        set_nan(responseMainADCK);
        set_nan(responseMainOffset);
        set_nan(responseMainSensitivity);
        set_nan(responseBackupADCK);
        set_nan(responseBackupOffset);
        set_nan(responseBackupSensitivity);

        primaryForce = NAN;
        secondaryForce = NAN;
    }

    float primaryForce;
    float secondaryForce;

    float responseMainADCK[4];
    float responseMainOffset[4];
    float responseMainSensitivity[4];
    float responseBackupADCK[4];
    float responseBackupOffset[4];
    float responseBackupSensitivity[4];

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override {}

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override {}

    void processChangeILCMode(uint8_t address, uint16_t mode) override {}

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override {}

    void processResetServer(uint8_t address) override {}

    void processHardpointForceStatus(uint8_t address, uint8_t status, int32_t encoderPosition,
                                     float loadCellForce) override {}

    virtual void processSAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce) override {
        primaryForce = primaryLoadCellForce;
        secondaryForce = NAN;
    }

    virtual void processDAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce,
                                       float secondaryLoadCellForce) override {
        primaryForce = primaryLoadCellForce;
        secondaryForce = secondaryLoadCellForce;
    }

    void processCalibrationData(uint8_t address, float mainADCK[4], float mainOffset[4],
                                float mainSensitivity[4], float backupADCK[4], float backupOffset[4],
                                float backupSensitivity[4]) override {
        memcpy(responseMainADCK, mainADCK, sizeof(responseMainADCK));
        memcpy(responseMainOffset, mainOffset, sizeof(responseMainOffset));
        memcpy(responseMainSensitivity, mainSensitivity, sizeof(responseMainSensitivity));

        memcpy(responseBackupADCK, backupADCK, sizeof(responseBackupADCK));
        memcpy(responseBackupOffset, backupOffset, sizeof(responseBackupOffset));
        memcpy(responseBackupSensitivity, backupSensitivity, sizeof(responseBackupSensitivity));
    }

    void processMezzaninePressure(uint8_t address, float primaryPush, float primaryPull, float secondaryPush,
                                  float secondaryPull) override;
};

void TestElectromechanicalPneumaticILC::processMezzaninePressure(uint8_t address, float primaryPush,
                                                                 float primaryPull, float secondaryPush,
                                                                 float secondaryPull) {
    REQUIRE(address == 18);
    REQUIRE(primaryPush == 3.141592f);
    REQUIRE(primaryPull == 1.3456f);
    REQUIRE(secondaryPush == -3.1468f);
    REQUIRE(secondaryPull == -127.657f);
}

TEST_CASE("Test SSA force set", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.setSAAForceOffset(18, true, 31.45);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE(ilc.read<uint8_t>() == 75);
    REQUIRE(ilc.read<uint8_t>() == 0xFF);
    REQUIRE(ilc.read<int24_t>().value == 31450);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(18);
    response.write<uint8_t>(75);
    response.write<uint8_t>(8);
    response.write<float>(31.45);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    REQUIRE(ilc.primaryForce == Approx(31.45));
    REQUIRE(isnan(ilc.secondaryForce));
}

TEST_CASE("Test DAA force set", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.setDAAForceOffset(212, false, -123.45, 345.10);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 212);
    REQUIRE(ilc.read<uint8_t>() == 75);
    REQUIRE(ilc.read<uint8_t>() == 0x00);
    REQUIRE(ilc.read<int24_t>().value == -123450);
    // hex(345100) = 0x05 44 0C
    REQUIRE(ilc.read<uint8_t>() == 0x05);
    REQUIRE(ilc.read<uint8_t>() == 0x44);
    REQUIRE(ilc.read<uint8_t>() == 0x0C);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(212);
    response.write<uint8_t>(75);
    response.write<uint8_t>(8);
    response.write<float>(-123.45);
    response.write<float>(345.10);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    REQUIRE(ilc.primaryForce == Approx(-123.45));
    REQUIRE(ilc.secondaryForce == Approx(345.10));
}

TEST_CASE("Test SAA force actuator readout", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.reportForceActuatorForceStatus(18);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE(ilc.read<uint8_t>() == 76);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(18);
    response.write<uint8_t>(76);
    response.write<uint8_t>(8);
    response.write<float>(13.7);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    REQUIRE(ilc.primaryForce == Approx(13.7));
    REQUIRE(isnan(ilc.secondaryForce));
}

TEST_CASE("Test DAA force actuator readout", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.reportForceActuatorForceStatus(23);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 23);
    REQUIRE(ilc.read<uint8_t>() == 76);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(23);
    response.write<uint8_t>(76);
    response.write<uint8_t>(8);
    response.write<float>(15.9);
    response.write<float>(-67.4);
    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    REQUIRE(ilc.primaryForce == Approx(15.9));
    REQUIRE(ilc.secondaryForce == Approx(-67.4));
}

TEST_CASE("Test set offset and sensitivity", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.setOffsetAndSensitivity(231, 1, 2.34, -4.56);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 231);
    REQUIRE(ilc.read<uint8_t>() == 81);
    REQUIRE(ilc.read<uint8_t>() == 1);
    REQUIRE(ilc.read<float>() == 2.34f);
    REQUIRE(ilc.read<float>() == -4.56f);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 37000);
}

TEST_CASE("Test parsing of calibration data", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.reportCalibrationData(17);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 17);
    REQUIRE(ilc.read<uint8_t>() == 110);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(17);
    response.write<uint8_t>(110);
    auto write4 = [&response](float base) {
        for (int i = 0; i < 4; i++) {
            response.write<float>(base * i);
        }
    };

    write4(3.141592);
    write4(2);
    write4(-56.3211);
    write4(2021.5788);
    write4(789564687.4545);
    write4(-478967.445456);

    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));

    auto check4 = [](float base, float values[4]) {
        for (int i = 0; i < 4; i++) {
            REQUIRE(values[i] == base * i);
        }
    };

    check4(3.141592, ilc.responseMainADCK);
    check4(2, ilc.responseMainOffset);
    check4(-56.3211, ilc.responseMainSensitivity);
    check4(2021.5788, ilc.responseBackupADCK);
    check4(789564687.4545, ilc.responseBackupOffset);
    check4(-478967.445456, ilc.responseBackupSensitivity);
}

TEST_CASE("Test parsing of pressure data", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc, response;

    ilc.reportMezzaninePressure(18);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE(ilc.read<uint8_t>() == 119);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE(ilc.readWaitForRx() == 1800);

    response.write<uint8_t>(18);
    response.write<uint8_t>(119);

    response.write<float>(3.141592f);
    response.write<float>(1.3456f);
    response.write<float>(-127.657f);
    response.write<float>(-3.1468f);

    response.writeCRC();

    REQUIRE_NOTHROW(ilc.processResponse(response.getBuffer(), response.getLength()));
}
