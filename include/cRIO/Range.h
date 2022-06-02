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

#ifndef RANGE_H_
#define RANGE_H_

#include <spdlog/spdlog.h>

#include <cRIO/LimitTrigger.h>

namespace LSST {
namespace cRIO {

/**
 * Utility range checking functions.
 */
class Range {
public:
    /**
     * Template function to check if a value is inside a given range.
     *
     * @tparam T value type. All parameters shall have the same type.
     * @param min range minimal value
     * @param max range maximal value
     * @param value value to test
     *
     * @return true if value >= min and value <= max
     */
    template <typename T>
    static bool InRange(T min, T max, T value) {
        return value >= min && value <= max;
    }

    template <typename T>
    static T CoerceIntoRange(T min, T max, T value) {
        return (value < min) ? min : (value > max) ? max : value;
    }

    template <typename T>
    static bool InRangeAndCoerce(T min, T max, T value, T* output) {
        if (Range::InRange(min, max, value)) {
            (*output) = value;
            return true;
        } else {
            (*output) = Range::CoerceIntoRange(min, max, value);
            return false;
        }
    }

    /**
     * Tests if value is in given range. Fires LimitTrigger if conditions
     * aren't met. Reset LimitTrigger if conditions returned back to normal.
     *
     * @tparam T test value type
     * @tparam TArgs variadic arguments for LimitTrigger
     * @param min range minimal value
     * @param max range maximal value
     * @param value test value
     * @param limitTrigger LimitTrigger subclass to report problems with range
     * @param lArgs LimitTrigger arguments
     *
     * @return true if value >= min and value <= max
     */
    template <typename T, typename... TArgs>
    static bool InRangeTrigger(T min, T max, T value, LimitTrigger<TArgs...>& limitTrigger, TArgs... lArgs) {
        bool inRange = InRange(min, max, value);
        if (inRange == false) {
            limitTrigger.check(lArgs...);
        } else {
            limitTrigger.reset();
        }
        return inRange;
    }
};

}  // namespace cRIO
}  // namespace LSST
#endif /* RANGE_H_ */
