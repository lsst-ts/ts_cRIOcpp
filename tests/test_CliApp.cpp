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

#include <iostream>
#include <vector>

#include <catch2/catch_all.hpp>

#include <cRIO/CliApp.h>

using namespace LSST::cRIO;
using namespace Catch::Matchers;

class CClass : public CliApp {
public:
    CClass(const char* name, const char* description)
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

    int testDouble(command_vec cmds) {
        test_count++;
        CHECK(cmds.size() == 1);
        CHECK_THAT(std::stod(cmds[0], nullptr), WithinRel(M_PI, 0.00001));
        return 0;
    }

protected:
    void processArg(int opt, char* optarg) override;
};

void CClass::processArg(int opt, char* optarg) {
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
    CClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&CClass::testCmd, &cli, std::placeholders::_1), "s", 0,
                   "[ALL|command]", "Prints all command or command help.");

    const int argc = 3;
    const char* const argv[argc] = {"test", "testcmd", "tt"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    CHECK(cmds.size() == 2);
    CHECK(cmds[0] == "testcmd");
    CHECK(cmds[1] == "tt");

    CHECK(cli.test_count == 0);
    cli.processCmdVector(cmds);
    CHECK(cli.test_count == 1);
}

TEST_CASE("Test int format", "[CliApp]") {
    CClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&CClass::testInt, &cli, std::placeholders::_1), "I", 0, "[address]",
                   "Prints memory at given address.");

    const int argc = 1;
    const char* const argv[argc] = {"test"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    CHECK(cmds.size() == 0);

    cmds = {"testcmd", "8701"};

    CHECK(cli.test_count == 0);
    CHECK(cli.processCmdVector(cmds) == 0);
    CHECK(cli.test_count == 1);

    cmds = {"testcmd"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "0x21FD", "0x123A"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "21FD"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "0x21FD"};
    CHECK(cli.processCmdVector(cmds) == 0);

    CHECK(cli.test_count == 2);
}

TEST_CASE("Test Hex format", "[CliApp]") {
    CClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&CClass::testHex, &cli, std::placeholders::_1), "H", 0, "[address]",
                   "Prints memory at given address.");

    const int argc = 1;
    const char* const argv[argc] = {"test"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    CHECK(cmds.size() == 0);

    cmds = {"testcmd", "0x123A"};

    CHECK(cli.test_count == 0);
    CHECK(cli.processCmdVector(cmds) == 0);
    CHECK(cli.test_count == 1);

    cmds = {"testcmd", "123A"};
    CHECK(cli.processCmdVector(cmds) == 0);

    cmds = {"testcmd"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "0x123A", "0x123A"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "0x123AG"};
    CHECK(cli.processCmdVector(cmds) == -1);

    CHECK(cli.test_count == 2);
}

TEST_CASE("Test double format", "[CliApp]") {
    CClass cli("name", "description");
    cli.addCommand("testcmd", std::bind(&CClass::testDouble, &cli, std::placeholders::_1), "D", 0,
                   "[address]", "Set a parameter.");

    const int argc = 1;
    const char* const argv[argc] = {"test"};

    command_vec cmds = cli.processArgs(argc, (char**)argv);
    CHECK(cmds.size() == 0);

    cmds = {"testcmd", std::to_string(M_PI)};

    CHECK(cli.test_count == 0);
    CHECK(cli.processCmdVector(cmds) == 0);
    CHECK(cli.test_count == 1);

    cmds = {"testcmd"};
    CHECK(cli.processCmdVector(cmds) == -1);

    cmds = {"testcmd", "1.23f", "1.45f"};
    CHECK(cli.processCmdVector(cmds) == -1);

    CHECK(cli.test_count == 1);
}

TEST_CASE("Print decoded buffer", "[CliApp]") {
    std::ostringstream os1;
    std::vector<uint16_t> buf1({0x8000, 0x1233, 0x9233});
    CliApp::printDecodedBuffer(buf1.data(), buf1.size(), os1);
    CHECK(os1.str() == " invalid timestamp   ");

    std::ostringstream os2;
    std::vector<uint16_t> buf2({0x0000, 0x0000, 0x3b9a, 0xca00});
    CliApp::printDecodedBuffer(buf2.data(), buf2.size(), os2);
    CHECK(os2.str() == " TS:           1.000");

    std::ostringstream os3;
    std::vector<uint16_t> buf3({0x0012, 0x3456, 0x789a, 0xbcde, 0x8000, 0x1233, 0x9233});
    CliApp::printDecodedBuffer(buf3.data(), buf3.size(), os3);
    CHECK(os3.str() == " TS:     5124095.576 X    W 19 R 19");
}
