/*
 * This file is part of LSST M1M3 support system package.
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

#ifndef __CRIO_INTERRUPTHANDLER__
#define __CRIO_INTERRUPTHANDLER__

namespace LSST {
namespace cRIO {

/**
 * Abstract class for interrupt handlers. This design allow a Task to become an interrupt handler.
 */
class InterruptHandler {
public:
    virtual void handleInterrupt(uint8_t interrupt) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif /* !__CRIO_INTERRUPTHANDLER__ */
