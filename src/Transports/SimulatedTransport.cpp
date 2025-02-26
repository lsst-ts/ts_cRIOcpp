/*
 * Class supporting software simulated connections.
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

#include <Transports/SimulatedTransport.h>

using namespace Transports;

SimulatedTransport::SimulatedTransport() : _response(), _bytes_written(0), _bytes_read(0) {}

void SimulatedTransport::write(const unsigned char* buf, size_t len) {
    generate_response(buf, len);
    _bytes_written += len;
}

std::vector<uint8_t> SimulatedTransport::read(size_t len, std::chrono::microseconds, LSST::cRIO::Thread*) {
    if (len == 0) {
        len = 1;
    };
    if (len < _response.size()) {
        auto ret = std::vector<uint8_t>(_response.begin(), _response.begin() + len);
        _bytes_read += len;
        _response = std::vector<uint8_t>(_response.begin() + len, _response.end());
        return ret;
    }

    auto ret = _response;
    _response.clear();
    _bytes_read += ret.size();
    return ret;
}

void SimulatedTransport::commands(Modbus::BusList& bus_list, std::chrono::microseconds timeout,
                                  LSST::cRIO::Thread* calling_thread) {
    auto end = std::chrono::steady_clock::now() + timeout;

    for (auto cmd : bus_list) {
        execute_command(cmd.buffer, bus_list, end, calling_thread);
    }

    bus_list.clear();
}

void SimulatedTransport::flush() {}

void SimulatedTransport::telemetry(uint64_t& write_bytes, uint64_t& read_bytes) {
    write_bytes = _bytes_written;
    read_bytes = _bytes_read;
}
