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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>

namespace LSST {
namespace cRIO {

/**
 * Parent class for all commands.
 *
 * Follows Command Pattern from Design Patterns. Encapsulates command executed
 * in ContollerThread. Pure virtual methed ::execute() shall be overriden in
 * child classes, implementing specific commands.
 *
 * @see ContollerThread
 */
class Command {
protected:
    int32_t commandID;

public:
    virtual ~Command();

    /**
     * Gets the command ID.
     */
    virtual int32_t getCommandID() { return commandID; }

    /**
     * Validates the command.
     *
     * @return true if command is valid and can be executed
     */
    virtual bool validate();

    /**
     * Executes the command.
     */
    virtual void execute() = 0;

    /**
     * Acknowledges the command is in progress.
     */
    virtual void ackInProgress();

    /**
     * Acknowledges the command has completed successfully.
     */
    virtual void ackComplete();

    /**
     * Acknowledges the command has failed.
     * @param[in] reason The reason why the command has failed.
     */
    virtual void ackFailed(std::string reason);
};

}  // namespace cRIO
}  // namespace LSST

#endif /* COMMAND_H_ */
