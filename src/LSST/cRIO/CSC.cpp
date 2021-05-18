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

#include <cRIO/CSC.h>

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#include <iostream>

namespace LSST {
namespace cRIO {

CSC::CSC(token) : CliApp() {
    _debugLevel = 0;
    _debugLevelSAL = 0;
    _keep_running = true;

    enabledSinks = 0;

    addArgument('d', "increases debugging (can be specified multiple times, default is info");
    addArgument('h', "prints this help");
}

void CSC::run() {
    // create threads


    // run threads
    while (_keep_running == true) {

    }

}

void CSC::processArg(int opt, const char* optarg) {
    switch (opt) {
        case 'd':
            _debugLevel++;
            break;
        case 'h':
            printAppHelp();
            exit(EXIT_SUCCESS);
        default:
            std::cerr << "Unknow option " << opt << std::endl;
            printAppHelp();
            exit(EXIT_FAILURE);
    }
}

void CSC::setSinks() {
    auto logger = std::make_shared<spdlog::async_logger>(
            _name, _sinks.begin(), _sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::set_default_logger(logger);
    spdlog::set_level(getSpdLogLogLevel());
}

spdlog::level::level_enum CSC::getSpdLogLogLevel() {
    return _debugLevel == 0 ? spdlog::level::info
                            : (_debugLevel == 1 ? spdlog::level::debug : spdlog::level::trace);
}

void CSC::startLog() {
    spdlog::init_thread_pool(8192, 1);
    if (enabledSinks & Sinks::STDOUT) {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        _sinks.push_back(stdout_sink);
    }
    if (enabledSinks & Sinks::DAILY) {
        auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(_name, 0, 0);
        _sinks.push_back(daily_sink);
    }
    if (enabledSinks & Sinks::SYSLOG) {
        auto syslog_sink =
                std::make_shared<spdlog::sinks::syslog_sink_mt>(_name, LOG_PID | LOG_CONS, LOG_USER, false);
        _sinks.push_back(syslog_sink);
    }

    setSinks();
}

}  // namespace cRIO
}  // namespace LSST
