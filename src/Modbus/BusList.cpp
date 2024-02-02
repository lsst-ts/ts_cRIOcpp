/*
 * Bus List handling communication (receiving and
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

#include <Modbus/BusList.h>

using namespace Modbus;

BusList::BusList() {}

void BusList::parse(uint8_t *data, size_t len) {
    Parser parser(std::vector<uint8_t>(data, data + len));
    for (auto action : _actions) {
        if (action.first == parser.func()) {
            action.second(parser);
        }
        return;
    }
}

void BusList::addResponse(uint8_t func, std::function<void(Parser)> action, uint8_t errorResponse,
                          std::function<void(uint8_t, uint8_t)> errorAction) {
    _actions[func] = action;
    _errorActions[errorResponse] =
            std::pair<uint8_t, std::function<void(uint8_t, uint8_t)>>(func, errorAction);
}
