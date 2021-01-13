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

#ifndef _cRIO_NiError_H_
#define _cRIO_NiError_H_

#include <stdexcept>
#include <string>

namespace LSST {
namespace cRIO {

typedef int32_t NiFpga_Status;

/**
 * Error thrown on FPGA problems. Provides what routine with explanatory
 * string.
 */
class NiError : public std::runtime_error {
public:
    /**
     * Construct the error.
     *
     * @param status Ni error status.
     */
    NiError(const char *msg, NiFpga_Status status);
};

/**
 * Throws NI exception if an error occurred.
 *
 * @param msg message associated with the error
 * @param status NI status
 *
 * @throw NiError if status != 0
 *
 * @see NiError
 */
void NiThrowError(const char *msg, int32_t status);

/**
 * Throws NI exception if an error occurred.
 *
 * @param msg message associated with the error
 * @param status NI status
 *
 * @throw NiError if status != 0
 *
 * @see NiError
 */
inline void NiThrowError(const std::string &msg, int32_t status) { NiThrowError(msg.c_str(), status); }

/**
 * Throws NI exception if an error occurred.
 *
 * @param func reporting function
 * @param ni_func Ni function name signaling the exception
 * @param status NI status
 *
 * @throw NiError if status != 0
 *
 * @see NiError
 */
void NiThrowError(const char *func, const char *ni_func, int32_t status);

}  // namespace cRIO
}  // namespace LSST

/**
 * Try to open FPGA. Throws error describing file and signature on failure.
 *
 * @param dir directory holding FPGA files
 * @param prefix prefix of _Bitfile and _Signature variables; see NI generated header file for reference
 * @param resource NI FPGA resource identification
 * @param attribute NI FPGA attribute
 * @param session NI FPGA session
 */
#define NiOpen(dir, prefix, resource, attribute, session)                                                \
    NiThrowError(std::string("NiFpga_Open: bitfile ") + dir + "/" + prefix##_Bitfile +                   \
                         " with expected signature " + prefix##_Signature + " on resource " + resource + \
                         " (check if file exists and signature matches)",                                \
                 NiFpga_Open((std::string(dir) + "/" + prefix##_Bitfile).c_str(), prefix##_Signature,    \
                             resource, attribute, session))

#endif  // ! _cRIO_NiError_H_
