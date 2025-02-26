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

#ifndef __cRIO_CSC_H
#define __cRIO_CSC_H

#include <string>

#include <spdlog/spdlog.h>

#include <cRIO/Application.h>
#include <cRIO/SimpleFPGA.h>
#include <cRIO/Singleton.h>

namespace LSST {
namespace cRIO {

/**
 * Application class for Commandable SAL Component (CSC). Allows running
 * application as daemon, handles generic SAL commands. When CSC is started as
 * daemon, a pipe is opened between controlling parent and child attempting to
 * run as daemon. A message specifying that daemon startes or failed is send on
 * pipe by child and read by the parent. See daemonOK() and daemonFailed(const
 * char*) for details.
 */
class CSC : public Application {
public:
    CSC(const char* name, const char* description);
    virtual ~CSC();

    /**
     * Runs CSC. Starts all threads, loads FPGA bitfile, run FPGA, waits for
     * exitControl command.
     *
     * @param fpga FPGA to open, initialize and run
     */
    int run(SimpleFPGA* fpga);

    /**
     * Returns current SAL debug level.
     *
     * @return current SAL debug level
     */
    int getDebugLevelSAL() { return _debugLevelSAL; }

    /**
     * Stops CSC.
     */
    void stop() { _keep_running = false; }

    /**
     * Returns configuration root.
     */
    std::string getConfigRoot() { return _configRoot; }

protected:
    virtual void processArg(int opt, char* arg);

    /**
     * Initialize CSC. Called after daemonization, with correct PID.
     */
    virtual void init() {}

    /**
     * Destroy CSC. Called after main loop finishes.
     */
    virtual void done() {}

    /**
     * One control loop run.
     *
     * @return 0 if loop shall stop, otherwise loop will continue to run.
     */
    virtual int runLoop() = 0;

    /**
     * Informs controlling parent that daemon was started and is running.
     */
    void daemonOK();

    /**
     * Informs controlling parent that daemon failed to run.
     *
     * @param msg message with details why daemon failed to start
     */
    void daemonFailed(const char* msg);

private:
    std::string _configRoot;

    int _debugLevelSAL;
    bool _keep_running;

    struct DaemonOptions {
        DaemonOptions() {
            pidfile = NULL;
            timeout = 30;
            fork = false;
        }
        const char* pidfile;
        std::string user;
        std::string group;
        bool fork;
        int timeout;
    };

    DaemonOptions _daemon;

    void _startLog();
    int _daemonize();

    int _startPipe[2];

    const char* _fpgaDebugPath;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !__cRIO_CSC_H
