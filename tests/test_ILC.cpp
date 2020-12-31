/*
 * This file is part of LSST M1M3 SS test suite. Tests ILC generic functions.
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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <memory>
#include <cmath>
#include <iostream>

#include <cRIO/ILC.h>

using namespace LSST::cRIO;

TEST_CASE("Generic functions", "[ILC]") {
    ILC ilc;

    ilc.reportServerID(125);
    ilc.reportServerStatus(31);
    ilc.resetServer(134);

    ilc.reset();

    REQUIRE(ilc.read<uint8_t>() == 125);
    REQUIRE(ilc.read<uint8_t>() == 17);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE_NOTHROW(ilc.skipRead());

    REQUIRE(ilc.read<uint8_t>() == 31);
    REQUIRE(ilc.read<uint8_t>() == 18);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE_NOTHROW(ilc.skipRead());

    REQUIRE(ilc.read<uint8_t>() == 134);
    REQUIRE(ilc.read<uint8_t>() == 107);
    REQUIRE_NOTHROW(ilc.checkCRC());
    REQUIRE_NOTHROW(ilc.readEndOfFrame());
    REQUIRE_NOTHROW(ilc.skipRead());
}
