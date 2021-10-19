/*
 * This file is part of LSST M1M3 thermal system package.
 *
 * Developed for the LSST Data Management System.
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

#ifndef _Settings_Alias_h
#define _Settings_Alias_h

#include <string>
#include <vector>
#include <map>

namespace LSST {
namespace cRIO {
namespace Settings {

/**
 * Loads settings. Provides functions to query settings. Aliases are used to
 * resolve configuration name passed in start command.
 *
 * Aliases are used for path to configuration set.
 *
 * Alias file is a YAML file with tags marking alias sets, containing Set: and
 * Version: elements. See example below:
 *
 * @code
 * Default:
 *   Set: "Default"
 *   Version: 1.2
 *
 * @endcode
 */
class Alias {
public:
    /**
     * Loads application settings.
     *
     * @param filename aliases filename
     */
    void load(const std::string& filename);

    /**
     * Returns pair of (set,version)
     *
     * @param label alias label
     *
     * @return pair of (set,version)
     */
    std::pair<std::string, std::string> getAlias(std::string label);

    /**
     * Returns path for aliases.
     *
     * @param label alias label
     *
     * @return string with path to configuration files
     */
    std::string getPath(std::string label);

private:
    std::map<std::string, std::pair<std::string, std::string>> _aliases;
};

}  // namespace Settings
}  // namespace cRIO
}  // namespace LSST

#endif  //! _Settings_Alias_h
