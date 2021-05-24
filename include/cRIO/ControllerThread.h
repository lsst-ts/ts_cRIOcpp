/*
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

#ifndef CONTROLLERTHREAD_H_
#define CONTROLLERTHREAD_H_

#include <cRIO/Command.h>
#include <cRIO/Singleton.h>
#include <cRIO/Thread.h>

#include <queue>

namespace LSST {
namespace cRIO {

/**
 * The controller thread is responsible for executing commands.
 *
 * Holds command queue. Command is enqueued in SubscriberThread after being
 * received from SAL. Command is then dequeued in ControllerThread and passed
 * to the Controller::_execute method. Singleton, as only a single instance
 * should occur in an application. Runs in a single thread - provides guarantee
 * that only a single command is being executed at any moment.
 */
class ControllerThread final : public Thread, public Singleton<ControllerThread> {
public:
    ControllerThread(token);
    ~ControllerThread();

    /* Put command into queue.
     *
     * @param command Command to enqueue. ControllerThread takes ownership of
     * the passed Command and will dispose it (delete it) after command is
     * executed or when queue is cleared.
     */
    void enqueue(Command* command);

protected:
    void run() override;

private:
    void _runCommands();
    void _clear();
    void _execute(Command* command);

    std::queue<Command*> _queue;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CONTROLLERTHREAD_H_ */
