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

#ifndef _formatDuration_H_
#define _formatDuration_H_

#include <chrono>
#include <sstream>
#include <string>

using namespace std::chrono;

/***
 * Template function to format chrono::duration to human readable string.
 *
 * @parametr timeunit
 *
 * @return human-readable string (such as 1m:41s:341.179.283ns)
 */
template <typename T>
inline std::string formatDuration(T timeunit) {
    nanoseconds ns = duration_cast<nanoseconds>(timeunit);
    std::ostringstream os;
    bool foundNonZero = false;
    os.fill('0');
    typedef duration<int, std::ratio<86400 * 365>> years;
    const auto y = duration_cast<years>(ns);
    if (y.count()) {
        foundNonZero = true;
        os << y.count() << "y:";
        ns -= y;
    }
    typedef duration<int, std::ratio<86400>> days;
    const auto d = duration_cast<days>(ns);
    if (d.count()) {
        foundNonZero = true;
        os << d.count() << "d:";
        ns -= d;
    }
    const auto h = duration_cast<hours>(ns);
    if (h.count() || foundNonZero) {
        foundNonZero = true;
        os << h.count() << "h:";
        ns -= h;
    }
    const auto m = duration_cast<minutes>(ns);
    if (m.count() || foundNonZero) {
        foundNonZero = true;
        os << m.count() << "m:";
        ns -= m;
    }
    const auto s = duration_cast<seconds>(ns);
    if (s.count() || foundNonZero) {
        foundNonZero = true;
        os << s.count() << "s:";
        ns -= s;
    }
    const auto ms = duration_cast<milliseconds>(ns);
    if (ms.count() || foundNonZero) {
        if (foundNonZero) {
            os << std::setw(3);
        }
        os << ms.count() << ".";
        ns -= ms;
        foundNonZero = true;
    }
    const auto us = duration_cast<microseconds>(ns);
    if (us.count() || foundNonZero) {
        if (foundNonZero) {
            os << std::setw(3);
        }
        os << us.count() << ".";
        ns -= us;
    }
    os << std::setw(3) << ns.count() << "ns";
    return os.str();
}

#endif  ///!_formatDuration_H_
