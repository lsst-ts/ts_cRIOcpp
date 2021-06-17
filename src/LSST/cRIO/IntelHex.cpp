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

#include <fstream>

using namespace LSST::cRIO;

IntelHex::IntelHex() {}

void IntelHex::load(const std::string &fileName) {
    std::ifstream inputStream(fileName);
    load(inputStream);
    inputStream.close();
}

void IntelHex::load(std::istream &inputStream) {
    _hexData.clear();
    _lineNo = 0;
    std::string lineText;
    bool allZero = true;
    bool ignoreData = false;
    while (std::getline(inputStream, lineText)) {
        _lineNo++;
        IntelHexLine hexLine;
        _processLine(lineText.c_str(), &hexLine);
        switch (hexLine.RecordType) {
            case IntelRecordType::Data:
                if (!ignoreData) {
                    _hexData.push_back(hexLine);
                }
                break;
            case IntelRecordType::ExtendedLinearAddress:
                // Basically if the data is non-zero that means we are skipping a bunch of address which
                // means we are at the end of the file and what we are skipping is a bunch of filler so
                // lets not bother doing anything and just ignore all of the data
                allZero = true;
                for (unsigned int i = 0; i < hexLine.Data.size() && allZero; ++i) {
                    allZero = hexLine.Data[i] == 0;
                }
                ignoreData = !allZero;
                break;
            default:
                break;
        }
    }
}

void IntelHex::_processLine(const char *line, IntelHexLine *hexLine) {
    hexLine->StartCode = line[0];
    int offset = 1;
    if (hexLine->StartCode != ':') {
        throw LoadError(_lineNo, 0xFFFF,
                        fmt::format("Invalid IntelHexLine StartCode '{}' expecting '{}'",
                                    (int)hexLine->StartCode, (int)':'));
    }

    unsigned int byteCount = 0;
    unsigned int address = 0;
    unsigned int recordType = 0;
    int returnCode = sscanf(line + offset, "%2X%4X%2X", &byteCount, &address, &recordType);
    if (returnCode != 3) {
        throw LoadError(_lineNo, 0xFFFF, "Unable to Parse ByteCount, Address, and RecordType for line.");
    }
    hexLine->ByteCount = (char)byteCount;
    hexLine->Address = (unsigned short)address;
    hexLine->RecordType = (IntelRecordType::Types)recordType;
    offset += 8;
    for (int i = 0; i < hexLine->ByteCount; ++i) {
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
    checksum += hexLine->ByteCount;
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
