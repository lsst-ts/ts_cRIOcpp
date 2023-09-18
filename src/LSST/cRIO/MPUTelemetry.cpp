/*
 * Telemetry for Modbus Processing Unit.
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

#include <endian.h>
#include <iomanip>

#include <spdlog/fmt/fmt.h>

#include <cRIO/ModbusBuffer.h>
#include <cRIO/MPUTelemetry.h>

namespace LSST {
namespace cRIO {

MPUTelemetry::MPUTelemetry(uint8_t *data) {
    writeBytes = be32toh(*(reinterpret_cast<uint32_t *>(data + 0)));
    readBytes = be32toh(*(reinterpret_cast<uint32_t *>(data + 4)));
    readTimedout = be16toh(*(reinterpret_cast<uint16_t *>(data + 8)));
}

std::ostream &operator<<(std::ostream &os, const MPUTelemetry &tel) {
    os << std::setw(20) << "Write bytes: " << tel.writeBytes << std::endl
       << std::setw(20) << "Read bytes: " << tel.readBytes << std::endl
       << std::setw(20) << "Read timedout: " << tel.readTimedout << std::endl;
    return os;
}

}  // namespace cRIO
}  // namespace LSST
