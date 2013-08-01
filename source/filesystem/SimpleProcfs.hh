/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIMPLEPROCFS_HH_
#define SIMPLEPROCFS_HH_

#include "filesystem/Directory.hh"
#include "debug/SimpleDebugCollector.hh"

/*!
 * \brief	SimpleProcfs - is a directory that could be configured by scl.
 *
 * \ingroup filesystem
 *
 * This is a basic debugging "proc" directory. This implementation only
 * supprts a SimpleDebugCollector which can be used as a CharacterDevice to read
 * debugging information fomr it.
 *
 */

class SimpleProcfs: public Directory {
public:
    SimpleProcfs();

    ~SimpleProcfs();

private:
    SimpleDebugCollector* simpleDebugCollector;
};

#endif /*SIMPLEPROCFS_HH_*/
