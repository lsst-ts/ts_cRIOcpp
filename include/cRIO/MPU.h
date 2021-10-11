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

#include <list>
#include <map>
#include <vector>
#include <cstdint>

namespace LSST {
namespace cRIO {

/**
 * Modbus Processing Unit (MPU) commands. Please see
 * https://github.com/lsst-ts/Modbus_Processing_Unit for command details.
 */
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
const static uint8_t TELEMETRY_32 = 32;
const static uint8_t TELEMETRY_64 = 33;
const static uint8_t EXIT = 255;
}  // namespace MPUCommands

/**
 * The Modbus Processing Unit class. Prepares buffer with commands to send to
 * FPGA, read responses & telemetry values.
 */
class MPU : public ModbusBuffer {
public:
    /**
     * Contruct MPU class.
     *
     * @param bus MPU bus number (internal FPGA identifier)
     * @param mpu_address MPU ModBus address
     */
    MPU(uint8_t bus, uint8_t mpu_address);

    void clearCommanded() {
        clear();
        _commands.clear();
    }

    /**
     * Returns bus number (internal FPGA identifier).
     *
     * @return MPU bus number
     */
    uint8_t getBus() { return _bus; }

    void setAddress(uint8_t address) { _mpu_address = address; }

    bool containsRead() { return _contains_read; }

    void writeEndOfFrame() override {}
    void writeWaitForRx(uint32_t timeoutMicros) override {}
    void writeRxEndFrame() override {}

    void readEndOfFrame() override {}

    void readInputStatus(uint16_t address, uint16_t count = 1, uint8_t timeout = 100);

    /**
     * Reads holding register(s).
     *
     * @param address register address
     * @param count number of registers to read
     * @param timeout timeout for register readout (in ms)
     */
    void readHoldingRegisters(uint16_t address, uint16_t count, uint8_t timeout = 100);

    /**
     * Write single register.
     *
     * @param address register address
     * @param value register value
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegister(uint16_t address, uint16_t value, uint8_t timeout = 100);

    /**
     * Sets modbus holding registers.
     *
     * @param address register address
     * @param values register values
     * @param count number of registers to write
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegisters(uint16_t address, uint16_t *values, uint8_t count, uint8_t timeout = 100);

    /**
     * Returns commands buffer.
     *
     * @return current command buffer
     */
    uint8_t *getCommands() { return _commands.data(); }

    std::vector<uint8_t> getCommandVector() { return _commands; }

    bool getInputStatus(uint16_t address) { return _inputStatus.at(address); }
    uint16_t getRegister(uint16_t address) { return _registers.at(address); }

private:
    std::vector<uint8_t> _commands;
    uint8_t _bus;
    uint8_t _mpu_address;

    bool _contains_read;

    std::list<std::pair<uint16_t, uint16_t>> _readInputStatus;
    std::list<uint16_t> _readRegisters;
    std::list<std::pair<uint16_t, uint16_t>> _presetRegister;
    std::list<std::pair<uint16_t, uint16_t>> _presetRegisters;

    std::map<uint16_t, bool> _inputStatus;
    std::map<uint16_t, uint16_t> _registers;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_MPU_H_ */
