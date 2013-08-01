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

#include "ARMv4TFixedIntervalTimer.hh"
#include "assemblerFunctions.hh"

ARMv4TFixedIntervalTimer::ARMv4TFixedIntervalTimer() :
    TimerDevice() {

	// initialize timer
    isEnabled = false;
    time = 0;
    elapsedCycles = 0;
    tickRate = 0;

}

ARMv4TFixedIntervalTimer::~ARMv4TFixedIntervalTimer() {
    disable();
}

void ARMv4TFixedIntervalTimer::doHardwareStuffOnTick() {
}

ErrorT ARMv4TFixedIntervalTimer::enable() {

	this->setTimer(time);

	isEnabled = true;
    return cOk;
}

ErrorT ARMv4TFixedIntervalTimer::disable() {

	isEnabled = false;

    return cOk;
}

ErrorT ARMv4TFixedIntervalTimer::tick() {
    this->doHardwareStuffOnTick();

    return TimerDevice::tick();
}
