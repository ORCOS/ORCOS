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

#include "PPC405Watchdog.hh"
#include <error.hh>

/*
 * TCR[WP] -> 00 nach reset
 * 0,0 2^17 clocks
 * 0,1 2^21 clocks
 * 1,0 2^25 clocks
 * 1,1 2^29 clocks
 *
 * TCR[WRC] -> 00 nach reset
 * 0,0 no watchdog reset will occur
 * 0,1 core rest will be forced by the watchdog
 * 1,0 chip reset will be forced by the watchdog
 * 1,1 system reset will be forced by the watchdog
 *
 * TCR[WIE] -> 0 nach Reset
 * 0 disable watchdog interrupt
 * 1 enable watchdog interrupt
 *
 * MSR[CE] -> 0 nach reset
 * 0 critical interrupts disabled
 * 1 critical interrupts enabled
 *
 * TSR[ENW], TSR[WIS]
 * immer auf 0 setzen beim kick
 */

PPC405Watchdog::PPC405Watchdog( const char* name ) :
    Watchdog( name ) {
    enabled = false;
}

PPC405Watchdog::~PPC405Watchdog() {
    enabled = false;
}

ErrorT PPC405Watchdog::enable() {
    if ( !enabled ) {
        ppc_mtspr(TCR, ppc_mfspr(TCR) | PPC_TCR_WIE | (1<<30) | (1<<31));
    }

    enabled = true;

    // kich the watchdog once to be sure the ENW bit of TSC gets set
    this->kick();

    return cOk;
}

ErrorT PPC405Watchdog::kick() {
    if ( enabled ) {
        ppc_mtspr(TSR, PPC_TSR_WIS | PPC_TSR_ENW);
    }
    return cOk;
}

