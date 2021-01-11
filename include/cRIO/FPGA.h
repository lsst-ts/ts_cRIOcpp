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

#ifndef CRIO_FPGA_H_
#define CRIO_FPGA_H_

namespace LSST {
namespace cRIO {

/**
 * Interface class for cRIO FPGA. Subclasses can talk either to the real HW, or
 * be a software simulator.
 *
 * The correct class shall be instantiated in main controll loop before
 * starting any threads communicating with FPGA. It shall be then passed to
 * Controller subclass, which uses ILC (and similar) subclasses to form FPGA
 * commands and parse device replies.
 */
class FPGA {
public:
    /**
     * Finalize FPGA. This shall prevent issues when re-opening the FPGA.
     */
    virtual ~FPGA() { finalize(); }

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
     * Closes connection to FPGA.
     */
    virtual void close() = 0;

    /**
     * Writes buffer to command FIFO. Command FIFO is processed in
     * CommandMultiplexer Vi.
     *
     * @param data buffer to write to FIFO. First should be command
     * address/number, followed by command data.
     *
     * @param length lenght of the data buffer
     *
     * @param timeoutInMs timeout for write operation. 0 for no timeout
     * (throw exception when data cannot be written), -1 for no timeout (waits
     * indefintely for FIFO availability).
     *
     * @throw NiError on NI error
     */
    virtual void writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) = 0;

    void writeCommandFIFO(uint16_t data, int32_t timeout) { writeCommandFIFO(&data, 1, timeout); }

    /**
     * Performs request for various data stored inside FPGA. This allows access
     * for modbus responses etc.
     *
     * @param data request command followed by request parameters
     * @param length length of data buffer
     *
     * @param timeout timeout for write operation. 0 for no timeout
     * (throw exception when data cannot be written), -1 for no timeout (waits
     * indefinitely for FIFO availability).
     *
     * @throw NiError on NI error
     *
     * @see readU8ResponseFIFO
     * @see readU16ResponseFIFO
     */
    virtual void writeRequestFIFO(uint16_t* data, int32_t length, int32_t timeout) = 0;

    /**
     * Write single command into requestFIFO.
     *
     * @param data data to write
     * @param timeout timeout for data write in milliseconds
     *
     * @throw NiError on NI error
     */
    void writeRequestFIFO(uint16_t data, int32_t timeout) { writeRequestFIFO(&data, 1, timeout); }

    /**
     *
     * @throw NiError on NI error
     */
    virtual void readU16ResponseFIFO(uint16_t* data, int32_t length, int32_t timeout) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_FPGA_H_ */
