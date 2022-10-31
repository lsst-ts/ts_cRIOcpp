/*
 * This file is part of LSST M1M3 support system package.
 *
 * Developed for the Vera C. Rubin Telescope and Site System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STRINGJOIN_H_
#define STRINGJOIN_H_

#include <string>

namespace LSST {
namespace cRIO {

/**
 * Join iterable (list/vector/...) into string.
 *
 * @tparam T iterable type
 * @param list iterable to join
 * @param delimiter items delimiter. Defaults to ","
 *
 * @return string with items separeted by delimiter
 */
template <typename T>
std::string join(T list, wchar_t delimiter = ',') {
    std::string ret;
    for (auto i : list) {
        if (ret.size() != 0) {
            ret += delimiter;
        }
        ret += i;
    }
    return ret;
}

}  // namespace cRIO
}  // namespace LSST

#endif /* STRINGJOIN_H_ */
