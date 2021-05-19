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
    AClass(const char* description) : CliApp(description), interactive(false) {}
    bool interactive;

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

int test_count = 0;

int testCmd(command_vec cmds) {
    test_count++;
    return 42;
}

TEST_CASE("Test CliApp", "CliApp") {
    AClass cli("description");
    cli.addCommand("testcmd", testCmd, "s", 0, "[ALL|command]", "Prints all command or command help.");

    int argc = 3;
    const char* const argv[argc] = {"test", "testcmd", "tt"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 2);
    REQUIRE(cmds[0] == "testcmd");
    REQUIRE(cmds[1] == "tt");

    REQUIRE(test_count == 0);
    cli.processCmdVector(cmds);
    REQUIRE(test_count == 1);
}
