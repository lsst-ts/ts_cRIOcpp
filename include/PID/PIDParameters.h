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

#ifndef PIDPARAMETERS_H_
#define PIDPARAMETERS_H_

#include <math.h>

#include <yaml-cpp/yaml.h>

namespace LSST {
namespace PID {

/**
 * Parameters for PID calculations. Used in PID.
 */
struct PIDParameters {
    PIDParameters() { timestep = P = I = D = N = NAN; }

    PIDParameters(const PIDParameters &pid) {
        timestep = pid.timestep;
        P = pid.P;
        I = pid.I;
        D = pid.D;
        N = pid.N;
    }

    PIDParameters(double _timestep, double _p, double _i, double _d, double _n) {
        timestep = _timestep;
        P = _p;
        I = _i;
        D = _d;
        N = _n;
    }

    /**
     * Load parameters from YAML node.
     *
     * @param node YAML node with PID, N and Timestep keys
     *
     * @raise YAML::Exception on error
     */
    void load(const YAML::Node &node) {
        timestep = node["Timestep"].as<double>(1);
        P = node["P"].as<double>();
        I = node["I"].as<double>();
        D = node["D"].as<double>();
        N = node["N"].as<double>();
    }

    //* Length of step (seconds)
    double timestep;
    //* Proportional gain (unitless)
    double P;
    //* Integral gain (unitless)
    double I;
    //* Derivative gain (unitless)
    double D;
    /**
     * The N term. See (PID implementation)
     * [https://confluence.lsstcorp.org/pages/viewpage.action?pageId=34209829&preview=/34209829/135102468/PID%20Implementation%20in%20Software%20v_2.pdf]
     * for details. Setting this to 0 will cancle Kd (derivative term).
     */
    double N;
};

}  // namespace PID
}  // namespace LSST

#endif /* PIDPARAMETERS_H_ */
