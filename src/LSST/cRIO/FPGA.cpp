/*
 * Interface for FPGA communication.
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

#include <cRIO/FPGA.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <chrono>
#include <string.h>
#include <thread>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

void FPGA::ilcCommands(uint16_t cmd, ILC &ilc) {
    size_t requestLen = ilc.getLength() + 4;
    uint16_t data[requestLen];
    data[0] = cmd;
    data[1] = ilc.getLength();
    memcpy(data + 2, ilc.getBuffer(), ilc.getLength());
    data[requestLen - 1] = 0x7000;
    // TODO the "ModbusSoftwareTrigger" constant is FPGA specific
    // some effort would be needed to standartize FPGA commands
    data[requestLen] = 252;

    writeCommandFIFO(data, requestLen, 0);

    std::this_thread::sleep_for(1ms);

    // TODO should specify IRQs
    waitOnIrqs(0x02, 5000);
    ackIrqs(0x02);

    // get back response
    writeRequestFIFO(13, 0);

    uint16_t responseLen;

    readU16ResponseFIFO(&responseLen, 1, 20);
    if (responseLen <= 4) {
        throw std::runtime_error("FPGA::ilcCommands timeout on reseponse");
    }

    uint16_t buffer[responseLen];
    readU16ResponseFIFO(buffer, responseLen, 10);

    // response shall follow this format:
    // 4 bytes (forming uint64_t in low endian) beginning timestamp
    // data received from ILCs (& 0x9000)
    // end of frame (0xA000)
    // 8 bytes of end timestamp (& 0xB000)
    uint64_t *beginD = reinterpret_cast<uint64_t *>(buffer);
    uint64_t beginTs = le64toh(*beginD);
    uint64_t endTs = 0;
    int endTsShift = 0;

    uint16_t *dataStart = NULL;
    for (uint16_t *p = buffer + 4; p < buffer + responseLen; p++) {
        switch (*p & 0xF000) {
            // data..
            case 0x9000:
                if (dataStart == NULL) {
                    dataStart = p;
                }
                break;
            case 0xA0000:
                ilc.processResponse(dataStart, p - dataStart);
                dataStart = NULL;
                break;
            case 0xB000:
                if (endTsShift == 64) {
                    throw std::runtime_error("End timestamp received twice!");
                }

                endTs |= static_cast<uint64_t>((*p) & 0x00FF) << endTsShift;
                endTsShift += 8;
                break;
            default:
                throw std::runtime_error(fmt::format("Invalid reply: {0:04x} ({0})", *p));
        }
    }

    reportTime(beginTs, endTs);
}

}  // namespace cRIO
}  // namespace LSST
