/*
 * This file is part of LSST cRIOcpp test suite. Tests ILC generic functions.
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

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <cRIO/Settings/Alias.h>

using namespace LSST::cRIO::Settings;

TEST_CASE("Test Alias", "[Alias]") {
    Alias alias;
    REQUIRE_NOTHROW(alias.load("data/AliasGood.yaml"));

    auto a = alias.getAlias("Default");

    REQUIRE(a.first == "Default");
    REQUIRE(a.second == "1.2");

    REQUIRE_NOTHROW(a = alias.getAlias("Experimental"));

    REQUIRE(a.first == "Exp");
    REQUIRE(a.second == "2.13");

    REQUIRE_THROWS(alias.getAlias("Test"));
}

TEST_CASE("Throws on bad file", "[Alias]") {
    Alias alias;
    REQUIRE_THROWS(alias.load("data/test.yaml"));
}
