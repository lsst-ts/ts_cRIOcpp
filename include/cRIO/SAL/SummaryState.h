/*
 * This file is part of the LSST-TS distribution (https://github.com/lsst-ts).
 * Copyright © 2020 Petr Kubánek, Vera C. Rubin Observatory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SAL_SummaryState_
#define __SAL_SummaryState_

#include <cRIO/Singleton.h>

namespace LSST {
namespace cRIO {
namespace SAL {

template <typename SAL>
SummaryState : SAL, public Singleton<SummaryState> {
public:
    SummaryState(token) : _updated(false) { summaryState = -1; }

    void setState(int newState) {
        if (summaryState != newState) {
            _updated = true;
            summaryState = newState;
        }
    }

private:
    bool _updated;
};
}  // namespace SAL
}  // namespace cRIO
}  // namespace LSST

#endif  //! __SAL_SummaryState_
