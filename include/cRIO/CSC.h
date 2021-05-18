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

#include <CliApp.h>
#include <cRIO/Singleton.h>
#include <spdlog/spdlog.h>
#include <string>

namespace LSST {
namespace cRIO {

/**
 * Application class for Commandable SAL Component (CSC). Allows running
 * application as daemon, handles generic SAL commands.
 */
class CSC final : public CliApp, public Singleton<CSC> {
public:
    CSC(token);
    
    void setName(std::string name, const char* description);

    /**
     * Runs CSC. Starts all threads, waits for exitControl command.
     */
    void run();

    typedef enum { STDOUT = 0x01, DAILY = 0x02, SYSLOG = 0x04, SAL = 0x10 } Sinks;
    int enabledSinks;

protected:
    virtual void printUsage();
    virtual void processArg(int opt, const char* optarg);

    virtual void setSinks();

private:
    std::string _name;

    int _debugLevel;
    int _debugLevelSAL;
    bool _keep_running;

    std::vector<spdlog::sink_ptr> _sinks;

    spdlog::level::level_enum getSpdLogLogLevel();
    void startLog();
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !__cRIO_CSC_H
