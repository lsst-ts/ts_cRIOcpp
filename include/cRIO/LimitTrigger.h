/*
 * Abstract class for limitting an execution.
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

#ifndef LIMITTRIGGER_H_
#define LIMITTRIGGER_H_

namespace LSST {
namespace cRIO {

/**
 * Abstract interface class for limiting some action to predefined execution.
 * This class shall be used when a large number of messages (violation of some
 * criteria) can be send. The class can limit number of occurrences it will
 * actually run the execute code.
 *
 * @tparam TcheckArgs arguments passed to check and execute methods
 *
 * Example:
 *
 * @code{.cpp}
 *
 * class MyLimitTrigger: public LimitTrigger<const char*> {
 * public:
 *     MyLimitTrigger(int minOccurence) {
 *         reset();
 *         _minOccurence = minOccurence;
 *     }
 *
 *     bool trigger() override { return ++_counter >= _minOccurence; }
 *     void execute(const char *msg) override { std::cout << "Executed with message " << msg << std::endl; }
 *     void reset() override { _counter = 0; }
 *
 * private:
 *     int _counter;
 *     int _minOccurence;
 * };
 *
 * ....
 *     MyLimitTrigger limitTrigger(10);
 *
 *     ....
 *     limitTrigger.check();
 *     ....
 *
 * @endcode
 */
template <typename... TcheckArgs>
class LimitTrigger {
public:
    LimitTrigger() {}

    /**
     * Should be called from code when out of bounds conditions for triggering is detected.
     *
     * @param args arguments that will be passed to execute method if
     * triggering conditions are met.
     */
    void check(TcheckArgs... args) {
        if (trigger()) {
            execute(args...);
        }
    }

    /**
     * Pure virtual placeholder to reset any internal variables. Shall be
     * called after conditions are back inside bounds and no future triggering
     * is expected in the current system state.
     */
    virtual void reset() = 0;

protected:
    /**
     * Pure virtual placeholder for check if triggering criteria are met.
     *
     * @return true if execute method shall be executed
     */
    virtual bool trigger() = 0;

    /**
     * Pure virtual placeholder for executing limited action. Child subclasses
     * shall override this method and put the action (sending log message, ..)
     * which should be executed.
     *
     * @param args arguments passed to check
     * @see check
     */
    virtual void execute(TcheckArgs... args) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif  // !LIMITTRIGGER_H_
