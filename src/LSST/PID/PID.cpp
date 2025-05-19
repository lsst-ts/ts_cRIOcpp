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

#include "PID/PID.h"

using namespace LSST::PID;

constexpr double THAW_STEP = 50;

PID::PID(PIDParameters parameters) : _frozen(false), _offset(0) {
    _initial_parameters = parameters;

    update_parameters(parameters);
}

void PID::update_parameters(PIDParameters parameters) {
    _current_parameters = parameters;
    _calculate_intermediate_values();
}

void PID::restore_initial_parameters() { update_parameters(_initial_parameters); }

void PID::reset_previous_values() {
    _error_T2 = 0.0;
    _error_T1 = 0.0;
    _error = 0.0;
    _control_T2 = 0.0;
    _control_T1 = 0.0;
    _control = 0.0;
}

double PID::process(double setpoint, double measurement) {
    _error_T2 = _error_T1;
    _error_T1 = _error;
    _error = setpoint - measurement;
    _control_T2 = _control_T1;
    _control_T1 = _control;

    _control = _calculated_D * _control_T1 + _calculated_E * _control_T2 + _calculated_A * _error +
               _calculated_B * _error_T1 + _calculated_C * _error_T2;

    return _control + get_offset(nullptr);
}

void PID::freeze() {
    _offset = _control;
    _frozen = true;
}

double PID::get_offset(bool *changed) {
    if (abs(_offset) > 0 && _frozen == false) {
        if (abs(_offset) < (THAW_STEP + 1)) {
            _offset = 0;
        } else {
            _offset -= _offset > 0 ? THAW_STEP : -THAW_STEP;
        }
        if (changed != nullptr) {
            *changed = true;
        }
    }
    return _offset;
}

void PID::_calculate_intermediate_values() {
    double Kp = _current_parameters.P;
    double Ki = _current_parameters.I;
    double Kd = _current_parameters.D;
    double N = _current_parameters.N;
    double Ts = _current_parameters.timestep;
    _calculated_A = Kp + Kd * N;
    _calculated_B = -2.0 * Kp + Kp * N * Ts + Ki * Ts - 2.0 * Kd * N;
    _calculated_C = Kp - Kp * N * Ts - Ki * Ts + Ki * N * Ts * Ts + Kd * N;
    _calculated_D = 2.0 - N * Ts;
    _calculated_E = N * Ts - 1.0;
    reset_previous_values();
}
