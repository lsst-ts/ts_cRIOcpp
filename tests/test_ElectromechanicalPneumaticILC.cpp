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

#include <catch2/catch.hpp>

#include <cRIO/ElectromechanicalPneumaticILC.h>

using namespace LSST::cRIO;
using namespace Catch::Matchers;

class TestElectromechanicalPneumaticILC : public ElectromechanicalPneumaticILC {
public:
    TestElectromechanicalPneumaticILC() : ILC::ILCBusList(1), ElectromechanicalPneumaticILC(1) {
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
    }

    float primaryForce = NAN;
    float secondaryForce = NAN;

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

    void processStepperForceStatus(uint8_t address, uint8_t status, int32_t encoderPosition,
                                   float loadCellForce) override {}

    void processDCAGain(uint8_t address, float primaryGain, float secondaryGain) override {}

    void processHardpointLVDT(uint8_t address, float breakawayLVDT, float displacementLVDT) override {}

    void processSAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce) override {
        primaryForce = primaryLoadCellForce;
        secondaryForce = NAN;
    }

    void processDAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce,
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
    CHECK(address == 18);
    CHECK(primaryPush == 3.141592f);
    CHECK(primaryPull == 1.3456f);
    CHECK(secondaryPush == -3.1468f);
    CHECK(secondaryPull == -127.657f);
}

TEST_CASE("Test SSA force set", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.setSAAForceOffset(18, true, 31.45);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 18);
    CHECK(parser.func() == 75);
    CHECK(parser.read<uint8_t>() == 0xFF);
    CHECK(parser.read<Modbus::int24_t>().value == 31450);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(18);
    response.write<uint8_t>(75);
    response.write<uint8_t>(8);
    response.write<float>(31.45);
    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response));

    CHECK_THAT(ilc.primaryForce, WithinRel(31.45, 1e-6));
    CHECK(std::isnan(ilc.secondaryForce));
}

TEST_CASE("Test DAA force set", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.setDAAForceOffset(212, false, -123.45, 345.10);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 212);
    CHECK(parser.func() == 75);
    CHECK(parser.read<uint8_t>() == 0x00);
    CHECK(parser.read<Modbus::int24_t>().value == -123450);
    // hex(345100) = 0x05 44 0C
    CHECK(parser.read<uint8_t>() == 0x05);
    CHECK(parser.read<uint8_t>() == 0x44);
    CHECK(parser.read<uint8_t>() == 0x0C);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(212);
    response.write<uint8_t>(75);
    response.write<uint8_t>(8);
    response.write<float>(-123.45);
    response.write<float>(345.10);
    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));

    CHECK_THAT(ilc.primaryForce, WithinRel(-123.45, 1e-6));
    CHECK_THAT(ilc.secondaryForce, WithinRel(345.10, 1e-6));
}

TEST_CASE("Test SAA force actuator readout", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.reportForceActuatorForceStatus(18);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 18);
    CHECK(parser.func() == 76);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(18);
    response.write<uint8_t>(76);
    response.write<uint8_t>(8);
    response.write<float>(13.7);
    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));

    CHECK_THAT(ilc.primaryForce, WithinRel(13.7, 1e-6));
    CHECK(std::isnan(ilc.secondaryForce));
}

TEST_CASE("Test DAA force actuator readout", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.reportForceActuatorForceStatus(23);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 23);
    CHECK(parser.func() == 76);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(23);
    response.write<uint8_t>(76);
    response.write<uint8_t>(8);
    response.write<float>(15.9);
    response.write<float>(-67.4);
    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));

    CHECK_THAT(ilc.primaryForce, WithinRel(15.9, 1e-6));
    CHECK_THAT(ilc.secondaryForce, WithinRel(-67.4, 1e-6));
}

TEST_CASE("Test set offset and sensitivity", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.setOffsetAndSensitivity(231, 1, 2.34, -4.56);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 231);
    CHECK(parser.func() == 81);
    CHECK(parser.read<uint8_t>() == 1);
    CHECK(parser.read<float>() == 2.34f);
    CHECK(parser.read<float>() == -4.56f);
    CHECK_NOTHROW(parser.checkCRC());
}

TEST_CASE("Test parsing of calibration data", "[ElectromechanicalPneumaticILC]") {
    TestElectromechanicalPneumaticILC ilc;

    ilc.reportCalibrationData(17);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 17);
    CHECK(parser.func() == 110);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

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

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));

    auto check4 = [](float base, float values[4]) {
        for (int i = 0; i < 4; i++) {
            CHECK(values[i] == base * i);
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
    TestElectromechanicalPneumaticILC ilc;

    ilc.reportMezzaninePressure(18);

    Modbus::Parser parser(ilc[0].buffer);

    CHECK(parser.address() == 18);
    CHECK(parser.func() == 119);
    CHECK_NOTHROW(parser.checkCRC());

    Modbus::Buffer response;

    response.write<uint8_t>(18);
    response.write<uint8_t>(119);

    response.write<float>(3.141592f);
    response.write<float>(1.3456f);
    response.write<float>(-127.657f);
    response.write<float>(-3.1468f);

    response.writeCRC();

    CHECK_NOTHROW(ilc.parse(response.data(), response.size()));
}
