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

#include <cRIO/Thread.h>

namespace LSST {
namespace cRIO {

Thread::Thread() : keepRunning(false) { _thread = NULL; }

void Thread::start() {
    {
        std::lock_guard<std::mutex> lg(runMutex);
        if (_thread) {
            throw std::runtime_error("Thread: Cannot run a thread twice!");
        }
        keepRunning = true;
        _thread = new std::thread(&Thread::run, this);
    }
}

void Thread::stop() {
    {
        std::lock_guard<std::mutex> lg(runMutex);
        keepRunning = false;
    }
    if (_thread) {
        runCondition.notify_one();
        _thread->join();
        delete _thread;
        _thread = NULL;
    }
}

}  // namespace cRIO
}  // namespace LSST
