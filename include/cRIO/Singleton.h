/*
 * Singleton template.
 *
 * Developed for the Rubin Observatory Telescope and Site Software.
 * This product includes software developed by the LSST Project
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

#ifndef __Singleton__
#define __Singleton__

#include <memory>

namespace LSST {
namespace cRIO {

/**
 * Singleton template.
 *
 * Use as:
 *
 * @code
 *
 * class MySingleton final : public Singleton<MySingleton> {
 * public:
 *     MySingleton(token) : _int(4) {}
 *
 * private:
 *     int _int;
 * };
 *
 * @endcode
 */
template <typename T>
class Singleton {
public:
    static T& instance();

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton) = delete;

protected:
    struct token {};
    Singleton() {}
};

template <typename T>
T& Singleton<T>::instance() {
    static T instance{token{}};
    return instance;
}

}  // namespace cRIO
}  // namespace LSST

#endif  // ! __Singleton__
