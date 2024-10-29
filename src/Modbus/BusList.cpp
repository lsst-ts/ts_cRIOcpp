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

ErrorRecord::ErrorRecord() {
    last_error_function = 0;
    last_error_code = 0;
    error_count = 0;
}

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

BusList::BusList() {}

int BusList::responseLength(const std::vector<uint8_t> &response) { return -1; }

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

    for (auto func_record : _functions) {
        if (func_record.func == called) {
            func_record.action(parser);
            _parsed_index++;
            return;
        }

        if (func_record.error_reply == called) {
            uint8_t error = parser.read<uint8_t>();
            parser.checkCRC();
            bool new_error = _errors[address].record(called, error);

            if (func_record.error_action != nullptr) {
                func_record.error_action(address, error);
            } else {
                if (new_error) {
                    SPDLOG_WARN("Error reply from - function {} ({}), address {}", called, func_record.func,
                                address);
                }
            }
            _parsed_index++;
            return;
        }
    }
    throw UnexpectedResponse(address, called);
}

void BusList::addResponse(uint8_t func, std::function<void(Parser)> action, uint8_t error_reply,
                          std::function<void(uint8_t, uint8_t)> error_action) {
    _functions.emplace(_functions.end(), ResponseRecord(func, action, error_reply, error_action));
}
