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
    void run(std::unique_lock<std::mutex>& lock) override {
        while (keepRunning) {
            runCondition.wait(lock);
            std::this_thread::sleep_for(1ms);
        }
    }
};

TEST_CASE("Test thread join without stop", "[Thread]") {
    TestThread thread;
    thread.start();

    REQUIRE(true);
}

TEST_CASE("Test thread join with stop", "[Thread]") {
    TestThread thread;
    thread.start();

    REQUIRE_NOTHROW(thread.stop());

    REQUIRE(true);
}

TEST_CASE("Test thread destructor", "[Thread]") {
    TestThread* thread = new TestThread();
    thread->start();

    REQUIRE_NOTHROW(delete thread);
}

TEST_CASE("Test thread stop and destructor", "[Thread]") {
    TestThread* thread = new TestThread();
    thread->start();

    REQUIRE_NOTHROW(thread->stop());
    REQUIRE_NOTHROW(delete thread);
}
