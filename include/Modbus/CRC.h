/*
 * Implements Modbus CRC calculations.
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

#ifndef __Modbus_CRC__
#define __Modbus_CRC__

#include <cstdint>
#include <vector>

namespace Modbus {

/**
 * Class to calculate CRC. Example usage:
 *
 * @code{.cpp}
 * Modbus::CRC crc{};
 *
 * for (uint8_t d = 0; d < 0xFF; d++) {
 *     crc.add(d);
 * }
 * std::cout << "Modbus CRC is " << crc.get() << "(0x" << << std::setw(4) << std::setfill('0')
 *     << std::hex << crc.get() << ")" << std::endl;
 * @endcode
 */
class CRC {
public:
    /**
     * Construct CRC class. The class is ready to accept add calls (reset
     * don't need to be called).
     */
    CRC() { reset(); }

    CRC(const std::vector<uint8_t>& vec) {
        reset();
        for (auto data : vec) {
            add(data);
        }
    }

    /**
     * Construct CRC class and fill its buffer.
     *
     * @param buf data buffer
     * @param len data length
     */
    CRC(uint8_t* data, std::size_t len) {
        reset();
        for (std::size_t i = 0; i < len; i++) {
            add(data[i]);
        }
    }

    /**
     * Reset internal CRC counter.
     */
    void reset() { _crcCounter = 0xFFFF; }

    /**
     * Adds data to CRC buffer. Updates CRC value to match previous data.
     *
     * @param data data to be added
     */
    void add(uint8_t data);

    /**
     * Returns CRC value.
     *
     * @return buffer Modbus CRC
     */
    uint16_t get() { return _crcCounter; }

private:
    uint16_t _crcCounter;
};

}  // namespace Modbus

#endif /* !__Modbus_CRC__ */
