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

#include <iomanip>
#include <iostream>

#include <spdlog/fmt/fmt.h>

#include <cRIO/ModbusBuffer.h>
#include <cRIO/PrintILC.h>
#include <cRIO/SimulatedILC.h>

using namespace LSST::cRIO;

PrintILC::PrintILC(uint8_t bus) : ILC(bus), _printout(0), _lastAddress(0) {
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
    SimulatedILC buf;

    uint16_t v = htole16(dataCRC);
    buf.writeBuffer(reinterpret_cast<uint8_t *>(&v), 2);
    buf.write<uint16_t>(0);

    v = htole16(startAddress);
    buf.writeBuffer(reinterpret_cast<uint8_t *>(&v), 2);
    buf.write<uint16_t>(0);

    v = htole16(dataLength);
    buf.writeBuffer(reinterpret_cast<uint8_t *>(&v), 2);
    buf.write<uint16_t>(0);

    callFunction(address, 100, 500000, dataCRC, startAddress, dataLength, buf.getCalcCrc());
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
    clear();

    static constexpr int32_t ILC_TIMEOUT = 1000;

    reportServerStatus(address);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    switch (getLastMode(address)) {
        // those modes need fault first
        case ILCMode::Enabled:
            changeILCMode(address, ILCMode::Disabled);
        case ILCMode::Disabled:
            changeILCMode(address, ILCMode::Standby);
            break;
        case ILC::ILCMode::Fault:
            changeILCMode(address, ILCMode::ClearFaults);
            break;
        default:
            break;
    }

    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    if (getLastMode(address) != ILC::FirmwareUpdate) {
        changeILCMode(address, ILCMode::FirmwareUpdate);
        fpga->ilcCommands(*this, ILC_TIMEOUT);
        clear();
    }

    if (getLastMode(address) == ILC::Fault) {
        changeILCMode(address, ILCMode::ClearFaults);
        fpga->ilcCommands(*this, ILC_TIMEOUT);
        clear();
    }

    if (getLastMode(address) != ILC::FirmwareUpdate) {
        throw std::runtime_error("Cannot transition to FirmwareUpdate mode");
    }

    eraseILCApplication(address);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    _startAddress = 0;
    _dataLength = 0;

    _crc.reset();

    _writeHex(fpga, address, hex);

    writeApplicationStats(address, _crc.get(), _startAddress, _dataLength);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    writeVerifyApplication(address);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    changeILCMode(address, ILCMode::Standby);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
    clear();

    if (getLastMode(address) == ILC::Fault) {
        changeILCMode(address, ILCMode::ClearFaults);
        fpga->ilcCommands(*this, ILC_TIMEOUT);
        clear();
    }

    changeILCMode(address, ILCMode::Disabled);
    fpga->ilcCommands(*this, ILC_TIMEOUT);
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
    std::cout << "Mode: " << std::to_string(mode) << " - " << getModeStr(mode) << std::endl
              << "Status: " << std::hex << std::setw(4) << std::setfill('0') << +status << " "
              << fmt::format("{}", fmt::join(getStatusString(status), " | ")) << std::endl
              << "Faults: " << std::hex << std::setw(4) << std::setfill('0') << +faults << " "
              << fmt::format("{}", fmt::join(getFaultString(faults), " | ")) << std::endl;
}

void PrintILC::processChangeILCMode(uint8_t address, uint16_t mode) {
    printBusAddress(address);
    std::cout << "New mode: " << std::to_string(mode) << " - " << getModeStr(mode) << std::endl;
}

void PrintILC::processSetTempILCAddress(uint8_t address, uint8_t newAddress) {
    printBusAddress(address);
    std::cout << "New address " << std::to_string(newAddress) << std::endl;
}

void PrintILC::processResetServer(uint8_t address) {
    printBusAddress(address);
    std::cout << "Reseted " << static_cast<int>(address) << std::endl;
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
    std::cout << ".";
    std::cout.flush();
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
    if (address == _lastAddress) {
        return;
    }
    printSepline();
    std::cout << "Bus: " << std::to_string(getBus()) << " (" << static_cast<char>('A' - 1 + getBus()) << ")"
              << std::endl
              << "Address: " << std::to_string(address) << std::endl;
    _lastAddress = address;
}

void PrintILC::printSepline() {
    if (_printout > 0) {
        std::cout << std::endl;
    }
    _printout++;
}

void PrintILC::_writeHex(FPGA *fpga, uint8_t address, IntelHex &hex) {
    // align data to 256 bytes pages
    std::vector<uint8_t> data = hex.getData(_startAddress);

    size_t mod = data.size() % 256;
    if (mod == 0) {
        mod = 256;
    }

    // CRC is calculated only from data, skips filling
    for (auto d : data) {
        _crc.add(d);
    }

    std::cout << "Writing pages ";

    _dataLength = data.size();

    for (int i = mod; i < 256; i++) {
        data.push_back(((i % 4) == 3) ? 0x00 : 0xFF);
    }

    uint8_t *startData = data.data();
    uint8_t *endData = data.data() + data.size();
    uint16_t dataAddress = _startAddress;
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
        fpga->ilcCommands(*this, 5000);

        clear();
        dataAddress += 256;
    }

    std::cout << std::endl;
}
