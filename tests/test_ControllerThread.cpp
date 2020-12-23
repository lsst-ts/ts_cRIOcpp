/*
 * This file is part of LSST cRIO CPP tests. Tests ControllerThread.
 *
 * Developed for the Telescope & Site Software Systems.  This product includes
 * software developed by the LSST Project (https://www.lsst.org). See the
 * COPYRIGHT file at the top-level directory of this distribution for details
 * of code ownership.
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

#include <cRIO/Command.h>
#include <cRIO/ControllerThread.h>

#include <thread>

using namespace LSST::cRIO;
using namespace std::chrono_literals;

int tv = 0;

class TestCommand : public Command {
public:
    void execute() override { tv++; }
};

TEST_CASE("Run ControllerThread and join it", "[ControllerThread]") {
    REQUIRE(tv == 0);

    // run controller thread
    std::thread controller([] { ControllerThread::get().run(); });

    // enqueue command into controller thread
    ControllerThread::get().enqueue(new TestCommand());

    std::this_thread::sleep_for(100ms);

    // stop controller thread
    ControllerThread::get().stop();

    REQUIRE(tv == 1);

    controller.join();
}
