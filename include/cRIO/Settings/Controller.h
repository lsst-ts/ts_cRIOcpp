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

#ifndef _TS_Settings_Controller_h
#define _TS_Settings_Controller_h

#include <cRIO/Singleton.h>

#include <Settings/Alias.h>

namespace LSST {
namespace cRIO {
namespace Settings {

/**
 * Settings controller. Loads all application settings.
 */
class Controller : public cRIO::Singleton<Controller> {
public:
    Controller(token) {}

    void setRoot(const char* configRoot) {
        _configRoot = configRoot;
        _aliases.load(_configRoot + "/Base/AliasApplicationSettings.yaml");
    }

    virtual void load(const std::string& label) = 0;

private:
    std::string _configRoot;
    Settings::Alias _aliases;
};

}  // namespace Settings
}  // namespace cRIO
}  // namespace LSST

#endif  // !_TS_Settings_Controller_h
