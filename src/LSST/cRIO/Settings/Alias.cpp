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

#include <cRIO/Settings/Alias.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

using namespace LSST::cRIO::Settings;

void Alias::load(const std::string& filename) {
    _aliases.clear();

    SPDLOG_DEBUG("Loading aliases from {}", filename);

    try {
        YAML::Node doc = YAML::LoadFile(filename);

        for (auto it = doc.begin(); it != doc.end(); ++it) {
            std::string name = it->first.as<std::string>();
            _aliases[name] = std::pair<std::string, std::string>(it->second["Set"].as<std::string>(),
                                                                 it->second["Version"].as<std::string>());
            SPDLOG_DEBUG("Alias {}->{}:{}", it->first.as<std::string>(), _aliases[name].first,
                         _aliases[name].second);
        }
    } catch (YAML::Exception& ex) {
        throw std::runtime_error(fmt::format("YAML Loading {}: {}", filename, ex.what()));
    }
}

std::pair<std::string, std::string> Alias::getAlias(std::string label) {
    size_t del = label.find(',');
    if (del != std::string::npos) {
        return std::pair<std::string, std::string>(label.substr(0, del), label.substr(del + 1));
    }
    return _aliases.at(label);
}

std::string Alias::getPath(std::string label) {
    auto a = getAlias(label);
    return "/Sets/" + a.first + "/" + a.second + "/";
}
