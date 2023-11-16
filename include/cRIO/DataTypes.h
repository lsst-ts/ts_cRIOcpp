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

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <stdint.h>


/**
 * Number of ModBus subnets. Subnets are connected and commanded through cRIO
 * DIOs.
 */
#define SUBNET_COUNT 5

/// Total number of force actuators
#define FA_COUNT 156

/// Number of lateral force actuators.
#define FA_X_COUNT 12

/// Number of vertical force actuators.
#define FA_Y_COUNT 100

/// Number of actuators in Z axis - shall equal to total number of actuators.
#define FA_Z_COUNT 156

/// Number of secondary (X or Y) actuators.
#define FA_S_COUNT (FA_X_COUNT + FA_Y_COUNT)

/// Maximal number of Force Actuator near neighbors
#define FA_MAX_NEAR_COUNT 7

/// Number of Force Actuator neighbors
#define FA_FAR_COUNT 12

/// Number of hardpoints.
#define HP_COUNT 6

/// Number of supported bending modes.
#define BENDING_MODES 22

/// Conversion - millimeters in meter.
#define MILLIMETERS_PER_METER 1000.0

/// Conversion - micrometers in millimeter.
#define MICROMETERS_PER_MILLIMETER 1000.0

#endif /* DATATYPES_H_ */
