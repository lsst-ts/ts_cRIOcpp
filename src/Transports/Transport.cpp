/*
 * Transport base class for communication.
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

#include "Transports/Transport.h"

using namespace Transports;

void Transport::execute_command(std::vector<uint8_t> command, Modbus::BusList& bus_list,
                                std::chrono::time_point<std::chrono::steady_clock> end,
                                LSST::cRIO::Thread* calling_thread) {
    auto now = std::chrono::steady_clock::now();
    if (now >= end) {
        throw std::runtime_error("Timeout while waiting for Transport response");
    }

    write(command.data(), command.size());

    int expected_len = 0;
    std::vector<uint8_t> answer;

    while (now < end) {
        // read reply
        auto chunk = read(expected_len, std::chrono::duration_cast<std::chrono::microseconds>(end - now),
                          calling_thread);

        answer.insert(answer.end(), chunk.begin(), chunk.end());

        expected_len = bus_list.responseLength(answer);

        if (expected_len < 0) {
            expected_len = 0;
        } else {
            expected_len -= answer.size();
            if (expected_len <= 0) {
                break;
            }
        }

        now = std::chrono::steady_clock::now();
    }

    if (answer.empty()) {
        throw std::runtime_error(fmt::format("Empty answer to {}", Modbus::hexDump(command)));
    }

    bus_list.parse(answer);
    bus_list.reset();
}
