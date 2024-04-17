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

#include <string.h>
#include <thread>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <cRIO/FPGA.h>
#include <cRIO/MPU.h>

namespace LSST {
namespace cRIO {

FPGA::FPGA(fpgaType type) : SimpleFPGA(type) {
    switch (type) {
        case SS:
            _modbusSoftwareTrigger = 252;
            break;
        case TS:
            _modbusSoftwareTrigger = 252;
            break;
        case M2:
        case VMS:
            _modbusSoftwareTrigger = 0;
            break;
    }
}

void FPGA::ilcCommands(ILC::ILCBusList &ilc, int32_t timeout) {
    // no messages to send
    if (ilc.size() == 0) {
        return;
    }
    // construct buffer to send
    std::vector<uint16_t> data;

    uint8_t bus = ilc.getBus();

    data.push_back(getTxCommand(bus));
    data.push_back(0);
    data.push_back(FIFO::TX_WAIT_TRIGGER);
    data.push_back(FIFO::TX_TIMESTAMP);

    for (auto cmd : ilc) {
        for (auto b : cmd.buffer) {
            data.push_back(FIFO::TX_MASK | ((static_cast<uint16_t>(b)) << 1));
        }
        data.push_back(FIFO::TX_FRAMEEND);
        data.push_back(cmd.timming > 0x0FFF ? ((0x0FFF & ((cmd.timming / 1000) + 1)) | FIFO::TX_WAIT_LONG_RX)
                                            : (cmd.timming | FIFO::TX_WAIT_RX));
    }

    data.push_back(FIFO::TX_IRQTRIGGER);
    data[1] = data.size() - 2;

    data.push_back(_modbusSoftwareTrigger);

    writeCommandFIFO(data.data(), data.size(), 0);

    uint32_t irq = getIrq(bus);

    bool timedout = false;

    waitOnIrqs(irq, timeout, timedout);
    ackIrqs(irq);

    // get back response
    writeRequestFIFO(getRxCommand(bus), 0);

    uint16_t responseLen;

    readU16ResponseFIFO(&responseLen, 1, 20);
    if (responseLen < 4) {
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

    ilc.reset();

    std::vector<uint8_t> decoded;
    for (uint16_t *p = buffer + 4; p < buffer + responseLen; p++) {
        switch (*p & 0xF000) {
            // data..
            case FIFO::RX_MASK & 0xF000:
                decoded.push_back(static_cast<uint8_t>((*p >> 1) & 0xFF));
                break;
            case FIFO::RX_TIMESTAMP:
                if (endTsShift == 64) {
                    throw std::runtime_error("End timestamp received twice!");
                }

                endTs |= static_cast<uint64_t>((*p) & 0x00FF) << endTsShift;
                endTsShift += 8;
                // don't break here - data also ends when timestamp is received
            case FIFO::RX_ENDFRAME:
                if (decoded.empty() == false) {
                    ilc.parse(decoded);
                    decoded.clear();
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

    // ilc.checkCommandedEmpty();

    reportTime(beginTs, endTs);
}

void FPGA::mpuCommands(MPU &mpu, const std::chrono::duration<double> &timeout) {
    // construct buffer to send
    std::vector<uint8_t> data;

    data.push_back(mpu.getBus());
    data.push_back(0);

    uint8_t len = 0;

    for (auto cmd : mpu) {
        data.insert(data.end(), cmd.buffer.begin(), cmd.buffer.end());
        len += cmd.buffer.size();
    }

    data[1] = len;

    writeMPUFIFO(data, 0);
    readMPUFIFO(mpu);
}

}  // namespace cRIO
}  // namespace LSST
