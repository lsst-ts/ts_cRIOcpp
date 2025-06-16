/*
 * This file is part of LSST M1M3 SS test suite. Tests software PID.
 *
 * Developed for the Vera C. Rubin Telescope and Site System.
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

#include <catch2/catch_all.hpp>

#include <cmath>

#include "PID/LimitedPID.h"

using namespace LSST::PID;

TEST_CASE("Heater_PID", "[PID]") {
    PIDParameters pparams(5.0, 0.6, 1.0, 0.2, 0.2);

    LimitedPID pid(pparams, 0, 100);

    double action;

    for (int n = 0; n < 100; n++) {
        action = pid.process(10.0, 2 * sin(M_PI * (n / 50.0)));
        CHECK(action >= 0);
        CHECK(action <= 100);
    }

    for (int n = 0; n < 10000; n++) {
        action = pid.process(10.0, 10.5);
        if (n > 50) {
            CHECK(action == 0);
        } else {
            CHECK(action >= 0);
            CHECK(action <= 100);
        }
    }

    for (int n = 0; n < 100; n++) {
        action = pid.process(10.0, 9.5);

        if (n > 10) {
            CHECK(action > 0);
        }
    }
}
