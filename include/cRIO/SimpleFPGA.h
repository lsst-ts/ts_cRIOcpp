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

#ifndef CRIO_SimpleFPGA_H_
#define CRIO_SimpleFPGA_H_

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stddef.h>
#include <stdint.h>

#include <spdlog/spdlog.h>

#include <cRIO/ILC.h>
#include <cRIO/ModbusBuffer.h>

namespace LSST {
namespace cRIO {

/**
 * FPGA Type. SS (M1M3 Static Support), TS (M1M3 Thermal System), M2 (M2
 * Control System) or VMS (Vibration Monitoring System) are known and
 * supported.
 */
typedef enum { SS, TS, M2, VMS } fpgaType;

/**
 * Interface class for cRIO FPGA. Subclasses can talk either to the real HW, or
 * be a software simulator.
 *
 * The correct class shall be instantiated in main controll loop before
 * starting any threads communicating with FPGA. It shall be then passed to
 * Controller subclass. This is siplified FPGA version, intended for ILC-less
 * operatrions. Please see cRIO::FPGA for a class supporting ILCs and MPUs.
 */
class SimpleFPGA {
public:
    /**
     * Construct FPGA. Sets internal variable depending on FPGA type.
     *
     * @param type Either SS for Static Support FPGA or TS for Thermal System.
     */
    SimpleFPGA(fpgaType type);
    virtual ~SimpleFPGA();

    /**
     * Initialize FPGA.
     *
     * @throw NiError on NI error
     */
    virtual void initialize() = 0;

    /**
     * Load & run FPGA code, setup interrupts.
     *
     * @throw NiError on NI error
     */
    virtual void open() = 0;

    /**
     * Close FPGA, stop FPGA code.
     *
     * @throw NiError on NI error
     */
    virtual void close() = 0;

    /**
     * Should be called after closing FPGA.
     *
     * @throw NiError on NI error
     */
    virtual void finalize() = 0;

    /**
     * Open debug file.
     *
     * @param path debug file path
     */
    void openDebugFile(const char* path);

    void writeDebugFile(const char* message);

    template <typename dt>
    const void writeDebugFile(const char* message, dt* buf, size_t length) {
        if (_debug_stream.is_open()) {
            try {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                _debug_stream << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dZ%T:") << message << " "
                              << ModbusBuffer::hexDump<dt>(buf, length) << std::endl;
            } catch (const std::ios_base::failure& e) {
                SPDLOG_WARN("Cannot write to debug file: {}", e.what());
                closeDebugFile();
            }
        }
    }

    template <typename dt>
    const void writeDebugFile(const char* message, ModbusBuffer& mb) {
        if (_debug_stream.is_open()) {
            try {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                _debug_stream << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dZ%T:") << message << " "
                              << mb.hexDump<dt>(mb.getBuffer(), mb.getLength()) << std::endl;
            } catch (const std::ios_base::failure& e) {
                SPDLOG_WARN("Cannot write to debug file: {}", e.what());
                closeDebugFile();
            }
        }
    }

    void writeDebugFile(const char* message, ModbusBuffer& mb);

    void closeDebugFile();

private:
    // File to write FPGA communication
    std::ofstream _debug_stream;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_SimpleFPGA_H_ */
