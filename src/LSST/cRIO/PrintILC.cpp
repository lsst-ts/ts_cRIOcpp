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

#include <cRIO/PrintILC.h>
#include <iomanip>
#include <iostream>

using namespace LSST::cRIO;

PrintILC::PrintILC(uint8_t bus) : ILC(bus), _printout(0) {
    setAlwaysTrigger(true);
    addResponse(
            101,
            [this](uint8_t address) {
                checkCRC();
                processEraseILCApplication(address);
            },
            229);
}

void PrintILC::programILC(uint8_t address, IntelHex &hex) {
    changeILCMode(address, ILCMode::Standby);
    changeILCMode(address, ILCMode::FirmwareUpdate);
    changeILCMode(address, ILCMode::ClearFaults);

    changeILCMode(address, ILCMode::Standby);
    changeILCMode(address, ILCMode::Disabled);
}

void PrintILC::processServerID(uint8_t address, uint64_t uniqueID, uint8_t ilcAppType,
                               uint8_t networkNodeType, uint8_t ilcSelectedOptions,
                               uint8_t networkNodeOptions, uint8_t majorRev, uint8_t minorRev,
                               std::string firmwareName) {
    printBusAddress(address);
    std::cout << "UniqueID: " << std::hex << std::setw(8) << std::setfill('0') << (uniqueID) << std::endl
              << "ILC application type: " << std::to_string(ilcAppType) << std::endl
              << "Network node type: " << std::to_string(networkNodeType) << std::endl
              << "ILC selected options: " << std::to_string(ilcSelectedOptions) << std::endl
              << "Network node options: " << std::to_string(networkNodeOptions) << std::endl
              << "Firmware revision: " << std::to_string(majorRev) << "." << std::to_string(minorRev)
              << std::endl
              << "Firmware name: " << firmwareName << std::endl;
}

void PrintILC::processServerStatus(uint8_t address, uint8_t mode, uint16_t status, uint16_t faults) {
    printBusAddress(address);
    std::cout << "Mode: " << std::to_string(mode) << std::endl
              << "Status: " << std::hex << std::setw(4) << std::setfill('0') << (status) << std::endl
              << "Faults: " << std::hex << std::setw(4) << std::setfill('0') << (status) << std::endl;
}

void PrintILC::processChangeILCMode(uint8_t address, uint16_t mode) {
    printBusAddress(address);
    std::cout << "New mode: " << std::to_string(mode) << std::endl;
}

void PrintILC::processSetTempILCAddress(uint8_t address, uint8_t newAddress) {
    printBusAddress(address);
    std::cout << "New address " << std::to_string(newAddress) << std::endl;
}

void PrintILC::processResetServer(uint8_t address) {
    printBusAddress(address);
    std::cout << "Reseted." << std::endl;
}

void PrintILC::processEraseILCApplication(uint8_t address) {
    printBusAddress(address);
    std::cout << "ILC application erased." << std::endl;
}

void PrintILC::printBusAddress(uint8_t address) {
    printSepline();
    std::cout << "Bus: " << std::to_string(getBus()) << " (" << static_cast<char>('A' - 1 + getBus()) << ")"
              << std::endl
              << "Address: " << std::to_string(address) << std::endl;
}

void PrintILC::printSepline() {
    if (_printout > 0) {
        std::cout << std::endl;
    }
    _printout++;
}
