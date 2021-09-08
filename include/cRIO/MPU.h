/*
 * Modbus Processing Unit class.
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

#ifndef CRIO_MPU_H_
#define CRIO_MPU_H_

#include <cRIO/ModbusBuffer.h>

#include <vector>
#include <cstdint>

namespace LSST {
namespace cRIO {

namespace MPUCommands {
const static uint8_t STOP = 0;
const static uint8_t WRITE_BYTE = 1;
const static uint8_t WAIT_MS = 2;
const static uint8_t READ = 3;
const static uint8_t LOOP = 4;
const static uint8_t CHECK_CRC = 5;
const static uint8_t OUTPUT = 6;
const static uint8_t WRITE = 20;
const static uint8_t TELEMETRY_BYTE = 30;
const static uint8_t TELEMETRY_16 = 31;
const static uint8_t TELEMETRY_32 = 30;
const static uint8_t TELEMETRY_64 = 30;
const static uint8_t EXIT = 255;
}  // namespace MPUCommands

class MPUBuffer : public ModbusBuffer {
public:
    void writeEndOfFrame() override {}
    void writeWaitForRx(uint32_t timeoutMicros) override {}
    void writeRxEndFrame() override {}

    void readEndOfFrame() override {}

    void readHoldingRegisters(uint8_t mpu_address, uint16_t address, uint16_t count = 1);
};

class MPU {
public:
    MPU(uint8_t mpu_address);
    void readHoldingRegisters(uint16_t address, uint16_t count = 1, uint8_t timeout = 100);

    uint16_t* getCommands() { return _commands.data(); }

private:
    std::vector<uint16_t> _commands;
    uint8_t _mpu_address;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_MPU_H_ */
