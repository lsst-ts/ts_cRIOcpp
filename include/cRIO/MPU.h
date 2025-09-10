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
#include <Modbus/BusList.h>

namespace LSST {
namespace cRIO {

struct CommandedInfo {
    CommandedInfo(uint16_t _addres, uint16_t _length) : address(_addres), length(_length) {}
    uint16_t address;
    uint16_t length;
};

/**
 * The Modbus Processing Unit class. Prepares buffer with commands to send to
 * FPGA, read responses & telemetry values.
 *
 * Only a single command shall be send by the controlling computer. That's due
 * to ModBus protocol, which doesn't include address of register returned - so
 * any attempts to process multiple messages fail on the fact one doesn't know
 * which registers were returned.
 */
class MPU : public Modbus::BusList {
public:
    /**
     * Modbus basic functions.
     */
    enum MODBUS_CMD {
        READ_INPUT_STATUS = 2,
        READ_HOLDING_REGISTERS = 3,
        PRESET_HOLDING_REGISTER = 6,
        PRESET_HOLDING_REGISTERS = 16
    };

    /**
     * Construct MPU class.
     *
     * @param node_address MPU ModBus address
     */
    MPU(uint8_t node_address);

    int responseLength(const std::vector<uint8_t> &response) override;

    void missing_response() override;

    /**
     * Read input registers.
     */
    void readInputStatus(uint16_t start_register_address, uint16_t count = 1, uint32_t timing = 100000);

    /**
     * Reads holding register(s).
     *
     * @param address register(s) staring address
     * @param count number of registers to read
     * @param timing timeout for register readout (in microseconds)
     */
    void readHoldingRegisters(uint16_t start_register_address, uint16_t count, uint32_t timing = 1000000);

    /**
     * Write single register.
     *
     * @param address register starting address
     * @param value register value
     * @param timing timeout (in microseconds)
     */
    void presetHoldingRegister(uint16_t register_address, uint16_t value, uint32_t timing = 100000);

    /**
     * Sets modbus holding registers.
     *
     * @param address register address
     * @param values register values
     * @param timing timeout (in microseconds)
     */
    void presetHoldingRegisters(uint16_t start_register_address, const std::vector<uint16_t> &values,
                                uint32_t timing = 100000);

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
    bool getInputStatus(uint16_t input_address);

    /**
     * Returns register value. This access only cached values - register shall be first read
     * using readHoldingRegisters method.
     *
     * @param address register address
     *
     * @return cached regiter value
     *
     * @throws std::runtime_error when register value isn't cached
     */
    uint16_t getRegister(uint16_t address);

private:
    uint8_t _bus;
    uint8_t _node_address;

    /**
     * Address of register/input which will be read by the command
     */
    std::list<CommandedInfo> _commanded_info;

    std::mutex _registerMutex;

    std::map<uint16_t, bool> _inputStatus;
    std::map<uint16_t, uint16_t> _registers;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_MPU_H_ */
