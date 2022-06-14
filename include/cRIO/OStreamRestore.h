/*
 * This file is part of the LSST-TS distribution (https://github.com/lsst-ts).
 * Copyright © 2022 Petr Kubánek, Vera C. Rubin Observatory
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

#ifndef __OStreamRestore_h
#define __OStreamRestore_h

#include <ostream>

class OStreamRestore {
public:
    OStreamRestore(std::ostream& os) : _os(os), _fillchar(_os.fill()), _flags(_os.flags()) {}

    ~OStreamRestore() {
        _os.flags(_flags);
        _os.fill(_fillchar);
    }

private:
    std::ostream& _os;
    char _fillchar;
    std::ios_base::fmtflags _flags;
};

#endif  //! __OStreamRestore_h
