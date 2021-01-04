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

#include <cRIO/ILC.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace LSST {
namespace cRIO {

ILCUnknownResponse::ILCUnknownResponse(uint8_t address, uint8_t function)
        : std::runtime_error(fmt::format("Unknown function {1} (0x{1:02x}) in ILC response for address {0}",
                                         address, function)) {}

ILCException::ILCException(uint8_t address, uint8_t function, uint8_t exception)
        : std::runtime_error(
                  fmt::format("ILC Exception {2} (ILC address {0}, ILC response function {1} (0x{1:02x}))",
                              address, function, exception)) {}

void ILC::addResponse(uint8_t function, std::function<void(uint8_t)> action, uint8_t errorResponse,
                      std::function<void(uint8_t, uint8_t)> errorAction) {
    _actions[function] = action;
    _errorActions[errorResponse] =
            std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>(function, errorAction);
}

void ILC::processResponse(uint16_t *response, size_t length) {
    while (true) {
        uint8_t address = read<uint8_t>();
        uint8_t function = read<uint8_t>();

        try {
            _actions.at(function)(address);
        } catch (std::out_of_range &_ex) {
            try {
                auto errorAction = _errorActions.at(function);
                uint8_t exception = read<uint8_t>();
                checkCRC();
                if (errorAction.second) {
                    errorAction.second(address, exception);
                } else {
                    throw ILCException(address, function, exception);
                }
            } catch (std::out_of_range &_ex2) {
                throw ILCUnknownResponse(address, function);
            }
        }
    }
}

}  // namespace cRIO
}  // namespace LSST
