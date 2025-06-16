/*
 * This file is part of LSST cRIO package.
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

#include "PID/LimitedPID.h"

using namespace LSST::PID;

LimitedPID::LimitedPID(PIDParameters parameters, double action_min, double action_max)
        : PID(parameters), _action_min(action_min), _action_max(action_max) {}

double LimitedPID::process(double setpoint, double measurement) {
    PID::process(setpoint, measurement);

    _control = std::min(std::max(_control, _action_min), _action_max);

    return _control;
}
