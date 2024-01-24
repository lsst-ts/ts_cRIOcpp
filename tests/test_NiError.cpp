/*
 * This file is part of LSST M1M3 support system package.
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

#include <catch2/catch_test_macros.hpp>
#include <list>
#include <string>

#include <cRIO/NiError.h>

using namespace LSST::cRIO;

void callError() {
    try {
        NiThrowError(-61060, "In a Galaxy Far, {}", "Far Away");
    } catch (NiError &ni) {
        REQUIRE(ni.what() == std::string("In a Galaxy Far, Far Away: NiFpga_Status_IrqTimeout: The timeout "
                                         "expired before any of the IRQs were asserted"));
    }
}

void callWarning() {
    try {
        NiThrowError(63195, "What do you get if you multiply {} by {}? {:+.02f}", "six", "nine", 42.);
    } catch (NiWarning &ni) {
        REQUIRE(ni.what() ==
                std::string("What do you get if you multiply six by nine? +42.00: "
                            "NiFpga_Status_InvalidSession: The session is invalid or has been closed"));
    }
}

TEST_CASE("Test NiError", "[NiError]") {
    REQUIRE_THROWS_AS(NiThrowError(-61060, "In a Galaxy Far, {}", "Far Away"), NiError);
    REQUIRE_THROWS_AS(NiThrowError(61060, "In a Galaxy Far, {}", "Far Away"), NiWarning);

    REQUIRE_NOTHROW(callError());

    REQUIRE_NOTHROW(callWarning());
}
