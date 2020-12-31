/*
 * Implements generic ILC functions.
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

#include <cRIO/ModbusBuffer.h>

namespace LSST {
namespace cRIO {

/**
 * Class filling ModbusBuffer with commands. Should serve single subnet, so
 * allows sending messages with different node addresses.
 */
class ILC : public ModbusBuffer {
public:
    void reportServerID(uint8_t address) { callFunction(address, 17); }
    void reportServerStatus(uint8_t address) { callFunction(address, 18); }
    void changeILCMode(uint8_t address, uint16_t mode);
    void setTempILCAddress(uint8_t temporatyAddress);
    void reset(uint8_t address) { callFunction(address, 107); }

protected:
    /**
     * Add to buffer Modbus function. Assumes subnet, data lengths and triggers are
     * send by FPGA class.
     *
     * @param address ILC address on subnet
     * @param function ILC function to call
     */
    void callFunction(uint8_t address, uint8_t function);

    void callFunction(uint8_t address, uint8_t function, uint16_t p1);
};

}  // namespace cRIO
}  // namespace LSST
