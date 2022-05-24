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

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cRIO/CSC.h>
#include <cRIO/FPGA.h>

using namespace LSST::cRIO;

class TestFPGA : public FPGA {
public:
    TestFPGA() : FPGA(fpgaType::SS) {}
    void initialize() override {}
    void open() override {}
    void close() override {}
    void finalize() override {}
    uint16_t getTxCommand(uint8_t) override { return 0; }
    uint16_t getRxCommand(uint8_t) override { return 0; }
    uint32_t getIrq(uint8_t) override { return 0; }
    void writeMPUFIFO(MPU&) override {}
    void readMPUFIFO(MPU&) override {}
    void writeCommandFIFO(uint16_t*, size_t, uint32_t) override {}
    void writeRequestFIFO(uint16_t*, size_t, uint32_t) override {}
    void readU16ResponseFIFO(uint16_t*, size_t, uint32_t) override {}
    void waitOnIrqs(uint32_t, uint32_t, uint32_t*) override {}
    void ackIrqs(uint32_t) override {}
};

class TestCSC : public CSC {
public:
    TestCSC(const char* name, const char* description, bool call_done)
            : CSC(name, description), _call_done(call_done) {}

protected:
    void init() override;
    void done() override;
    int runLoop() override;

private:
    bool _call_done;
};

int _keepRunning = 1;

using namespace std::chrono_literals;

TestCSC csc("testcsc", "Test CSC object 1", true);

void _handler(int sig) {
    if (sig == SIGUSR1) {
        _keepRunning = 0;
    }
}

void TestCSC::init() {
    _keepRunning = 1;
    std::this_thread::sleep_for(2s);
    signal(SIGUSR1, &_handler);
    daemonOK();
}

void TestCSC::done() {
    if (_call_done) {
        kill(getppid(), SIGUSR2);
    }
}

int TestCSC::runLoop() {
    std::this_thread::sleep_for(10ms);
    return _keepRunning;
}

bool _child_shutdown = false;

void _childHandler(int sig) {
    if (sig == SIGUSR2) {
        _child_shutdown = true;
    }
}

TEST_CASE("Daemonize", "[CSC]") {
    optind = 1;

    const int argc = 4;
    char pid_template[200];
    strcpy(pid_template, "/tmp/test.pid-XXXXXX");
    char* pid_file = mktemp(pid_template);

    const char* const argv[argc] = {"test", "-p", pid_file, "TEST"};

    command_vec cmds = csc.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 1);
    REQUIRE(cmds[0] == "TEST");

    TestFPGA* fpga = new TestFPGA();

    csc.run(fpga);

    delete fpga;

    // open PID, kill what's in
    int pf = open(pid_file, O_RDONLY);
    REQUIRE(pf >= 0);
    char pid_buf[20];
    REQUIRE(read(pf, pid_buf, 20) > 0);
    REQUIRE(close(pf) == 0);

    int child_pid = std::stoi(pid_buf);
    REQUIRE(child_pid > 0);

    REQUIRE(signal(SIGUSR2, &_childHandler) != SIG_ERR);
    REQUIRE(_child_shutdown == false);
    REQUIRE(kill(child_pid, SIGUSR1) == 0);
    REQUIRE(unlink(pid_file) == 0);

    REQUIRE(waitpid(child_pid, NULL, 0) == child_pid);
    REQUIRE(_child_shutdown == true);
    REQUIRE(kill(child_pid, SIGUSR1) == -1);
    REQUIRE(errno == ESRCH);
}

void alarm_handler(int sig_t) { kill(getpid(), SIGUSR1); }

TEST_CASE("Run CSC", "[CSC]") {
    optind = 1;

    TestCSC csc2("testcsc2", "Test CSC object 2", false);
    const int argc = 2;
    const char* const argv[argc] = {"test2", "-f"};
    command_vec cmds = csc2.processArgs(argc, (char**)argv);
    REQUIRE(cmds.size() == 0);

    TestFPGA* fpga = new TestFPGA();

    _child_shutdown = false;
    REQUIRE(_child_shutdown == false);

    REQUIRE(signal(SIGALRM, &alarm_handler) != SIG_ERR);
    alarm(3);

    csc2.run(fpga);

    delete fpga;
    REQUIRE(_child_shutdown == false);
}
