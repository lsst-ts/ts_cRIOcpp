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
#include <iostream>
#include <memory>

#include "PID/PID.h"

using namespace LSST::PID;

TEST_CASE("Constant_PID", "[PID]") {
    PID *pids[6];
    PIDParameters pparams;
    pparams.timestep = 1;
    pparams.P = 1;
    pparams.I = 0;
    pparams.D = 0;
    pparams.N = 1;

    for (int i = 0; i < 6; i++) {
        pids[i] = new PID(pparams);
    }

    double te = 0;

    for (int i = 0; i < 6; i++) {
        for (int n = 0; n < 1000; n++) {
            te += pids[i]->process(n, n);
        }
    }

    CHECK(te == 0);

    for (int i = 0; i < 6; i++) {
        delete pids[i];
    }
}

TEST_CASE("PID_convergence", "[PID]") {
    PID *pids[6];
    PIDParameters pparams;
    pparams.timestep = 0.1;
    pparams.P = 0.5;
    pparams.I = 0.4;
    pparams.D = 0.1;
    pparams.N = 0.2;

    for (int i = 0; i < 6; i++) {
        pids[i] = new PID(pparams);
    }

    for (int i = 0; i < 6; i++) {
        double u0 = pids[i]->process(1000, 0);
        CHECK(u0 == 520);
        for (int n = 0; n < 1000; n += 100) {
            CHECK(fabs(pids[i]->process(1000, n)) < u0 * 1.2);
        }

        double m = 1000;

        int n = 1000000;
        bool e_low = false;
        double u;

        for (; n > 0; n -= 1) {
            u = pids[i]->process(1000, m);

            m = 1000 + (n / 10000) * sin((180 * M_PI) / n);

            if (fabs(u) < 1 && e_low == false) {
                CHECK(n < 890000);
                e_low = true;
            }
        }

        CHECK(u < 1);
    }

    for (int i = 0; i < 6; i++) {
        delete pids[i];
    }
}
