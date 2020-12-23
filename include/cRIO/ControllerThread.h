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

#ifndef CONTROLLERTHREAD_H_
#define CONTROLLERTHREAD_H_

#include <cRIO/Command.h>

#include <condition_variable>
#include <mutex>
#include <queue>

namespace LSST {
namespace cRIO {

/**
 * @brief The controller thread is responsible for executing commands.
 *
 * Holds command queue. Command is enqueued in SubscriberThread after being
 * received from SAL. Command is then dequeued in ControllerThread and passed
 * to the Controller::execute method. Singleton, as only a single instance
 * should occur in an application. Runs in a single thread - provides guarantee
 * that only single command is being executed in any moment.
 */
class ControllerThread {
public:
    ControllerThread();
    ~ControllerThread();

    /**
     * @brief Return singleton instance.
     *
     * @return singleton instance
     */
    static ControllerThread& get();

    void run();
    void stop();

    /**
     * @brief Put command into queue.
     */
    void enqueue(Command* command);

private:
    ControllerThread& operator=(const ControllerThread&) = delete;
    ControllerThread(const ControllerThread&) = delete;

    void _clear();
    void _execute(Command* command);

    bool _keepRunning;
    std::mutex _mutex;
    std::queue<Command*> _queue;
    std::condition_variable _cv;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CONTROLLERTHREAD_H_ */
