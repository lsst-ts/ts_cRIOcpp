/*
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

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <cRIO/DataTypes.h>
#include <sys/time.h>

namespace LSST {
namespace cRIO {

/**
 * Utility functions for time manipulations.
 */
namespace Timestamp {
const uint64_t NSINSEC = 1000000000;  //* seconds in nanosecond
/**
 * Converts raw (nanoseconds) timestamp into seconds.
 *
 * @param raw raw value (nanoseconds)
 *
 * @return seconds
 */
inline double fromRaw(uint64_t raw) { return ((double)raw) / NSINSEC; }

/**
 * Converts seconds into raw value (nanoseconds).
 *
 * @param timestamp value (seconds)
 *
 * @return raw value (nanoseconds)
 */
inline uint64_t toRaw(double timestamp) { return (uint64_t)(timestamp * (double)NSINSEC); }

/**
 * Converts FPGA (nanoseconds) timestamp into seconds.
 *
 * @param raw FPGA value (nanoseconds)
 *
 * @return seconds
 */
inline double fromFPGA(uint64_t timestamp) { return ((double)timestamp) / NSINSEC; }

/**
 * Converts seconds into FPGA value (nanoseconds).
 *
 * @param timestamp value (seconds)
 *
 * @return FPGA value (nanoseconds)
 */
inline uint64_t toFPGA(double timestamp) { return (uint64_t)(timestamp * (double)NSINSEC); }
}  // namespace Timestamp

}  // namespace cRIO
}  // namespace LSST

#endif /* TIMESTAMP_H_ */
