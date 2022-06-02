/*
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

#ifndef TABLELOADER_H_
#define TABLELOADER_H_

#include <fstream>
#include <string>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <cRIO/Limit.h>
#include <cRIO/DataTypes.h>
#include <cRIO/Settings/Path.h>

namespace LSST {
namespace cRIO {

class TableLoader {
public:
    template <typename t>
    static void loadTable(int rowsToSkip, int columnsToSkip, int columnsToKeep, std::vector<t>* data,
                          const std::string& filename);
    static void loadLimitTable(int rowsToSkip, int columnsToSkip, std::vector<Limit>* data,
                               const std::string& filename);
};

template <typename t>
void TableLoader::loadTable(int rowsToSkip, int columnsToSkip, int columnsToKeep, std::vector<t>* data,
                            const std::string& filename) {
    typedef boost::tokenizer<boost::escaped_list_separator<char> > tokenizer;

    std::string fullPath = Settings::Path::getFilePath(filename);
    std::ifstream inputStream(fullPath.c_str());
    if (!inputStream.is_open()) {
        throw std::runtime_error("Cannot open " + fullPath + ": " + strerror(errno));
    }
    std::string lineText;
    int32_t lineNumber = 0;
    data->clear();
    while (std::getline(inputStream, lineText)) {
        boost::trim_right(lineText);
        if (lineNumber >= rowsToSkip && !lineText.empty()) {
            tokenizer tok(lineText);
            tokenizer::iterator i = tok.begin();
            for (int j = 0; j < columnsToSkip; j++) {
                ++i;
            }
            for (int j = 0; j < columnsToKeep; j++) {
                data->push_back(boost::lexical_cast<t>(*i));
                ++i;
            }
        }
        lineNumber++;
    }
    inputStream.close();
}

}  // namespace cRIO
}  // namespace LSST

#endif /* TABLELOADER_H_ */
