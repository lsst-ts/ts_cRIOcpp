/*
 * NI Error class. Shall be thrown on any Ni-related errors.
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

#include "cRIO/NiError.h"
#include "cRIO/NiStatus.h"

namespace LSST {
namespace cRIO {

NiError::NiError(const char *msg, NiFpga_Status status) : std::runtime_error(NiStatus(status)) {
    NiReportError(msg, status);
}

void NiThrowError(const char *msg, int32_t status) {
    if (status != 0) {
        throw NiError(msg, status);
    }
}

void NiThrowError(const char *func, const char *ni_func, int32_t status) {
    NiThrowError(std::string(func) + " " + ni_func, status);
}

}  // namespace cRIO
}  // namespace LSST
