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

#include <cRIO/FPGA.h>
#include <cRIO/IntelHex.h>
#include <ILC/ILCBusList.h>

namespace LSST {
namespace cRIO {

/**
 * Virtual class printing out @glos{ILC} responses to basic (implemented in all ILCs)
 * commands. Intended for command line applications. Adds functions to program
 * @glos{ILC} from provided Intel Hex File.
 */
class PrintILC : public virtual ILC::ILCBusList {
public:
    PrintILC(uint8_t bus);

    /**
     * Writes newly programmed application statistics and CRCs. Calculates
     * fourth @glos{ILC} function 100 argument - ModBus 16bit CRC from input
     * arguments.
     *
     * @param address @glos{ILC} address
     * @param dataCRC data ModBus 16bit CRC. This is calculated fom
     * @param startAddress start memory address. Lowest start address from all function 103 calls.
     * @param dataLength data length (unshrunk). Highest start address from all function 103 + 256.
     *
     * @see programILC
     */
    void writeApplicationStats(uint8_t address, uint16_t dataCRC, uint16_t startAddress, uint16_t dataLength);

    /**
     * Erase @glos{ILC} application.
     *
     * @param address @glos{ILC} address
     */
    void eraseILCApplication(uint8_t address) {
        callFunction(address, ILC_CLI_CMD::ERASE_APPLICATION, 500000);
    }

    /**
     * Writes @glos{ILC} firmware page. Calls functions 103. Please note that every
     * fourth byte from hexfile is skipped.
     *
     * @param address @glos{ILC} address
     * @param startAddress firmware start address
     * @param length data length. Shall be 192
     * @param data firmware data
     *
     * @see programILC
     */
    void writeApplicationPage(uint8_t address, uint16_t startAddress, uint16_t length,
                              std::vector<uint8_t> data) {
        callFunction(address, ILC_CLI_CMD::WRITE_APPLICATION_PAGE, 500000, startAddress, length, data);
    }

    /**
     * Verifies firmware upload.
     *
     * @param address @glos{ILC} address
     */
    void writeVerifyApplication(uint8_t address) {
        callFunction(address, ILC_CLI_CMD::WRITE_VERIFY_APPLICATION, 500000);
    }

    /**
     * Programs ILC. Executes a sequence of commands as follow:
     *
     * 1. put @glos{ILC} into standby mode
     * 2. put @glos{ILC} into firmware update mode
     * 3. clears @glos{ILC} faults
     * 4. erase @glos{ILC} application
     * 5. write @glos{ILC} application
     * 6. write @glos{ILC} application statistics
     * 7. verify applications
     * 8. put @glos{ILC} into standby mode
     * 9. put @glos{ILC} into disabled mode
     *
     * @note Every fourth byte in input hex file is unused. @glos{ILC} bootloader
     * expand input pages, adds 0x00 after every third byte. Control checksum
     * passed in function call 100 (Write Application Stats), addresses passed
     * in functions 100 and 102 (Write Application Page) are calculated and
     * taken from the full, unshrunk (expanded) firmware (e.g. from what is
     * written in hex file).
     *
     * @param fpga FPGA object
     * @param address @glos{ILC} address
     * @param hex Intel Hex file to load into ILC
     */
    void programILC(FPGA *fpga, uint8_t address, IntelHex &hex);

    /**
     * Please consult LTS-646 for details about the ILC commands.
     */
    enum ILC_CLI_CMD {
        WRITE_APPLICATION_STATS = 100,
        ERASE_APPLICATION = 101,
        WRITE_APPLICATION_PAGE = 102,
        WRITE_VERIFY_APPLICATION = 103
    };

protected:
    void processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType, uint8_t networkNodeType,
                         uint8_t ilcSelectedOptions, uint8_t networkNodeOptions, uint8_t majorRev,
                         uint8_t minorRev, std::string firmwareName) override;

    void processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) override;

    void processChangeILCMode(uint8_t address, uint16_t mode) override;

    void processSetTempILCAddress(uint8_t address, uint8_t newAddress) override;

    void processResetServer(uint8_t address) override;

    /**
     * Called when Erase @glos{ILC} Application (@glos{ILC} function 101) is acknowledged.
     *
     * @param address @glos{ILC} address
     */
    virtual void processEraseILCApplication(uint8_t address);

    /**
     * Called when Write Application Stats (ILC function 100) is acknowledged.
     *
     * @param address @glos{ILC} address
     */
    virtual void processWriteApplicationStats(uint8_t address);

    /**
     * Called when Write Application Page (ILC function 102) is acknowledged.
     *
     * @param address @glos{ILC} address
     */
    virtual void processWriteApplicationPage(uint8_t address);

    /**
     * Called when Verify User Application (ILC function 103) is acknowledged.
     *
     * Status:
     * <ol>
     *   <li><b>0x0000</b> all OK, success</li>
     *   <li><b>0x00FF</b> application stats error</li>
     *   <li><b>0xFF00</b> application error</li>
     *   <li><b>0xFFFF</b> application stats and application error</li>
     * </ol>
     *
     * @param address @glos{ILC} address
     * @param status program @glos{ILC} status
     */
    virtual void processVerifyUserApplication(uint8_t address, uint16_t status);

    /**
     * Prints bus and @glos{ILC} address. Used inside various responses to
     * print out which @glos{ILC} replied.
     *
     * @param address @glos{ILC} address
     */
    virtual void printBusAddress(uint8_t address);

    virtual void printSepline();

private:
    int _printout;
    uint8_t _lastAddress;
    ModbusBuffer::CRC _crc;
    uint16_t _startAddress;
    uint16_t _dataLength;

    void _writeHex(FPGA *fpga, uint8_t address, IntelHex &hex);
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !_cRIO_PrintILC_
