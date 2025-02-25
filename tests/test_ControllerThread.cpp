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

#include <atomic>

#include <catch2/catch.hpp>

#include <cRIO/ControllerThread.h>
#include <cRIO/InterruptHandler.h>
#include <cRIO/Task.h>

#include <TestFPGA.h>

using namespace LSST::cRIO;
using namespace std::chrono_literals;

std::atomic<int> tv = 0;
std::atomic<int> iv = 0;

class TestTask : public Task {
public:
    task_return_t run() override {
        tv++;
        return 10000;
    }
};

TEST_CASE("Run ControllerThread and stop it", "[ControllerThread]") {
    tv = 0;
    REQUIRE(tv == 0);

    // run controller thread
    ControllerThread::instance().start();

    std::this_thread::sleep_for(100ms);

    // enqueue command into controller thread
    ControllerThread::instance().enqueue(std::make_shared<TestTask>());

    std::this_thread::sleep_for(1ms);

    // stop controller thread
    ControllerThread::instance().stop();

    REQUIRE(tv == 1);
}

TEST_CASE("Queue to controller before run", "[ControllerThread]") {
    tv = 0;
    REQUIRE(tv == 0);

    for (int i = 0; i < 10; i++) {
        ControllerThread::instance().enqueue(std::make_shared<TestTask>());
    }

    // run controller thread
    ControllerThread::instance().start();

    std::this_thread::sleep_for(10ms);

    // stop controller thread
    ControllerThread::instance().stop();

    REQUIRE(tv == 10);
}

TEST_CASE("Queue at different times", "[ControllerThread]") {
    tv = 0;
    REQUIRE(tv == 0);

    auto when = std::chrono::steady_clock::now() + 500ms;

    for (int i = 0; i < 3; i++) {
        ControllerThread::instance().enqueue_at(std::make_shared<TestTask>(), when);
        when -= 1ms;
    }

    when = std::chrono::steady_clock::now() + 200ms;

    for (int i = 0; i < 4; i++) {
        ControllerThread::instance().enqueue_at(std::make_shared<TestTask>(), when);
        when -= 1ms;
    }

    when = std::chrono::steady_clock::now();

    for (int i = 0; i < 2; i++) {
        ControllerThread::instance().enqueue_at(std::make_shared<TestTask>(), when);
        when += 1ms;
    }

    ControllerThread::instance().start();

    std::this_thread::sleep_for(100ms);

    CHECK(tv == 2);

    std::this_thread::sleep_for(200ms);

    CHECK(tv == 6);

    std::this_thread::sleep_for(300ms);

    CHECK(tv == 9);

    ControllerThread::instance().stop();

    REQUIRE(tv == 9);
}

std::atomic<int> rv = 0;

class TestRescheduledTask : public Task {
public:
    TestRescheduledTask(task_return_t wait) : _wait(wait) {}

    task_return_t run() override {
        rv += 1;
        return _wait;
    }

private:
    task_return_t _wait;
};

TEST_CASE("Task rescheduling", "[ControllerThread]") {
    rv = 0;
    REQUIRE(rv == 0);

    for (int i = 0; i < 3; i++) {
        ControllerThread::instance().enqueue(std::make_shared<TestRescheduledTask>(20));
    }

    for (int i = 0; i < 2; i++) {
        ControllerThread::instance().enqueue(std::make_shared<TestRescheduledTask>(30));
    }

    ControllerThread::instance().start();

    std::this_thread::sleep_for(22ms);

    CHECK(rv == 8);

    std::this_thread::sleep_for(10ms);

    CHECK(rv == 10);

    std::this_thread::sleep_for(70ms);

    CHECK(rv == 26);

    ControllerThread::instance().stop();

    REQUIRE(rv == 26);
}

class TestHandler : public InterruptHandler {
public:
    void handleInterrupt(uint8_t interrupt) override { iv++; }
};

class TestHandlerTask : public TestHandler, public Task {
public:
    task_return_t run() override {
        tv++;
        return 10000;
    }
};

class TestTaskHandler : public TestTask, public InterruptHandler {
public:
    void handleInterrupt(uint8_t interrupt) override { iv++; }
};

TEST_CASE("Handling interrupts", "[ControllerThread]") {
    iv = 0;
    REQUIRE(iv == 0);

    for (int i = 0; i < 3; i++) {
        ControllerThread::instance().enqueue(std::make_shared<TestRescheduledTask>(20));
    }

    for (int i = 0; i < 3; i++) {
        ControllerThread::instance().setInterruptHandler(std::make_shared<TestHandler>(), i + 1);
    }

    TestFPGA fpga;
    fpga.setSimulatedIRQs(0x7);

    TestILC ilc(1);

    REQUIRE_NOTHROW(ControllerThread::instance().startInterruptWatcherTask(&fpga));

    ControllerThread::instance().start();

    std::this_thread::sleep_for(22ms);

    CHECK(iv == 6);

    ControllerThread::instance().stop();

    REQUIRE(iv == 6);
}
