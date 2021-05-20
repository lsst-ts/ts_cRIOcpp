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

#ifndef __cRIO_CSC_H
#define __cRIO_CSC_H

#include <cRIO/Application.h>
#include <cRIO/Singleton.h>
#include <spdlog/spdlog.h>
#include <string>

namespace LSST {
namespace cRIO {

/**
 * Application class for Commandable SAL Component (CSC). Allows running
 * application as daemon, handles generic SAL commands.
 */
class CSC : public Application, public Singleton<CSC> {
public:
    CSC(token);

    virtual ~CSC();

    void setName(std::string name, const char* description);

    /**
     * Runs CSC. Starts all threads, waits for exitControl command.
     */
    void run();

    const char* _configRoot;

protected:
    virtual void processArg(int opt, char* optarg);

    virtual void init() {}

    /**
     * One control loop run.
     *
     * @return 0 if loop shall stop, otherwise loop will continue to run.
     */
    virtual int runLoop() { return 1; }

private:
    std::string _name;

    int _debugLevelSAL;
    bool _keep_running;

    struct DaemonOptions {
        DaemonOptions() {
            pidfile = NULL;
            timeout = 30;
        }
        const char* pidfile;
        std::string user;
        std::string group;
        int timeout;
    };

    DaemonOptions _daemon;

    const char* _pidfile;

    void _startLog();
    int _daemonize();
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !__cRIO_CSC_H
