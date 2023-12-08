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

#include <chrono>
#include <stddef.h>
#include <stdint.h>

#include <cRIO/ILC.h>
#include <cRIO/SimpleFPGA.h>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

class MPU;

/**
 * Interface class for cRIO FPGA. Subclasses can talk either to the real HW, or
 * be a software simulator.
 *
 * The correct class shall be instantiated in main controll loop before
 * starting any threads communicating with FPGA. It shall be then passed to
 * Controller subclass, which uses ILC (and similar) subclasses to form FPGA
 * commands and parse device replies.
 */
class FPGA : public SimpleFPGA {
public:
    /**
     * Construct FPGA. Sets internal variable depending on FPGA type.
     *
     * @param type Either SS for Static Support FPGA or TS for Thermal System.
     */
    FPGA(fpgaType type);

    virtual ~FPGA() {}

    /**
     * Returns command used on CommandFIFO to write data to Modbus bus
     * (transmit FIFO).
     *
     * @param bus bus number (1 based)
     *
     * @return command number
     */
    virtual uint16_t getTxCommand(uint8_t bus) = 0;

    /**
     * Returns command used on RequestFIFO to read response from Modbus FIFO.
     *
     * @param bus bus number (1 based)
     *
     * @return command number
     */
    virtual uint16_t getRxCommand(uint8_t bus) = 0;

    /**
     * Returns IRQ associated with given bus.
     *
     * @param bus bus number (1 based)
     *
     * @return IRQ (bit based)
     */
    virtual uint32_t getIrq(uint8_t bus) = 0;

    /**
     * Send commands from ILC.
     *
     * @param ilc ILC class. That contains ModbusBuffer with commands. Its
     * ILC::getBus() method returns bus used for communication.
     *
     * @see ILC
     * @see ILC::getBus()
     * @see ILC::processResponse()
     */
    void ilcCommands(ILC& ilc);

    /**
     * Sends MPU commands to command FIFO. MPU command buffer must be filled
     * before calling this method. Read outs data if data output was specified in MPU commands. If you would
     * like to split commanding and reading code, please use writeMPUFIFO and readMPUFIFO.
     *
     * @param mpu Modbus Processing Unit containing the commands.
     * @param timeout timeout to sleep before reading. Default to 500ms.
     */
    void mpuCommands(MPU& mpu, const std::chrono::duration<double>& timeout = 500ms);

    /**
     * Commands FPGA to write to MPU commands buffer. Data to write are passed
     * along in mpu parameter - you need to fill the MPU commands prior to
     * calling this method.
     *
     * @param mpu Modbus Processing Unit to write
     */
    virtual void writeMPUFIFO(MPU& mpu) = 0;

    /**
     * Commands FPGA to copy MPU output FIFO to FPGA-C/C++ output FIFO. This
     * method will dump data from MPU to FIFO which C/C++ can read, and reads
     * the data.
     *
     * @param mpu Modbus Processing Unit to read the data
     *
     * @return data read from the port
     */
    virtual std::vector<uint8_t> readMPUFIFO(MPU& mpu) = 0;

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
     * indefinitely for FIFO availability).
     *
     * @throw NiError on NI error
     */
    virtual void writeCommandFIFO(uint16_t* data, size_t length, uint32_t timeout) = 0;

    void writeCommandFIFO(uint16_t data, uint32_t timeout) { writeCommandFIFO(&data, 1, timeout); }

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
    virtual void writeRequestFIFO(uint16_t* data, size_t length, uint32_t timeout) = 0;

    /**
     * Write single command into requestFIFO.
     *
     * @param data data to write
     * @param timeout timeout for data write in milliseconds
     *
     * @throw NiError on NI error
     */
    void writeRequestFIFO(uint16_t data, uint32_t timeout) { writeRequestFIFO(&data, 1, timeout); }

    /**
     *
     * @param data
     * @param length
     * @param timeout
     *
     * @throw NiError on NI error
     */
    virtual void readU16ResponseFIFO(uint16_t* data, size_t length, uint32_t timeout) = 0;

    /**
     * Wait for given IRQs.
     *
     * @param irqs IRQ mask. Each interrupt corresponds to a bit
     * @param timeout timeout in milliseconds
     * @param timedout true if call timedouted, false if finished in time
     * @param triggered
     *
     * @note the method shall allocate context (for IRQ handling) as needed from the current thread id
     */
    virtual void waitOnIrqs(uint32_t irqs, uint32_t timeout, bool& timedout, uint32_t* triggered = NULL) = 0;

    /**
     * Acknowledges IRQs. Clear IRQs on FPGA.
     *
     * @param irqs bitmask of IRQs to clear. FPGA has 32 IRQs channels, each
     * bit represents an interrupt.
     */
    virtual void ackIrqs(uint32_t irqs) = 0;

protected:
    /**
     * Called when full 64 bit timestamp is received.
     *
     * @param timestamp received timestamp
     */
    virtual void reportTime(uint64_t begin, uint64_t end) {}

private:
    uint16_t _modbusSoftwareTrigger;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CRIO_FPGA_H_ */
