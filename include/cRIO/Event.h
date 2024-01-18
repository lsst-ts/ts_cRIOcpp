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

#ifndef EVENT_H_
#define EVENT_H_

#include <cRIO/Task.h>

namespace LSST {
namespace cRIO {

/**
 * Parent class for all events.
 *
 * Follows Command Pattern from Design Patterns. Encapsulates command executed
 * in ControllerThread. Pure virtual methed ::received() shall be overriden in
 * child classes, implementing reaction to specified events.
 *
 * @see ControllerThread
 */
class Event : public Task {
public:
    virtual ~Event();

    std::chrono::milliseconds run() override {
        recieved();
        return Task::DONT_RESCHEDULE;
    }

    virtual void recieved() = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* EVENT_H_ */
