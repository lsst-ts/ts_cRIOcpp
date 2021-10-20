/*
 * This file is part of LSST cRIOcpp test suite. Tests FPGA Cli App.
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

#include <cRIO/PrintILC.h>
#include <cRIO/FPGACliApp.h>
#include <iostream>

using namespace LSST::cRIO;

class AClass : public FPGACliApp {
public:
    AClass(const char* name, const char* description) : FPGACliApp(name, description) {
        addILC(std::make_shared<PrintILC>(1));
        addILC(std::make_shared<PrintILC>(4));
    }

    FPGA* newFPGA(const char* dir) override { return NULL; }
    ILCUnits getILCs(command_vec arguments) override {
        ILCUnits units;
        return units;
    }

    void test();
};

void AClass::test() {
    REQUIRE(getILC(0)->getBus() == 1);
    REQUIRE(getILC(1)->getBus() == 4);
}

TEST_CASE("Test CliApp", "[CliApp]") {
    AClass cli("name", "description");

    cli.test();
}
