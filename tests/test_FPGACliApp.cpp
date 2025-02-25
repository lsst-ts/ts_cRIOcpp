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

#include <iostream>

#include <catch2/catch.hpp>

#include <cRIO/PrintILC.h>
#include <cRIO/FPGACliApp.h>

#include <TestFPGA.h>

using namespace LSST::cRIO;

int testRuns = 0;
int disabledCount = 0;

std::vector<std::pair<int, int>> disabled;

void testAction(ILCUnit u) {
    CHECK(!(u.first->getBus() == 1 && u.second == 20));
    CHECK(!(u.first->getBus() == 4 && u.second == 2));

    for (auto d : disabled) {
        CHECK(!(u.first->getBus() == d.first && u.second == d.second));
        disabledCount++;
    }

    testRuns++;
}

class AClass : public FPGACliApp {
public:
    AClass(const char* name, const char* description) : FPGACliApp(name, description) {
        addILC(std::make_shared<PrintILC>(1));
        addILC(std::make_shared<PrintILC>(4));

        addILCCommand("test", &testAction, "Test ILC calls");
    }

    FPGA* newFPGA(const char* dir, bool& fpga_singleton) override { return new TestFPGA(); }
    ILCUnits getILCs(command_vec arguments) override;
};

ILCUnits AClass::getILCs(command_vec arguments) {
    ILCUnits units;

    if (arguments.size() == 0) {
        units.push_back(ILCUnit(getILC(0), 2));
        units.push_back(ILCUnit(getILC(0), 3));

        units.push_back(ILCUnit(getILC(1), 10));
        units.push_back(ILCUnit(getILC(1), 11));
        return units;
    }

    for (auto a : arguments) {
        auto del = a.find("/");
        units.push_back(ILCUnit(getILC(stoi(a.substr(0, del))), stoi(a.substr(del + 1))));
    }

    return units;
}

TEST_CASE("Test FPGA CliApp", "[FPGACliApp]") { AClass cli("name", "description"); }

TEST_CASE("Test getILCs", "[FPGACliApp]") {
    AClass cli("name", "description");

    auto ilcs = cli.getILCs(command_vec{});
    CHECK(ilcs.size() == 4);
}

TEST_CASE("Test disable/enable ILC", "[FPGACliApp]") {
    AClass cli("name", "description");

    REQUIRE_NOTHROW(cli.processCmdVector({"open"}));

    CHECK(testRuns == 0);
    REQUIRE_NOTHROW(cli.processCmdVector({"test", "0/2"}));
    CHECK(testRuns == 1);

    disabled.push_back(std::pair<int, int>(1, 2));
    REQUIRE_NOTHROW(cli.processCmdVector({"@disable", "0/2"}));
    CHECK(testRuns == 1);

    CHECK(disabledCount == 0);
    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 4);
    CHECK(disabledCount == 3);

    testRuns = 0;
    disabledCount = 0;
    REQUIRE_NOTHROW(cli.processCmdVector({"@enable", "0/2"}));
    CHECK(testRuns == 0);

    disabled.clear();

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 4);
    CHECK(disabledCount == 0);
}

TEST_CASE("Tests multiple disable/enable", "[FPGACliApp]") {
    AClass cli("name", "description");

    REQUIRE_NOTHROW(cli.processCmdVector({"open"}));

    testRuns = 0;
    disabledCount = 0;

    disabled.push_back(std::pair<int, int>(4, 10));
    REQUIRE_NOTHROW(cli.processCmdVector({"@disable", "1/10"}));
    CHECK(testRuns == 0);
    CHECK(disabledCount == 0);

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 3);
    CHECK(disabledCount == 3);

    REQUIRE_NOTHROW(cli.processCmdVector({"@disable", "0/2"}));

    testRuns = 0;
    disabledCount = 0;

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 2);
    CHECK(disabledCount == 2);

    REQUIRE_NOTHROW(cli.processCmdVector({"@disable", "0/3"}));

    testRuns = 0;
    disabledCount = 0;

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 1);
    CHECK(disabledCount == 1);

    REQUIRE_NOTHROW(cli.processCmdVector({"@disable", "1/11"}));

    testRuns = 0;
    disabledCount = 0;

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 0);
    CHECK(disabledCount == 0);

    REQUIRE_NOTHROW(cli.processCmdVector({"@enable", "1/11"}));

    testRuns = 0;
    disabledCount = 0;

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 1);
    CHECK(disabledCount == 1);

    REQUIRE_NOTHROW(cli.processCmdVector({"@enable"}));

    testRuns = 0;
    disabledCount = 0;
    disabled.clear();

    REQUIRE_NOTHROW(cli.processCmdVector({"test"}));
    CHECK(testRuns == 4);
    CHECK(disabledCount == 0);
}
