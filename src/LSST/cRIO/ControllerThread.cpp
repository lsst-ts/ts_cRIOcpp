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
#include <stdexcept>
#include <thread>

#include <spdlog/spdlog.h>

#include <cRIO/ControllerThread.h>

using namespace std::chrono_literals;

using namespace LSST::cRIO;

ControllerThread::ControllerThread(token) { SPDLOG_DEBUG("ControllerThread: ControllerThread()"); }

ControllerThread::~ControllerThread() {
    delete _interruptWatcherThread;
    _clear();
}

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

void ControllerThread::startInterruptWatcherTask(FPGA* fpga) {
    _interruptWatcherThread = new InterruptWatcherThread(fpga);
    _interruptWatcherThread->start();
}

void ControllerThread::setInterruptHandler(std::shared_ptr<InterruptHandler> handler, uint8_t irq) {
    if ((irq == 0) || (irq > CRIO_INTERRUPTS)) {
        throw std::runtime_error(fmt::format("Interrupt number should fall between 1 and {} - {} specified",
                                             CRIO_INTERRUPTS, irq));
    }
    if (_interruptHandlers[irq - 1] != nullptr) {
        throw std::runtime_error(
                fmt::format("Cannot set handler for interrupt {}, as the handler was already set", irq));
    }
    _interruptHandlers[irq - 1] = handler;
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

void ControllerThread::checkInterrupts(uint32_t triggeredIterrupts) {
    for (uint8_t i = 0; i < CRIO_INTERRUPTS; i++, triggeredIterrupts >>= 1) {
        if ((triggeredIterrupts & 0x01) == 0x01) {
            if (_interruptHandlers[i] == nullptr) {
                SPDLOG_WARN("FPGA signaled non-handled interrupt {}.", i + 1);
                continue;
            }
            _interruptHandlers[i]->handleInterrupt(i + 1);
        }
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
                _taskQueue.push(TaskEntry(std::chrono::steady_clock::now() + std::chrono::milliseconds(wait),
                                          task.second));
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
