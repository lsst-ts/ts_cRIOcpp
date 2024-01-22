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

#include <stdexcept>

#include <cRIO/Thread.h>

using namespace std::chrono_literals;

using namespace LSST::cRIO;

Thread::~Thread() { stop(); }

void Thread::start(std::chrono::microseconds timeout) {
    {
        std::unique_lock<std::mutex> lg(runMutex);
        if (_thread) {
            throw std::runtime_error("Thread: Cannot run a thread twice!");
        }
        keepRunning = true;
        _thread = new std::thread(&Thread::_run, this);
        if (_startCondition.wait_for(lg, timeout, [this]() { return _threadStarted == true; }) == false) {
            throw std::runtime_error("Thread: Was not started!");
        }
    }
}

void Thread::stop(std::chrono::microseconds timeout) {
    {
        std::lock_guard<std::mutex> lg(runMutex);
        keepRunning = false;
    }
    runCondition.notify_one();
    {
        std::unique_lock<std::mutex> lg(runMutex);
        if (_thread) {
            if (_startCondition.wait_for(lg, timeout, [this]() { return _threadStarted == false; }) ==
                false) {
                throw std::runtime_error("Thread: Cannot stop thread!");
            }
            _thread->join();
            delete _thread;
            _thread = NULL;
        }
    }
}

bool Thread::joinable() {
    std::lock_guard<std::mutex> lg(runMutex);
    if (_thread == nullptr) return false;
    return _thread->joinable();
}

bool Thread::isRunning() {
    std::lock_guard<std::mutex> lg(runMutex);
    return _threadStarted;
}

void Thread::_run() {
    {
        std::unique_lock<std::mutex> lock(runMutex);
        _threadStarted = true;
        _startCondition.notify_one();
        run(lock);
        _threadStarted = false;
        _startCondition.notify_one();
    }
}
