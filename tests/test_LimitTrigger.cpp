/*
 * This file is part of LSST cRIO CPP tests. Tests LimitTrigger.
 *
 * Developed for the Telescope & Site Software Systems.  This product includes
 * software developed by the LSST Project (https://www.lsst.org). See the
 * COPYRIGHT file at the top-level directory of this distribution for details
 * of code ownership.
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

#include <cRIO/LimitTrigger.h>

using namespace LSST::cRIO;

class MyLimitTrigger : public LimitTrigger<const char *> {
public:
    MyLimitTrigger(int minOccurence) : executedCount(0) {
        reset();
        _minOccurence = minOccurence;
    }

    void reset() override {
        _counter = 0;
        message = NULL;
    }

    int executedCount;
    const char *message;

protected:
    bool trigger() override { return ++_counter >= _minOccurence; }
    void execute(const char *msg) override {
        executedCount++;
        message = msg;
    }

private:
    int _counter;
    int _minOccurence;
};

TEST_CASE("Simple LimitTrigger", "[LimitTrigger]") {
    MyLimitTrigger limitTrigger(10);
    for (int i = 0; i < 9; i++) limitTrigger.check("pre-test");

    REQUIRE(limitTrigger.executedCount == 0);
    REQUIRE(limitTrigger.message == NULL);

    const char *m1 = "should trigger";
    limitTrigger.check(m1);
    REQUIRE(limitTrigger.executedCount == 1);
    REQUIRE(limitTrigger.message == m1);

    const char *m2 = "next trigger";
    limitTrigger.check(m2);
    REQUIRE(limitTrigger.executedCount == 2);
    REQUIRE(limitTrigger.message == m2);

    limitTrigger.reset();

    for (int i = 0; i < 9; i++) limitTrigger.check("pre-test after reset");

    REQUIRE(limitTrigger.executedCount == 2);

    const char *m3 = "triggers again!";
    limitTrigger.check(m3);
    REQUIRE(limitTrigger.executedCount == 3);
    REQUIRE(limitTrigger.message == m3);
}
