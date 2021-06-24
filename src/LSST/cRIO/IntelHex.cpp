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
        IntelRecordType::Types recordType;
        _processLine(lineText.c_str(), &hexLine, recordType);
        switch (recordType) {
            case IntelRecordType::Data:
<<<<<<< HEAD
                if (extensionData == false) {
=======
                if (extensionData) {
                    for (auto d : hexLine.Data) {
                        if (d != 0) {
                            throw LoadError(_lineNo, 0xFFFF,
                                            fmt::format("Non zero data in extension - {}", d));
                        }
                    }
                } else {
>>>>>>> continue with firmwareUpdate implementation
                    _hexData.push_back(hexLine);
                }
                break;
            case IntelRecordType::ExtendedLinearAddress:
                // ILCs doesn't support extended addressing.
                // Ignore all data above 0xFFFF address
<<<<<<< HEAD
                if (hexLine.data.size() != 2) {
                    throw LoadError(
                            _lineNo, 0xFFFF,
                            fmt::format("Invalid extension size - expected 2, got {}", hexLine.data.size()));
                }
                extensionData = *(reinterpret_cast<uint16_t *>(hexLine.data.data())) > 0;
=======
                if (hexLine.Data.size() != 2) {
                    throw LoadError(
                            _lineNo, 0xFFFF,
                            fmt::format("Invalid extension size - expected 2, got {}", hexLine.Data.size()));
                }
                extensionData = *(reinterpret_cast<uint16_t *>(hexLine.Data.data())) > 0;
>>>>>>> continue with firmwareUpdate implementation
                break;
            case IntelRecordType::EndOfFile:
                return;
            default:
                break;
        }
    }
}

<<<<<<< HEAD
<<<<<<< HEAD
std::vector<uint8_t> IntelHex::getData(uint16_t &startAddress) {
    _sortByAddress();

    startAddress = _hexData.front().address;

    uint16_t lastCopied = startAddress;

    std::vector<uint8_t> ret;

    for (auto hd : _hexData) {
        for (uint16_t i = lastCopied; i < hd.address; i++) {
            ret.push_back(((i % 4) == 3) ? 0x00 : 0xff);
        }

        size_t start = ret.size();
        ret.resize(ret.size() + hd.data.size());
        memcpy(ret.data() + start, hd.data.data(), hd.data.size());

        lastCopied = hd.address + hd.data.size();
    }

    return ret;
}

void IntelHex::_processLine(const char *line, IntelHexLine *hexLine, IntelRecordType::Types &recordType) {
=======
void IntelHex::_processLine(const char *line, IntelHexLine *hexLine) {
>>>>>>> continue with firmwareUpdate implementation
=======
void IntelHex::_processLine(const char *line, IntelHexLine *hexLine, IntelRecordType::Types &recordType) {
>>>>>>> write app data
    int offset = 1;
    if (line[0] != ':') {
        throw LoadError(_lineNo, 0xFFFF,
                        fmt::format("Invalid IntelHexLine StartCode '{}' expecting '{}'", line[0], ':'));
    }

    unsigned int byteCount = 0;
    unsigned int address = 0;
    unsigned int recType = 0;
    int returnCode = sscanf(line + offset, "%2X%4X%2X", &byteCount, &address, &recType);
    if (returnCode != 3) {
        throw LoadError(_lineNo, 0xFFFF, "Unable to Parse ByteCount, Address, and RecordType for line.");
    }
<<<<<<< HEAD
    hexLine->address = (unsigned short)address;
=======
    hexLine->Address = (unsigned short)address;
>>>>>>> write app data
    recordType = static_cast<IntelRecordType::Types>(recType);
    offset += 8;
    for (unsigned int i = 0; i < byteCount; ++i) {
        unsigned int value = 0;
        returnCode = sscanf(line + offset, "%2X", &value);
        offset += 2;
        if (returnCode >= 0) {
            hexLine->data.push_back(static_cast<uint8_t>(value));
        } else {
            throw LoadError(_lineNo, address, "Unable to parse DataByte " + std::to_string(i));
        }
    }
    unsigned int expectedChecksum = 0;
    returnCode = sscanf(line + offset, "%2X", &expectedChecksum);
    if (returnCode != 1) {
        throw LoadError(_lineNo, address, "Unable to parse Checksum");
    }
    uint8_t checksum = 0;
    checksum += byteCount;
    checksum += static_cast<uint8_t>((hexLine->address & 0xFF00) >> 8);
    checksum += static_cast<uint8_t>(hexLine->address & 0x00FF);
    checksum += recordType;
    for (unsigned int i = 0; i < hexLine->data.size(); ++i) {
        checksum += hexLine->data[i];
    }
    checksum = ~checksum;
    checksum += 1;
    if (checksum != expectedChecksum) {
        throw LoadError(_lineNo, address,
                        fmt::format("Checksum mismatch, expecting 0x{:02X}, got 0x{:02X}", expectedChecksum,
                                    checksum));
    }
}

void IntelHex::_sortByAddress() {
    std::sort(_hexData.begin(), _hexData.end(),
              [](IntelHexLine a, IntelHexLine b) { return a.address < b.address; });
}
