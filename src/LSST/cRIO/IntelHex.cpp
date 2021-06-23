/*
 * Reads Intel hex file
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

#include <cRIO/IntelHex.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <fstream>

using namespace LSST::cRIO;

IntelHex::IntelHex() {
    _startAddress = 0;
    _endAddress = 0;
}

void IntelHex::load(const std::string &fileName) {
    std::ifstream inputStream(fileName);
    load(inputStream);
    inputStream.close();
}

void IntelHex::load(std::istream &inputStream) {
    _hexData.clear();
    _lineNo = 0;
    std::string lineText;
    bool extensionData = false;
    while (std::getline(inputStream, lineText)) {
        _lineNo++;
        IntelHexLine hexLine;
        _processLine(lineText.c_str(), &hexLine);
        switch (hexLine.RecordType) {
            case IntelRecordType::Data:
                if (extensionData) {
                    for (auto d : hexLine.Data) {
                        if (d != 0) {
                            throw LoadError(_lineNo, 0xFFFF,
                                            fmt::format("Non zero data in extension - {}", d));
                        }
                    }
                } else {
                    _hexData.push_back(hexLine);
                }
                break;
            case IntelRecordType::ExtendedLinearAddress:
                // ILCs doesn't support extended addressing.
                // Ignore all data above 0xFFFF address
                if (hexLine.Data.size() != 2) {
                    throw LoadError(
                            _lineNo, 0xFFFF,
                            fmt::format("Invalid extension size - expected 2, got {}", hexLine.Data.size()));
                }
                extensionData = *(reinterpret_cast<uint16_t *>(hexLine.Data.data())) > 0;
                break;
            case IntelRecordType::EndOfFile:
                return;
            default:
                break;
        }
    }
}

void IntelHex::_processLine(const char *line, IntelHexLine *hexLine) {
    int offset = 1;
    if (line[0] != ':') {
        throw LoadError(_lineNo, 0xFFFF,
                        fmt::format("Invalid IntelHexLine StartCode '{}' expecting '{}'", line[0], ':'));
    }

    unsigned int byteCount = 0;
    unsigned int address = 0;
    unsigned int recordType = 0;
    int returnCode = sscanf(line + offset, "%2X%4X%2X", &byteCount, &address, &recordType);
    if (returnCode != 3) {
        throw LoadError(_lineNo, 0xFFFF, "Unable to Parse ByteCount, Address, and RecordType for line.");
    }
    hexLine->Address = (unsigned short)address;
    hexLine->RecordType = (IntelRecordType::Types)recordType;
    offset += 8;
    for (unsigned int i = 0; i < byteCount; ++i) {
        unsigned int value = 0;
        returnCode = sscanf(line + offset, "%2X", &value);
        offset += 2;
        if (returnCode >= 0) {
            hexLine->Data.push_back((char)value);
        } else {
            throw LoadError(_lineNo, address, "Unable to parse DataByte " + std::to_string(i));
        }
    }
    unsigned int expectedChecksum = 0;
    returnCode = sscanf(line + offset, "%2X", &expectedChecksum);
    hexLine->Checksum = expectedChecksum;
    if (returnCode != 1) {
        throw LoadError(_lineNo, address, "Unable to parse Checksum");
    }
    char checksum = 0;
    checksum += byteCount;
    checksum += (char)((hexLine->Address & 0xFF00) >> 8);
    checksum += (char)(hexLine->Address & 0x00FF);
    checksum += hexLine->RecordType;
    for (unsigned int i = 0; i < hexLine->Data.size(); ++i) {
        checksum += hexLine->Data[i];
    }
    checksum = ~checksum;
    checksum += 1;
    if (checksum != hexLine->Checksum) {
        throw LoadError(_lineNo, address,
                        fmt::format("Checksum mismatch, expecting 0x{:02X}, got 0x{:02X}", hexLine->Checksum,
                                    checksum));
    }
}

void IntelHex::_sortByAddress() {
    std::sort(_hexData.begin(), _hexData.end(),
              [](IntelHexLine a, IntelHexLine b) { return a.Address < b.Address; });
}

void IntelHex::_fillAppData() {
    // initialize appData. Empty record is three 0xFF followed by 0x00.
    for (size_t i = 0; i < 0xFFFF; i += 4) {
        static const uint8_t emptyData[4] = {0xFF, 0xFF, 0xFF, 0x00};
        memcpy(_appData + i, emptyData, 4);
    }
    _startAddress = 0xFFFF;
    _endAddress = 0;

    for (auto hd : _hexData) {
        switch (hd.RecordType) {
            case IntelRecordType::Data:
                _startAddress = std::min(_startAddress, hd.Address);
                _endAddress = std::max(_endAddress, hd.Address + hd.Data.size());
                memcpy(_appData, hd.Data.data(), hd.Data.size());
                break;
            default:
                break;
        }
    }
}
