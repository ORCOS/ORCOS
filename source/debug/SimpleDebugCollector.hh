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

#ifndef SIMPLEDEBUGCOLLECTOR_HH_
#define SIMPLEDEBUGCOLLECTOR_HH_

#include "hal/CharacterDevice.hh"

/*!
 *
 * \ingroup debug
 *
 * \brief	SimpleDebugCollector is a device that is mapped into the filesystems /proc directory.
 * 			By calling its read method it dynamically reads system status information
 * 			like memory usage, number of threads, etc.
 *
 * 			This is more a proof of concept device, which collects all available information in the device.
 * 			More sophisticated devices like /proc/meminfo /proc/cpuinfo /proc/schedulerstate etc. could
 * 			be implemented in this way, too.
 */

class SimpleDebugCollector: public CharacterDevice {
public:
    SimpleDebugCollector();

    ~SimpleDebugCollector() {
    }


    inline ErrorT readByte(char* p_byte) {
        return (cNotImplemented );
    }

    inline ErrorT writeByte(char c_byte) {
        return (cNotImplemented );
    }

    ErrorT readBytes(char *bytes, unint4 &length);

    inline ErrorT writeBytes(const char *bytes, unint4 length) {
        return (cNotImplemented );
    }
};

#endif /*SIMPLEDEBUGCOLLECTOR_HH_*/
