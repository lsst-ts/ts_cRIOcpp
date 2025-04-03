/*
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the Vera C.Rubin Observatory Project
 * (https://www.lsst.org). See the COPYRIGHT file at the top-level directory of
 * this distribution for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include <chrono>
#include <queue>
#include <memory>
#include <utility>
#include <vector>

#include <cRIO/Task.h>

namespace LSST {
namespace cRIO {

typedef std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<Task>> task_t;

/**
 * Scheduling entry. Holds task to execute and time when it shall be executed.
 */
class TaskEntry : public task_t {
public:
    TaskEntry(std::chrono::steady_clock::time_point when, std::shared_ptr<Task> what) : task_t(when, what) {}
};

/**
 * Task queue management structure. Time-based priority queue, providing a
 * quick access to the next (time-wise) task to execute.
 */
class TaskQueue : public std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> {
public:
    /**
     * Remove all task copies from the queue.
     *
     * @param what Task to remove from the queue.
     *
     * @return True if at least a single instance of the task was removed.
     */
    bool remove(std::shared_ptr<Task> what);
};

}  // namespace cRIO
}  // namespace LSST

#endif /* !__TASKQUEUE_H__ */
