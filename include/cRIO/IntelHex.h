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

#ifndef CRIO_INTELHEX_H_
#define CRIO_INTELHEX_H_

#include <istream>
#include <string>
#include <vector>

namespace LSST {
namespace cRIO {

struct IntelRecordType {
    enum Types {
        Data = 0,
        EndOfFile = 1,
        ExtendedSegmentAddress = 2,
        StartSegmentAddress = 3,
        ExtendedLinearAddress = 4,
        StartLinearAddress = 5
    };
};

struct IntelHexLine {
    char StartCode;
    char ByteCount;
    unsigned short Address;
    IntelRecordType::Types RecordType;
    std::vector<char> Data;
    char Checksum;
};

struct ILCApplicationStats {
    unsigned short DataCRC;
    unsigned short StartAddress;
    unsigned short DataLength;
    unsigned short StatsCRC;
};

/**
 * Class to read and parse Intel hex file.
 */
class IntelHex {
public:
    IntelHex();

    /**
     * Parse & load Intel Hex file.
     *
     * @param fileName hex filename
     */
    bool load(const std::string &fileName);
    bool load(std::istream &inputStream);

private:
    bool _processLine(const char *line, IntelHexLine *hexLine);

    std::vector<unsigned char> _appData;
    std::vector<IntelHexLine> _hexData;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !CRIO_INTELHEX_H_
