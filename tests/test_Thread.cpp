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
#include <catch2/catch.hpp>

#include <cRIO/Thread.h>

using namespace LSST::cRIO;
using namespace std::chrono_literals;

class TestThread : public Thread {
protected:
    void run() override {
        std::unique_lock<std::mutex> lock(runMutex);
        while (keepRunning) {
            runCondition.wait(lock);
            std::this_thread::sleep_for(100ms);
        }
    }
};

TEST_CASE("Test thread join without stop", "[Thread]") {
    TestThread thread;
    REQUIRE_NOTHROW(thread.join());

    REQUIRE(true);
}
