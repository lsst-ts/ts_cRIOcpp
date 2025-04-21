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

#ifndef CONTROLLERTHREAD_H_
#define CONTROLLERTHREAD_H_

#include <atomic>
#include <chrono>
#include <memory>

#include <cRIO/FPGA.h>
#include <cRIO/InterruptHandler.h>
#include <cRIO/InterruptWatcherTask.h>
#include <cRIO/Singleton.h>
#include <cRIO/Task.h>
#include <cRIO/TaskQueue.h>
#include <cRIO/Thread.h>

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

    void startInterruptWatcherTask(FPGA* fpga);

    /* Add task into queue.
     *
     * @param task Task to enqueue.
     */
    void enqueue(std::shared_ptr<Task> task);

    /**
     * Add task into queue to execute at the given time.
     */
    void enqueue_at(std::shared_ptr<Task> task, std::chrono::steady_clock::time_point when);

    /**
     * Remove task from queue. All copies of the tasks will be removed.
     *
     * @param task task to be removed
     */
    bool remove(std::shared_ptr<Task> task);

    /**
     * Remove all tasks from the queue.
     */
    void clear();

    /**
     * Return numer of tasks in the queue.
     */
    size_t size() { return _task_queue.size(); }

    /**
     * Sets interrupt handler.
     *
     * @param handler new interrupt handler
     * @param irq irq number
     */
    void setInterruptHandler(std::shared_ptr<InterruptHandler> handler, uint8_t irq);

    static void setExitRequested() { instance()._exit_requested = true; }

    static bool exitRequested() { return instance()._exit_requested; }

    void checkInterrupts(uint32_t triggeredIterrupts);

protected:
    void run(std::unique_lock<std::mutex>& lock) override;

private:
    void _process_tasks();

    TaskQueue _task_queue;

    std::atomic<bool> _exit_requested = false;

    static constexpr uint8_t CRIO_INTERRUPTS = 32;

    /**
     * Tasks run at specific interrupt.
     */
    std::shared_ptr<InterruptHandler> _interrupt_handlers[CRIO_INTERRUPTS] = {
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    InterruptWatcherThread* _interrupt_watcher_thread = nullptr;

    friend class InterruptWatcherTask;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* CONTROLLERTHREAD_H_ */
