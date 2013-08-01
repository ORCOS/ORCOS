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

#ifndef FAILMON_HH
#define FAILMON_HH


#include "hal/CallableObject.hh"
#include "process/WorkerThread.hh"

#define FAIL_ADDR          0xcb000000

int AISgetErrorCounter() { return *( (volatile unsigned int*) FAIL_ADDR );}

class FailureMonitor: public CallableObject {

private:
	unsigned int lasterror;
	unsigned int failure_threshold;

public:
    FailureMonitor(  );
    ~FailureMonitor() {};

    //! Callback method which is used for periodic lookup processes
    void callbackFunc( void* param );
};

#endif
