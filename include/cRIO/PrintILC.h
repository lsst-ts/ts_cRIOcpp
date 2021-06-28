/*
 * Implements ILC printing out response.
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

#ifndef _cRIO_PrintILC_
#define _cRIO_PrintILC_

#include <cRIO/ILC.h>
#include <cRIO/FPGA.h>

namespace LSST {
namespace cRIO {

/**
 * Virtual class printing out ILC responses to basic (implemented in all ILCs)
 * commands. Intended for command line applications. Adds functions to program
 * ILC from provided Intel Hex File.
 */
class PrintILC : public virtual ILC {
public:
    PrintILC(uint8_t bus);

    /**
     * Writes newly programmed application statistics and CRCs.
     *
     * @param address ILC address
     * @param dataCRC
     * @param startAddress
     * @param dataLength
     */
    void writeApplicationStats(uint8_t address, uint16_t dataCRC, uint16_t startAddress, uint16_t dataLength);

    /**
     * Erase ILC application.
     *
     * @param address ILC address
     */
    void eraseILApplication(uint8_t address) { callFunction(address, 101, 500000); }

    void writeApplicationPage(uint8_t address, uint16_t startAddress, uint16_t length, uint8_t *data);

    void writeVerifyApplication(uint8_t address) { callFunction(address, 103, 500000); }

    /**
     * Programs ILC. Executes a sequence of commands as follow:
     *
     * 1. put ILC into standby mode
     * 2. put ILC into firmware update mode
     * 3. clears ILC faults
     * 4. erase ILC application
     * 5. write ILC application
     * 6. write ILC application statistics
     * 7. verify applications
     * 8. put ILC into standby mode
     * 9. put ILC into disabled mode
     *
     * @param fpga FPGA object
     * @param address ILC address
     * @param hex Intel Hex file to load into ILC
     */
    void programILC(FPGA *fpga, uint8_t address, IntelHex &hex);

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override;

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override;

    void processChangeILCMode(uint8_t address, uint16_t mode) override;

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override;

    void processResetServer(uint8_t address) override;

    virtual void processEraseILCApplication(uint8_t address);

    virtual void processWriteApplicationStats(uint8_t address);

    virtual void processWriteApplicationPage(uint8_t address);

    virtual void processVerifyUserApplication(uint8_t address, uint16_t status);

    virtual void printBusAddress(uint8_t address);

    virtual void printSepline();

private:
    int _printout;

    void _writeHex(FPGA *fpga, uint8_t address, IntelHex &hex, uint16_t &dataCRC, uint16_t &startAddress,
                   uint16_t &dataLength);
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !_cRIO_ILC_
