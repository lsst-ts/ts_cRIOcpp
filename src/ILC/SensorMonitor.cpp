/*
 * Sensor Monitor ILC handling.
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

#include <vector>

#include <spdlog/spdlog.h>

#include <ILC/SensorMonitor.h>

using namespace ILC;

SensorMonitor::SensorMonitor(uint8_t bus) : ILCBusList(bus) {
    addResponse(
            SENSOR_VALUES,
            [this](Modbus::Parser parser) {
                std::vector<float> values;
                // there should be 2 bytes (address, function), 4 bytes floats and 2 bytes CRC - so the size()
                // shall be multiple of 4
                if (parser.size() % 4 != 0) {
                    throw std::runtime_error(
                            fmt::format("Invalid reponse length - expected 4*x, received {}", parser.size()));
                }

                for (int i = 1; i < static_cast<int>(parser.size()) / 4; i++) {
                    values.push_back(parser.read<float>());
                }

                processSensorValues(parser.address(), values);
            },
            SENSOR_VALUES | 0x80);
}
