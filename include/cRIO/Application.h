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

#ifndef __cRIO_Application_H
#define __cRIO_Application_H

#include <CliApp.h>
#include <spdlog/spdlog.h>

namespace LSST {
namespace cRIO {

class Application : public CliApp {
public:
    Application(const char* _description);

    void run();

protected:
    virtual void printUsage();
    virtual void processArg(int opt, const char* optarg);

    virtual void setSinks();

    int enabledSinks;

private:
    int debugLevel;
    int debugLevelSAL;

    std::vector<spdlog::sink_ptr> sinks;

    spdlog::level::level_enum getSpdLogLogLevel();
    void startLog();
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !__cRIO_Application_H
