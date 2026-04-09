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

#ifndef __Transports_FPGASerialPort__
#define __Transports_FPGASerialPort__

#include <chrono>

#include "cRIO/Thread.h"
#include "FPGASerialDevice.h"

namespace Transports {

/**
 * Thread running serial port communication, using FPGA resources. A virtual
 * (pseudo serial) port is opened and offered to external programs to
 * communicate with the device.
 */
class FPGASerialPort : public FPGASerialDevice, public LSST::cRIO::Thread {
public:
    FPGASerialPort(uint32_t fpga_session, int write_fifo, int read_fifo, const char* device_name,
                   std::chrono::microseconds quiet_time);

    virtual ~FPGASerialPort();

    /**
     * Opens and initialize pseudo-serial connection.
     */
    void init_pt();

    std::string tty_name;

protected:
    void run(std::unique_lock<std::mutex>& lock) override;

private:
    int _port_fd;

    std::string _device_name;

    int _buffer_len = 100;
    std::chrono::microseconds _read_timeout;
};

}  // namespace Transports

#endif  // !__Transports_FPGASerialPort__
