/*
 * Serial communication through FPGA FIFOs.
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

#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <spawn.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

#include <spdlog/spdlog.h>

#include "Transports/FPGASerialPort.h"

using namespace Transports;

FPGASerialPort::FPGASerialPort(uint32_t fpga_session, int write_fifo, int read_fifo, const char* device_name,
                               std::chrono::microseconds quiet_time)
        : FPGASerialDevice(fpga_session, write_fifo, read_fifo, quiet_time),
          _device_name(device_name),
          _read_timeout(1) {
    _port_fd = -1;
}

FPGASerialPort::~FPGASerialPort() {
    if (_port_fd > 0) {
        ::close(_port_fd);
    }
}

void FPGASerialPort::init_pt() {
    if (_port_fd > 0) {
        throw std::runtime_error(fmt::format("Port for device {} already assigned.", _device_name));
    }

    int _port_fd = posix_openpt(O_RDWR);
    grantpt(_port_fd);
    unlockpt(_port_fd);

    tty_name = ptsname(_port_fd);

    SPDLOG_DEBUG("Port {} created for device {}.", tty_name, _device_name);
}

void FPGASerialPort::run(std::unique_lock<std::mutex>& lock) {
    while (keepRunning) {
        auto data = read(_buffer_len, _read_timeout);

        if (data.size() > 0) {
            ::write(_port_fd, data.data(), data.size());
        }

        unsigned char buffer[_buffer_len];

        auto bytes = ::read(_port_fd, buffer, _buffer_len);
        if (bytes > 0) {
            write(buffer, bytes);
        }

        runCondition.wait(lock);
    }
}
