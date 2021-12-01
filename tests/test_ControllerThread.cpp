/*
 * This file is part of LSST cRIO CPP tests. Tests ControllerThread.
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

#include <cRIO/Command.h>
#include <cRIO/ControllerThread.h>

using namespace LSST::cRIO;
using namespace std::chrono_literals;

int tv;

class TestCommand : public Command {
public:
    void execute() override { tv++; }
};

TEST_CASE("Run ControllerThread and stop it", "[ControllerThread]") {
    tv = 0;
    REQUIRE(tv == 0);

    // run controller thread
    ControllerThread::instance().start();

    std::this_thread::sleep_for(100ms);

    // enqueue command into controller thread
    ControllerThread::instance().enqueue(new TestCommand());

    std::this_thread::sleep_for(1ms);

    // stop controller thread
    ControllerThread::instance().stop();

    REQUIRE(tv == 1);
}

TEST_CASE("Queue to controller before run", "[ControllerThread]") {
    tv = 0;
    REQUIRE(tv == 0);

    for (int i = 0; i < 10; i++) {
        ControllerThread::instance().enqueue(new TestCommand());
    }

    // run controller thread
    ControllerThread::instance().start();

    std::this_thread::sleep_for(10ms);

    // stop controller thread
    ControllerThread::instance().stop();

    REQUIRE(tv == 10);
}
