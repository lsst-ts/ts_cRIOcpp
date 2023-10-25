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

#include <stddef.h>
#include <stdint.h>

#include <cRIO/ILC.h>

namespace LSST {
namespace cRIO {

/**
 * FPGA Type. SS (Static Support), TS (Thermal System) or VMS (Vibration
 * Monitoring System) are known and supported.
 */
typedef enum { SS, TS, VMS } fpgaType;

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
    SimpleFPGA(fpgaType type) {}
    virtual ~SimpleFPGA() {}

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
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_SimpleFPGA_H_ */
