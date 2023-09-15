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

/**
 * Class representing readouts of MPU telemetry.
 */
class MPUTelemetry {
public:
    /**
     * Construct MPUTelemetry from data received from FPGA.
     *
     * @param data[45] data as received from FPGA
     */
    MPUTelemetry(uint8_t* data);
    uint32_t writeBytes;     /// Number of bytes written
    uint32_t readBytes;      /// Number of bytes read
    uint16_t readTimedout;   /// read timedout counter

    friend std::ostream& operator<<(std::ostream& os, const MPUTelemetry& tel);

    /**
     * Updates MPU telemetry SAL message structure.
     *
     * @tparam message class with telemetry filed. Shall be SAL declared class
     * @param msg message to check and update
     *
     * @return true if message shall be send, false if updates are minor and it should not be send
     */
    template <class message>
    bool sendUpdates(message* msg) {
        return false;

        /**
    if (msg->readBytes != readBytes || msg->writeTimedout != writeTimedout ||
        msg->instructionPointerOnError != instructionPointerOnError || msg->errorCode != errorCode) {
        send = true;
    }
    msg->instructionPointer = instructionPointer;
    msg->outputCounter = outputCounter;
    msg->inputCounter = inputCounter;
    msg->outputTimeouts = outputTimeouts;
    msg->inputTimeouts = inputTimeouts;
    msg->instructionPointerOnError = instructionPointerOnError;
    msg->writeTimeout = writeTimeout;
    msg->readTimeout = readTimeout;
    msg->errorCode = errorCode;
    return send; */
    }
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! CRIO_MPUTELEMETRY_H_
