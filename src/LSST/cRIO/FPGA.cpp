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
#include <cRIO/ModbusBuffer.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <chrono>
#include <string.h>
#include <thread>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

FPGA::FPGA(fpgaType type) {
    switch (type) {
        case SS:
            _modbusSoftwareTrigger = 252;
            break;
        case TS:
            _modbusSoftwareTrigger = 252;
            break;
    }
}

void FPGA::ilcCommands(ILC &ilc) {
    size_t requestLen = ilc.getLength();
    if (requestLen == 0) {
        return;
    }
    requestLen += 5;

    uint16_t data[requestLen];

    uint8_t bus = ilc.getBus();

    data[0] = getTxCommand(bus);
    data[1] = ilc.getLength() + 2;
    data[2] = FIFO::TX_WAIT_TRIGGER;
    memcpy(data + 3, ilc.getBuffer(), ilc.getLength() * sizeof(uint16_t));
    data[requestLen - 2] = FIFO::TX_IRQTRIGGER;
    data[requestLen - 1] = _modbusSoftwareTrigger;

    writeCommandFIFO(data, requestLen, 0);

    std::this_thread::sleep_for(1ms);

    uint32_t irq = getIrq(bus);

    waitOnIrqs(irq, 5000);
    ackIrqs(irq);

    // get back response
    writeRequestFIFO(getRxCommand(bus), 0);

    uint16_t responseLen;

    readU16ResponseFIFO(&responseLen, 1, 20);
    if (responseLen <= 4) {
        if (responseLen > 0) {
            uint16_t buffer[responseLen];
            readU16ResponseFIFO(buffer, responseLen, 10);
        }
        throw std::runtime_error("FPGA::ilcCommands timeout on response: " + std::to_string(responseLen));
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
            case FIFO::RX_MASK & 0xF000:
                if (dataStart == NULL) {
                    dataStart = p;
                }
                break;
            case FIFO::RX_TIMESTAMP:
                if (endTsShift == 64) {
                    throw std::runtime_error("End timestamp received twice!");
                }

                endTs |= static_cast<uint64_t>((*p) & 0x00FF) << endTsShift;
                endTsShift += 8;
                // don't break here - data also ends when timestamp is received
            case FIFO::RX_ENDFRAME:
                if (dataStart) {
                    ilc.processResponse(dataStart, p - dataStart);
                    dataStart = NULL;
                    reportTime(beginTs, endTs);
                    beginTs = endTs;
                    endTs = 0;
                    endTsShift = 0;
                }
                break;
            default:
                throw std::runtime_error(fmt::format("Invalid reply: {0:04x} ({0})", *p));
        }
    }

    ilc.checkCommandedEmpty();

    reportTime(beginTs, endTs);
}

void FPGA::mpuCommands(MPU &mpu) {
    writeMPUFIFO(mpu);

    if (mpu.containsRead()) {
        readMPUFIFO(mpu);
    }
}

}  // namespace cRIO
}  // namespace LSST
