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

#include <cstdlib>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cRIO/CSC.h>

using namespace LSST::cRIO;

class TestCSC : public CSC {
public:
    int _keepRunning;

protected:
    virtual void init();
    virtual int runLoop();
};

void _handler(int sig) {
    if (sig == SIGUSR1) {
        dynamic_cast<TestCSC*>(&TestCSC::instance())->_keepRunning = 0;
    }
};

using namespace std::chrono_literals;

void TestCSC::init() {
    _keepRunning = 1;
    signal(SIGUSR1, _handler);
    printf("OK\n");
}

int TestCSC::runLoop() {
    std::this_thread::sleep_for(10ms);
    return _keepRunning;
}

TEST_CASE("Daemonize", "[Daemonize]") {
    int argc = 4;
    char pid_template[200];
    strcpy(pid_template, "/tmp/test.pid-XXXXXX");
    char* pid_file = mktemp(pid_template);

    char* argv[argc] = {"test", "-p", pid_file, "TEST"};

    command_vec cmds = TestCSC::instance().processArgs(argc, argv);
    REQUIRE(cmds.size() == 1);
    REQUIRE(cmds[0] == "TEST");

    TestCSC::instance().run();

    // open PID, kill what's in
    int pf = open(pid_file, O_RDONLY);
    REQUIRE(pf >= 0);
    char pid_buf[20];
    REQUIRE(read(pf, pid_buf, 20) > 0);
    REQUIRE(close(pf) == 0);

    int child_pid = std::stoi(pid_buf);
    REQUIRE(child_pid > 0);
    REQUIRE(kill(child_pid, SIGUSR1) == 0);
    REQUIRE(unlink(pid_file) == 0);

    REQUIRE(waitpid(child_pid, NULL, 0) == child_pid);
    REQUIRE(kill(child_pid, SIGUSR1) == -1);
    REQUIRE(errno == ESRCH);
}
