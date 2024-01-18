/*
 * This file is part of LSST M1M3 support system package.
 *
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

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include <cRIO/ControllerThread.h>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

ControllerThread::ControllerThread(token) : _exitRequested(false) {
    SPDLOG_DEBUG("ControllerThread: ControllerThread()");
}

ControllerThread::~ControllerThread() { _clear(); }

void ControllerThread::enqueue(std::shared_ptr<Task> task) {
    enqueue_at(task, std::chrono::steady_clock::now() + 1ms);
}

void ControllerThread::enqueue_at(std::shared_ptr<Task> task,
                                  std::chrono::time_point<std::chrono::steady_clock> when) {
    SPDLOG_TRACE("ControllerThread: enqueue at {}",
                 std::chrono::duration_cast<std::chrono::seconds>(when.time_since_epoch()).count());
    {
        std::lock_guard<std::mutex> lg(runMutex);
        try {
            if (task->validate()) {
                _taskQueue.push(TaskEntry(when, task));
            }
        } catch (std::exception& ex) {
            task->reportException(ex);
        }
    }
    runCondition.notify_one();
}

void ControllerThread::run(std::unique_lock<std::mutex>& lock) {
    SPDLOG_INFO("ControllerThread: Run");
    // process already queued tasks
    _processTasks();
    while (keepRunning) {
        if (_taskQueue.empty()) {
            runCondition.wait(lock);
        } else {
            runCondition.wait_until(lock, _taskQueue.top().first);
        }
        _processTasks();
    }
    SPDLOG_INFO("ControllerThread: Completed");
}

// runMutex must be locked by calling method to guard _taskQueue access!!
void ControllerThread::_processTasks() {
    if (_taskQueue.empty()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto task = _taskQueue.top();
    while (task.first <= now) {
        _taskQueue.pop();
        try {
            auto wait = task.second->run();
            if (wait != Task::DONT_RESCHEDULE) {
                _taskQueue.push(TaskEntry(std::chrono::steady_clock::now() + wait, task.second));
            }
        } catch (std::exception& ex) {
            task.second->reportException(ex);
        }
        if (_taskQueue.empty()) {
            return;
        }
        task = _taskQueue.top();
    }
}

void ControllerThread::_clear() {
    SPDLOG_TRACE("ControllerThread: _clear()");
    {
        std::lock_guard<std::mutex> lg(runMutex);
        TaskQueue empty;
        std::swap(_taskQueue, empty);
    }
}

}  // namespace cRIO
}  // namespace LSST
