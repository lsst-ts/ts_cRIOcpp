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
 * Modbus Processing Unit (MPU) commands.
 */
typedef enum {
    MPU_WRITE = 1,
    MPU_READ_US = 2,
    MPU_READ_MS = 3,
    MPU_WAIT_US = 100,
    MPU_WAIT_MS = 101,
    MPU_IRQ = 240,
    MPU_TELEMETRY = 254,
    MPU_RESET = 255
} MPUCommands;

typedef enum { WAIT_WRITE, WAIT_READ, WAIT_IDLE } loop_state_t;

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
class MPU : public Modbus::BusList {
public:
    /**
     * Contruct MPU class.
     *
     * @param bus MPU bus number (internal FPGA identifier)
     */
    MPU(uint8_t bus);

    void reset() override;

    /**
     * Wait for given number of microseonds.
     *
     * @param us number of microseconds to wait
     */
    void waitUs(uint16_t us);

    /**
     * Wait for given number of milliseconds.
     *
     * @param ms number of milliseconds to wait
     */
    void waitMs(uint16_t ms);

    virtual uint32_t getIrq() { return 1 << (10 + getBus()); }

    void readInputStatus(uint8_t address, uint16_t register_address, uint16_t count = 1,
                         uint16_t timeout = 100) {
        callFunction(address, 2, timeout, register_address, count);
    }

    /**
     * Reads holding register(s).
     *
     * @param address modbus unit address
     * @param register_address register address
     * @param count number of registers to read
     * @param timeout timeout for register readout (in ms)
     */
    void readHoldingRegisters(uint8_t address, uint16_t register_address, uint16_t count,
                              uint16_t timeout = 100) {
        callFunction(address, 3, timeout, register_address, count);
    }

    /**
     * Write single register.
     *
     * @param address modbus unit address
     * @param register_address register address
     * @param value register value
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegister(uint8_t address, uint16_t register_address, uint16_t value,
                               uint16_t timeout = 100) {
        callFunction(address, 6, timeout, register_address, value);
    }

    /**
     * Sets modbus holding registers.
     *
     * @param address modbus unit address
     * @param register_address register address
     * @param values register values
     * @param count number of registers to write
     * @param timeout timeout (in ms)
     */
    void presetHoldingRegisters(uint8_t address, uint16_t register_address, uint16_t *values, uint8_t count,
                                uint16_t timeout = 100);

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

    /**
     * Exception raised when read timeouts.
     */
    class IRQTimeout : public std::runtime_error {
    public:
        IRQTimeout(std::vector<uint8_t> _data);
        std::vector<uint8_t> data;
    };

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
    bool runLoop(FPGA &fpga);

    loop_state_t getLoopState() { return _loop_state; }

    /**
     * Returns cached input state. Input state shall be cached - queried with
     * readInputStatus method.
     *
     * @param address input state address
     *
     * @return cached input state
     *
     * @throws std::out_of_range if the address isn't cached
     */

    bool getInputStatus(uint8_t address, uint16_t input_address) {
        return _inputStatus.at(std::pair(address, input_address));
    }

    /**
     * Returns register value. This access only cached values - register shall be first read
     * using readHoldingRegisters method.
     *
     * @param address device modbus address
     * @param register register address
     *
     * @return cached regiter value
     *
     * @throws std::runtime_error when register value isn't cached
     */
    uint16_t getRegister(uint8_t address, uint16_t register_address) {
        std::lock_guard<std::mutex> lg(_registerMutex);
        try {
            return _registers.at(std::pair(address, register_address));
        } catch (std::out_of_range &e) {
            throw std::runtime_error(fmt::format("Cannot retrieve holding register {}", address));
        }
    }

private:
    std::mutex _registerMutex;

    std::map<std::pair<uint8_t, uint16_t>, bool> _inputStatus;
    std::map<std::pair<uint8_t, uint16_t>, uint16_t> _registers;

    std::chrono::microseconds _loop_timeout;
    std::chrono::time_point<std::chrono::steady_clock> _loop_next_read;
    loop_state_t _loop_state;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_MPU_H_ */
