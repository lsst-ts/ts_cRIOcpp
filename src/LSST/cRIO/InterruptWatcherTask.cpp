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

#include <memory>

#include <cRIO/ControllerThread.h>
#include <cRIO/InterruptWatcherTask.h>

using namespace LSST::cRIO;

void InterruptWatcherThread::run(std::unique_lock<std::mutex>& lock) {
    while (keepRunning) {
        lock.unlock();
        bool timedout = false;
        _fpga->waitOnIrqs(0xFFFFFFFF, 20, timedout, &_triggeredInterrupts);
        if (timedout == false) {
            ControllerThread::instance().enqueue(
                    std::make_shared<InterruptWatcherTask>(_triggeredInterrupts));
        }
        lock.lock();
    }
}

task_return_t InterruptWatcherTask::run() {
    ControllerThread::instance().checkInterrupts(_triggeredInterrupts);
    return Task::DONT_RESCHEDULE;
}
