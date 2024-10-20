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

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace LSST {
namespace cRIO {

/**
 * Abstract thread class. Intended to provide a while loop to run various
 * threads. Provides pure virtual run method with lock, that should be override
 * in descendants.
 */
class Thread {
public:
    virtual ~Thread();

    /**
     * Starts the thread. Starts new thread running the loop.
     *
     * @param timeout start timeout. Defaults to 1ms.
     *
     * @note If the call blocks indefinitely, most likely cause is the
     * overridden run method not releasing locks (either directly, or
     * indirectly through conditional variable wait).
     *
     * @throw runtime_error when thread was already started
     */
    void start(std::chrono::microseconds timeout = 5ms);

    /**
     * Stops and join thread.
     *
     * @param timeout wait for this time to make sure thread is stopped
     *
     * @throw runtime_error when timeout is crossed
     */
    void stop(std::chrono::microseconds timeout = 2ms);

    /**
     * Returns true if thread is joinable (~is running).
     *
     * @return true if thread is started and running. False otherwise (thread
     * doesn't exists, not started, joined).
     */
    bool joinable();

    /**
     * Returns whenever thread is running.
     *
     * @return true if the thread is running.
     */
    bool isRunning();

    /**
     * Waits until given time and while thread is running (wasn't stopped).
     *
     * @param abs_time time to wait
     *
     * @return false if thread was requested to stop while waiting, otherwise true.
     */
    bool wait_until(const std::chrono::time_point<std::chrono::steady_clock>& abs_time);

protected:
    /**
     * Mutex protecting keepRunning access, can be used in condition variable.
     */
    std::mutex runMutex;
    std::atomic<bool> keepRunning = false;

    /**
     * Condition variable for outside notifications. Notified when keepRunning
     * changed to false. Can be used in subclasses.
     */
    std::condition_variable runCondition;

    /**
     * Pure virtual method. Must be overloaded in children classes. Shall run
     * some loop as long as keepRunning is true. Shall call
     * runCondition.wait[|_for|_until] to wait for outside notifications.
     *
     * @note The code is expected to call runCondition.wait at least once, or
     * to unlock the lock for some time.
     *
     * @param lock locked unique lock. Should be used as parameter to
     * runCondition.wait call.
     */
    virtual void run(std::unique_lock<std::mutex>& lock) = 0;

private:
    std::thread* _thread = nullptr;

    /*
     * Condition for start detection. Notified on call to _run. Without this,
     * SIGABRT will be raised when deleting _thread while initializing (e.g.
     * delete Thread object right after call to start()).
     */
    std::condition_variable _startCondition;
    bool _threadStarted = false;

    void _run();
};

}  // namespace cRIO
}  // namespace LSST

#endif /* __cRIO_THREAD_H__ */
