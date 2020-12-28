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

#include <cRIO/ControllerThread.h>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

ControllerThread::ControllerThread() : _keepRunning(true) {
    SPDLOG_DEBUG("ControllerThread: ControllerThread()");
}

ControllerThread::~ControllerThread() {
    _thread = NULL;
    _clear();
}

ControllerThread& ControllerThread::get() {
    static ControllerThread controllerThread;
    return controllerThread;
}

void ControllerThread::run() {
    if (_thread) {
        throw std::runtime_error("ControllerThread: Cannot run a thread twice!");
    }
    _thread = new std::thread(&ControllerThread::_run, this);
    _cv.notify_one();
}

void ControllerThread::stop() {
    {
        std::lock_guard<std::mutex> lg(_mutex);
        _keepRunning = false;
    }
    _cv.notify_one();
    _thread->join();
    delete _thread;
    _thread = NULL;
}

void ControllerThread::_run() {
    SPDLOG_INFO("ControllerThread: Start");
    std::unique_lock<std::mutex> lock(_mutex);
    // runs commands already queued
    _runCommands();
    while (_keepRunning) {
        _cv.wait(lock);
        _runCommands();
    }
    SPDLOG_INFO("ControllerThread: Completed");
}

// _mutex must be locked by calling method to guard _queue access!!
void ControllerThread::_runCommands() {
    while (!_queue.empty()) {
        Command* command = _queue.front();
        _queue.pop();
        _execute(command);
    }
}

void ControllerThread::_clear() {
    SPDLOG_TRACE("ControllerThread: _clear()");
    {
        std::lock_guard<std::mutex> lg(_mutex);
        while (!_queue.empty()) {
            Command* command = _queue.front();
            delete command;
        }
        std::queue<Command*>().swap(_queue);
    }
}

void ControllerThread::enqueue(Command* command) {
    SPDLOG_TRACE("ControllerThread: enqueue()");
    {
        std::lock_guard<std::mutex> lg(_mutex);
        _queue.push(command);
    }
    _cv.notify_one();
}

void ControllerThread::_execute(Command* command) {
    SPDLOG_TRACE("ControllerThread: _execute()");
    try {
        command->ackInProgress();
        command->execute();
        command->ackComplete();
    } catch (std::exception& e) {
        SPDLOG_ERROR("Cannot execute command: {}", e.what());
        command->ackFailed(e.what());
    }

    delete command;
}

}  // namespace cRIO
}  // namespace LSST
