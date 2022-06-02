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

#include <atomic>

#include <catch2/catch_test_macros.hpp>

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
    REQUIRE_NOTHROW(thread.start());

    REQUIRE(true);
}

TEST_CASE("Test thread join with stop", "[Thread]") {
    TestThread thread;
    REQUIRE_NOTHROW(thread.start());

    REQUIRE(thread.joinable() == true);
    // wait for more than default 2ms, as thread is sleeping for 1ms and processing
    // on CI can sometimes take longer then 1ms
    REQUIRE_NOTHROW(thread.stop(5ms));
    REQUIRE(thread.joinable() == false);

    REQUIRE(true);
}

TEST_CASE("Test thread multiple stop calls", "[Thread]") {
    TestThread thread;
    REQUIRE_NOTHROW(thread.start());

    REQUIRE(thread.joinable() == true);
    REQUIRE_NOTHROW(thread.stop(5ms));
    REQUIRE(thread.joinable() == false);
    REQUIRE_NOTHROW(thread.stop());
    REQUIRE(thread.joinable() == false);

    REQUIRE(true);
}

std::atomic<int> stop_calls(0);
std::atomic<int> stop_success(0);
std::atomic<int> stop_failed(0);

class StopThread : public Thread {
public:
    StopThread(TestThread* testThread) : _testThread(testThread) {}
    void run(std::unique_lock<std::mutex>& lock) override {
        while (keepRunning) {
            try {
                _testThread->stop(10us);
                stop_success++;
            } catch (std::runtime_error&) {
                stop_failed++;
            }

            stop_calls++;
            runCondition.wait_for(lock, 1ms);
        }
    }

private:
    TestThread* _testThread;
};

TEST_CASE("Test thread multiple stop calls from multiple threads", "[Thread]") {
    TestThread thread;
    REQUIRE_NOTHROW(thread.start());

    REQUIRE(thread.joinable() == true);

    StopThread* stops[20];
    for (int i = 0; i < 20; i++) {
        stops[i] = new StopThread(&thread);
        REQUIRE_NOTHROW(stops[i]->start(5ms));
    }

    std::this_thread::sleep_for(10ms);

    REQUIRE(thread.joinable() == false);
    REQUIRE(stop_calls > 20);
    REQUIRE(stop_success > 0);
    REQUIRE(stop_success + stop_failed == stop_calls);

    for (auto i = 0; i < 10; i++) {
        REQUIRE_NOTHROW(stops[i]->stop());
        REQUIRE_NOTHROW(delete stops[i]);
    }

    for (auto i = 10; i < 20; i++) {
        REQUIRE_NOTHROW(delete stops[i]);
    }

    REQUIRE(true);
}

TEST_CASE("Test thread destructor", "[Thread]") {
    TestThread* thread = new TestThread();
    REQUIRE_NOTHROW(thread->start());

    REQUIRE_NOTHROW(delete thread);
}

TEST_CASE("Test thread stop and destructor", "[Thread]") {
    TestThread* thread = new TestThread();
    REQUIRE_NOTHROW(thread->start());

    REQUIRE_NOTHROW(thread->stop());
    REQUIRE_NOTHROW(delete thread);
}

TEST_CASE("Test thread is running", "[Thread]") {
    TestThread* thread = new TestThread();
    REQUIRE_NOTHROW(thread->start());

    REQUIRE_NOTHROW(thread->isRunning() == true);

    REQUIRE_NOTHROW(thread->stop());

    REQUIRE_NOTHROW(thread->isRunning() == false);

    REQUIRE_NOTHROW(delete thread);
}
