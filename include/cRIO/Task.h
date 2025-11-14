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

#ifndef __CRIO_TASK__
#define __CRIO_TASK__

#include <chrono>
#include <exception>

namespace LSST {
namespace cRIO {

typedef uint32_t task_return_t;

/**
 * Parent class for all tasks queued to operate on FPGA.
 */
class Task {
public:
    Task() {}
    virtual ~Task() {}

    /**
     * Validates the task. Run by managing queue before adding the task to the queue.
     *
     * @return true if command is valid and can be executed
     */
    virtual bool validate() { return true; }

    virtual task_return_t run() = 0;

    /**
     * Report exception raised during task processing.
     *
     * @param ex exception raised during processing
     */
    virtual void reportException(const std::exception &ex) {};

    static constexpr task_return_t DONT_RESCHEDULE = 0xFFFFFFFF;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* !__CRIO_TASK__ */
