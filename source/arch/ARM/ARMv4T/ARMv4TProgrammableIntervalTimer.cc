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

#include "ARMv4TProgrammableIntervalTimer.hh"
#include <assembler.h>
#include <kernel/Kernel.hh>

extern "C" Kernel* theOS;

ARMv4TProgrammableIntervalTimer::ARMv4TProgrammableIntervalTimer() {

    // initialize timer
    isEnabled = false;
    time = 0;
    elapsedCycles = 0;
    tickRate = 0;

}

ARMv4TProgrammableIntervalTimer::~ARMv4TProgrammableIntervalTimer() {
    disable();
}

ErrorT ARMv4TProgrammableIntervalTimer::enable() {

    this->setTimer(time);

    isEnabled = true;
    return cOk ;
}

ErrorT ARMv4TProgrammableIntervalTimer::disable() {

    isEnabled = false;

    return cOk ;
}

ErrorT ARMv4TProgrammableIntervalTimer::setTimer(unint4 t) {

    tickRate = t;  // tickRate = cycles/tick

    TimerDevice::setTimer(t);
    return cOk ;
}

