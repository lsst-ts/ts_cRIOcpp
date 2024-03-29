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
    uint16_t address;
    std::vector<uint8_t> data;
};

class LoadError : public std::runtime_error {
public:
    LoadError(size_t line, uint16_t address, const std::string &arg) : std::runtime_error(arg) {
        _line = line;
        _address = address;
    }

private:
    uint16_t _address;
    size_t _line;
};

/**
 * Class to read and parse Intel hex file. Provides methods to faciliate
 * loading firmware into ILC.
 */
class IntelHex {
public:
    IntelHex();

    /**
     * Parse & load Intel Hex file.
     *
     * @param fileName hex filename
     *
     * @throws LoadError on error
     */
    void load(const std::string &fileName);
    void load(std::istream &inputStream);

    /**
     * Returns data to be written into ILC. Only three bytes out of every four
     * are written, but full buffer, as assembled from hex file lines, is
     * returned.
     *
     * @param startAddress returns memory start address
     * @return vector with data
     */
    std::vector<uint8_t> getData(uint16_t &startAddress);

private:
    void _processLine(const char *line, IntelHexLine *hexLine, IntelRecordType::Types &recordType);
    void _sortByAddress();

    std::vector<IntelHexLine> _hexData;
    size_t _lineNo;

    /**
     * Contains firmware data.
     */
    std::vector<uint8_t> _appData;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !CRIO_INTELHEX_H_
