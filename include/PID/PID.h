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

#ifndef PID_H_
#define PID_H_

#include <atomic>

#include "PID/PIDParameters.h"

namespace LSST {
namespace PID {

/**
 * Implements PID discrete time controller. Used for filtering forces measured
 * from hardpoints for balance force component corrections to force actuators.
 *
 * See PID discussion at
 * [Confluence](https://confluence.lsstcorp.org/pages/viewpage.action?pageId=34209829)
 * for details. The [PID Implementation in
 * Software](https://confluence.lsstcorp.org/pages/viewpage.action?pageId=34209829&preview=/34209829/135102468/PID%20Implementation%20in%20Software%20v_2.pdf)
 * has details about the calculations.
 *
 * @see LSST::M1M3::SS::BalanceForceComponent
 */
class PID {
public:
    /**
     * Constructs PID.
     *
     * @param parameters PID parameters struct
     * @param publisher SAL interface
     */
    PID(PIDParameters parameters);

    /**
     * Update PID parameters.
     */
    void update_parameters(PIDParameters parameters);
    void restore_initial_parameters();
    void reset_previous_values();

    /**
     * Run PID calculations, produce output.
     */
    double process(double setpoint, double measurement);

    /**
     * Keep constant PID output. Used during slews.
     */
    void freeze();

    /**
     * Remove PID freeze flag.
     */
    void thaw() { _frozen = false; }

    double get_offset(bool *changed);

protected:
    PIDParameters _current_parameters;

    double _calculated_A;
    double _calculated_B;
    double _calculated_C;
    double _calculated_D;
    double _calculated_E;

    // process values
    double _error;
    double _error_T1;
    double _error_T2;
    double _control;
    double _control_T1;
    double _control_T2;

private:
    //* initial parameters passed in constructor
    PIDParameters _initial_parameters;

    void _calculate_intermediate_values();

    std::atomic_bool _frozen;
    double _offset;
};

} /* namespace PID */
} /* namespace LSST */

#endif /* PID_H_ */
