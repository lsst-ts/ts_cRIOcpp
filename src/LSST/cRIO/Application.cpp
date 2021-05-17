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

#include <cRIO/Application.h>

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#include <iostream>

namespace LSST {
namespace cRIO {

Application::Application(const char* _description) : CliApp(_description) {
    debugLevel = 0;
    debugLevelSAL = 0;

    enabledSinks = 0;

    addArgument('d', "increases debugging (can be specified multiple times, default is info");
    addArgument('h', "prints this help");
}

void Application::processArg(int opt, const char* optarg) {
    switch (opt) {
        case 'd':
            debugLevel++;
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

void Application::setSinks() {
    auto logger = std::make_shared<spdlog::async_logger>("M1M3support", sinks.begin(), sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
    spdlog::set_default_logger(logger);
    spdlog::set_level(getSpdLogLogLevel());
}

spdlog::level::level_enum Application::getSpdLogLogLevel() {
    return debugLevel == 0 ? spdlog::level::info
                           : (debugLevel == 1 ? spdlog::level::debug : spdlog::level::trace);
}

void Application::startLog() {
    spdlog::init_thread_pool(8192, 1);
    if (enabledSinks & 0x01) {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(stdout_sink);
    }
    if (enabledSinks & 0x02) {
        auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("M1M3support", 0, 0);
        sinks.push_back(daily_sink);
    }
    if (enabledSinks & 0x04) {
        auto syslog_sink = std::make_shared<spdlog::sinks::syslog_sink_mt>("M1M3support", LOG_PID | LOG_CONS,
                                                                           LOG_USER, false);
        sinks.push_back(syslog_sink);
    }

    setSinks();
}

}  // namespace cRIO
}  // namespace LSST
