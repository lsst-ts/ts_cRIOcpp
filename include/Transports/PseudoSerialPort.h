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

#ifndef __Transports_PseudoSerialPort__
#define __Transports_PseudoSerialPort__

#include <chrono>
#include <thread>

#include "Transport.h"

namespace Transports {

/**
 * Encapsulates Transport to work as pseudo-serial port.
 */
class PseudoSerialPort : public Transport {
public:
    PseudoSerialPort(std::shared_ptr<Transport> real_port, const char* device_name);

    virtual ~PseudoSerialPort();

    void init_pt();

    void start();

    void open() override;
    void close() override;
    void write(const unsigned char* buf, size_t len) override;
    std::vector<uint8_t> read(size_t len, std::chrono::microseconds timeout,
                              LSST::cRIO::Thread* calling_thread = NULL) override;
    void commands(Modbus::BusList& bus_list, std::chrono::microseconds timeout,
                  LSST::cRIO::Thread* calling_thread = NULL) override;
    void flush() override;
    void telemetry(uint64_t& write_bytes, uint64_t& read_bytes) override;

    std::string tty_name;

protected:
    void run();

private:
    int _port_fd;

    std::string _device_name;

    int _buffer_len = 100;
    std::chrono::microseconds _read_timeout;

    std::shared_ptr<Transport> _real_port;

    std::thread* _thread;
};

}  // namespace Transports

#endif  // !__Transports_PseudoSerialPort__
