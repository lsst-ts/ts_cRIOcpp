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
    void changeILCMode(uint8_t address, uint16_t mode) { callFunction(address, 65, mode); }
    void setTempILCAddress(uint8_t temporaryAddress) { callFunction(255, 72, temporaryAddress); }
    void resetServer(uint8_t address) { callFunction(address, 107); }
};

}  // namespace cRIO
}  // namespace LSST
