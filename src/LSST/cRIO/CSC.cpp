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
#include <grp.h>
#include <pwd.h>
#include <signal.h>

namespace LSST {
namespace cRIO {

CSC::CSC(token) : Application() {
    _debugLevel = 0;
    _debugLevelSAL = 0;
    _keep_running = true;

    enabledSinks = 0;

    addArgument('d', "increases debugging (can be specified multiple times, default is info");
    addArgument('f', "runs on foreground, don't log to file");
    addArgument('h', "prints this help");
    addArgument('p', "PID file, started as daemon on background", ':');
    addArgument('u', "<user>:<group> run under user & group", ':');
}

CSC::~CSC() {}

void CSC::run() {
    // create threads

    int ret_d = _daemonize();
    if (ret_d == -1) {
        // run threads
        while (_keep_running == true) {
        }
    }
}

void CSC::processArg(int opt, char* optarg) {
    switch (opt) {
        case 'd':
            _debugLevel++;
            break;
        case 'f':
            enabledSinks |= Sinks::STDOUT;
            break;
        case 'h':
            printAppHelp();
            exit(EXIT_SUCCESS);
        case 'p':
            _daemon.pidfile = optarg;
            enabledSinks |= Sinks::SYSLOG | Sinks::SAL;
            break;
        case 'u': {
            char* sep = strchr(optarg, ':');
            if (sep) {
                *sep = '\0';
                _daemon.user = optarg;
                _daemon.group = sep + 1;
            } else {
                _daemon.user = _daemon.group = optarg;
            }
            break;
        }
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

void CSC::_startLog() {
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

int CSC::_daemonize() {
    // daemon is expected to sends to stdout either OK if started, or error message otherwise
    // if no response is received within _daemon.timeout seconds, startup isn't confirmed
    int startPipe[2] = {-1, -1};

    if (_daemon.pidfile) {
        struct passwd* runAs = NULL;
        struct group* runGroup = NULL;
        if (_daemon.user.length() > 0) {
            runAs = getpwnam(_daemon.user.c_str());
            if (runAs == NULL) {
                std::cerr << "Error: Cannot find user " << _daemon.user << std::endl;
                exit(EXIT_FAILURE);
            }
            runGroup = getgrnam(_daemon.group.c_str());
            if (runGroup == NULL) {
                std::cerr << "Error: Cannot find group " << _daemon.group << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        if (pipe(startPipe) == -1) {
            std::cerr << "Error: Cannot create pipe for child/start process: " << strerror(errno)
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        pid_t child = fork();
        if (child < 0) {
            std::cerr << "Error: Cannot fork:" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        if (child > 0) {
            close(startPipe[1]);
            std::ofstream pidf(_daemon.pidfile, std::ofstream::out);
            pidf << child;
            pidf.close();
            if (pidf.fail()) {
                std::cerr << "Error: Cannot write to PID file " << _daemon.pidfile << ": " << strerror(errno)
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            if (runAs != NULL) {
                if (chown(_daemon.pidfile, runAs->pw_uid, runGroup->gr_gid)) {
                    std::cerr << "Error: Cannot change owner of " << _daemon.pidfile << ":" << strerror(errno)
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            char retbuf[2000];
            memset(retbuf, 0, sizeof(retbuf));
            signal(SIGALRM, [](int) {
                std::cerr << "Error: Start timeouted, see syslog for details." << std::endl;
                exit(EXIT_FAILURE);
            });
            alarm(_daemon.timeout);
            read(startPipe[0], retbuf, 2000);
            if (strcmp(retbuf, "OK") == 0) {
                return EXIT_SUCCESS;
            }
            std::cerr << retbuf << std::endl;
            return EXIT_FAILURE;
        }
        close(startPipe[0]);
        _startLog();
        if (runAs != NULL) {
            setuid(runAs->pw_uid);
            setgid(runGroup->gr_gid);
            SPDLOG_DEBUG("Running as {}:{}", _daemon.user, _daemon.group);
        }
        if (!(enabledSinks & 0x01)) {
            close(0);
            close(1);
            close(2);
            int nf = open("/dev/null", O_RDWR);
            dup(nf);
            dup(nf);
            dup(nf);
        }
    } else {
        _startLog();
    }
    return -1;
}

}  // namespace cRIO
}  // namespace LSST
