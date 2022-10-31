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

#ifndef __SAL_Event_
#define __SAL_Event_

#include <cRIO/Event.h>

#define SAL_EVENT_CLASS(prefix, sal, event)                                \
    class SAL_##event : public cRIO::Event {                               \
    public:                                                                \
        SAL_##event(prefix##_logevent_##event##C* data) : params(*data) {} \
                                                                           \
        void received() override;                                          \
                                                                           \
    private:                                                               \
        prefix##_logevent_##event##C params;                               \
    }

#endif  //! __SAL_Event_
