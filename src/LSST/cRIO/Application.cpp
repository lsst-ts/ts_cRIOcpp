/*
 * This file is part of LSST M1M3 support system package.
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

#include "cRIO/Application.h"

#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <cmath>
#include <csignal>

#include <spdlog/async.h>

namespace LSST {
namespace cRIO {

using namespace std;

Application::~Application() {}

void Application::addArgument(const char arg, const char* help, const char modifier) {
    _arguments.push_back(Argument(arg, help, modifier));
}

command_vec Application::processArgs(int argc, char* const argv[]) {
    command_vec argcommand;

    setName(basename(argv[0]));

    // parse as options only string before commands
    // as commands can include negative number (-1..), don't allow getopt
    // processing of command part

    int commandStart = argc;

    char pargs[2 * _arguments.size() + 1];
    char* p = pargs;
    for (auto arg : _arguments) {
        *p = arg.arg;
        p++;
        if (arg.modifier != 0) {
            *p = arg.modifier;
            p++;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            commandStart = i;

            for (; i < argc; i++) {
                argcommand.push_back(argv[i]);
            }

            break;
        }

        const char* a = strchr(pargs, argv[i][1]);

        if (a && a[1] == ':' && ((strlen(argv[i]) == 2) || (argv[i][1] == '-'))) {
            i++;
        }
    }

    int opt = -1;

    while ((opt = getopt(commandStart, argv, pargs)) != -1) {
        processArg(opt, optarg);
    }

    return argcommand;
}

void Application::setDebugLevel(int newLevel) {
    _debugLevel = newLevel;
    spdlog::set_level(getSpdLogLogLevel());
}

void Application::setSinks() {
    auto logger = std::make_shared<spdlog::async_logger>(
            _name, _sinks.begin(), _sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::set_default_logger(logger);
    spdlog::set_level(getSpdLogLogLevel());
}

spdlog::level::level_enum Application::getSpdLogLogLevel() {
    return _debugLevel == 0 ? spdlog::level::info
                            : (_debugLevel == 1 ? spdlog::level::debug : spdlog::level::trace);
}

void Application::printAppHelp() {
    cout << getName() << " " << _description << endl << endl;
    printUsage();
}

void Application::printUsage() {
    for (auto arg : _arguments) {
        cout << "  -" << arg.arg << " " << arg.help << endl;
    }
}

}  // namespace cRIO
}  // namespace LSST
