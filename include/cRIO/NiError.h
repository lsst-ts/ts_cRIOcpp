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

#include <spdlog/fmt/fmt.h>

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
     * @param msg Message associated with the error.
     * @param status Ni error status.
     */
    NiError(const std::string &msg, NiFpga_Status status);
};

/**
 * Thrown on FPGA warnings. The code can then decide if that shall fault with
 * warning, or continue working.
 */
class NiWarning : public std::runtime_error {
public:
    /**
     * Construct NI warning.
     *
     * @param msg message associated with warning
     * @param status NI error status
     */
    NiWarning(const std::string &msg, NiFpga_Status status);
};

/**
 * Throws NI exception if an error occurred. Ignores positive status values, as
 * those signal a warning  - see NiFpga_IsError.
 *
 * @param msg message associated with the error
 * @param status NI status
 *
 * @throw NiError if status < 0
 *
 * @see NiError
 */
inline void NiThrowError(const std::string &msg, NiFpga_Status status) {
    if (status == 0) {
        return;
    }
    if (status < 0) {
        throw NiError(msg, status);
    } else {
        throw NiWarning(msg, status);
    }
}

template <typename... dt>
void NiThrowError(NiFpga_Status status, const char *msg, const dt &...params) {
    if (status == 0) {
        return;
    }
    auto f_msg = fmt::format(msg, params...);
    if (status < 0) {
        throw NiError(f_msg, status);
    } else {
        throw NiWarning(f_msg, status);
    }
}

/**
 * Throws NI exception if an error occurred. Ignores positive status values, as
 * those signal a warning  - see NiFpga_IsError.
 *
 * @param func reporting function
 * @param ni_func Ni function name signaling the exception
 * @param status NI status
 *
 * @throw NiError if status < 0
 *
 * @see NiError
 */
void NiThrowError(const char *func, const char *ni_func, NiFpga_Status status);

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
#define NiOpen(dir, prefix, resource, attribute, session)                                                 \
    LSST::cRIO::NiThrowError(std::string("NiFpga_Open: bitfile ") + dir + "/" + prefix##_Bitfile +        \
                                     " with expected signature " + prefix##_Signature + " on resource " + \
                                     resource + " (check if file exists and signature matches)",          \
                             NiFpga_Open((std::string(dir) + "/" + prefix##_Bitfile).c_str(),             \
                                         prefix##_Signature, resource, attribute, session))

#endif  // ! _cRIO_NiError_H_
