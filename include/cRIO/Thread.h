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

#ifndef __cRIO_THREAD_H__
#define __cRIO_THREAD_H__

#include <condition_variable>
#include <mutex>
#include <thread>

namespace LSST {
namespace cRIO {

/**
 * Abstract thread class. Intended to provide a while loop to run various threads. Provides
 */
class Thread {
public:
    Thread();
    virtual ~Thread() { stop(); }

    /**
     * Starts the thread. Starts new thread running the loop.
     */
    void start();

    /**
     * Stops thread run.
     */
    void stop();

    /**
     * Join running thread. Waits for thread exits.
     */
    void join();

    /**
     * Returns true if thread is joinable (~is running).
     *
     * @return true if thread is started and running. False otherwise (thread
     * doesn't exists, not started, joined).
     */
    bool joinable() {
        if (_thread == nullptr) return false;
        return _thread->joinable();
    }

protected:
    /**
     * Mutex protecting keepRunning access, can be used in condition variable.
     */
    std::mutex runMutex;
    bool keepRunning;

    /**
     * Condition variable for outside notifications. Notified when keepRunning
     * changed to false. Can be used in subclasses.
     */
    std::condition_variable runCondition;

    /**
     * Pure virtual method. Must be overloaded in children classes. Shall run
     * some loop as long as keepRunning is true. Shall call
     * runCondition.wait[|_for|_until] to wait for outside notifications.
     */
    virtual void run() = 0;

private:
    std::thread *_thread;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* __cRIO_THREAD_H__ */
