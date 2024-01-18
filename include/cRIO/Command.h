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

#ifndef __CRIO_COMMAND_H_
#define __CRIO_COMMAND_H_

#include <string>

#include <cRIO/Task.h>

namespace LSST {
namespace cRIO {

/**
 * Parent class for all commands.
 *
 * Follows Command Pattern from Design Patterns. Encapsulates command executed
 * in ControllerThread. Pure virtual methed ::execute() shall be overriden in
 * child classes, implementing specific commands.
 *
 * @see ControllerThread
 */
class Command : public Task {
public:
    virtual ~Command();

    std::chrono::milliseconds run() override;

    /**
     * Executes the command.
     */
    virtual void execute() = 0;

    void reportException(const std::exception &ex) override { ackFailed(ex.what()); }

    /**
     * Acknowledges the command is in progress.
     */
    virtual void ackInProgress() = 0;

    /**
     * Acknowledges the command has completed successfully.
     */
    virtual void ackComplete() = 0;

    /**
     * Acknowledges the command has failed.
     * @param[in] reason The reason why the command has failed.
     */
    virtual void ackFailed(std::string reason) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* __CRIO_COMMAND_H_ */
