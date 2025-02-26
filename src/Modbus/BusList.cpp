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

#include <spdlog/spdlog.h>

#include <Modbus/BusList.h>

using namespace Modbus;

ErrorRecord::ErrorRecord() : last_error_function(0), last_error_code(0), error_count(0), last_occurence() {}

bool ErrorRecord::record(uint8_t func, uint8_t error) {
    last_occurence = std::chrono::steady_clock::now();

    if (last_error_function == func && error == last_error_code && error_count > 0) {
        error_count++;
        return false;
    }
    last_error_function = func;
    last_error_code = error;
    error_count++;
    return true;
}

void ErrorRecord::reset() {
    last_error_function = 0;
    last_error_code = 0;
    error_count = 0;
}

ErrorResponse::ErrorResponse(uint8_t address, uint8_t func)
        : std::runtime_error(
                  fmt::format("Error response - address {0}. response {1} ({1:02x}), function {2} ({2:02x})",
                              address, func, func & ~BusList::MODBUS_ERROR_MASK)) {}

BusList::BusList() : _functions(), _errors() {}

int BusList::responseLength(const std::vector<uint8_t> &) { return -1; }

void BusList::parse(Parser parser) {
    auto exp_address = at(_parsed_index).buffer.address();
    auto exp_func = at(_parsed_index).buffer.func();

    uint8_t address = parser.address();
    uint8_t called = parser.func();

    if (address != exp_address || (called & ~MODBUS_ERROR_MASK) != exp_func) {
        _parsed_index++;
        bool new_error = _errors[exp_address].record(called, 0xff);

        auto wr = WrongResponse(address, exp_address, called, exp_func);
        if (new_error) {
            SPDLOG_WARN(wr.what());
        }
        throw wr;
    }

    if (called & MODBUS_ERROR_MASK) {
        auto func = _functions.at(called & ~MODBUS_ERROR_MASK);
        if (func.error_action != nullptr) {
            func.error_action(address, called);
            _parsed_index++;
        } else {
            _parsed_index++;
            throw ErrorResponse(address, called);
        }
    } else {
        _functions.at(called).action(parser);
        _parsed_index++;
    }

    // throw UnexpectedResponse(address, called);
}

void BusList::add_response(uint8_t func, std::function<void(Parser)> action,
                           std::function<void(uint8_t, uint8_t)> error_action) {
    _functions.emplace(func, ResponseRecord(action, error_action));
}

void BusList::set_error_response(uint8_t func, std::function<void(uint8_t, uint8_t)> error_action) {
    _functions.at(func).error_action = error_action;
}
