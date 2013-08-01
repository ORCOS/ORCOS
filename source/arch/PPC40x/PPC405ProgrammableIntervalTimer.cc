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

#include "PPC405ProgrammableIntervalTimer.hh"

PPC405ProgrammableIntervalTimer::PPC405ProgrammableIntervalTimer() {
    isEnabled = false;
}

PPC405ProgrammableIntervalTimer::~PPC405ProgrammableIntervalTimer() {
    disable();
}

ErrorT PPC405ProgrammableIntervalTimer::enable() {
    isEnabled = true;

    this->setTimer( time );

    ppc_mtspr(TCR, ppc_mfspr(TCR) | PPC_TCR_PIE );

    return cOk;
}

ErrorT PPC405ProgrammableIntervalTimer::disable() {
    isEnabled = false;
    ppc_mtspr(TCR, ppc_mfspr(TCR) & ~(PPC_TCR_PIE ));
    ppc_mtspr(PIT, 0);
    ppc_mtspr(TSR, PPC_TSR_PIS);

    return cOk;
}

ErrorT PPC405ProgrammableIntervalTimer::setTimer( unint4 t ) {
    tickRate = t;
    // clear the pit tsr flag
    ppc_mtspr(TSR, PPC_TSR_PIS);
    ppc_mtspr(PIT,t);
    TimerDevice::setTimer( t );

    return cOk;
}


