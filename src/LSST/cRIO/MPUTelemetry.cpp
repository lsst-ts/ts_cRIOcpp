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

#include <cRIO/MPUTelemetry.h>

using namespace LSST::cRIO;

MPUTelemetry::MPUTelemetry(uint8_t *data) {
    instructionPointer = be16toh(*(reinterpret_cast<uint16_t *>(data + 0)));
    outputCounter = be64toh(*(reinterpret_cast<uint64_t *>(data + 2)));
    inputCounter = be64toh(*(reinterpret_cast<uint64_t *>(data + 10)));
    outputTimeouts = be64toh(*(reinterpret_cast<uint64_t *>(data + 18)));
    inputTimeouts = be64toh(*(reinterpret_cast<uint64_t *>(data + 26)));
    instructionPointerOnError = be16toh(*(reinterpret_cast<uint16_t *>(data + 34)));
    writeTimeout = be16toh(*(reinterpret_cast<uint16_t *>(data + 36)));
    readTimeout = be16toh(*(reinterpret_cast<uint16_t *>(data + 38)));
    errorStatus = data[40];
    errorCode = be16toh(*(reinterpret_cast<uint16_t *>(data + 41)));
}
