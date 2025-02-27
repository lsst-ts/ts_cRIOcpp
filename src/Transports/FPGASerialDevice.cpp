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

#include <cassert>

#include "cRIO/NiError.h"
#include "NiFpga/NiFpga.h"
#include "Transports/FPGASerialDevice.h"

using namespace LSST::cRIO;
using namespace Transports;

FPGASerialDevice::FPGASerialDevice(uint32_t fpga_session, int write_fifo, int read_fifo,
                                   std::chrono::microseconds quiet_time)
        : _fpga_session(fpga_session),
          _write_fifo(write_fifo),
          _read_fifo(read_fifo),
          _quiet_time(quiet_time) {}

void FPGASerialDevice::write(const unsigned char* buf, size_t len) {
    assert(len < 255);

    uint8_t header[2] = {1, static_cast<uint8_t>(len)};
    NiThrowError("Writing FIFO write header",
                 NiFpga_WriteFifoU8(_fpga_session, _write_fifo, header, 2, 0, NULL));
    NiThrowError("Writing FIFO write data",
                 NiFpga_WriteFifoU8(_fpga_session, _write_fifo, reinterpret_cast<const uint8_t*>(buf), len, 0,
                                    NULL));
}

std::vector<uint8_t> FPGASerialDevice::read(size_t len, std::chrono::microseconds timeout,
                                            LSST::cRIO::Thread* thread) {
    auto now = std::chrono::steady_clock::now();
    auto end = now + timeout;

    std::vector<uint8_t> ret;

    bool not_run = true;

    do {
        if (not_run == false) {
            auto end_wait = std::chrono::steady_clock::now() + std::chrono::milliseconds(10);

            auto end_time = end_wait < end ? end_wait : end;

            if (thread != NULL) {
                thread->wait_until(end_time);
            } else {
                std::this_thread::sleep_until(end_time);
            }
        } else {
            not_run = true;
        }

        uint8_t data[255];

        uint8_t req = 2;
        NiThrowError("Reading FIFO requesting response",
                     NiFpga_WriteFifoU8(_fpga_session, _write_fifo, &req, 1, 0, NULL));

        uint8_t response[2];
        NiThrowError("Reading FIFO reading response and its length",
                     NiFpga_ReadFifoU8(_fpga_session, _read_fifo, response, 2, 1, NULL));

        if (response[0] != 2) {
            throw std::runtime_error(
                    fmt::format("Invalid reply from FIFO #{} - {}, expected 2", _read_fifo, response[0]));
        }
        if (response[1] != 0) {
            NiThrowError("ThermalFPGA::readMPUFIFO: reading response",
                         NiFpga_ReadFifoU8(_fpga_session, _read_fifo, data, response[1], 0, NULL));

            ret.insert(ret.end(), data, data + response[1]);
            if (ret.size() >= len) {
                break;
            }
        }

        now = std::chrono::steady_clock::now();
    } while (end > now);

    return ret;
}

void FPGASerialDevice::commands(Modbus::BusList& bus_list, std::chrono::microseconds timeout,
                                Thread* calling_thread) {
    auto end = std::chrono::steady_clock::now() + timeout;

    for (auto cmd : bus_list) {
        execute_command(cmd.buffer, bus_list, end, calling_thread);
        std::this_thread::sleep_for(_quiet_time);
    }

    bus_list.clear();
}

void FPGASerialDevice::flush() {
    uint8_t req = 3;
    NiThrowError("Reading FIFO requesting port flush",
                 NiFpga_WriteFifoU8(_fpga_session, _write_fifo, &req, 1, 0, NULL));

    uint8_t response;
    NiThrowError("Reading FIFO flush response ",
                 NiFpga_ReadFifoU8(_fpga_session, _read_fifo, &response, 1, 1, NULL));

    if (response != req) {
        throw std::runtime_error(
                fmt::format("Invalid response from FIFO #{} on flush request - expected 3, recieved {}",
                            _read_fifo, response));
    }
}

void FPGASerialDevice::telemetry(uint64_t& write_bytes, uint64_t& read_bytes) {
    uint8_t req = 0;
    NiThrowError("Reading FIFO requesting telemetry",
                 NiFpga_WriteFifoU8(_fpga_session, _write_fifo, &req, 1, 0, NULL));

    uint8_t response[17];
    NiThrowError("Reading FIFO reading telemetry",
                 NiFpga_ReadFifoU8(_fpga_session, _read_fifo, response, 17, 1, NULL));

    if (response[0] != 0) {
        throw std::runtime_error(
                fmt::format("Invalid response from FIFO #{} on telemetry request - expected 0, received {}",
                            _read_fifo, response[0]));
    }

    write_bytes = be64toh(*(reinterpret_cast<const uint64_t*>(response + 1)));
    read_bytes = be64toh(*(reinterpret_cast<const uint64_t*>(response + 9)));
}
