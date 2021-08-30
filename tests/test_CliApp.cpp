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
#include <catch/catch.hpp>

#include <cRIO/CliApp.h>
#include <iostream>

using namespace LSST::cRIO;

class AClass : public CliApp {
public:
    AClass(const char* name, const char* description)
            : CliApp(name, description), interactive(false), test_count(0) {}
    bool interactive;
    int test_count;

    int testCmd(command_vec cmds) {
        test_count++;
        return 42;
    }

    int testHex(command_vec cmds) {
        test_count++;
        CHECK(cmds.size() == 1);
        CHECK(std::stoi(cmds[0], nullptr, 16) == 0x123A);
        return 0;
    }

    int testInt(command_vec cmds) {
        test_count++;
        CHECK(cmds.size() == 1);
        CHECK(std::stoi(cmds[0], nullptr, 0) == 0x21FD);
        return 0;
    }

protected:
    void processArg(int opt, char* optarg) override;
};

void AClass::processArg(int opt, char* optarg) {
    switch (opt) {
        case 'h':
            printAppHelp();
            break;
        case 'i':
            interactive = true;
            break;
        default:
            std::cerr << "Unknown argument: " << static_cast<char>(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

TEST_CASE("Test CliApp", "[CliApp]") {
    AClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&AClass::testCmd, &cli, std::placeholders::_1), "s", 0,
                   "[ALL|command]", "Prints all command or command help.");

    int argc = 3;
    const char* const argv[argc] = {"test", "testcmd", "tt"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 2);
    REQUIRE(cmds[0] == "testcmd");
    REQUIRE(cmds[1] == "tt");

    REQUIRE(cli.test_count == 0);
    cli.processCmdVector(cmds);
    REQUIRE(cli.test_count == 1);
}

TEST_CASE("Test Hex format", "[CliApp]") {
    AClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&AClass::testHex, &cli, std::placeholders::_1), "H", 0, "[address]",
                   "Prints memory at given address.");

    int argc = 1;
    const char* const argv[argc] = {"test"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 0);

    command_vec cmds_in = {"testcmd", "0x123A"};

    REQUIRE(cli.test_count == 0);
    cli.processCmdVector(cmds_in);
    REQUIRE(cli.test_count == 1);

    command_vec cmds_failed = {"testcmd"};
    REQUIRE(cli.processCmdVector(cmds_failed) == -1);

    command_vec cmds_failed_2 = {"testcmd", "0x123A", "0x123A"};
    REQUIRE(cli.processCmdVector(cmds_failed_2) == -1);
}
