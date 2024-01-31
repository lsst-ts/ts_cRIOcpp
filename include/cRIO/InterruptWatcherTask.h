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

#ifndef __cRIO_INTERRUPTWATCHERTASK__
#define __cRIO_INTERRUPTWATCHERTASK__

#include <chrono>

#include <cRIO/FPGA.h>
#include <cRIO/Task.h>
#include <cRIO/Thread.h>

namespace LSST {
namespace cRIO {

class InterruptWatcherThread : public Thread {
public:
    InterruptWatcherThread(FPGA* fpga) : _fpga(fpga) {}
    ~InterruptWatcherThread() { stop(std::chrono::seconds(2)); }

protected:
    void run(std::unique_lock<std::mutex>& lock) override;

private:
    FPGA* _fpga;
    uint32_t _triggeredInterrupts = 0;
};

class InterruptWatcherTask : public Task {
public:
    /**
     * Construct a task for handling incoming interrupts. When interrupt is
     * received, the task is queued into ControllerThread and run.
     *
     * @param triggeredIterrupts
     */
    InterruptWatcherTask(uint32_t triggeredIterrupts) : _triggeredInterrupts(triggeredIterrupts) {}

    task_return_t run() override;

private:
    uint32_t _triggeredInterrupts;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* ! __cRIO_INTERRUPTWATCHERTASK__ */
