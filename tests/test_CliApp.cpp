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
    AClass(const char* description) : CliApp(description), interactive(false), test_reached(false) {}
    bool interactive;
    bool test_reached;

    int testCmd(command_vec cmds) {
        test_reached = true;
        return 42;
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
            std::cerr << "Unknow command: " << static_cast<char>(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

TEST_CASE("Test CliApp", "CliApp") {
    AClass cli("description");
    cli.addCommand("testcmd", [&cli](command_vec cmds) int { return cli.testCmd(cmds); }, "s", 0,
                   "[ALL|command]", "Prints all command or command help.");

    int argc = 2;
    const char* const argv[argc] = {"test", "testcmd"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    REQUIRE(cmds[0] == "testcmd");

    REQUIRE(cli.test_reached == false);
    cli.processCmdVector(cmds);
    REQUIRE(cli.test_reached == true);
}
