/*
 * This file is part of LSST cRIOcpp test suite. Tests SimulationBuffer functions.
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

#include <memory>
#include <cmath>

#include <catch2/catch_test_macros.hpp>

#include <cRIO/SimulatedILC.h>

using namespace LSST::cRIO;

TEST_CASE("Simulate response", "[SimulationBuffer]") {
    SimulatedILC ilc;

    ilc.write<float>(0.123);
    ilc.write(-6758.1234f);
    ilc.writeCRC();

    uint16_t* buf = ilc.getBuffer();

    REQUIRE(buf[0] == 0x927a);
    REQUIRE(buf[1] == 0x93f6);
    REQUIRE(buf[2] == 0x93ce);
    REQUIRE(buf[3] == 0x92da);

    REQUIRE(buf[4] == 0x938a);
    REQUIRE(buf[5] == 0x93a6);
    REQUIRE(buf[6] == 0x9260);
    REQUIRE(buf[7] == 0x93fa);

    ilc.reset();

    REQUIRE(ilc.read<float>() == 0.123f);
    REQUIRE(ilc.read<float>() == -6758.1234f);
    REQUIRE_NOTHROW(ilc.checkCRC());
}
