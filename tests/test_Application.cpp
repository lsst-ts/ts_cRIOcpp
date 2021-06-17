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

#include <cRIO/Application.h>
#include <cRIO/Thread.h>

using namespace LSST::cRIO;
using namespace std::chrono_literals;

class AClass : public Application {
public:
    AClass(const char* description) : Application(description) {}

protected:
    void processArg(int opt, char* optarg) override;
};

void AClass::processArg(int opt, char* optarg) {
    switch (opt) {
        case 'h':
            printAppHelp();
            break;
        default:
            std::cerr << "Unknown argument: " << static_cast<char>(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

TEST_CASE("Test Application", "[Application]") {
    AClass app("description");
    app.addArgument('h', "print help");

    int argc = 3;
    const char* const argv[argc] = {"test", "-h", "tt"};

    command_vec cmds = app.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 1);
    REQUIRE(cmds[0] == "tt");
}

class Thread1 : public Thread {
    void run() override { std::this_thread::sleep_for(50ms); }
};

TEST_CASE("Test Application threading", "[Application]") {
    AClass app("test threading");

    auto t1_1 = new Thread1();
    auto t1_2 = new Thread1();

    app.addThread(t1_1);
    app.addThread(t1_2);

    REQUIRE(app.runningThreads() == 2);
    app.stopAllThreads();
    REQUIRE(app.runningThreads() == 0);
}

TEST_CASE("Test Thread management - stopping thread", "[Application]") {
    AClass app("test threading");

    auto t1_1 = new Thread1();
    auto t1_2 = new Thread1();

    app.addThread(t1_1);
    app.addThread(t1_2);

    REQUIRE(app.runningThreads() == 2);

    t1_1->stop();
    REQUIRE(app.runningThreads() == 1);

    app.stopAllThreads();
    REQUIRE(app.runningThreads() == 0);
}