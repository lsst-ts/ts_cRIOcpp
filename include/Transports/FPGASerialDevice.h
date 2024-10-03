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

#ifndef __Transports_FPGASerialDevice__
#define __Transports_FPGASerialDevice__

#include <chrono>

#include "Transport.h"

namespace Transports {

/**
 * Communicate with various serial devices hooked on cRIO serial ports.
 */
class FPGASerialDevice : public Transport {
public:
    /**
     * Construct object to communicate with serial device connected to cRIO.
     * The FPGA in cRIO shall use SerialDevice.vi to communicate with the device.
     *
     * @param fpga_session
     * @param write_fifo
     * @param read_fifo
     */
    FPGASerialDevice(uint32_t fpga_session, int write_fifo, int read_fifo);

    void write(const unsigned char* buf, size_t len) override;

    std::vector<uint8_t> read(size_t len, std::chrono::microseconds timeout,
                              LSST::cRIO::Thread* calling_thread = NULL) override;

    void commands(Modbus::BusList& bus_list, std::chrono::microseconds timeout,
                  LSST::cRIO::Thread* calling_thread = NULL) override;

    void telemetry(uint64_t& write_bytes, uint64_t& read_bytes) override;

private:
    uint32_t _fpga_session;
    int _write_fifo;
    int _read_fifo;
};

}  // namespace Transports

#endif  // !__Transports_FPGASerialDevice__
