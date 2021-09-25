/*
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

#ifndef CRIO_SIMULATEDILC_H_
#define CRIO_SIMULATEDILC_H_

#include <cRIO/ModbusBuffer.h>

namespace LSST {
namespace cRIO {

class SimulatedILC : public ModbusBuffer {
public:
    SimulatedILC() : ModbusBuffer() {}
    SimulatedILC(uint16_t* buffer, size_t length) : ModbusBuffer(buffer, length) {}

    void readEndOfFrame() override {}
    void writeEndOfFrame() override { pushBuffer(FIFO::RX_ENDFRAME); }
    void writeWaitForRx(uint32_t timeoutMicros) override {}
    void writeRxEndFrame() override {}

    uint16_t getByteInstruction(uint8_t data) override {
        processDataCRC(data);
        return FIFO::RX_MASK | ((static_cast<uint16_t>(data)) << 1);
    }

    void writeFPGATimestamp(uint64_t timestamp) {
        for (int i = 0; i < 4; i++) {
            pushBuffer(timestamp & 0xFFFF);
            timestamp >>= 16;
        }
    }

    void writeRxTimestamp(uint64_t timestamp) {
        for (int i = 0; i < 8; i++) {
            pushBuffer(FIFO::RX_TIMESTAMP | (timestamp & 0xFF));
            timestamp >>= 8;
        }
    }

protected:
    uint8_t readInstructionByte() override {
        if (endOfBuffer()) {
            throw EndOfBuffer();
        }
        return (uint8_t)((getCurrentBufferAndInc() >> 1) & 0xFF);
    }
};

}  // namespace cRIO
}  // namespace LSST

#endif  // CRIO_SIMULATIONBUFFER_H_

