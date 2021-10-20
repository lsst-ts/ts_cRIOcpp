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

#include <cRIO/Settings/Path.h>
#include <spdlog/spdlog.h>

using namespace LSST::cRIO::Settings;

void Path::setRootPath(std::string rootPath) {
    SPDLOG_DEBUG("Settings::Path: setRootPath(\"{}\")", rootPath);

    auto test_dir = [rootPath](std::string dir) {
        struct stat dirstat;
        if (stat(dir.c_str(), &dirstat)) {
            throw std::runtime_error("Directory " + rootPath + "doesn't exist: " + strerror(errno));
        }
        if (!(dirstat.st_mode & (S_IFLNK | S_IFDIR))) {
            throw std::runtime_error(rootPath + " isn't directory or link");
        }
    };

    test_dir(rootPath);

    instance()._rootPath = rootPath;
}

std::string Path::getFilePath(std::string filename) {
    if (filename[0] == '/') return filename;
    return instance()._rootPath + "/" + filename;
}
