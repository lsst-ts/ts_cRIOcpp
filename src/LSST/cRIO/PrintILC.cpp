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

#include <cRIO/ModbusBuffer.h>
#include <cRIO/PrintILC.h>

#include <iomanip>
#include <iostream>

using namespace LSST::cRIO;

PrintILC::PrintILC(uint8_t bus) : ILC(bus), _printout(0) {
    setAlwaysTrigger(true);

    addResponse(
            100,
            [this](uint8_t address) {
                checkCRC();
                processWriteApplicationStats(address);
            },
            228);

    addResponse(
            101,
            [this](uint8_t address) {
                checkCRC();
                processEraseILCApplication(address);
            },
            229);

    addResponse(
            102,
            [this](uint8_t address) {
                checkCRC();
                processWriteApplicationPage(address);
            },
            238);

    addResponse(
            103,
            [this](uint8_t address) {
                uint16_t status = read<uint16_t>();
                checkCRC();
                processVerifyUserApplication(address, status);
            },
            231);
}

void PrintILC::writeApplicationStats(uint8_t address, uint16_t dataCRC, uint16_t startAddress,
                                     uint16_t dataLength) {
    uint8_t buffer[12];
    *(reinterpret_cast<uint16_t *>(buffer + 0)) = htons(dataCRC);
    *(reinterpret_cast<uint16_t *>(buffer + 2)) = 0;
    *(reinterpret_cast<uint16_t *>(buffer + 4)) = htons(startAddress);
    *(reinterpret_cast<uint16_t *>(buffer + 6)) = 0;
    *(reinterpret_cast<uint16_t *>(buffer + 8)) = htons(dataLength);
    *(reinterpret_cast<uint16_t *>(buffer + 10)) = 0;

    ModbusBuffer::CRC crc;
    for (auto d : buffer) {
        crc.add(d);
    }
    callFunction(address, 100, 500000, dataCRC, startAddress, dataLength, crc.get());
}

void PrintILC::writeApplicationPage(uint8_t address, uint16_t startAddress, uint16_t length, uint8_t *data) {
    write(address);
    write<uint8_t>(102);
    write(startAddress);
    write(length);
    writeBuffer(data, length);
    writeCRC();
    writeEndOfFrame();
    writeWaitForRx(500000);

    pushCommanded(address, 102);
}

void PrintILC::programILC(FPGA *fpga, uint8_t address, IntelHex &hex) {
    printBusAddress(address);

    changeILCMode(address, ILCMode::Standby);
    fpga->ilcCommands(*this);
    clear();

    changeILCMode(address, ILCMode::FirmwareUpdate);
    fpga->ilcCommands(*this);
    clear();

    changeILCMode(address, ILCMode::ClearFaults);
    fpga->ilcCommands(*this);
    clear();

    uint16_t dataCRC;
    uint16_t startAddress;
    uint16_t dataLength;

    _writeHex(fpga, address, hex, dataCRC, startAddress, dataLength);

    writeApplicationStats(address, dataCRC, startAddress, dataLength);
    fpga->ilcCommands(*this);
    clear();

    writeVerifyApplication(address);
    fpga->ilcCommands(*this);
    clear();

    changeILCMode(address, ILCMode::Standby);
    fpga->ilcCommands(*this);
    clear();

    changeILCMode(address, ILCMode::Disabled);
    fpga->ilcCommands(*this);
    clear();
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

void PrintILC::processWriteApplicationStats(uint8_t address) {
    printBusAddress(address);
    std::cout << "New ILC application stats written." << std::endl;
}

void PrintILC::processEraseILCApplication(uint8_t address) {
    printBusAddress(address);
    std::cout << "ILC application erased." << std::endl;
}

void PrintILC::processWriteApplicationPage(uint8_t address) {
    printBusAddress(address);
    std::cout << "Page written." << std::endl;
}

void PrintILC::processVerifyUserApplication(uint8_t address, uint16_t status) {
    printBusAddress(address);
    uint8_t exception = 0;
    switch (status) {
        case 0x0000:
            std::cout << "Verified user application." << std::endl;
            return;
        case 0x00FF:
            std::cout << "Application Stats Error" << std::endl;
            exception = 1;
            break;
        case 0xFF00:
            std::cout << "Application Error" << std::endl;
            exception = 2;
            break;
        case 0xFFFF:
            std::cout << "Application Stats and Application Error" << std::endl;
            exception = 3;
            break;
        default:
            std::cout << "Uknown status: " << status << std::endl;
            exception = 4;
            break;
    }
    throw Exception(address, 102, exception);
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

void PrintILC::_writeHex(FPGA *fpga, uint8_t address, IntelHex &hex, uint16_t &dataCRC,
                         uint16_t &startAddress, uint16_t &dataLength) {
    // align data to 256 bytes pages
    std::vector<uint8_t> data = hex.getData(startAddress);

    size_t mod = data.size() % 256;
    if (mod == 0) return;

    for (int i = mod; i < 256; i++) {
        data.push_back(((i % 4) == 3) ? 0x00 : 0xFF);
    }

    ModbusBuffer::CRC crc;
    for (auto d : data) {
        crc.add(d);
    }
    dataCRC = crc.get();

    dataLength = data.size();

    uint8_t *startData = data.data();
    uint8_t *endData = data.data() + data.size();
    uint16_t dataAddress = startAddress;
    while (startData < endData) {
        uint8_t page[192];
        int i = 0;
        while (i < 192) {
            for (int j = 0; j < 3; j++) {
                page[i] = *startData;
                i++;
                startData++;
            }
            // skip every fourth byte
            startData++;
        }
        writeApplicationPage(address, dataAddress, 192, page);
        fpga->ilcCommands(*this);

        clear();
        dataAddress += 256;
    }
}
