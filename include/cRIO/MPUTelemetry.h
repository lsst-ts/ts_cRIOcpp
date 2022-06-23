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

#ifndef CRIO_MPUTELEMETRY_H_
#define CRIO_MPUTELEMETRY_H_

#include <cstdint>

namespace LSST {
namespace cRIO {

class MPUTelemetry {
public:
    MPUTelemetry(uint8_t data[45]);
    uint16_t instructionPointer;
    uint64_t outputCounter;
    uint64_t inputCounter;
    uint64_t outputTimeouts;
    uint64_t inputTimeouts;
    uint16_t instructionPointerOnError;
    uint16_t writeTimeout;
    uint16_t readTimeout;
    uint8_t errorStatus;
    uint16_t errorCode;
    uint16_t modbusCRC;

    uint16_t calculatedCRC;

    /**
     * Check that received CRC matches calculated CRC.
     *
     * @throws std::runtime_exception on mismatch
     */
    void checkCRC();
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! CRIO_MPUTELEMETRY_H_
