/*
 * Interface for FPGA communication.
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

#include <chrono>
#include <iomanip>

#include <spdlog/spdlog.h>

#include <cRIO/SimpleFPGA.h>

using namespace LSST::cRIO;

SimpleFPGA::SimpleFPGA(fpgaType type) {
    _debug_stream.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);
}

SimpleFPGA::~SimpleFPGA() { closeDebugFile(); }

void SimpleFPGA::openDebugFile(const char* path) {
    try {
        closeDebugFile();
        _debug_stream.open(path, std::ios_base::trunc);
        SPDLOG_INFO("Opened FPGA debug file {}", path);
    } catch (const std::ios_base::failure& e) {
        if (_debug_stream.good()) {
            SPDLOG_WARN("Cannot open debug file {}: {}", path, e.what());
        }
    }
}

void SimpleFPGA::writeDebugFile(const char* message) {
    if (_debug_stream.is_open()) {
        try {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            _debug_stream << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dZ%T:") << message << std::endl;
        } catch (const std::ios_base::failure& e) {
            SPDLOG_WARN("Cannot write to debug file: {}", e.what());
            closeDebugFile();
        }
    }
}

void SimpleFPGA::closeDebugFile() {
    if (_debug_stream.is_open()) {
        _debug_stream.close();
    }
}
