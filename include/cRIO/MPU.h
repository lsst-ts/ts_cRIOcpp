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

#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <cstdint>

#include <spdlog/fmt/fmt.h>

#include <cRIO/FPGA.h>
#include <cRIO/ModbusBuffer.h>

namespace LSST {
namespace cRIO {

/**
 * Modbus Processing Unit (MPU) commands. Please see
 * https://github.com/lsst-ts/ts_M1M3Thermal/blob/develop/doc/version-history.rst
 */
namespace MPUCommands {
const static uint8_t WRITE = 1;
const static uint8_t READ_US = 2;
const static uint8_t READ_MS = 3;
const static uint8_t IRQ = 240;
const static uint8_t TELEMETRY = 254;
const static uint8_t CLEAR = 255;
}  // namespace MPUCommands

typedef enum { WRITE, READ, IDLE } loop_state_t;

/**
 * The Modbus Processing Unit class. Prepares buffer with commands to send to
 * FPGA, read responses & telemetry values.
 *
 * Values can be queried in a loop. Code then calls registered callback
 * function everytime new data become available. For code to work that way,
 * interrupt (
 *
 * If FPGA is instructed to raise an interrupt during (most likely at the end
 * of) buffer execution with the 240 opcode,
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

    void clearCommanded();

    /**
     * Returns bus number (internal FPGA identifier).
     *
     * @return MPU bus number
     */
    uint8_t getBus() { return _bus; }

    virtual uint32_t getIrq() { return 1 << (10 + getBus()); }

    void setAddress(uint8_t address) { _mpu_address = address; }

    bool containsRead() { return _contains_read; }

    void writeEndOfFrame() override {}
    void writeWaitForRx(uint32_t timeoutMicros) override {}
    void writeRxEndFrame() override {}

    void readEndOfFrame() override {}

    void readInputStatus(uint16_t address, uint16_t count = 1, uint16_t timeout = 100);

    /**
     * Reads holding register(s).
     *
     * @param address register address
     * @param count number of registers to read
     * @param timeout timeout for register readout (in ms)
     */
    void readHoldingRegisters(uint16_t address, uint16_t count, uint16_t timeout = 100);

    /**
     * Write single register.
     *
     * @param address register address
     * @param value register value
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegister(uint16_t address, uint16_t value, uint16_t timeout = 100);

    /**
     * Sets modbus holding registers.
     *
     * @param address register address
     * @param values register values
     * @param count number of registers to write
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegisters(uint16_t address, uint16_t *values, uint8_t count, uint16_t timeout = 100);

    /**
     * Returns commands buffer.
     *
     * @return current command buffer
     */
    std::vector<uint8_t> getCommandVector();

    /***
     * Called to set loop read timeout.
     *
     * @param callback function to call when new data are available
     * @param timeout timeout. If data cannot be retrieved within timeout,
     * callback function is called with timedout parameter set to true.
     */
    void setLoopTimeOut(std::chrono::microseconds timeout) { _loop_timeout = timeout; }

    /***
     * Called to write commands to retrieve values needed in loopRead.
     */
    virtual void loopWrite() = 0;

    /***
     * Called to process data read in the loop.
     *
     * @param timeout
     */
    virtual void loopRead(bool timedout) = 0;

    /**
     * Runs command loop. This is a state machine, with state stored in
     * _loopStatus variable.
     *
     * In WRITE state, it calls loopWrite to prepare commands for FPGA.
     *
     * In READ state, it waits for bus IRQ. If wait succeed (IRQ was triggered),
     * it reads the output data into a buffer, process the acquired buffer, and
     * calls callback (if set).
     *
     * @param FPGA fpga used to process MPU commands.
     */
    void runLoop(FPGA &fpga);

    loop_state_t getLoopState() { return _loop_state; }

    /***
     * Returns cached input state. Input state shall be cached - queried with
     * readInputStatus method.
     *
     * @param address input state address
     *
     * @return cached input state
     *
     * @throws std::out_of_range if the address isn't cached
     */

    bool getInputStatus(uint16_t address) { return _inputStatus.at(address); }

    /***
     * Returns register value. This access only cached values - register shall be first read
     * using readHoldingRegisters method.
     *
     * @param address register address
     *
     * @return cached regiter value
     *
     * @throws std::runtime_error when register value isn't cached
     */
    uint16_t getRegister(uint16_t address) {
        std::lock_guard<std::mutex> lg(_registerMutex);
        try {
            return _registers.at(address);
        } catch (std::out_of_range &e) {
            throw std::runtime_error(fmt::format("Cannot retrive holding register {}", address));
        }
    }

private:
    void _pushTimeout(uint16_t timeout);

    std::vector<uint8_t> _commands;
    uint8_t _bus;
    uint8_t _mpu_address;

    std::mutex _registerMutex;

    bool _contains_read;

    std::list<std::pair<uint16_t, uint16_t>> _readInputStatus;
    std::list<uint16_t> _readRegisters;
    std::list<std::pair<uint16_t, uint16_t>> _presetRegister;
    std::list<std::pair<uint16_t, uint16_t>> _presetRegisters;

    std::map<uint16_t, bool> _inputStatus;
    std::map<uint16_t, uint16_t> _registers;

    std::chrono::microseconds _loop_timeout;
    std::chrono::time_point<std::chrono::steady_clock> _loop_next_read;
    loop_state_t _loop_state;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_MPU_H_ */
